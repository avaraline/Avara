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
        switch (elem.tagName.toLowerCase()) {
            case "color":
                if (elem.hasAttribute("frame")) {
                    ctx.strokeStyle = elem.getAttribute("frame");
                }
                else if (elem.hasAttribute("fill")) {
                    ctx.fillStyle = elem.getAttribute("fill");
                }
                break;
            case "rect":
                var t = parseInt(elem.getAttribute("t")),
                    l = parseInt(elem.getAttribute("l")),
                    b = parseInt(elem.getAttribute("b")),
                    r = parseInt(elem.getAttribute("r")),
                    w = parseInt(elem.getAttribute("w"));
                var rx = parseInt(elem.getAttribute("rx") ?? 0),
                    ry = parseInt(elem.getAttribute("ry") ?? 0);
                ctx.beginPath();
                if (ry > 0) {
                    ctx.roundRect(l, t, (r - l), (b - t), ry);
                }
                else {
                    ctx.rect(l, t, (r - l), (b - t));
                }
                ctx.fill();
                ctx.stroke();
                break;
            case "oval":
                break;
            case "arc":
                // <arc t="1448" l="437" b="1565" r="548" w="1" start="180" angle="90"></arc>
                var t = parseInt(elem.getAttribute("t")),
                    l = parseInt(elem.getAttribute("l")),
                    b = parseInt(elem.getAttribute("b")),
                    r = parseInt(elem.getAttribute("r")),
                    w = parseInt(elem.getAttribute("w"));
                var rx = (r - l) / 2,
                    ry = (b - t) / 2,
                    cx = l + rx,
                    cy = t + ry;
                var start = parseInt(elem.getAttribute("start")) - 90,
                    extent = parseInt(elem.getAttribute("angle"));
                ctx.beginPath();
                if (equalish(rx, ry)) {
                    ctx.moveTo(cx, cy);
                    ctx.arc(cx, cy, rx, deg2rad(start), deg2rad(start + extent));
                    ctx.lineTo(cx, cy);
                }
                else {
                    ctx.ellipse(l + rx, t + ry, rx, ry, 0, deg2rad(start), deg2rad(start + extent));
                }
                ctx.fill();
                ctx.stroke();
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
