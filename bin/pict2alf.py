#!/usr/bin/env python3

"""
Converts Avara level PICT resources to ALF (Avara Level Format)
"""

import enum
from math import sqrt
import os
import re
import struct
import sys
import numpy as np

from alf_condense import VESTIGIAL_ATTRS
from avarascript import Element, ScriptParseError, object_context, parse_script
from dumb_round import dumb_round
from applergb import applergb_to_srgb

DEBUG_PARSER = False
METERS_PER_POINT = 5 / 72


def debug(fmt, *args, **kwargs):
    if DEBUG_PARSER:
        print(fmt % args, file=sys.stderr, flush=True)


def postprocess(element: Element) -> Element:
    # Calculate the true `angle` and remove the `extent` attribute for all
    # "arc-based" objects.
    if "angle" in element.attrs and "extent" in element.attrs:
        if element.tag == "Dome":
            element.attrs["quarters"] = str(round(element.attrs["extent"] / 90))
            del element.attrs["extent"]
        else:
            angle = element.attrs["angle"] + (element.attrs["extent"] / 2) + 180
            angle = angle if angle < 360 else angle - 360
            element.attrs["angle"] = dumb_round(angle)
            del element.attrs["extent"]
    # Convert line breaks to XML entities in the `information` attribute.
    if "information" in element.attrs:
        element.attrs["information"] = element.attrs["information"] \
            .replace("\n", "&#10;") \
            .replace("\r", "&#10;")
    # Strip out any irrelevant attributes that may be present.
    if element.tag in VESTIGIAL_ATTRS.keys():
        element.attrs = {k: v for k, v in element.attrs.items()
                         if k not in VESTIGIAL_ATTRS[element.tag]}
    return element


class Rect:
    def __init__(self, t, l, b, r):
        self.top, self.left, self.bottom, self.right = t, l, b, r

    @property
    def width(self):
        return self.right - self.left

    @property
    def height(self):
        return self.bottom - self.top

    @property
    def center(self):
        return Point((self.left + self.right) / 2, (self.top + self.bottom) / 2)

    def __str__(self):
        return "(%d, %d) -> (%d, %d)" % (self.left, self.top, self.right, self.bottom)

    def __eq__(self, other):
        return (
            self.top == other.top
            and self.left == other.left
            and self.bottom == other.bottom
            and self.right == other.right
        )

    def box(self):
        return "%d %d %d %d" % (self.left, self.top, self.width, self.height)

    def offset(self, origin):
        return self.__class__(
            self.top - origin.y,
            self.left - origin.x,
            self.bottom - origin.y,
            self.right - origin.x,
        )

    def scale(self, scale=METERS_PER_POINT, stroke=0):
        return self.__class__(
            (self.top + (stroke >> 1)) * scale,
            (self.left + (stroke >> 1)) * scale,
            (self.bottom - ((stroke + 1) >> 1)) * scale,
            (self.right - ((stroke + 1) >> 1)) * scale,
        )


class Point:
    def __init__(self, x, y):
        self.x, self.y = x, y

    def __str__(self):
        return "(%d, %d)" % (self.x, self.y)


class Color:
    def __init__(self, r, g, b, bpp=16):
        self.r = r
        self.g = g
        self.b = b
        self.bpp = bpp

    def hex(self):
        assert(self.bpp == 16)
        c = [self.r, self.g, self.b]
        colorvec = [x / 65535.0 for x in c]
        srgb_8 = [int(round(255.0 * x)) for x in applergb_to_srgb(colorvec)]
        thehex = "#%02x%02x%02x" % tuple(srgb_8)
        assert(len(thehex) == 7)
        return thehex

    def __str__(self):
        return self.hex()


class Verb(enum.Enum):
    FRAME = 0
    PAINT = 1


class DataBuffer:
    def __init__(self, data):
        self.data = data
        self.size = len(self.data)
        self.pos = 0

    def __bool__(self):
        return self.pos < self.size

    def read(self, num, advance=True):
        buf = self.data[self.pos : self.pos + num]
        if advance:
            self.pos += num
        return buf

    def skip(self, num):
        self.pos += num

    def unpack(self, fmt):
        unpack_fmt = "!" + fmt
        size = struct.calcsize(unpack_fmt)
        nums = struct.unpack(unpack_fmt, self.data[self.pos : self.pos + size])
        self.pos += size
        return nums

    def short(self):
        return self.unpack("h")[0]

    def ushort(self):
        return self.unpack("H")[0]

    def char(self):
        return self.unpack("b")[0]

    def uchar(self):
        return self.unpack("B")[0]

    def fixed(self):
        return self.unpack("l")[0] / 65536.0

    def byte(self):
        return self.read(1)[0]

    def rect(self):
        return Rect(self.short(), self.short(), self.short(), self.short())

    def point(self):
        return Point(self.short(), self.short())

    def color(self):
        return Color(self.ushort(), self.ushort(), self.ushort())

    def long(self):
        return self.unpack("i")[0]

    def align(self):
        if self.pos % 2:
            self.pos += 1


