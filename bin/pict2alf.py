#!/usr/bin/env python3

"""
Converts Avara level PICT resources to ALF (Avara Level Format)
"""

import enum
import struct
import sys

from avarascript import Element, object_context, parse_script

DEBUG_PARSER = False
METERS_PER_POINT = 5 / 72


def debug(fmt, *args, **kwargs):
    if DEBUG_PARSER:
        print(fmt % args, file=sys.stderr, flush=True)


def dumb_round(num, repeats=6):
    s = str(num)
    if "." not in s:
        return s
    zeros = "0" * repeats
    nines = "9" * repeats
    if zeros in s:
        return s.split(zeros, 1)[0].rstrip(".")
    if nines in s:
        # How many digits before the nines is how many we want to round to.
        places = len(s.split(nines, 1)[0].split(".")[1]) or None
        return str(round(num, places))
    return s


# How's this for continuous integration.
assert dumb_round(10.00000007) == "10"
assert dumb_round(4.500000022) == "4.5"
assert dumb_round(0.999999987) == "1", dumb_round(0.999999987)
assert dumb_round(0.049999999) == "0.05"


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
        r = self.r >> (self.bpp - 8)
        g = self.g >> (self.bpp - 8)
        b = self.b >> (self.bpp - 8)
        return "#%02x%02x%02x" % (r, g, b)

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

    def read(self, num):
        buf = self.data[self.pos : self.pos + num]
        self.pos += num
        return buf

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


class TextOp(AvaraOperation):
    def __init__(self, text):
        self.tokens = parse_script(text)

    def process(self, context):
        for t in self.tokens:
            if t.process(context):
                yield t.element(context)


class GroupStart(AvaraOperation):
    pass


class GroupEnd(AvaraOperation):
    pass


class RectOp(AvaraOperation):
    tag = "rect"

    def __init__(self, context, verb, rect):
        self.verb = verb
        self.fill = context.fill_color
        self.frame = context.frame_color
        self.stroke = context.stroke
        self.radius = context.radius
        self.rect = rect.offset(context.origin).scale(stroke=self.stroke)

    def process(self, context):
        context.update(
            {
                "fill": self.fill,
                "frame": self.frame,
                "x": dumb_round(self.rect.left),
                "z": dumb_round(self.rect.top),
                "w": dumb_round(self.rect.width),
                "d": dumb_round(self.rect.height),
            }
        )
        if self.stroke == 1 and self.tag == "rect":
            attrs = object_context("Wall", context)
            if self.radius:
                attrs["h"] = dumb_round(self.radius * context["pixelToThickness"])
            elif context.get("wallHeight"):
                try:
                    # If wallHeight is numeric (not a variable/forumla), promote it to "h"
                    float(context["wallHeight"])
                    attrs["h"] = context["wallHeight"]
                except ValueError:
                    pass
            if context.get("wa"):
                try:
                    # If wa is numeric, promote it to "y"
                    float(context["wa"])
                    attrs["y"] = context["wa"]
                except ValueError:
                    pass
                # wa is reset after every Wall
                del context["wa"]
            yield Element("Wall", **attrs)


class RoundRectOp(RectOp):
    tag = "rect"


class ArcOp(RectOp):
    tag = "arc"

    def __init__(self, context, verb, rect, start, extent):
        super().__init__(context, verb, rect)
        self.start = start
        self.extent = extent

    def process(self, context):
        context.update(
            {
                "fill": self.fill,
                "frame": self.frame,
                "cx": dumb_round(self.rect.center.x),
                "cy": dumb_round(self.rect.center.y),
                "r": dumb_round(max(self.rect.width, self.rect.height) / 2),
                "angle": self.start,
                "extent": self.extent,
            }
        )
        return []


class OvalOp(ArcOp):
    tag = "oval"

    def __init__(self, context, verb, rect):
        super().__init__(context, verb, rect, 0, 360)


class DrawContext:
    def __init__(self, frame, **attrs):
        self.frame = frame
        self.operations = []
        self.buffered = []

        self.origin = Point(0, 0)
        self.radius = 0  # Rounding radius (oval size)
        self.stroke = 0  # Current pen size
        self.fg = Color(255, 255, 255)  # Current foreground
        self.bg = Color(255, 255, 255)  # Current background (not used)

        # These are set from the current foreground color on paint/frame operations.
        self.fill_color = self.fg
        self.frame_color = self.fg

        # For [Frame|Paint]Same[Shape] operations.
        self.last_rect = None
        self.last_rrect = None
        self.last_arc = None
        self.last_oval = None

    def group_start(self):
        self.operations.append(GroupStart())

    def group_close(self):
        self.operations.append(GroupEnd())

    def flush_text(self):
        if not self.buffered:
            return
        self.operations.append(TextOp("\n".join(self.buffered)))
        self.buffered = []

    def update_colors(self, verb):
        if verb == Verb.PAINT:
            self.fill_color = self.fg
        elif verb == Verb.FRAME:
            self.frame_color = self.fg

    def rect(self, rect, verb):
        self.update_colors(verb)
        if verb == Verb.FRAME:
            self.operations.append(RectOp(self, verb, rect))
        self.last_rect = rect

    def rrect(self, rect, verb):
        self.update_colors(verb)
        if verb == Verb.FRAME:
            self.operations.append(RoundRectOp(self, verb, rect))
        self.last_rrect = rect

    def oval(self, rect, verb):
        self.update_colors(verb)
        if verb == Verb.FRAME:
            self.operations.append(OvalOp(self, verb, rect))
        self.last_oval = rect

    def arc(self, rect, start, extent, verb):
        self.update_colors(verb)
        self.operations.append(ArcOp(self, verb, rect, start, extent))
        self.last_arc = rect

    def text(self, s):
        if not s.strip():
            return
        self.buffered.append(s)

    def close(self):
        self.flush_text()
        context = {
            "pixelToThickness": 1 / 8,
        }
        root = Element("map")
        for op in self.operations:
            for el in op.process(context):
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
        context.text(text)


class DHText(Operation):
    def parse(self, data, context):
        dh, size = data.read(2)
        text = data.read(size).decode("macintosh")
        context.text(text)


class DVText(Operation):
    def parse(self, data, context):
        dv, size = data.read(2)
        text = data.read(size).decode("macintosh")
        context.text(text)


class DHDVText(Operation):
    def parse(self, data, context):
        dh, dv, size = data.read(3)
        text = data.read(size).decode("macintosh")
        context.text(text)


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
    # 0x90: BitsRect,
    # 0x91: BitsRgn,
    # 0x9A: DirectBitsRect,
    # 0x9B: DirectBitsRgn,
    # 0x98: PackBitsRect,
    # 0x99: PackBitsRgn,
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
        with open(sys.argv[1], "rb") as input_file:
            print(parse_pict(input_file.read()))
