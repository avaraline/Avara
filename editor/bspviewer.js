const preview = document.getElementById("preview");
const loadbtn = document.getElementById("loadbtn");
const bspnum = document.getElementById("bspnum");

let camera, scene, renderer, mesh;

function disposeArray() {
    this.array = null;
}

function loadBSP(number) {
    fetch("../rsrc/bsps/" + number + ".json").then(response => response.json()).then((data) => {
        positions = [];
        normals = [];
        colors = [];

        for (const poly of data.polys) {
            for (const tri of poly.tris) {
                Array.prototype.push.apply(positions, data.points[tri]);
                Array.prototype.push.apply(normals, data.normals[poly.normal]);
            }
        }

        let geometry = new THREE.BufferGeometry();
        geometry.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3).onUpload(disposeArray));
        geometry.setAttribute('normal', new THREE.Float32BufferAttribute(normals, 3).onUpload(disposeArray));

        let material = new THREE.MeshLambertMaterial({
            flatShaded: true,
            color: "red",
            side: THREE.DoubleSide
        });

        if (mesh) {
            scene.remove(mesh);
        }

        mesh = new THREE.Mesh(geometry, material);
        scene.add(mesh);

        camera.position.z = Math.max(2.0, data.radius1 * 3.0);
    });
}

function init() {
    camera = new THREE.PerspectiveCamera(70, preview.offsetWidth / preview.offsetHeight, 0.01, 100);
    camera.position.z = 10;

    scene = new THREE.Scene();

    const light = new THREE.AmbientLight(0x404040);
    scene.add(light);

    const dirLight1 = new THREE.DirectionalLight(0xffffff, 0.4);
    dirLight1.position.set(0, -10, 0);
    scene.add(dirLight1);

    const dirLight2 = new THREE.DirectionalLight(0xffffff, 0.4);
    dirLight2.position.set(0, 0, -10);
    scene.add(dirLight2);

    renderer = new THREE.WebGLRenderer({antialias: true});
    renderer.setSize(preview.offsetWidth, preview.offsetHeight);
    renderer.setAnimationLoop(animation);

    preview.appendChild(renderer.domElement);
}

function animation(time) {
    if (mesh) {
        mesh.rotation.x = time / 3000;
        mesh.rotation.y = time / 4000;
    }
    renderer.render(scene, camera);
}

loadbtn.addEventListener("click", (event) => {
    loadBSP(bspnum.value);
});

bspnum.addEventListener("keyup", (event) => {
    if (event.key == "Enter") {
        loadBSP(bspnum.value);
    }
});

init();
loadBSP(bspnum.value);