class AvaraOperation(object):
    def __str__(self):
        return ""

    def merge(self, other):
        return False

    def process(self, context):
        return []


def fix_quirks(text):
    # Fix mismatched quotes - bunch of these everywhere.
    if text.count('"') % 2 > 0:
        text += '"'
    # Curly quotes.
    text = text.replace("“", '"').replace("”", '"')
    # Bunch of these in The Lexicon set.
    # text = text.replace("group = @centery =", "group = @center y =")
    # Unescaped quotes inside quotes.
    text = text.replace(' "Trotsky" ', ' ""Trotsky"" ')
    text = text.replace(' "DuKK" ', ' ""DuKK"" ')
    text = text.replace(' "A"s ', ' ""A""s ')
    text = text.replace(' "Forseti" ', ' ""Forseti"" ')
    text = text.replace(' "Seven" ', ' ""Seven"" ')
    # Stranded
    text = text.replace('de;taY', 'deltaY')
    # Fortifications
    text = text.replace('wallHwe', '')
    # Disk-o-tech -- not sure how this worked before?
    text = text.replace("snDoory =", "snDoor y =")
    text = text.replace("= 0speed =", "= 0 speed =")
    # Some DC3 control characters in geezer sets - senile old bags.
    text = text.replace(chr(19), "")
    # Fix comments.
    text = text.replace("/ *", "/*").replace("* /", "*/")
    # Some comments missing the trailing slash.
    if text.endswith("*"):
        text += "/"
    # The parser will ignore extraneous "end"s, but some people forgot to close objects.
    return text + "\nend"


class TextOp(AvaraOperation):
    def __init__(self, text):
        self.text = text

    def process(self, context):
        fixed_script = fix_quirks(self.text)
        try:
            for t in parse_script(fixed_script):
                if t.process(context):
                    yield postprocess(t.element(context))
        except ScriptParseError:
            debug("Script error in:\n%s", fixed_script)
            # Strip the "end" we added if there's still an error.
            text = fixed_script[:-3].strip()
            text = re.sub(r"ambient(\s*)=", "ambient.i\1=", text)
            yield postprocess(Element("script", text, **context))


class GroupStart(AvaraOperation):
    pass


class GroupEnd(AvaraOperation):
    pass


class RectOp(AvaraOperation):
    tag = "rect"

    def __init__(self, context, verb, rect):
        self.verb = verb
        self.color = str(context.fg)
        self.stroke = context.stroke
        self.radius = context.radius
        self.rect = rect.offset(context.origin).scale(stroke=self.stroke)

    def process(self, context):
        if self.verb == Verb.PAINT:
            context["color"] = self.color
        elif self.verb == Verb.FRAME:
            context["color.1"] = self.color
            h = (
                0
                if self.tag == "rect"
                else dumb_round(self.radius * context["pixelToThickness"])
            )
            if self.stroke == 1:
                attrs = {
                    "color": context.get("color", self.color),
                    "color.1": context.get("color.1", self.color),
                    "x": dumb_round(self.rect.center.x),
                    "z": dumb_round(self.rect.center.y),
                    "w": dumb_round(self.rect.width),
                    "d": dumb_round(self.rect.height),
                    "h": h,
                }
                if context.get("wa"):
                    try:
                        # If wa is numeric, promote it to "y"
                        float(context["wa"])
                        attrs["y"] = context["wa"]
                    except ValueError:
                        pass
                    # wa is reset after every Wall
                    del context["wa"]
                yield postprocess(Element("Wall", **attrs))
            else:
                context.update(
                    {
                        "x": dumb_round(self.rect.center.x),
                        "z": dumb_round(self.rect.center.y),
                        "w": dumb_round(self.rect.width),
                        "d": dumb_round(self.rect.height),
                        "h": h,
                    }
                )


