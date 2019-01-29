import struct


def bytes_to_int(some_bytes):
    return struct.unpack('>i', some_bytes)[0]


def bytes_to_short(some_bytes):
    return struct.unpack('>h', some_bytes)[0]


def bytes_to_unsigned_short(some_bytes):
    return struct.unpack('>H', some_bytes)[0]


# def bytes_to_string(some_bytes):
#    return struct.unpack('>' + str(len(some_bytes)) + 's', some_bytes)[0]
# in python 3, we can just decode the string with "macintosh" encoding
def bytes_to_string(some_bytes):
    return some_bytes.decode("macintosh")


def byte_to_unsigned_tiny_int(byte):
    # in python 3, APPARENTLY slicing off
    # a single byte creates an int object
    # so we don't need to do anything here
    return byte
    # tiny_int = b"\x00%b" % bytes(byte)[:1]
    # return struct.unpack('>h', tiny_int)[0]


def byte_to_signed_tiny_int(byte):
    num = byte_to_unsigned_tiny_int(byte)
    if num > 127:
        return 0 - num
    return num


def bytes_to_long(some_bytes):
    return struct.unpack('>l', some_bytes)[0]


def bytes_to_unsigned_long(some_bytes):
    return struct.unpack('>L', some_bytes)[0]


def bytes_to_fixed(some_bytes):
    # "Fixed" numbers in avara are stored
    # in a 8 bit long type, two unsigned ints
    assert(len(some_bytes) == 4)
    whole = bytes_to_short(some_bytes[0:2])
    # maximum unsigned 8-bit int is 65535
    frac = bytes_to_unsigned_short(some_bytes[2:4]) / 65535.0
    # print("Whole: %d Frac: %f" % (whole, frac))
    return whole + frac


def chunks(l, n):
    for i in range(0, len(l), n):
        yield l[i:i + n]


def list_of_fixedsize_recs(data, offset, size, count, the_class):
    list_end = offset + (size * count)
    list_data = data[offset:list_end]
    return [the_class(x) for x in chunks(list_data, size)]
