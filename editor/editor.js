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
const canvas = document.getElementById("map");
const preview = document.getElementById("preview");

function deg2rad(d) {
    return (Math.PI / 180.0) * d;
}

function equalish(a, b) {
    return Math.abs(a - b) < 10;
}

function handleColor(ctx, elem) {
    if (elem.hasAttribute("frame")) {
        ctx.strokeStyle = elem.getAttribute("frame");
    }
    if (elem.hasAttribute("fill")) {
        ctx.fillStyle = elem.getAttribute("fill");
    }
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

function redraw() {
    var doc = new DOMParser().parseFromString(source.value, "text/html");
    var map = doc.querySelector("map");
    canvas.width = map.getAttribute("width");
    canvas.height = map.getAttribute("height");
    var ctx = canvas.getContext("2d");
    var scale = Math.min(preview.offsetWidth / canvas.width, 1.0);
    ctx.scale(scale, scale);
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    [...map.children].forEach((elem) => {
        handleColor(ctx, elem);
        switch (elem.tagName.toLowerCase()) {
            case "color":
                break;
            case "rect":
                var rect = getRect(elem);
                ctx.beginPath();
                if (rect.radius > 0) {
                    ctx.roundRect(rect.left, rect.top, rect.width, rect.height, rect.radius);
                }
                else {
                    ctx.rect(rect.left, rect.top, rect.width, rect.height);
                }
                ctx.fill();
                if (rect.stroke) {
                    ctx.lineWidth = rect.stroke;
                    ctx.stroke();
                }
                break;
            case "oval":
                break;
            case "arc":
                var rect = getRect(elem);
                // QuickDraw arcs were 0deg on positive-y, canvas is positive-x.
                var start = parseInt(elem.getAttribute("start")) - 90,
                    extent = parseInt(elem.getAttribute("extent"));
                var rx = rect.width / 2,
                    ry = rect.height / 2;
                ctx.beginPath();
                if (equalish(rx, ry)) {
                    ctx.moveTo(rect.cx, rect.cy);
                    ctx.arc(rect.cx, rect.cy, rx, deg2rad(start), deg2rad(start + extent));
                    ctx.lineTo(rect.cx, rect.cy);
                }
                else {
                    ctx.ellipse(rect.cx, rect.cy, rx, ry, 0, deg2rad(start), deg2rad(start + extent));
                }
                ctx.fill();
                if (rect.stroke) {
                    ctx.lineWidth = rect.stroke;
                    ctx.stroke();
                }
                break;
            case "script":
                break;
            default:
                console.log("Unknown map element", elem);
        }
    });
}

source.addEventListener("change", (event) => {
    redraw();
});

/*
fetch("bwadi.alf").then(response => response.text()).then((data) => {
    source.value = data;
    redraw();
});
*/