class RoundRectOp(RectOp):
    tag = "rrect"


class ArcOp(AvaraOperation):
    tag = "arc"

    def __init__(self, context, verb, rect, start, extent):
        self.verb = verb
        self.color = str(context.fg)
        self.rect = rect.offset(context.origin).scale(stroke=context.stroke)
        self.start = start
        self.extent = extent

    def process(self, context):
        if self.verb == Verb.PAINT:
            context["color"] = self.color
        elif self.verb == Verb.FRAME:
            context["color.1"] = self.color

        context.update(
            {
                "cx": dumb_round(self.rect.center.x),
                "cz": dumb_round(self.rect.center.y),
                "r": dumb_round(max(self.rect.width, self.rect.height) / 2),
                "angle": self.start,
                "extent": self.extent,
            }
        )


class OvalOp(ArcOp):
    tag = "oval"

    def __init__(self, context, verb, rect):
        super().__init__(context, verb, rect, 0, 360)

    def process(self, context):
        super().process(context)
        context.update(
            {
                "ox": context["cx"],
                "oz": context["cz"],
            }
        )


def copy_attrs(wall, obj):
    for attr in ("x", "z", "w", "d", "h", "color", "color.1"):
        if attr in wall.attrs:
            obj.attrs[attr] = wall.attrs[attr]
    if "y" in wall.attrs:
        if "y" in obj.attrs:
            # If the Wall and WallDoor both have y, add them
            # together (this is what Avara does/did).
            obj.attrs["y"] = dumb_round(float(obj.attrs["y"]) + float(wall.attrs["y"]))
        else:
            # Otherwise, copy the y from the Wall to the WallDoor.
            obj.attrs["y"] = wall.attrs["y"]


class DrawContext:
    def __init__(self, frame, **attrs):
        self.frame = frame
        self.operations = []
        self.buffered = []

        self.origin = Point(0, 0)
        self.textpos = Point(0, 0)
        self.radius = 0  # Rounding radius (oval size)
        self.stroke = 1  # Current pen size
        self.fg = Color(255, 255, 255)  # Current foreground
        self.bg = Color(255, 255, 255)  # Current background (not used)

        # For [Frame|Paint]Same[Shape] operations.
        self.last_rect = None
        self.last_rrect = None
        self.last_arc = None
        self.last_oval = None

    def group_start(self):
        # self.operations.append(GroupStart())
        pass

    def group_close(self):
        # self.operations.append(GroupEnd())
        pass

    def flush_text(self):
        if not self.buffered:
            return
        text = ""
        last_y = None
        for x, y, s in self.buffered:
            if y != last_y:
                text += "\n"
            text += s
            last_y = y
        if self.operations and isinstance(self.operations[-1], TextOp):
            # Group as much script text together as possible.
            self.operations[-1].text += "\n" + text
        else:
            self.operations.append(TextOp(text))
        self.buffered = []

    def rect(self, rect, verb):
        self.flush_text()
        self.operations.append(RectOp(self, verb, rect))
        self.last_rect = rect

    def rrect(self, rect, verb):
        self.flush_text()
        self.operations.append(RoundRectOp(self, verb, rect))
        self.last_rrect = rect

    def oval(self, rect, verb):
        self.flush_text()
        self.operations.append(OvalOp(self, verb, rect))
        self.last_oval = rect

    def arc(self, rect, start, extent, verb):
        self.flush_text()
        self.operations.append(ArcOp(self, verb, rect, start, extent))
        self.last_arc = rect

    def text(self, s, x=None, y=None, dh=0, dv=0):
        if x is not None:
            self.textpos.x = x
        if y is not None:
            self.textpos.y = y
        if dh:
            self.textpos.x += dh
        if dv:
            self.textpos.y += dv
        self.buffered.append((self.textpos.x, self.textpos.y, s))

    def close(self):
        self.flush_text()
        context = {
            "pixelToThickness": 1 / 8,
        }
        root = Element("map")
        for op in self.operations:
            for el in op.process(context) or []:
                if root.children:
                    if el.tag in ("WallDoor", "WallSolid"):
                        last_wall = root.pop_last("Wall")
                        if last_wall:
                            copy_attrs(last_wall, el)
                        else:
                            print("Wall not found for", el.tag, file=sys.stderr)
                    elif el.tag in ("Field", "FreeSolid"):
                        if "shape" not in el.attrs:
                            # Fields and FreeSolids only use the last wall if there was
                            # no custom shape set on them.
                            last_wall = root.pop_last("Wall")
                            if last_wall:
                                copy_attrs(last_wall, el)
                            else:
                                print("Wall not found for", el.tag, file=sys.stderr)
                root.children.append(el)
        return root.xml()


