from .datatypes import *
from Converter.helpers import *


class Operation (object):
    length = 0

    def get_length(self):
        """
        Returns the length (in bytes) of the static data of this
        operation
        """
        return self.length

    def parse(self, bytes):
        pass


class VariableLengthOperation (Operation):
    vari_length = 0

    def parse_variable(self, bytes):
        pass

    def get_variable_length(self):
        return self.vari_length


class VariableReserved(VariableLengthOperation):
    length = 2

    def parse(self, bytes):
        self.vari_length = bytes_to_short(bytes)


class NOP (Operation):
    pass


class ClipRegion (VariableLengthOperation):
    length = 2

    def parse(self, bytes):
        self.vari_length = bytes_to_short(bytes) - 2

    def parse_variable(self, bytes):
        self.region = Rect(bytes)


class FillRegion (VariableLengthOperation):
    length = 2

    def parse(self, bytes):
        self.vari_length = bytes_to_short(bytes) - 2

    def parse_variable(self, bytes):
        self.region = Rect(bytes)


class TextFont (Operation):
    length = 2

    def parse(self, bytes):
        self.font = bytes_to_short(bytes)


class TextFace (Operation):
    length = 1

    def parse(self, bytes):
        self.face = byte_to_unsigned_tiny_int(bytes)


class PenSize (Operation):
    length = 4

    def parse(self, bytes):
        self.size = Point(bytes)


class PenMode (Operation):
    length = 2

    def parse(self, bytes):
        self.mode = bytes_to_short(bytes)


class PenPattern (Operation):
    length = 8

    def parse(self, bytes):
        self.pattern = bytes


class OvalSize (Operation):
    length = 4

    def parse(self, bytes):
        self.size = Point(bytes)


class Origin (Operation):
    length = 4

    def parse(self, bytes):
        self.dh = bytes_to_short(bytes[:2])
        self.dv = bytes_to_short(bytes[2:])


class TextSize (Operation):
    length = 2

    def parse(self, bytes):
        self.size = bytes_to_short(bytes)


class TextRatio (Operation):
    length = 8

    def parse(self, bytes):
        self.numerator = Point(bytes[0:4])
        self.denominator = Point(bytes[4:8])


class PenLocationHFractional (Operation):
    length = 2

    def parse(self, bytes):
        self.pos = bytes_to_short(bytes)


class RGBForegroundColor (Operation):
    length = 6

    def parse(self, bytes):
        self.red = bytes_to_unsigned_short(bytes[0:2])
        self.green = bytes_to_unsigned_short(bytes[2:4])
        self.blue = bytes_to_unsigned_short(bytes[4:6])


class RGBBackgroundColor (Operation):
    length = 6

    def parse(self, bytes):
        self.red = bytes_to_unsigned_short(bytes[0:2])
        self.green = bytes_to_unsigned_short(bytes[2:4])
        self.blue = bytes_to_unsigned_short(bytes[4:6])


class DefaultHighlight (Operation):
    length = 0


class OpColor (Operation):
    length = 6

    def parse(self, bytes):
        self.red = bytes_to_unsigned_short(bytes[0:2])
        self.green = bytes_to_unsigned_short(bytes[2:4])
        self.blue = bytes_to_unsigned_short(bytes[4:6])


class ShortLine (Operation):
    length = 6

    def parse(self, bytes):
        self.start = Point(bytes)
        self.dh = byte_to_signed_tiny_int(bytes[2])
        self.dv = byte_to_signed_tiny_int(bytes[3])


class ShortLineFrom (Operation):
    length = 2

    def parse(self, bytes):
        self.dh = byte_to_signed_tiny_int(bytes[0])
        self.dv = byte_to_signed_tiny_int(bytes[1])


class LongText (VariableLengthOperation):
    length = 5

    def parse(self, bytes):
        self.loc = Point(bytes[0:4])
        self.vari_length = byte_to_unsigned_tiny_int(bytes[4])

    def parse_variable(self, bytes):
        self.text = bytes_to_string(bytes)


