#!/usr/bin/env python3

"""
Converts Avara level PICT resources to Inkscape 0.9x compatible SVG
https://developer.apple.com/library/archive/documentation/mac/QuickDraw/QuickDraw-458.html
https://developer.apple.com/library/archive/documentation/mac/QuickDraw/QuickDraw-202.html#MARKER-14-167
https://github.com/Paolo-Maffei/OpenNT/blob/5c5b979ec08c17d3ca2eb70e8aad62d26515d01c/com/ole32/olecnv32/quickdrw.c
https://github.com/ImageMagick/ImageMagick/blob/master/coders/pict.c
Technical Note QD14 - QuickDraw's Internal Picture Defnition
Inside Macintosh - More Macintosh Toolbox
Inside Macintosh - Imaging with QuickDraw
"""

import struct
import math
import json
import sys
from lxml import etree

DEBUG_PARSER = True


class Rect:

    def __init__(self, t, l, b, r):
        self.top, self.left, self.bottom, self.right = t, l, b, r
        self.width = self.right - self.left
        self.height = self.bottom - self.top

    def __str__(self):
        return '(%d, %d) -> (%d, %d)' % (self.left, self.top, self.right, self.bottom)

    def box(self):
        return '%d %d %d %d' % (self.left, self.top, self.width, self.height)


class Point:

    def __init__(self, x, y):
        self.x, self.y = x, y

    def __str__(self):
        return '(%d, %d)' % (self.x, self.y)


class Color:

    def __init__(self, r, g, b, bpp=16):
        self.r = r
        self.g = g
        self.b = b
        self.bpp = bpp

    def hex(self):
        r = self.r >> (self.bpp - 8)
        g = self.g >> (self.bpp - 8)
        b = self.b >> (self.bpp - 8)
        return '#%02x%02x%02x' % (r, g, b)

    def __str__(self):
        return self.hex()


class DataBuffer:

    def __init__(self, data):
        self.data = data
        self.size = len(self.data)
        self.pos = 0

    def __bool__(self):
        return self.pos < self.size

    def read(self, num):
        buf = self.data[self.pos:self.pos + num]
        self.pos += num
        return buf

    def unpack(self, fmt):
        unpack_fmt = '!' + fmt
        size = struct.calcsize(unpack_fmt)
        nums = struct.unpack(unpack_fmt, self.data[self.pos:self.pos + size])
        self.pos += size
        return nums

    def short(self):
        return self.unpack('h')[0]

    def ushort(self):
        return self.unpack('H')[0]

    def char(self):
        return self.unpack('b')[0]

    def uchar(self):
        return self.unpack('B')[0]

    def fixed(self):
        fixd = self.unpack('l')[0]
        return fixd / 65536.0

    def byte(self):
        return self.read(1)[0]

    def rect(self):
        return Rect(self.short(), self.short(), self.short(), self.short())

    def point(self):
        return Point(self.short(), self.short())

    def color(self):
        return Color(self.ushort(), self.ushort(), self.ushort())

    def long(self):
        return self.unpack('i')[0]

    def align(self):
        if self.pos % 2:
            self.pos += 1


# XML Namespaces for lxml

DCNS = "http://purl.org/dc/elements/1.1/"
CCNS = "http://creativecommons.org/ns#"
RDFNS = "http://www.w3.org/1999/02/22-rdf-syntax-ns#"
SVGNS = "http://www.w3.org/2000/svg"
SPNS = "http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
ISNS = "http://www.inkscape.org/namespaces/inkscape"
XMLNS = "http://www.w3.org/XML/1998/namespace"
NSMAP = {
    "dc": DCNS,
    "cc": CCNS,
    "rdf": RDFNS,
    "svg": SVGNS,
    "sodipodi": SPNS,
    "inkscape": ISNS,
    "xml": XMLNS
}
TEXTSTYLE = "font-size:9px;font-family:monospace;-inkscape-font-specification:monospace;word-spacing:0px;fill:#000000;fill-opacity:1;"


def ns(thing, ns):
    return "{%s}%s" % (ns, thing)


