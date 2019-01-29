from .operations import *
import binascii
from Converter.helpers import *


def parse(pict):
    if(len(pict) < 40):
        print("ERROR: Pict is malformed (smaller than header size)")
        return
    # The first 40 bytes are header information
    if bytes_to_int(pict[10:14]) != 0x001102ff:
        print("ERROR: Not a version 2 PICT")
        return

    data = pict[40:]

    ops = []
    while len(data) > 0:
        curData = data
        opcode = bytes_to_short(data[:2])
        op = Factory.get_op(opcode)
        if op is None:
            print('ERROR: Unknown opcode ' + str(hex(opcode)))
            return

        # if isinstance(op, operations.VariableReserved):
            # print binascii.hexlify(data)

        data = data[2:]

        length = op.get_length()

        if length > 0:
            op.parse(data[:length])
            data = data[length:]

        vari_length = 0
        if isinstance(op, VariableLengthOperation):
            vari_length = op.get_variable_length()
            op.parse_variable(data[:vari_length])
            data = data[vari_length:]

        # By the PICT specification, if the data length is odd
        # it adds an additional null byte on the end
        # This checks if the length is odd and if so checks
        # for a null byte (which must be there, by the spec)
        if (vari_length + length) % 2:
            if not byte_to_signed_tiny_int(data[0]) == 0x00:
                print('ERROR: Null byte expected')
                print(binascii.hexlify(curData))
                return
            data = data[1:]
        ops.append(op)

        if isinstance(op, EndPict):
            break

    return ops
