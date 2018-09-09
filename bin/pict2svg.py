#!/usr/bin/env python3
# Converts PICT files to SVG
# See https://developer.apple.com/library/archive/documentation/mac/QuickDraw/QuickDraw-458.html

import os
import struct
import sys
import re
import math


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
        buf = data[self.pos:self.pos + num]
        self.pos += num
        return buf

    def unpack(self, fmt):
        size = struct.calcsize(fmt)
        nums = struct.unpack('!' + fmt, data[self.pos:self.pos + size])
        self.pos += size
        return nums

    def short(self):
        return self.unpack('h')[0]

    def ushort(self):
        return self.unpack('H')[0]

    def byte(self):
        return self.read(1)[0]

    def rect(self):
        return Rect(self.short(), self.short(), self.short(), self.short())

    def point(self):
        return Point(self.short(), self.short())

    def color(self):
        return Color(self.ushort(), self.ushort(), self.ushort())

    def align(self):
        if self.pos % 2:
            self.pos += 1


class SVGContext:

    def __init__(self, **attrs):
        self.xml = ''
        self.buffered = []
        self.textpos = Point(0, 0)
        self.write('svg', xmlns='http://www.w3.org/2000/svg', close=False, **attrs)
        self.x = 0
        self.y = 0
        self.w = 1
        self.r = Point(0, 0)
        self.fg = Color(0, 0, 0)
        self.bg = Color(255, 255, 255, bpp=8)

    def write(self, element, close=True, data=None, **attrs):
        if self.buffered:
            text = '\r'.join(self.buffered)
            self.buffered = []
            self.write('text', data=text, x=self.textpos.x, y=self.textpos.y)
            self.textpos = Point(0, 0)
        parts = [element]
        for name, value in attrs.items():
            # name = re.sub(r'([A-Z])', r'-\1', name).strip().lower()
            name = name.replace('_', '-')
            parts.append('{}="{}"'.format(name, str(value).replace('"', '&quot;')))
        if data is not None:
            data = str(data).replace('<', '&lt;').replace('>', '&gt')
            self.xml += '<{}>{}</{}>\n'.format(' '.join(parts), data, element)
        else:
            if close:
                parts.append('/')
            self.xml += '<{}>\n'.format(' '.join(parts))

    def text(self, s, x=None, y=None):
        if x and not self.textpos.x:
            self.textpos.x = x
        if y and not self.textpos.y:
            self.textpos.y = y
        self.buffered.append(str(s))

    def close(self):
        self.xml += '</svg>\n'
        return self.xml


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


class PenSize (Operation):

    def parse(self, data, context):
        size = data.point()


class PenMode (Operation):

    def parse(self, data, context):
        mode = data.short()


class PenPattern (Operation):

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


class RGBBackgroundColor (Operation):

    def parse(self, data, context):
        context.bg = data.color()


class DefaultHighlight (Operation):
    pass


class OpColor (Operation):

    def parse(self, data, context):
        color = data.color()


class ShortLine (Operation):

    def parse(self, data, context):
        start = data.point()
        dh, dv = data.read(2)


class ShortLineFrom (Operation):

    def parse(self, data, context):
        dh, dv = data.read(2)


class LongText (Operation):

    def parse(self, data, context):
        loc = data.point()
        size = data.byte()
        text = data.read(size).decode('macintosh')
        context.text(text, x=context.x + loc.x, y=context.y + loc.y)


class DHText (Operation):

    def parse(self, data, context):
        dh, size = data.read(2)
        text = data.read(size).decode('macintosh')
        context.text(text, x=context.x + dh, y=context.y)


class DVText (Operation):

    def parse(self, data, context):
        dv, size = data.read(2)
        text = data.read(size).decode('macintosh')
        context.text(text, x=context.x, y=context.y + dv)


class DHDVText (Operation):

    def parse(self, data, context):
        dh, dv, size = data.read(3)
        text = data.read(size).decode('macintosh')
        context.text(text, x=context.x + dh, y=context.y + dv)


