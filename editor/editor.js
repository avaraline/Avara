// Taken from https://stackoverflow.com/a/7838871
CanvasRenderingContext2D.prototype.roundRect = function (x, y, w, h, r) {
    if (w < 2 * r) r = w / 2;
    if (h < 2 * r) r = h / 2;
    this.beginPath();
    this.moveTo(x + r, y);
    this.arcTo(x + w, y    , x + w, y + h, r);
    this.arcTo(x + w, y + h, x    , y + h, r);
    this.arcTo(x    , y + h, x    , y    , r);
    this.arcTo(x    , y    , x + w, y    , r);
    this.closePath();
}

const source = document.getElementById("source");
const preview3D = document.getElementById("preview3D");
const preview2D = document.getElementById("preview2D");
const canvas2D = document.getElementById("map2D");

let camera, scene, renderer, wallMesh;
let overX = 0, overY = 100, overZ = 0;


function deg2rad(d) {
    return (Math.PI / 180.0) * d;
}

function equalish(a, b) {
    return Math.abs(a - b) < 10;
}

function handleColor(ctx, draw, elem) {
    if (elem.hasAttribute("frame")) {
        ctx.frameColor = elem.getAttribute("frame");
        draw.strokeStyle = ctx.frameColor;
    }
    if (elem.hasAttribute("fill")) {
        ctx.fillColor = elem.getAttribute("fill");
        draw.fillStyle = ctx.fillColor;
    }
}

function handleScript(ctx, elem) {
    var wa = /wa\s*=\s*([\d\.\-]+)/.exec(elem.textContent);
    var wallHeight = /wallHeight\s*=\s*([\d\.\-]+)/.exec(elem.textContent);
    if (wa) ctx.wa = parseFloat(wa[1]);
    if (wallHeight) ctx.wallHeight = parseFloat(wallHeight[1]);
}

function getRect(elem) {
    var x = parseInt(elem.getAttribute("x")),
        y = parseInt(elem.getAttribute("y")),
        w = parseInt(elem.getAttribute("w")),
        h = parseInt(elem.getAttribute("h")),
        s = parseInt(elem.getAttribute("s") ?? 0),
        r = parseInt(elem.getAttribute("r") ?? 0);
    return {
        top: y,
        left: x,
        bottom: y + h,
        right: x + w,
        width: w,
        height: h,
        cx: x + (w / 2),
        cy: y + (h / 2),
        stroke: s,
        radius: r
    };
}

function solveOrientation(x, y, h) {
    var minGuess = -(0x3C00 / 65536),
        maxGuess = (0x3C00 / 65536);
    var solved = {
        w: 0,
        angle: 0
    };
    for (var i = 0; i < 20; i++) {
        var guess = (minGuess + maxGuess) / 2;
        var sinGuess = Math.sin(guess * 2 * Math.PI),
            cosGuess = Math.cos(guess * 2 * Math.PI);
        solved.w = (x - (h * sinGuess)) / cosGuess;
        var result = (solved.w * sinGuess) + (h * cosGuess);
        if (result > y) {
            maxGuess = guess;
        }
        else {
            minGuess = guess;
        }
    }
    solved.angle = (minGuess + maxGuess) / 2;
    return solved;
}

function checkRamp(ctx, elem) {
    if (!elem.textContent.includes("Ramp")) return;

    var match = /deltaY\s*=\s*([\d\.\-]+)/m.exec(elem.textContent);
    var deltaY = match ? parseFloat(match[1]) : 1;

    match = /\by\s*=\s*([\d\.\-]+)/m.exec(elem.textContent);
    var height = parseFloat(match[1]); // + baseHeight

    var heading = ctx.lastArcAngle / 360;

    var sx = (ctx.lastRect.width * 5 / 72),
        sy = 0.1, // iThickness?
        sz = (ctx.lastRect.height * 5 / 72);
    var cx = (ctx.lastRect.right + ctx.lastRect.left) * (5 / 144),
        cy = height,
        cz = (ctx.lastRect.bottom + ctx.lastRect.top) * (5 / 144);

    var wall = wallMesh.clone();
    wall.position.set(cx, cy + (deltaY / 2), cz);

    var headingSign = heading < 0.875 && heading > 0.375 ? -1 : 1;
    if ((heading >= 0.125 && heading < 0.375) || (heading >= 0.625 && heading < 0.875)) {
        var solved = solveOrientation(sz, deltaY, sy);
        sx /= 2;
        sy /= 2;
        sz = (solved.w + (solved.w / 512)) / 2;
        wall.scale.set(sx, sy, sz);
        wall.rotateX(solved.angle * 2 * Math.PI * headingSign);
    }
    else {
        var solved = solveOrientation(sx, deltaY, sy);
        sx = (solved.w + (solved.w / 512)) / 2;
        sy /= 2;
        sz /= 2;
        wall.scale.set(sx, sy, sz);
        wall.rotateZ(solved.angle * 2 * Math.PI * headingSign);
    }

    wall.material = new THREE.MeshLambertMaterial({
        color: ctx.fillColor,
        side: THREE.DoubleSide
    });
    scene.add(wall);
}