class Operation:
    length = 0

    def parse(self, data, context):
        data.read(self.length)


class VariableReserved(Operation):
    def parse(self, data, context):
        size = data.short()
        data.read(size)


class NOOP(Operation):
    pass


class SkipRegion(Operation):
    def parse(self, data, context):
        total = data.short()
        region = data.rect()
        data.read(total - 10)


# This function reads embedded image pixmaps
PIXMAP_BIT = 0x8000


def pixmap(data, clipped=False):
    if clipped:
        skip = data.read(4)
    row_bytes = data.short()
    is_pixmap = row_bytes & PIXMAP_BIT != 0
    row_bytes = row_bytes & 0x7FFF
    bounds = data.rect()
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
    return (is_pixmap, bounds, row_bytes)


# Color table for embedded 32 bit color images
def color_table(data):
    ct_seed = data.long()
    trans_index = data.short()
    ct_size = data.short()
    ct = []
    while ct_size >= 0:
        ct.append(data.short())
        ct_size -= 1


def pixdata(data, pmap):
    bounds = pmap[1]
    row_bytes = pmap[2]
    lines_to_read = bounds.height
    if row_bytes < 8:
        data_size = row_bytes * lines_to_read
        pixdata = data.read(data_size)
    else:
        for i in range(lines_to_read):
            sl_size = data.ushort() if row_bytes > 250 else data.uchar()
            scanline = data.read(sl_size)


class BitsRect(Operation):
    def parse(self, data, context):
        pmap = pixmap(data)
        if pmap[0]:
            color_table(data)
        src_rect = data.rect()
        dst_rect = data.rect()
        mode = data.short()
        pixdata(data, pmap)


class BitsRgn(Operation):
    def parse(self, data, context):
        pmap = pixmap(data)
        if pmap[0]:
            color_table(data)
        src_rect = data.rect()
        dst_rect = data.rect()
        mode = data.short()
        mask_rgn_size = data.short()
        mask_rgn = data.read(mask_rgn_size - 2)
        pixdata(data, pmap)


class PackBitsRect(Operation):
    def parse(self, data, context):
        pmap = pixmap(data)
        if pmap[0]:
            color_table(data)
        src_rect = data.rect()
        dst_rect = data.rect()
        mode = data.short()
        pixdata(data, pmap)


class PackBitsRgn(Operation):
    def parse(self, data, context):
        pmap = pixmap(data)
        if pmap[0]:
            color_table(data)
        src_rect = data.rect()
        dst_rect = data.rect()
        mode = data.short()
        mask_rgn_size = data.short()
        mask_rgn = data.read(mask_rgn_size - 2)
        pixdata(data, pmap)


class DirectBitsRect(Operation):
    def parse(self, data, context):
        pmap = pixmap(data, clipped=True)
        src_rect = data.rect()
        dest_rect = data.rect()
        mode = data.short()
        pixdata(data, pmap)


class DirectBitsRgn(Operation):
    def parse(self, data, context):
        pmap = pixmap(data, clipped=True)
        src_rect = data.rect()
        dst_rect = data.rect()
        mode = data.short()
        mask_rgn_size = data.short()
        mask_rgn = data.read(mask_rgn_size - 2)
        pixdata(data, pmap)


class TextFont(Operation):
    def parse(self, data, context):
        font = data.short()


class TextFace(Operation):
    def parse(self, data, context):
        face = data.byte()


class TextMode(Operation):
    def parse(self, data, context):
        mode = data.short()


class PenSize(Operation):
    def parse(self, data, context):
        size = data.point()
        context.stroke = max(size.x, size.y)


class PenMode(Operation):
    def parse(self, data, context):
        mode = data.short()


class PenPattern(Operation):
    def parse(self, data, context):
        pattern = data.read(8)


class FillPattern(Operation):
    def parse(self, data, context):
        pattern = data.read(8)


class OvalSize(Operation):
    def parse(self, data, context):
        size = data.point()
        context.radius = max(size.x, size.y)


class Origin(Operation):
    def parse(self, data, context):
        dh, dv = data.unpack("hh")
        context.origin.x += dh
        context.origin.y += dv
        context.textpos.x += dh
        context.textpos.y += dv