class FrameRectangle (Operation):

    def parse(self, data, context):
        rect = data.rect()
        context.write('rect', x=rect.left + context.x, y=rect.top + context.y, width=rect.width, height=rect.height, fill='none', stroke=context.fg, stroke_width=context.w)


class PaintRectangle (Operation):

    def parse(self, data, context):
        rect = data.rect()
        context.write('rect', x=rect.left + context.x, y=rect.top + context.y, width=rect.width, height=rect.height, fill=context.fg)


class FrameSameRectangle (Operation):
    pass


class PaintSameRectangle (Operation):
    pass


class FrameRoundedRectangle (Operation):

    def parse(self, data, context):
        rect = data.rect()
        context.write('rect', x=rect.left + context.x, y=rect.top + context.y, width=rect.width, height=rect.height, fill='none', stroke=context.fg, stroke_width=context.w, rx=context.r.x, ry=context.r.y)


class PaintRoundedRectangle (Operation):

    def parse(self, data, context):
        rect = data.rect()
        context.write('rect', x=rect.left + context.x, y=rect.top + context.y, width=rect.width, height=rect.height, fill=context.fg, rx=context.r.x, ry=context.r.y)


class FrameSameRoundedRectangle (Operation):
    pass


class PaintSameRoundedRectangle (Operation):
    pass


class FrameOval (Operation):

    def parse(self, data, context):
        rect = data.rect()


class PaintOval (Operation):

    def parse(self, data, context):
        rect = data.rect()


class FrameSameOval (Operation):
    pass


class PaintSameOval (Operation):
    pass


def arc_path(context, rect, start, angle):
    start -= 90
    x = context.x + rect.left + (rect.width // 2)
    y = context.y + rect.top + (rect.height // 2)
    radius = min(rect.width / 2, rect.height / 2)
    start_x = x + (radius * math.cos(math.radians(start)))
    start_y = y + (radius * math.sin(math.radians(start)))
    end_x = x + (radius * math.cos(math.radians(start + angle)))
    end_y = y + (radius * math.sin(math.radians(start + angle)))
    d = ['M', start_x, start_y, 'A', radius, radius, 0, 1, end_x, end_y]
    return ' '.join(str(p) for p in d)


class FrameArc (Operation):

    def parse(self, data, context):
        rect = data.rect()
        start, angle = data.unpack('hh')
        path = arc_path(context, rect, start, angle)
        context.write('path', d=path, stroke=context.fg)


class PaintArc (Operation):

    def parse(self, data, context):
        rect = data.rect()
        start, angle = data.unpack('hh')
        path = arc_path(context, rect, start, angle)
        context.write('path', d=path, fill=context.fg)


class FrameSameArc (Operation):

    def parse(self, data, context):
        start, angle = data.unpack('hh')


class PaintSameArc (Operation):

    def parse(self, data, context):
        start, angle = data.unpack('hh')


class ShortComment (Operation):

    def parse(self, data, context):
        kind = data.short()


class LongComment (Operation):

    def parse(self, data, context):
        kind, size = data.unpack('hh')
        text = data.read(size).decode('macintosh')


class EndPict (Operation):
    pass


class Version (Operation):
    pass


class Header (Operation):
    length = 24


PICT_OPCODES = {
    0x0: NOOP,
    0x1: ClipRegion,
    0x3: TextFont,
    0x4: TextFace,
    0x7: PenSize,
    0x8: PenMode,
    0x9: PenPattern,
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
    # 0x71: PaintPoly,
    0xa0: ShortComment,
    0xa1: LongComment,
    0x84: FillRegion,
    0xff: EndPict,
    0x02ff: Version,
    0x0c00: Header,
}


def parse_pict(data):
    buf = DataBuffer(data)
    size = buf.short()
    frame = buf.rect()
    context = SVGContext(viewBox=frame.box())
    while buf:
        opcode = buf.short()
        op = PICT_OPCODES[opcode]()
        if isinstance(op, EndPict):
            break
        # print(op.__class__.__name__)
        op.parse(buf, context)
        buf.align()
    return context.close()


if __name__ == '__main__':
    data = open(sys.argv[1], 'rb').read()
    print(parse_pict(data))
