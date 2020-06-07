from .helpers import *


class Reader(object):

    def get_name(self, raw, offset):
        if offset == -1:
            return ""
        length = bytes_to_short(b'\x00' + raw[offset:offset + 1])
        return bytes_to_string(raw[offset + 1:offset + 1 + length])

    def get_data(self, raw, offset):
        length = bytes_to_int(raw[offset:offset + 4])
        return raw[offset + 4:offset + 4 + length]

    def parse(self, rawData):
        resources = {}

        header = rawData[0:16]

        dataOffset = bytes_to_int(header[0:4])
        mapOffset = bytes_to_int(header[4:8])
        dataLength = bytes_to_int(header[8:12])
        mapLength = bytes_to_int(header[12:16])
        data = rawData[dataOffset:dataOffset + dataLength]
        # print(data)
        resmap = rawData[mapOffset:mapOffset + mapLength]

        nameOffset = bytes_to_short(resmap[26:28])

        # this is off by one (?) so we add one here
        numTypes = bytes_to_short(resmap[28:30]) + 1
        typeLength = 8 * (numTypes)
        # the part of the header that tells us the offset lies
        rawTypes = resmap[30:30 + typeLength]
        rawList = resmap[30 + typeLength:nameOffset]
        rawNames = resmap[nameOffset:]
        types = {}
        for i in range(numTypes):
            name = bytes_to_string(rawTypes[8 * i:(8 * i) + 4])
            number = bytes_to_short(rawTypes[(8 * i) + 4:(8 * i) + 6]) + 1
            offset = bytes_to_short(rawTypes[(8 * i) + 6:(8 * i) + 8])

            types[name] = {'offset': offset, 'number': number}

        list = {}
        for k, v in iter(types.items()):
            # Offset supplied is wrong
            realOffset = ((v['offset'] - typeLength) - 2)
            rawResources = rawList[realOffset:realOffset +
                                   ((v['number']) * 12)]
            list[k] = {}

            for i in range(v['number']):
                off = i * 12
                the_id = bytes_to_short(rawResources[off:off + 2])
                nameOffset = bytes_to_short(rawResources[off + 2:off + 4])
                # Data offset is 3 bytes long, need an extra one
                dataOffset = bytes_to_int(b"\x00" +
                                          rawResources[off + 5:off + 8])
                list[k][the_id] = {
                    'nameOffset': nameOffset,
                    'dataOffset': dataOffset
                }

        for the_type, definitions in iter(list.items()):
            resources[the_type] = {}

            for the_id, offsets in iter(definitions.items()):
                resource = {
                    'name': self.get_name(rawNames, offsets['nameOffset']),
                    'data': self.get_data(data, offsets['dataOffset'])
                }
                resources[the_type][the_id] = resource

        return resources