class SVGContext:

    def __init__(self, frame, **attrs):
        x = frame.right
        y = frame.bottom
        self.root = etree.Element("svg", nsmap = NSMAP)
        self.root.set("xmlns", "http://www.w3.org/2000/svg")

        self.root.set("width", str(x) + "px")
        self.root.set("height", str(y) + "px")
        self.root.set("viewBox", "0 0 %d %d" % (x, y))

        self.root.set("version", "1.1")
        self.root.set("id", "svg8")

        # TODO: set doc name properly
        self.root.set(ns("docname", SPNS), "test.svg")

        self.set_inkscape_options()

        self.layer = etree.SubElement(self.root, "g")
        self.layer.set(ns("label", ISNS), "Layer 1")
        self.layer.set(ns("groupmode", ISNS), "layer")
        self.layer.set("id", "layer1")

        self.current = self.layer
        self.parents = []
        self.depth = 0

        # self.xml = ''
        self.buffered = []
        self.textpos = Point(0, 0)
        # self.write('svg', xmlns='http://www.w3.org/2000/svg', close=False, **attrs)
        self.x = 0
        self.y = 0
        self.w = 1
        self.r = Point(0, 0)
        self.fg = Color(255, 255, 255)
        self.bg = Color(255, 255, 255)
        self.id = 0

        self.last_rect = None
        self.last_rrect = None
        self.last_arc = None
        self.last_oval = None

        self.save_rect = None
        self.save_rrect = None
        self.save_oval = None

        self.save_arc = None
        self.save_arc_x = None
        self.save_arc_y = None
        self.save_arc_start = None
        self.save_arc_angle = None

    def set_inkscape_options(self):
        defs = etree.SubElement(self.root, "defs")
        defs.set("id", "defs2")

        view = etree.SubElement(self.root, ns("namedview", SPNS))
        view.set("id", "base")
        view.set("pagecolor", "#ffffff")
        view.set("bordercolor", "#666666")
        view.set(ns("pageopacity", ISNS), "0.0")
        view.set(ns("zoom", ISNS), "100")
        view.set(ns("cx", ISNS), "0")
        view.set(ns("cy", ISNS), "0")
        view.set(ns("document-units", ISNS), "px")
        view.set(ns("current-layer", ISNS), "layer1")
        view.set(ns("window-x", ISNS), "-8")
        view.set(ns("window-y", ISNS), "-8")
        view.set("inkscape.window-maximized", "1")
        view.set("showguides", "true")
        view.set(ns("guide-bbox", ISNS), "true")

        grid = etree.SubElement(view, ns("grid", ISNS))
        grid.set("type", "xygrid")
        grid.set("id", "grid1")

        meta = etree.SubElement(self.root, "metadata")
        meta.set("id", "metadata1")
        rdf = etree.SubElement(meta, ns("RDF", RDFNS))
        work = etree.SubElement(rdf, ns("Work", CCNS))
        work.set(ns("about", RDFNS), "")
        fmt = etree.SubElement(work, ns("format", DCNS))
        fmt.text = "image/svg+xml"
        typ = etree.SubElement(work, ns("type", DCNS))
        typ.set(ns("resource", RDFNS), "http://purl.org/dc/dcmitype/StillImage")
        title = etree.SubElement(work, ns("title", DCNS))


    def group_start(self):
        self.depth += 1
        self.parents.append(self.current)
        self.current = self.element("g")

    def group_close(self):
        if len(self.parents) < 1:
            if DEBUG_PARSER:
                print("Group close with no group open...")
            return

        self.depth -= 1
        self.current = self.parents[-1]
        self.parents = self.parents[:-1]

    def element(self, tag):
        e = etree.SubElement(self.current, tag)
        e.set("id", self.getid(tag))
        return e

    def getid(self, thing):
        self.id += 1
        return (thing + "%s") % self.id

    def text(self, s, x=None, y=None, dh=0, dv=0):
        if x:
            self.textpos.x = x
        if y:
            self.textpos.y = y
        if dh:
            self.textpos.x += dh
        if dv:
            self.textpos.y += dv

        self.buffered.extend([{
            "string": str(x),
            "x": self.textpos.x,
            "y": self.textpos.y
        } for x in s.split('\r')])

    def flush_text(self):
        if len(self.buffered) < 1:
            return
        x = self.buffered[0]["x"]
        y = self.buffered[0]["y"]

        text = etree.SubElement(self.current, "text")
        text.set(ns("space", XMLNS), "preserve")
        text.set("style", TEXTSTYLE)
        text.set("x", str(x))
        text.set("y", str(y))
        text.set("id", self.getid("text"))

        last_v = self.buffered[0]["y"]
        last_dv = 0
        for idx, line in enumerate(self.buffered):
            # handle line breaks -- if the y difference 
            # between these two lines is 2x the last
            # difference, add a line break
            dv = line["y"] - last_v
            if dv == last_dv * 2 and dv != 0:
                # empty tspan makes a linebreak
                self.tspan(text, "", line["x"], dv // 2)
            last_v = line["y"]
            last_dv = dv

            self.tspan(text, line["string"], line["x"], line["y"])

        self.buffered = []

    def tspan(self, text, string, x, y):
        if len(string) == 0:
            return
        tspan = etree.SubElement(text, "tspan")
        try:
            tspan.text = string
        except ValueError:
            tspan.getparent().remove(tspan)
            return
        tspan.set(ns("role", SPNS), "line")
        tspan.set("id", self.getid("tspan"))
        tspan.set("x", str(x))
        tspan.set("y", str(y))
        tspan.set("style", "stroke-width:0.25;")
        # print(string.encode('ascii', 'ignore'))

    def fill(self, el):
        oldstyle = el.get("style")
        fill = "fill: %s; " % self.fg
        if oldstyle:
            el.set("style", oldstyle + fill)
        else:
            el.set("style", fill)

    def stroke(self, el):
        oldstyle = el.get("style")
        stroke = "stroke: %s; stroke-width: %s; " % (self.fg, str(self.w))
        if oldstyle:
            el.set("style", oldstyle + stroke)
        else:
            el.set("style", stroke)

    def rect_data(self, rect, el):
        el.set("x", str(rect.left + self.x))
        el.set("y", str(rect.top + self.y))
        el.set("width", str(rect.width))
        el.set("height", str(rect.height))

    def null_fill(self, el):
        if el == None:
            return
        # use this to ensure that no element goes without a 
        # fill instruction, otherwise stuff looks weird
        style = el.get("style")
        #if not style:
        #    el.set("style", "fill: none;")
        #    return
        #if "fill" not in style:
        #    el.set("style", style + "fill: none;")

    def close(self):
        # tree.write(filename, encoding="UTF-8")
        svg = etree.ElementTree(self.root)
        xml_str = etree.tostring(svg, pretty_print=True, encoding="UTF-8")
        return xml_str

class Operation:
    length = 0

    def parse(self, data, context):
        data.read(self.length)


class VariableReserved (Operation):

    def parse(self, data, context):
        size = data.short()
        data.read(size)


class NOOP (Operation):
    pass


class SkipRegion (Operation):

    def parse(self, data, context):
        total = data.short()
        region = data.rect()
        data.read(total - 10)


# This function reads embedded image pixmaps
PIXMAP_BIT = 0x8000
def pixmap(data, clipped=False):
    if DEBUG_PARSER:
        print("pixmap")
    if clipped:
        if DEBUG_PARSER:
            print("clipped, skipping 4 bytes")
        skip = data.read(4)
    row_bytes = data.short()

    is_pixmap = (row_bytes & PIXMAP_BIT != 0)
    row_bytes = row_bytes & 0x7FFF

    if DEBUG_PARSER:
        print(f"is_pixmap: {is_pixmap}")
        print(f"row bytes: {row_bytes}")

    bounds = data.rect()

    if DEBUG_PARSER:
        print(f"bounds: {bounds}")
    
    if is_pixmap:
        version = data.short()
        pack_type = data.short()
        pack_size = data.long()
        hres = data.fixed()
        vres = data.fixed()
        pixel_type = data.short()
        pixel_size = data.short()
        cmp_count = data.short()
        cmp_size = data.short()
        plane_bytes = data.long()
        pmtable = data.long()
        reserved = data.long()
        if DEBUG_PARSER:
            print("version: %s" % version)
            print("pack_type: %s" % pack_type)
            print("pack_size: %s" % pack_size)
            print("hres: %s" % hres)
            print("vres: %s" % vres)
            print("pixel_type: %s" % pixel_type)
            print("pixel_size: %s" % pixel_size)
            print("cmp_count: %s" % cmp_count)
            print("cmp_size: %s" % cmp_size)
            print("plane_bytes: %s" % plane_bytes)
            print("pmtable: %s" % pmtable)
            print("reserved: %s" % reserved)
        
    return (is_pixmap, bounds, row_bytes)


# Color table for embedded 32 bit color images
def color_table(data):
    ct_seed = data.long()
    trans_index = data.short()
    ct_size = data.short()
    if DEBUG_PARSER:
        print("ct_seed: %s" % ct_seed)
        print("trans_index: %s" % trans_index)
        print("ct_size: %s" % ct_size)
    ct = []
    while ct_size >= 0:
        ct.append(data.short())
        ct_size -= 1
    
    if DEBUG_PARSER:
        print("ct: %s" % ct)


def pixdata(data, pmap):
    bounds = pmap[1]
    row_bytes = pmap[2]

    lines_to_read = bounds.height

    if DEBUG_PARSER:
        print("row_bytes: %s" % row_bytes)
        print("lines_to_read: %d" % lines_to_read)

    if row_bytes < 8:
        if DEBUG_PARSER:
            print("unpacked")
        data_size = row_bytes * lines_to_read
        pixdata = data.read(data_size)
    else:
        if DEBUG_PARSER:
            print("packed")
        for i in range(lines_to_read):
            sl_size = data.ushort() if row_bytes > 250 else data.uchar()
            if DEBUG_PARSER:
                print(f"sl_size: {sl_size}")
            scanline = data.read(sl_size)
            if DEBUG_PARSER:
                print(f"{i+1}, {sl_size}, {scanline}")


# Didn't end up using this, leaving it just in case
class SkipReserved_x97 (Operation):

    def parse(self, data, context):
        length = data.short()
        data.read(length)


class BitsRect (Operation):

    def parse(self, data, context):
        if DEBUG_PARSER:
            print("BitsRect")
            print(" ".join(format(x, '02x') for x in data.data[data.pos:data.pos+300]))
        pmap = pixmap(data)
        if pmap[0]:
            color_table(data)
        src_rect = data.rect()
        dst_rect = data.rect()
        mode = data.short()

        if DEBUG_PARSER:
            print(f"pmap: {pmap}")
            print(f"src_rect: {src_rect}")
            print(f"dst_rect: {dst_rect}")
            print(f"mode: {mode}")

        pixdata(data, pmap)


class BitsRgn (Operation):

    def parse(self, data, context):
        if DEBUG_PARSER:
            print("BitsRgn")
        pmap = pixmap(data)
        if pmap[0]:
            color_table(data)
        src_rect = data.rect()
        dst_rect = data.rect()
        mode = data.short()

        mask_rgn_size = data.short()
        if DEBUG_PARSER:
            print(f"mask_rgn_size: {mask_rgn_size}")
        mask_rgn = data.read(mask_rgn_size - 2)
        pixdata(data, pmap)


class PackBitsRect (Operation):

    def parse(self, data, context):
        if DEBUG_PARSER:
            print("PackBitsRect")
        pmap = pixmap(data)
        if pmap[0]:
            color_table(data)
        src_rect = data.rect()
        dst_rect = data.rect()
        mode = data.short()

        if DEBUG_PARSER:
            print(f"pmap: {pmap}")
            print(f"src_rect: {src_rect}")
            print(f"dst_rect: {dst_rect}")
            print(f"mode: {mode}")

        pixdata(data, pmap)


class PackBitsRgn (Operation):

    def parse(self, data, context):
        if DEBUG_PARSER:
            print("PackBitsRgn")
        pmap = pixmap(data)
        if pmap[0]:
            color_table(data)
        src_rect = data.rect()
        dst_rect = data.rect()
        mode = data.short()
        mask_rgn_size = data.short()
        if DEBUG_PARSER:
            print(f"mask_rgn_size: {mask_rgn_size}")
        mask_rgn = data.read(mask_rgn_size - 2)

        pixdata(data, pmap)


class DirectBitsRect (Operation):

    def parse(self, data, context):
        if DEBUG_PARSER:
            print("DirectBitsRect")
        pmap = pixmap(data, clipped=True)
        src_rect = data.rect()
        dest_rect = data.rect()
        mode = data.short()

        pixdata(data, pmap)


class DirectBitsRgn (Operation):

    def parse(self, data, context):
        if DEBUG_PARSER:
            print("DirectBitsRgn")
        pmap = pixmap(data, clipped=True)
        src_rect = data.rect()
        dst_rect = data.rect()
        mode = data.short()
        
        mask_rgn_size = data.short()
        if DEBUG_PARSER:
            print(f"mask_rgn_size: {mask_rgn_size}")
        mask_rgn = data.read(mask_rgn_size - 2)
        
        pixdata(data, pmap)


class ClipRegion (Operation):

    def parse(self, data, context):
        total = data.short()
        region = data.rect()
        data.read(total - 10)


class FillRegion (Operation):

    def parse(self, data, context):
        total = data.short()
        region = data.rect()
        data.read(total - 10)


class TextFont (Operation):

    def parse(self, data, context):
        font = data.short()


class TextFace (Operation):

    def parse(self, data, context):
        face = data.byte()


class TextMode (Operation):

    def parse (self, data, context):
        mode = data.short()


class PenSize (Operation):

    def parse(self, data, context):
        size = data.point()
        context.w = size.x

class PenMode (Operation):

    def parse(self, data, context):
        mode = data.short()


class PenPattern (Operation):

    def parse(self, data, context):
        pattern = data.read(8)


class FillPattern (Operation):

    def parse(self, data, context):
        pattern = data.read(8)


class OvalSize (Operation):

    def parse(self, data, context):
        context.r = data.point()


class Origin (Operation):

    def parse(self, data, context):
        dh, dv = data.unpack('hh')
        context.x -= dh
        context.y -= dv
        context.textpos.x -= dh
        context.textpos.y -= dv


class TextSize (Operation):

    def parse(self, data, context):
        size = data.short()


class TextRatio (Operation):

    def parse(self, data, context):
        numerator = data.point()
        denominator = data.point()


class PenLocationHFractional (Operation):

    def parse(self, data, context):
        pos = data.short()


class RGBForegroundColor (Operation):

    def parse(self, data, context):
        context.fg = data.color()
        # print(context.fg)


class RGBBackgroundColor (Operation):

    def parse(self, data, context):
        context.bg = data.color()
        # print(context.bg)


class DefaultHighlight (Operation):
    pass


class OpColor (Operation):

    def parse(self, data, context):
        color = data.color()


class Line (Operation):
    def parse(self, data, context):
        start = data.point()
        end = data.point()
        context.x += end.x
        context.y += end.y


class LineFrom (Operation):
    def parse(self, data, context):
        p = data.point()
        context.x += p.x
        context.y += p.y


class ShortLine (Operation):

    def parse(self, data, context):
        start = data.point()
        dh, dv = data.read(2)
        line = context.element("line")
        line.set("x1", str(start.x))
        line.set("y1", str(start.y))
        context.x += dh
        context.y += dv
        line.set("x2", str(context.x))
        line.set("y2", str(context.y))
        line.set("stroke", "black")
        if DEBUG_PARSER:
            print(f"ShortLine {str(start)} -> {dh} {dv}")


class ShortLineFrom (Operation):

    def parse(self, data, context):
        dh, dv = data.read(2)
        line = context.element("line")
        line.set("x1", str(context.x))
        line.set("y1", str(context.y))
        context.x += dh
        context.y += dv
        line.set("x2", str(context.x))
        line.set("y2", str(context.y))
        line.set("stroke", "black")
        if DEBUG_PARSER:
            print(f"ShortLineFrom {dh} {dv}")


class LongText (Operation):

    def parse(self, data, context):
        loc = data.point()
        size = data.byte()
        text = data.read(size).decode('macintosh')
        # LongText is (top, left) so we flip x/y
        context.text(text, x=loc.y, y=loc.x)


class DHText (Operation):
    def parse(self, data, context):
        dh, size = data.read(2)
        text = data.read(size).decode('macintosh')
        if DEBUG_PARSER:
            print(f"DHText: dh {dh} size {size}")
            print(text.encode('utf-8'))
        context.text(text, dh = dh)


class DVText (Operation):

    def parse(self, data, context):
        dv, size = data.read(2)
        text = data.read(size).decode('macintosh')
        if DEBUG_PARSER:
            print(f"DVText: dv {dv} size {size}")
            print(text.encode('utf-8'))
        context.text(text, dv = dv)


class DHDVText (Operation):

    def parse(self, data, context):
        dh, dv, size = data.read(3)
        text = data.read(size).decode('macintosh')
        if DEBUG_PARSER:
            print(f"DHDVText: dh {dh} dv {dv} size {size}")
            print(text.encode('utf-8'))
        context.text(text, dh = dh, dv = dv)


class FrameRectangle (Operation):

    def parse(self, data, context):
        rect = data.rect()
        context.save_rect = rect
        svgrect = context.element("rect")
        context.rect_data(rect, svgrect)
        context.stroke(svgrect)
        context.null_fill(context.last_rect)
        context.last_rect = svgrect


class PaintRectangle (Operation):

    def parse(self, data, context):
        rect = data.rect()
        context.save_rect = rect
        svgrect = context.element("rect")
        context.rect_data(rect, svgrect)
        context.fill(svgrect)
        context.null_fill(context.last_rect)
        context.last_rect = svgrect

class FrameSameRectangle (Operation):

    def parse(self, data, context):
        if context.last_rect is None:
            if DEBUG_PARSER:
                print("FrameSameRectangle with no last rect")
            return
        context.stroke(context.last_rect)


class PaintSameRectangle (Operation):

    def parse(self, data, context):
        if context.last_rect is None:
            if DEBUG_PARSER:
                print("PaintSameRectangle with no last rect")
            return
        context.fill(context.last_rect)


class FrameRoundedRectangle (Operation):

    def parse(self, data, context):
        rect = data.rect()
        svgrrect = context.element("rect")
        context.rect_data(rect, svgrrect)
        svgrrect.set("ry", str(context.r.y))
        context.stroke(svgrrect)
        context.null_fill(context.last_rrect)
        context.last_rrect = svgrrect

class PaintRoundedRectangle (Operation):

    def parse(self, data, context):
        rect = data.rect()
        svgrrect = context.element("rect")
        context.rect_data(rect, svgrrect)
        svgrrect.set("ry", str(context.r.y))
        context.fill(svgrrect)
        context.null_fill(context.last_rrect)
        context.last_rrect = svgrrect


class FrameSameRoundedRectangle (Operation):

    def parse(self, data, context):
        if context.last_rrect is None:
            if DEBUG_PARSER:
                print("FrameSameRoundedRectangle with no last rrect")
            return
        context.stroke(context.last_rrect)


class PaintSameRoundedRectangle (Operation):
    
    def parse(self, data, context):
        if context.last_rrect is None:
            if DEBUG_PARSER:
                print("PaintSameRoundedRectangle with no last rrect")
            return
        context.fill(context.last_rrect)


class FrameOval (Operation):

    def parse(self, data, context):
        rect = data.rect()
        ellipse = context.element("ellipse")

        rx = rect.width / 2.0
        ry = rect.height / 2.0
        cx = rect.left + rx
        cy = rect.top + ry

        ellipse.set("cx", str(cx))
        ellipse.set("cy", str(cy))
        ellipse.set("rx", str(rx))
        ellipse.set("ry", str(ry))

        context.stroke(ellipse)
        context.null_fill(context.last_oval)
        context.last_oval = ellipse


class PaintOval (Operation):

    def parse(self, data, context):
        rect = data.rect()
        ellipse = context.element("ellipse")

        rx = rect.width / 2.0
        ry = rect.height / 2.0
        cx = rect.left + rx
        cy = rect.top + ry

        ellipse.set("cx", str(cx))
        ellipse.set("cy", str(cy))
        ellipse.set("rx", str(rx))
        ellipse.set("ry", str(ry))

        context.fill(ellipse)
        context.null_fill(context.last_oval)
        context.last_oval = ellipse



class FrameSameOval (Operation):

    def parse(self, data, context):
        if context.last_oval is None:
            if DEBUG_PARSER:
                print("FrameSameOval with no last oval")
            return
        context.stroke(context.last_oval)


class PaintSameOval (Operation):

    def parse(self, data, context):
        if context.last_oval is None:
            if DEBUG_PARSER:
                print("PaintSameOval with no last oval")
            return
        context.fill(context.last_oval)


def arc_path(context, rect, start, angle, arc):
    if rect is None:
        return
    
    start -= 90

    rx = rect.width // 2
    ry = rect.height // 2

    cx = context.x + rect.left + rx
    cy = context.y + rect.top + ry


    p1 = polar_to_cartesian(cx, cy, rx, ry, start)
    p2 = polar_to_cartesian(cx, cy, rx, ry, start + angle)

    dfmt = "M %d %d L %d %d A %d %d %d %d %d %d %d L %d %d"

    d = dfmt % (cx, cy, p1[0], p1[1], rx, ry, 0, 0, 1, p2[0], p2[1], cx, cy)

    
    arc.set(ns("type", SPNS), "arc")
    arc.set(ns("cx", SPNS), str(cx))
    arc.set(ns("cy", SPNS), str(cy))
    arc.set(ns("rx", SPNS), str(rx))
    arc.set(ns("ry", SPNS), str(ry))
    arc.set(ns("start", SPNS), str(math.radians(start)))
    arc.set(ns("end", SPNS), str(math.radians(start + angle)))
    
    arc.set("d", d)

def polar_to_cartesian(cx, cy, rx, ry, angle):
    rads = math.radians(angle)
    x = cx + rx * math.cos(rads)
    y = cy + ry * math.sin(rads)
    return (x, y)


class FrameArc (Operation):

    def parse(self, data, context):
        rect = data.rect()
        context.save_arc = rect
        start, angle = data.unpack('hh')
        context.save_arc_start = start
        context.save_arc_angle = angle
        context.save_arc_x = context.x
        context.save_arc_y = context.y

        arc = context.element("path")
        arc_path(context, rect, start, angle, arc)
        context.stroke(arc)
        context.last_arc = arc


class PaintArc (Operation):

    def parse(self, data, context):
        rect = data.rect()
        context.save_arc = rect
        start, angle = data.unpack('hh')
        context.save_arc_start = start
        context.save_arc_angle = angle

        arc = context.element("path")
        arc_path(context, rect, start, angle, arc)
        context.fill(arc)
        context.last_arc = arc


class FrameSameArc (Operation):

    def parse(self, data, context):
        start, angle = data.unpack('hh')
        if (start == context.save_arc_start and
            angle == context.save_arc_angle):
            context.stroke(context.last_arc)
        else:
            arc = context.element("path")
            context.save_arc_start = start
            context.save_arc_angle = angle
            context.save_arc_x = context.x
            context.save_arc_y = context.y
            arc_path(context, context.save_arc, start, angle, arc)
            context.stroke(arc)
            context.last_arc = arc


class PaintSameArc (Operation):

    def parse(self, data, context):
        start, angle = data.unpack('hh')
        if (start == context.save_arc_start and
            angle == context.save_arc_angle and
            context.x == context.save_arc_x and
            context.y == context.save_arc_y):
            context.fill(context.last_arc)
        else:
            arc = context.element("path")
            context.save_arc_start = start
            context.save_arc_angle = angle
            context.save_arc_x = context.x
            context.save_arc_y = context.y
            arc_path(context, context.save_arc, start, angle, arc)
            context.fill(arc)
            context.last_arc = arc


class ShortComment (Operation):

    def parse(self, data, context):
        kind = data.short()
        if DEBUG_PARSER:
            try:
                print("ShortComment: %s - %s" % (kind, PICT_COMMENTS[kind]))
            except KeyError:
                print("ShortComment: %s - unknown" % kind)
        if kind == 151:
            # picTextEnd
            context.flush_text()
        if kind == 140:
            # open group
            context.group_start()
        if kind == 141:
            # close group
            context.group_close()

class LongComment (Operation):

    def parse(self, data, context):
        kind, size = data.unpack('hh')
        value = data.read(size)
        if DEBUG_PARSER:
            if kind in PICT_COMMENTS:
                print("LongComment: %s - %s" % (kind, PICT_COMMENTS[kind]))
            else:
                print(f"LongComment: {kind} - unknown")
            print("size: %s" % size)
            print("value: %s" % " ".join(format(x, '02x') for x in value))


class EndPict (Operation):
    pass


class Version (Operation):
    def parse(self, data, context):
        version = data.read(1)
        if DEBUG_PARSER:
            print("Version: %s" % version)


class Header (Operation):
    length = 24

PICT_OPCODES = {
    0x0: NOOP,
    0x1: ClipRegion,
    0x3: TextFont,
    0x4: TextFace,
    0x5: TextMode,
    0x7: PenSize,
    0x8: PenMode,
    0x9: PenPattern,
    0xa: FillPattern,
    0xb: OvalSize,
    0xc: Origin,
    0xd: TextSize,
    0x10: TextRatio,
    0x11: Version,
    0x15: PenLocationHFractional,
    0x1a: RGBForegroundColor,
    0x1b: RGBBackgroundColor,
    0x1e: DefaultHighlight,
    0x1f: OpColor,
    0x20: Line,
    0x21: LineFrom,
    0x22: ShortLine,
    0x23: ShortLineFrom,
    0x28: LongText,
    0x29: DHText,
    0x2a: DVText,
    0x2b: DHDVText,
    0x2c: VariableReserved,
    0x2e: VariableReserved,
    0x30: FrameRectangle,
    0x31: PaintRectangle,
    0x38: FrameSameRectangle,
    0x39: PaintSameRectangle,
    0x40: FrameRoundedRectangle,
    0x41: PaintRoundedRectangle,
    0x48: FrameSameRoundedRectangle,
    0x49: PaintSameRoundedRectangle,
    0x50: FrameOval,
    0x51: PaintOval,
    0x58: FrameSameOval,
    0x59: PaintSameOval,
    0x60: FrameArc,
    0x61: PaintArc,
    0x68: FrameSameArc,
    0x69: PaintSameArc,
    # these are used for region/polygon data
    0x70: SkipRegion,
    0x71: SkipRegion,
    0x72: SkipRegion,
    0x73: SkipRegion,
    0x74: SkipRegion,
    0x75: SkipRegion,
    0x76: SkipRegion,
    0x77: SkipRegion,
    0x80: SkipRegion,
    0x81: SkipRegion,
    0x8c: NOOP,
    # picture data
    0x90: BitsRect,
    0x91: BitsRgn,
    0x9a: DirectBitsRect,
    0x9b: DirectBitsRgn,
    0x98: PackBitsRect,
    0x99: PackBitsRgn,
    # comment fields
    0xa0: ShortComment,
    0xa1: LongComment,
    0x84: FillRegion,
    0xff: EndPict,
    0x02ff: Version,
    0x0c00: Header,
}

PICT_COMMENTS = {
    100: "ApplicationComment",
    130: "DrawingBegin",
    131: "DrawingEnd",
    140: "GroupBegin",
    141: "GroupEnd",
    # text related
    150: "TextBegin",
    151: "TextEnd",
    152: "StringBegin",
    153: "StringEnd",
    154: "TextCenter",
    155: "LineLayoutOff",
    156: "LineLayoutOn",
    157: "ClientLineLayout",
    # graphics related
    142: "BitmapBegin",
    143: "BitmapEnd",
    160: "PolyBegin",
    161: "PolyEnd",
    163: "PolyIgnore",
    164: "PolySmooth",
    165: "PolyClose",
    170: "ArrowBeginStart",
    171: "ArrowBeginEnd",
    172: "ArrowBeginBoth",
    173: "ArrowEnd",
    197: "SetGrayLevel",
    200: "RotateBegin",
    201: "RotateEnd",
    202: "RotateCenter",
    # lines
    180: "DashedLine",
    181: "DashedStop",
    182: "SetLineWidth",
    # postscript
    190: "PostScriptBegin",
    191: "PostScriptEnd",
    192: "PostScriptHandle",
    193: "PostScriptFile",
    194: "TextIsPostScript",
    195: "ResourcePS",
    196: "PSBeginNoSave"
}


class PictParseError(Exception):
    pass


def parse_pict(fn, data):
    #print(data[:150])
    buf = DataBuffer(data)
    size = buf.short()
    frame = buf.rect()
    #print(size, frame)
    context = SVGContext(frame)
    while buf:
        opcode = buf.short()
        if DEBUG_PARSER:
            print("0x%04x" % opcode)
        try:
            op = PICT_OPCODES[opcode]()
        except KeyError:
            print("No support for opcode 0x%s" % format(opcode, '02x'))
            print(" ".join(format(x, '02x') for x in buf.data[buf.pos:buf.pos+300]))
            raise PictParseError
        if isinstance(op, EndPict):
            break
        if DEBUG_PARSER:
            print("%s - 0x%s - %s" % (fn, format(opcode, '02x'), op.__class__.__name__))
        try:
            op.parse(buf, context)
        except ValueError:
            print("UHOH (ValueError on parse)")
            # context.close(fn)
            raise
        buf.align()
    return context.close()


if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("usage: pict2svg.py <pict raw data file> <output file>.svg")
        print("see rsrc2files to export all levelset data")
        exit(1)
    else:
        try:
            with open(sys.argv[1], "rb") as input_file:
                with open(sys.argv[2], "w", encoding="utf-8") as xml_file:
                    xml_file.write(parse_pict("", input_file))
        except:
            raise