class TextSize(Operation):
    def parse(self, data, context):
        size = data.short()


class TextRatio(Operation):
    def parse(self, data, context):
        numerator = data.point()
        denominator = data.point()


class PenLocationHFractional(Operation):
    def parse(self, data, context):
        pos = data.short()


class RGBForegroundColor(Operation):
    def parse(self, data, context):
        context.fg = data.color()


class RGBBackgroundColor(Operation):
    def parse(self, data, context):
        context.bg = data.color()


class DefaultHighlight(Operation):
    pass


class OpColor(Operation):
    def parse(self, data, context):
        color = data.color()


class Line(Operation):
    def parse(self, data, context):
        start = data.point()
        end = data.point()
        # context.origin.x += end.x
        # context.origin.y += end.y


class LineFrom(Operation):
    def parse(self, data, context):
        p = data.point()
        # context.origin.x += p.x
        # context.origin.y += p.y


class ShortLine(Operation):
    def parse(self, data, context):
        start = data.point()
        dh, dv = data.read(2)
        # context.origin.x += dh
        # context.origin.y += dv


class ShortLineFrom(Operation):
    def parse(self, data, context):
        dh, dv = data.read(2)
        # context.origin.x += dh
        # context.origin.y += dv


class LongText(Operation):
    def parse(self, data, context):
        loc = data.point()
        size = data.byte()
        text = data.read(size).decode("macintosh")
        # LongText is (top, left) so we flip x/y
        context.text(text, x=loc.y, y=loc.x)


class DHText(Operation):
    def parse(self, data, context):
        dh, size = data.read(2)
        text = data.read(size).decode("macintosh")
        context.text(text, dh=dh)


class DVText(Operation):
    def parse(self, data, context):
        dv, size = data.read(2)
        text = data.read(size).decode("macintosh")
        context.text(text, dv=dv)


class DHDVText(Operation):
    def parse(self, data, context):
        dh, dv, size = data.read(3)
        text = data.read(size).decode("macintosh")
        context.text(text, dh=dh, dv=dv)


class FrameRectangle(Operation):
    def parse(self, data, context):
        rect = data.rect()
        context.rect(rect, Verb.FRAME)


class PaintRectangle(Operation):
    def parse(self, data, context):
        rect = data.rect()
        context.rect(rect, Verb.PAINT)


class FrameSameRectangle(Operation):
    def parse(self, data, context):
        if context.last_rect is None:
            debug("FrameSameRectangle with no last rect")
            return
        context.rect(context.last_rect, Verb.FRAME)


class PaintSameRectangle(Operation):
    def parse(self, data, context):
        if context.last_rect is None:
            debug("PaintSameRectangle with no last rect")
            return
        context.rect(context.last_rect, Verb.PAINT)


class FrameRoundedRectangle(Operation):
    def parse(self, data, context):
        rect = data.rect()
        context.rrect(rect, Verb.FRAME)


class PaintRoundedRectangle(Operation):
    def parse(self, data, context):
        rect = data.rect()
        context.rrect(rect, Verb.PAINT)


class FrameSameRoundedRectangle(Operation):
    def parse(self, data, context):
        if context.last_rrect is None:
            debug("FrameSameRoundedRectangle with no last rrect")
            return
        context.rrect(context.last_rrect, Verb.FRAME)


class PaintSameRoundedRectangle(Operation):
    def parse(self, data, context):
        if context.last_rrect is None:
            debug("PaintSameRoundedRectangle with no last rrect")
            return
        context.rrect(context.last_rrect, Verb.PAINT)


class FrameOval(Operation):
    def parse(self, data, context):
        rect = data.rect()
        context.oval(rect, Verb.FRAME)


class PaintOval(Operation):
    def parse(self, data, context):
        rect = data.rect()
        context.oval(rect, Verb.PAINT)


class FrameSameOval(Operation):
    def parse(self, data, context):
        if context.last_oval is None:
            debug("FrameSameOval with no last oval")
            return
        context.oval(context.last_oval, Verb.FRAME)


class PaintSameOval(Operation):
    def parse(self, data, context):
        if context.last_oval is None:
            debug("PaintSameOval with no last oval")
            return
        context.oval(context.last_oval, Verb.PAINT)


class FrameArc(Operation):
    def parse(self, data, context):
        rect = data.rect()
        start = data.short()
        extent = data.short()
        context.arc(rect, start, extent, Verb.FRAME)