function redraw() {
    var doc = new DOMParser().parseFromString(source.value, "text/html");
    var map = doc.querySelector("map");
    var ctx = {
        fillColor: "#ffffff",
        frameColor: "#000000",
        wa: 0,
        wallHeight: 3,
        lastRect: null,
        lastArcAngle: 0
    };
    // 2D setup
    canvas2D.width = map.getAttribute("width");
    canvas2D.height = map.getAttribute("height");
    var draw = canvas2D.getContext("2d");
    var scale = Math.min(preview2D.offsetWidth / canvas2D.width, 1.0);
    draw.scale(scale, scale);
    draw.clearRect(0, 0, canvas2D.width, canvas2D.height);
    // 3D setup
    scene.clear();
    addLights();
    var minX = 0, maxX = 0, minZ = 0, maxZ = 0, maxY = 5;
    [...map.children].forEach((elem) => {
        handleColor(ctx, draw, elem);
        switch (elem.tagName.toLowerCase()) {
            case "color":
                break;
            case "rect":
                var rect = getRect(elem);
                // 2D
                draw.beginPath();
                if (rect.radius > 0) {
                    draw.roundRect(rect.left, rect.top, rect.width, rect.height, rect.radius);
                }
                else {
                    draw.rect(rect.left, rect.top, rect.width, rect.height);
                }
                draw.fill();
                if (rect.stroke) {
                    draw.lineWidth = rect.stroke;
                    draw.stroke();
                }
                // 3D
                if (rect.stroke == 1) {
                    var wall = wallMesh.clone();
                    var sx = (rect.width / 72) * 2.5,
                        sy = (rect.radius > 0 ? ((rect.radius / 8) + 1) : ctx.wallHeight) / 2,
                        sz = (rect.height / 72) * 2.5;
                    var cx = (rect.right + rect.left) * (5 / 144),
                        cy = sy + ctx.wa,
                        cz = (rect.bottom + rect.top) * (5 / 144);
                    if (cx < minX) minX = cx;
                    if (cx > maxX) maxX = cx;
                    if (cz < minZ) minZ = cz;
                    if (cz > maxZ) maxZ = cz;
                    wall.position.set(cx, cy, cz);
                    wall.scale.set(sx, sy, sz);

                    // This is cheating...
                    var match = /midYaw\s*=\s*([\d\.\-]+)/.exec(elem.textContent);
                    var midYaw = match ? parseFloat(match[1]) : 0;
                    if (midYaw) {
                        wall.rotateY(deg2rad(midYaw));
                    }

                    wall.material = new THREE.MeshLambertMaterial({
                        color: ctx.fillColor,
                        side: THREE.DoubleSide
                    });
                    ctx.wa = 0;
                    scene.add(wall);
                }
                ctx.lastRect = rect;
                break;
            case "oval":
                break;
            case "arc":
                var rect = getRect(elem);
                // QuickDraw arcs were 0deg on positive-y, canvas is positive-x.
                var start = parseInt(elem.getAttribute("start")) - 90,
                    extent = parseInt(elem.getAttribute("extent"));
                // 2D
                var rx = rect.width / 2,
                    ry = rect.height / 2;
                draw.beginPath();
                if (equalish(rx, ry)) {
                    draw.moveTo(rect.cx, rect.cy);
                    draw.arc(rect.cx, rect.cy, rx, deg2rad(start), deg2rad(start + extent));
                    draw.lineTo(rect.cx, rect.cy);
                }
                else {
                    draw.ellipse(rect.cx, rect.cy, rx, ry, 0, deg2rad(start), deg2rad(start + extent));
                }
                draw.fill();
                if (rect.stroke) {
                    draw.lineWidth = rect.stroke;
                    draw.stroke();
                }
                ctx.lastArcAngle = (720 - (start + extent / 2)) % 360;
                break;
            case "script":
                break;
            default:
                console.log("Unknown map element", elem);
        }
        checkRamp(ctx, elem);
        handleScript(ctx, elem);
    });
    overX = (minX + maxX) / 2;
    overY = maxY;
    overZ = (minZ + maxZ) / 2;
    camera.lookAt(overX, overY, overZ);
}

function disposeArray() {
    this.array = null;
}

function loadBSP(number) {
    return fetch("bsps/" + number + ".json").then(response => response.json()).then((data) => {
        positions = [];
        normals = [];
        colors = [];

        for (const poly of data.polys) {
            for (const tri of poly.tris) {
                for (const idx of tri) {
                    Array.prototype.push.apply(positions, data.points[idx]);
                    Array.prototype.push.apply(normals, poly.normal);
                }
            }
        }

        let geometry = new THREE.BufferGeometry();
        geometry.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3).onUpload(disposeArray));
        geometry.setAttribute('normal', new THREE.Float32BufferAttribute(normals, 3).onUpload(disposeArray));

        let material = new THREE.MeshLambertMaterial({
            color: "red",
            side: THREE.DoubleSide
        });

        return new THREE.Mesh(geometry, material);
    });
}

function addLights() {
    const light = new THREE.AmbientLight(0x404040);
    scene.add(light);

    const dirLight1 = new THREE.DirectionalLight(0xffffff, 0.4);
    dirLight1.position.set(0, -10, 0);
    scene.add(dirLight1);

    const dirLight2 = new THREE.DirectionalLight(0xffffff, 0.4);
    dirLight2.position.set(0, 0, -10);
    scene.add(dirLight2);
}

function init() {
    camera = new THREE.PerspectiveCamera(70, preview3D.offsetWidth / preview3D.offsetHeight, 0.1, 2000);
    scene = new THREE.Scene();

    renderer = new THREE.WebGLRenderer({antialias: true});
    renderer.setSize(preview3D.offsetWidth, preview3D.offsetHeight);
    renderer.setAnimationLoop(animation);

    preview3D.appendChild(renderer.domElement);
}

function animation(time) {
    camera.position.x = overX + (Math.cos(time / 3000) * 100);
    camera.position.y = overY + 50;
    camera.position.z = overZ + (Math.sin(time / 3000) * 100);
    camera.lookAt(overX, overY, overZ);
    renderer.render(scene, camera);
}

source.addEventListener("change", (event) => {
    redraw();
});

loadBSP(400).then((mesh) => {
    wallMesh = mesh;
    init();
    fetch("grimoire.alf").then(response => response.text()).then((data) => {
        source.value = data;
        redraw();
    });
});
