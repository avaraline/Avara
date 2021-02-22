#!/usr/bin/env python3

"""
Converts Avara level PICT resources to ALF (Avara Level Format)
"""

import enum
import struct
import sys

DEBUG_PARSER = False


def debug(fmt, *args, **kwargs):
    if DEBUG_PARSER:
        print(fmt % args, file=sys.stderr, flush=True)


class Rect:
    def __init__(self, t, l, b, r):
        self.top, self.left, self.bottom, self.right = t, l, b, r

    @property
    def width(self):
        return self.right - self.left

    @property
    def height(self):
        return self.bottom - self.top

    def __str__(self):
        return "(%d, %d) -> (%d, %d)" % (self.left, self.top, self.right, self.bottom)

    def box(self):
        return "%d %d %d %d" % (self.left, self.top, self.width, self.height)

    def offset(self, origin):
        return self.__class__(
            self.top - origin.y,
            self.left - origin.x,
            self.bottom - origin.y,
            self.right - origin.x,
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

    def color(self, context):
        attr = "frame" if self == Verb.FRAME else "fill"
        return '<color %s="%s" />' % (attr, context.fg)


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


class DrawContext:
    def __init__(self, frame, **attrs):
        self.frame = frame
        self.elements = []
        self.buffered = []

        self.origin = Point(0, 0)
        self.r = Point(0, 0)  # Rounding radius (oval size)
        self.fg = Color(255, 255, 255)
        self.bg = Color(255, 255, 255)

        self.fillColor = self.fg
        self.frameColor = self.fg
        self.stroke = 0

        self.wrote_text = False
        self.tag_open = None
        self.path = []
        self.current_id = 0
        self.last_id = 0

        self.last_rect = None
        self.last_rrect = None
        self.last_arc = None
        self.last_oval = None

        self.save_arc = None
        self.save_arc_x = None
        self.save_arc_y = None
        self.save_arc_start = None
        self.save_arc_extent = None

    def group_start(self):
        self.current_id += 1
        self.path.append(self.current_id)

    def group_close(self):
        if self.path:
            self.current_id = self.path.pop()

    def flush_text(self):
        wrote_text = False
        if self.tag_open:
            if self.buffered:
                if self.current_id == self.last_id:
                    self.elements.append("\n".join(self.buffered))
                    self.elements.append("  </%s>" % self.tag_open)
                else:
                    self.elements[-1] += "</%s>" % self.tag_open
                    lines = (
                        ['  <script type="avarascript">']
                        + self.buffered
                        + ["  </script>"]
                    )
                    self.elements.append("\n".join(lines))
                wrote_text = True
            else:
                self.elements[-1] += "</%s>" % self.tag_open
            self.tag_open = None
        elif self.buffered:
            lines = ['  <script type="avarascript">'] + self.buffered + ["  </script>"]
            self.elements.append("\n".join(lines))
            wrote_text = True
        self.buffered = []
        return wrote_text

    def emit(self, tag, rect, **attrs):
        frame = rect.offset(self.origin)
        attrs.update(
            {
                "x": frame.left,
                "y": frame.top,
                "w": frame.width,
                "h": frame.height,
                "fill": self.fillColor,
                "frame": self.frameColor,
            }
        )
        if self.stroke:
            attrs["s"] = self.stroke
        attrs_formatted = " ".join('%s="%s"' % (k, v) for k, v in attrs.items())
        el = "  <%s %s>" % (tag, attrs_formatted)
        self.elements.append(el)
        self.tag_open = tag
        self.last_id = self.current_id

    def element(self, tag, verb, rect, **attrs):
        self.wrote_text = self.flush_text()
        if verb == Verb.FRAME:
            self.frameColor = self.fg
            # If this is framing an arc that was just painted, we can remove the
            # previous <arc> element (so long as it didn't contain script).
            if (
                tag == "arc"
                and not self.wrote_text
                and self.elements
                and self.elements[-1].startswith("  <arc")
                and attrs["start"] == self.save_arc_start
                and attrs["extent"] == self.save_arc_extent
                and self.origin.x == self.save_arc_x
                and self.origin.y == self.save_arc_y
            ):
                self.elements.pop()
            self.emit(tag, rect, **attrs)
        elif verb == Verb.PAINT:
            self.fillColor = self.fg
            if tag == "arc":
                self.emit(tag, rect, **attrs)

    def rect(self, rect, verb):
        self.element("rect", verb, rect)
        self.last_rect = rect

    def rrect(self, rect, verb):
        self.element("rect", verb, rect, r=max(self.r.x, self.r.y))
        self.last_rrect = rect

    def oval(self, rect, verb):
        self.element("oval", verb, rect)
        self.last_oval = rect

    def arc(self, rect, start, extent, verb):
        self.element("arc", verb, rect, start=start, extent=extent)
        self.last_arc = rect
        self.save_arc_start = start
        self.save_arc_extent = extent
        self.save_arc_x = self.origin.x
        self.save_arc_y = self.origin.y

    def text(self, s):
        if not s.strip():
            return
        self.buffered.append("    " + s)

    def close(self):
        self.flush_text()
        return '<map width="%s" height="%s">\n%s\n</map>' % (
            self.frame.width,
            self.frame.height,
            "\n".join(self.elements),
        )


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
        context.r = data.point()


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