class PaintArc(Operation):
    def parse(self, data, context):
        rect = data.rect()
        start = data.short()
        extent = data.short()
        context.arc(rect, start, extent, Verb.PAINT)


class FrameSameArc(Operation):
    def parse(self, data, context):
        start = data.short()
        extent = data.short()
        if context.last_arc is None:
            debug("FrameSameArc with no last arc")
            return
        context.arc(context.last_arc, start, extent, Verb.FRAME)


class PaintSameArc(Operation):
    def parse(self, data, context):
        start = data.short()
        extent = data.short()
        if context.last_arc is None:
            debug("PaintSameArc with no last arc")
            return
        context.arc(context.last_arc, start, extent, Verb.PAINT)


class ShortComment(Operation):
    def parse(self, data, context):
        kind = data.short()
        debug("ShortComment(%s=%s)", kind, PICT_COMMENTS.get(kind, "unknown"))
        if kind == 151:
            # picTextEnd
            context.flush_text()
        if kind == 140:
            # open group
            context.group_start()
        if kind == 141:
            # close group
            context.group_close()


class LongComment(Operation):
    def parse(self, data, context):
        kind, size = data.unpack("hh")
        value = data.read(size)
        hex_value = " ".join(format(x, "02x") for x in value)
        debug(
            "LongComment(%s=%s) %s", kind, PICT_COMMENTS.get(kind, "unknown"), hex_value
        )


class EndPict(Operation):
    pass


class Version(Operation):
    def parse(self, data, context):
        version = data.read(1)
        debug("Version: %s", version)


class Header(Operation):
    length = 24


PICT_OPCODES = {
    0x0: NOOP,
    0x1: SkipRegion,
    0x3: TextFont,
    0x4: TextFace,
    0x5: TextMode,
    0x7: PenSize,
    0x8: PenMode,
    0x9: PenPattern,
    0xA: FillPattern,
    0xB: OvalSize,
    0xC: Origin,
    0xD: TextSize,
    0x10: TextRatio,
    0x11: Version,
    0x15: PenLocationHFractional,
    0x1A: RGBForegroundColor,
    0x1B: RGBBackgroundColor,
    0x1E: DefaultHighlight,
    0x1F: OpColor,
    0x20: Line,
    0x21: LineFrom,
    0x22: ShortLine,
    0x23: ShortLineFrom,
    0x28: LongText,
    0x29: DHText,
    0x2A: DVText,
    0x2B: DHDVText,
    0x2C: VariableReserved,
    0x2E: VariableReserved,
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
    0x8C: NOOP,
    # picture data
    0x90: BitsRect,
    0x91: BitsRgn,
    0x9A: DirectBitsRect,
    0x9B: DirectBitsRgn,
    0x98: PackBitsRect,
    0x99: PackBitsRgn,
    # comment fields
    0xA0: ShortComment,
    0xA1: LongComment,
    0x84: SkipRegion,
    0xFF: EndPict,
    0x02FF: Version,
    0x0C00: Header,
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
    196: "PSBeginNoSave",
}


class PictParseError(Exception):
    pass


def parse_pict(data):
    buf = DataBuffer(data)
    if buf.read(4, advance=False) == b"PICT":
        buf.skip(4)
    size = buf.short()
    frame = buf.rect()
    debug("Size=%s Frame=%s", size, frame)
    context = DrawContext(frame)
    while buf:
        opcode = buf.short()
        try:
            op = PICT_OPCODES[opcode]()
        except KeyError:
            print("No support for opcode 0x%s" % format(opcode, "02x"))
            print(" ".join(format(x, "02x") for x in buf.data[buf.pos : buf.pos + 300]))
            raise PictParseError
        if isinstance(op, EndPict):
            break
        debug("0x%s - %s", format(opcode, "02x"), op.__class__.__name__)
        op.parse(buf, context)
        buf.align()
    return context.close()


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("usage: pict2alf.py <PICT.r>")
        sys.exit(1)
    else:
        path = sys.argv[1]
        if os.path.isdir(path):
            for name in os.listdir(path):
                if name.startswith("."):
                    continue
                full_path = os.path.join(path, name)
                with open(full_path, "rb") as input_file:
                    with open(full_path + ".alf", "w") as output_file:
                        output_file.write(parse_pict(input_file.read()))
        else:
            with open(path, "rb") as input_file:
                print(parse_pict(input_file.read()))
