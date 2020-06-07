from rsrc_tools.helpers import *


def tiny_int_from_single_byte(byte):
    tiny_int = b"\x00%b" % byte
    return struct.unpack('>h', tiny_int)[0]

def tiny_int_from_two_bytes(the_bytes):
    return struct.unpack('>h', the_bytes)[0]


def parse(resources):
    if 'TMPL' not in resources:
        print("No TMPL for these resources")
        return {}
    
    tmpl_res = resources['TMPL']
    tmpls = tmpl_res.keys()

    result = {}
    result["TMPL"] = {}
    # each TMPL resource defines another resource type
    for key in tmpls:
        res_name = tmpl_res[key]['name']

        if res_name not in resources:
            # skip cus we don't have this resource type
            continue

        if res_name == "BSPT" or res_name == "HSND":
            # skip this cus we have a different 
            # converter for that.
            continue

        tmpl = tmpl_res[key]['data']

        structure = []
        # we consume all the template values into a structure 
        # to use later
        while len(tmpl) > 1:
            # name is a pascal string with no padding
            namelength = tmpl[0]
            tmpl = tmpl[1:]
            dtname = (tmpl[0:namelength]).decode('macroman')
            tmpl = tmpl[namelength:]
            # always a TNAM, or four character string
            restype = (tmpl[:4]).decode('macroman')
            tmpl = tmpl[4:]

            structure.append({
                'name': dtname,
                'type': restype
                })

        result["TMPL"][res_name] = structure
        # print(structure)
        # now we go open the resources that we can find
        # with these templates and attempt to parse them
        result[res_name] = {}
        the_res = resources[res_name]
        for key in the_res.keys():
            result[res_name][key] = {}
            instance = the_res[key]
            name = instance["name"]
            data = instance["data"]

            _, stuff = read_data_with_template(data, structure)
            result[res_name][key] = stuff
        # return a big dict with both the TMPL and anything else
        # we found to parse
        # print(result)
    return result

def read_data_with_template(data, structure):
    # this is an awful recursive thing to parse the list types available in ResEdit
    result = {}
    end = -1
    panic = 0
    for i, dtype in enumerate(structure):
        
        if i < end:
            # skip items if we're inside a list 
            continue

        t = dtype["type"]

        if t == "OCNT":
            # a OCNT means we have a sub-list of records coming up, with the fields
            # of the records being between the LSTC and LSTE records following
            data, count = get_value(data, t)
            
            # the next index is a LSTC and we don't need that for the sub records
            start = i + 2

            # get the last field on the sub records
            end = next((index for (index, d) in 
                enumerate(structure) if d["name"] == structure[start - 1]["name"] and d["type"] == "LSTE"))
            
            # print ("list of %d things, start: %d end: %d" % (count, start, end))

            # create a "substructure" for those records in this list
            substructure = structure[start:end]

            # add  one to the end value so that we don't attempt to
            # process the LSTE element later on as a field
            end += 1

            # print ("with substructure %s" % substructure)
            the_list = []

            if count == 0:
                # zero count list means there will be nothing 
                # left to parse, move on to the next thing
                # print("zero count list")
                continue

            for i in range(0, count):
                # recursively read the list and append results to our result
                # by passing the inside of the list type to this function again
                data, the_result = read_data_with_template(data, substructure)
                the_list.append(the_result)
                # this is usually a weird separator, i think by default its "*****"
                list_key = structure[start-1]["name"]
                result[list_key] = the_list
                pass

        # this skips values inside a list type
        elif i > end:
            # print(dtype)
            data, the_value = get_value(data, t)
            # print(the_value)
            result[dtype["name"]] = the_value

    return data, result

def get_value(the_data, dtype):
    # this data was compiled from ResEdit Reference For ResEdit 2.1
    # published by Apple Computer Developer Press 1995
    # page 79-80 (93-94 of PDF)
    # I only added the datatypes we needed. There are quite a few 
    # more available for TMPL fields. This is where you want to add
    # more field types if you need them for something else.

    # TNAM: Type Name (Four characters)
    if dtype == "TNAM":
        return (the_data[4:], bytes_to_string(the_data[:4]))
    
    # OCNT: "One Count" works with LSTC and LSTE to define a list type.
    if dtype == "OCNT":
        value = bytes_to_short(the_data[:2])
        return (the_data[2:], value)
    
    # ESTR: Pascal string (length byte followed by characters)
    # padded to even length
    if dtype == "ESTR":
        eat_one_more_byte = False
        length = tiny_int_from_single_byte(the_data[:1])
        # if we have an EVEN number of charcters, there is
        # a padding byte on the end, because the length byte
        # counts as one--so we need to eat an extra byte afterwards
        if length % 2 == 0:
            eat_one_more_byte = True
        # Even padded pascal string
        the_str = bytes_to_string(the_data[1:length + 1])
        if eat_one_more_byte:
            return (the_data[length + 2:], the_str)
        else:
            return (the_data[length + 1:], the_str)
    
    # DWRD: Decimal word (two bytes)
    if dtype == "DWRD":
        value = bytes_to_short(the_data[:2])
        return (the_data[2:], value)

    # DLNG: Decimal long word (four bytes)
    if dtype == "DLNG":
        value = bytes_to_int(the_data[:4])
        return (the_data[4:], value)

    # FIXD: 16:16 fixed point number
    # this one wasn't in the book. I got it from
    # http://www.mathemaesthetics.com/ResTemplates.html
    # it is four bytes long
    if dtype == "FIXD":
        value = bytes_to_fixed(the_data[:4])
        return (the_data[4:], value)
    
    # BOOL: Boolean (two bytes)
    if dtype == "BOOL":
        value = bytes_to_short(the_data[:2])
        the_bool = value > 0
        return (the_data[2:], the_bool)
    
    # HLNG: Hex long word (four bytes)
    if dtype == "HLNG":
        value = bytes_to_int(the_data[:4])
        return (the_data[4:], value)

    # HEXD: Hex dump of remaining bytes in resource
    # can only be at the END of a resource so we can
    # just return everything as bytes
    if dtype == "HEXD":
        return (None, the_data)