class DHText (VariableLengthOperation):
    length = 2

    def parse(self, bytes):
        self.dh = byte_to_unsigned_tiny_int(bytes[0])
        self.vari_length = byte_to_unsigned_tiny_int(bytes[1])

    def parse_variable(self, bytes):
        self.text = bytes_to_string(bytes)


class DVText (VariableLengthOperation):
    length = 2

    def parse(self, bytes):
        self.dv = byte_to_unsigned_tiny_int(bytes[0])
        self.vari_length = byte_to_unsigned_tiny_int(bytes[1])

    def parse_variable(self, bytes):
        self.text = bytes_to_string(bytes)


class DHDVText (VariableLengthOperation):
    length = 3

    def parse(self, bytes):
        self.dh = byte_to_unsigned_tiny_int(bytes[0])
        self.dv = byte_to_unsigned_tiny_int(bytes[1])
        self.vari_length = byte_to_unsigned_tiny_int(bytes[2])

    def parse_variable(self, bytes):
        self.text = bytes_to_string(bytes)


class FrameRectangle (Operation):
    length = 8

    def parse(self, bytes):
        self.rect = Rect(bytes)


class PaintRectangle (Operation):
    length = 8

    def parse(self, bytes):
        self.rect = Rect(bytes)


class FrameSameRectangle (Operation):
    length = 0


class PaintSameRectangle (Operation):
    length = 0


class FrameRoundedRectangle (Operation):
    length = 8

    def parse(self, bytes):
        self.rect = Rect(bytes)


class PaintRoundedRectangle (Operation):
    length = 8

    def parse(self, bytes):
        self.rect = Rect(bytes)


class FrameSameRoundedRectangle (Operation):
    length = 0


class PaintSameRoundedRectangle (Operation):
    length = 0


class FrameOval (Operation):
    length = 8

    def parse(self, bytes):
        self.rect = Rect(bytes)


class PaintOval (Operation):
    length = 8

    def parse(self, bytes):
        self.rect = Rect(bytes)


class FrameSameOval (Operation):
    length = 0


class PaintSameOval (Operation):
    length = 0


class FrameArc (Operation):
    length = 12

    def parse(self, bytes):
        self.rect = Rect(bytes[0:8])
        self.startAngle = bytes_to_short(bytes[8:10])
        self.arcAngle = bytes_to_short(bytes[10:12])


class PaintArc (Operation):
    length = 12

    def parse(self, bytes):
        self.rect = Rect(bytes[0:8])
        self.startAngle = bytes_to_short(bytes[8:10])
        self.arcAngle = bytes_to_short(bytes[10:12])


class FrameSameArc (Operation):
    length = 4

    def parse(self, bytes):
        self.startAngle = bytes_to_short(bytes[0:2])
        self.arcAngle = bytes_to_short(bytes[2:4])


class PaintSameArc (Operation):
    length = 4

    def parse(self, bytes):
        self.startAngle = bytes_to_short(bytes[0:2])
        self.arcAngle = bytes_to_short(bytes[2:4])


class ShortComment (Operation):
    length = 2

    def parse(self, bytes):
        self.kind = bytes_to_short(bytes)

    def __repr__(self):
        print("ShortComment: %d" % self.kind)


class LongComment (VariableLengthOperation):
    length = 4

    def parse(self, bytes):
        self.kind = bytes_to_short(bytes[0:2])
        self.vari_length = bytes_to_short(bytes[2:4])

    def parse_variable(self, bytes):
        self.comment = bytes_to_string(bytes)


class EndPict (Operation):
    length = 0


class Factory (object):
    opcodes = {
        0x0: NOP,
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
        0xff: EndPict
    }

    @staticmethod
    def get_op(opcode):
        if opcode not in Factory.opcodes:
            return None

        return Factory.opcodes[opcode]()


