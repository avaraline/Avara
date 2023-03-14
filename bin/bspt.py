#!/usr/bin/env python3

import sys
import struct
from itertools import zip_longest
import json

from applergb import applergb_to_srgb

try:
    import triangle
    import triangle.plot
    import numpy as np
except ImportError:
    print("triangle, numpy not found (pip install triangle numpy)")
    sys.exit(1)


DEBUG_ADDED_VERTS = False

if DEBUG_ADDED_VERTS:
    try:
        import matplotlib.pyplot as plt
    except ImportError:
        print("added face debug is enabled but matplotlib not found (pip install matplotlib)")
        sys.exit(1)




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



UniquePointLength = 16


class UniquePoint():
    size = 16

    def __init__(self, raw_data=None):
        if raw_data is None:
            self.x = 0
            self.y = 0
            self.z = 0
            self.w = 0
        else:
            assert(len(raw_data) == UniquePointLength)
            self.x = bytes_to_fixed(raw_data[0:4])
            self.y = bytes_to_fixed(raw_data[4:8])
            self.z = bytes_to_fixed(raw_data[8:12])
            self.w = bytes_to_fixed(raw_data[12:16])

    def __repr__(self):
        tup = (self.x, self.y, self.z, self.w)
        return "UniquePoint(%f %f %f %f)" % tup

    def serialize(self):
        return self.as_list_4()

    def as_list_3(self):
        return [self.x, self.y, self.z]

    def as_list_4(self):
        return [self.x, self.y, self.z, self.w]


NormalRecordLength = 8


class NormalRecord():
    def __init__(self, raw_data):
        assert(len(raw_data) == NormalRecordLength)
        self.normal_index = bytes_to_unsigned_short(raw_data[0:2])
        self.base_point_index = bytes_to_unsigned_short(raw_data[2:4])
        self.color_index = bytes_to_unsigned_short(raw_data[4:6])
        self.visibility_flags = bytes_to_unsigned_short(raw_data[6:8])

    def __repr__(self):
        tup = (self.normal_index, self.base_point_index, self.color_index, self.visibility_flags)
        return "NormalRecord (%d %d %d %d)" % tup

    def serialize(self):
        return [
            self.normal_index,
            self.base_point_index,
            self.color_index,
            self.visibility_flags
        ]


EdgeRecordLength = 4


class EdgeRecord():
    def __init__(self, raw_data):
        assert(len(raw_data) == EdgeRecordLength)
        self.a = bytes_to_unsigned_short(raw_data[0:2])
        self.b = bytes_to_unsigned_short(raw_data[2:4])

    def __repr__(self):
        tup = (self.a, self.b)
        return "EdgeRecord (%d %d)" % tup

    def serialize(self):
        return [self.a, self.b]


PolyRecordLength = 16


class PolyRecord():
    def __init__(self, raw_data):
        assert(len(raw_data) == PolyRecordLength)
        self.first_edge = bytes_to_unsigned_short(raw_data[0:2])
        self.edge_count = bytes_to_unsigned_short(raw_data[2:4])
        self.normal_index = bytes_to_unsigned_short(raw_data[4:6])
        self.front_poly = bytes_to_unsigned_short(raw_data[6:8])
        self.back_poly = bytes_to_unsigned_short(raw_data[8:10])
        self.visibility = bytes_to_unsigned_short(raw_data[10:12])
        self.reserved = bytes_to_unsigned_long(raw_data[12:16])

    def __repr__(self):
        tup = (self.first_edge, self.edge_count, self.normal_index, self.front_poly, self.back_poly, self.visibility)
        return "PolyRecord (%d %d %d %d %d %d)" % tup

    def serialize(self):
        return [
            self.first_edge,
            self.edge_count,
            self.normal_index,
            self.front_poly,
            self.back_poly,
            self.visibility,
            self.reserved
        ]


ColorRecordLength = (32 * 2) + 4


class ColorRecord():
    def __init__(self, raw_data):
        assert(len(raw_data) == ColorRecordLength)
        self.color_long = bytes_to_long(raw_data[0:4])

        self.color = [
            ((self.color_long >> 16) & 0xff),
            ((self.color_long >> 8) & 0xff),
            (self.color_long & 0xff),
            ((self.color_long >> 24) & 0xff),
        ]
        # leave first two marker colors alone but convert any other color to sRGB from AppleRGB
        if self.color != [254, 254, 254, 0] and self.color != [254, 0, 0, 0]:
            self.color = [int(round(255.0 * y)) for y in applergb_to_srgb([x / 255.0 for x in self.color[:3]])]
            if self.color != [3, 51, 255] and self.color != [146, 146, 146]:
                self.color.append(0xff) # use full alpha for any non-marker color
            else:
                self.color.append(0x00) # use empty alpha for the remaining marker colors
        self.color_long = (self.color[3] << 24) | (self.color[0] << 16) | (self.color[1] << 8) | self.color[2]

        # colorCache[32] (COLORCACHESIZE)
        # this is not useful to most people because
        # it was used to store intermediate shades
        # of a color when shading it with white light
        self.color_cache = [bytes_to_unsigned_short(x)
                            for x in chunks(raw_data[4:ColorRecordLength], 2)]

    def __repr__(self):
        tup = (self.color, self.color_cache)
        return "ColorRecord (%s %s)" % tup

    def serialize(self):
        return self.color


class BSP(object):
    # https://github.com/jmunkki/Avara/blob/master/src/Libraries/BSP/BSPResStructures.h
    def __init__(self, raw_data):

        self.ref_count = bytes_to_unsigned_short(raw_data[0:2])
        self.lock_count = bytes_to_unsigned_short(raw_data[2:4])

        self.enclosure_point = UniquePoint(raw_data[4:20])
        self.enclosure_radius = bytes_to_fixed(raw_data[20:24])

        self.min_bounds = UniquePoint(raw_data[24:40])
        self.max_bounds = UniquePoint(raw_data[40:56])

        self.normal_count = bytes_to_unsigned_long(raw_data[56:60])

        self.edge_count = bytes_to_unsigned_long(raw_data[60:64])
        self.poly_count = bytes_to_unsigned_long(raw_data[64:68])
        self.color_count = bytes_to_unsigned_long(raw_data[68:72])
        self.point_count = bytes_to_unsigned_long(raw_data[72:76])
        self.vector_count = bytes_to_unsigned_long(raw_data[76:80])
        self.unique_edge_count = bytes_to_unsigned_long(raw_data[80:84])

        if self.poly_count == 0:
            print("Shape %s has no faces, making empty shape" % self.name)
            self.normals = []
            self.polys = []
            self.edges = []
            self.colors = []
            self.points = []
            self.vectors = []
            self.unique_edges = []
            return

        normal_offset = bytes_to_unsigned_long(raw_data[84:88])
        poly_offset = bytes_to_unsigned_long(raw_data[88:92])
        edge_offset = bytes_to_unsigned_long(raw_data[92:96])
        color_offset = bytes_to_unsigned_long(raw_data[96:100])
        point_offset = bytes_to_unsigned_long(raw_data[100:104])
        vector_offset = bytes_to_unsigned_long(raw_data[104:108])
        unique_edge_offset = bytes_to_unsigned_long(raw_data[108:112])

        self.normals = list_of_fixedsize_recs(
            raw_data,
            normal_offset,
            NormalRecordLength,
            self.normal_count,
            NormalRecord)

        self.polys = list_of_fixedsize_recs(
            raw_data,
            poly_offset,
            PolyRecordLength,
            self.poly_count,
            PolyRecord)

        # Edges are a list of shorts indexing verticies
        self.edges = list_of_fixedsize_recs(
            raw_data,
            edge_offset,
            2,
            self.edge_count,
            bytes_to_unsigned_short)

        self.colors = list_of_fixedsize_recs(
            raw_data,
            color_offset,
            ColorRecordLength,
            self.color_count,
            ColorRecord)

        self.points = list_of_fixedsize_recs(
            raw_data,
            point_offset,
            UniquePointLength,
            self.point_count,
            UniquePoint)

        # vectors are 4 part just like unique point
        self.vectors = list_of_fixedsize_recs(
            raw_data,
            vector_offset,
            UniquePointLength,
            self.vector_count,
            UniquePoint)

        self.unique_edges = list_of_fixedsize_recs(
            raw_data,
            unique_edge_offset,
            EdgeRecordLength,
            self.unique_edge_count,
            EdgeRecord)

        assert(len(self.normals) == self.normal_count)
        assert(len(self.edges) == self.edge_count)
        assert(len(self.colors) == self.color_count)
        assert(len(self.points) == self.point_count)
        assert(len(self.vectors) == self.vector_count)
        assert(len(self.unique_edges) == self.unique_edge_count)

    def __repr__(self):
        r = "%s - Total verticies: %d" % (self.name, self.point_count)
        r += "\nmin bound: %s" % self.min_bounds
        r += "\nmax bound: %s" % self.max_bounds
        r += "\npoints: %s" % self.points
        r += "\nedges: %s" % self.edges
        r += "\npolys: %s" % self.polys
        r += "\nnormals: %s" % self.normals
        r += "\ncolors: %s" % self.colors
        r += "\nvectors: %s" % self.vectors
        r += "\nunique edges: %s" % self.unique_edges
        return r

    def serialize(self, int_colors=False):
        d = {}
        d["enclosure_point"] = self.enclosure_point.serialize()
        d["enclosure_radius"] = self.enclosure_radius
        d["min_bounds"] = self.min_bounds.serialize()
        d["max_bounds"] = self.max_bounds.serialize()
        d["points"] = serialize_list(self.points)
        d["normals"] = serialize_list(self.normals)
        d["edges"] = self.edges
        d["polys"] = serialize_list(self.polys)
        if int_colors:
            d["colors"] = [c.color_long for c in self.colors]
        else:
            d["colors"] = serialize_list(self.colors)
        d["vectors"] = serialize_list(self.vectors)
        d["unique_edges"] = serialize_list(self.unique_edges)
        return d

    def triangulate(self, int_colors=False):
        d = self.serialize(int_colors=int_colors)
        d["triangles_poly"] = list()
        d["triangles_verts_poly"] = list()
        # print(self.name)
        # avara stored individual shape faces as
        # N-gons
        for poly in self.polys:
            # lookup the normal record
            normal_rec = self.normals[poly.normal_index]
            normal_idx = normal_rec.normal_index
            normal = np.array(self.vectors[normal_idx].as_list_3())
            # get a unique list of vertices and a list of
            # edges which store indices into that list of vertices
            verts = []
            edges = []
            for idx in range(0, poly.edge_count):
                # get the edge values from indexes
                e = self.unique_edges[self.edges[poly.first_edge + idx]]
                # first make sure we have both points in this edge
                if e.a not in verts:
                    verts.append(e.a)
                if e.b not in verts:
                    verts.append(e.b)
                # then update our edge list with the points in this edge
                myidx_a = verts.index(e.a)
                myidx_b = verts.index(e.b)
                edges.append([myidx_a, myidx_b])

            if (poly.edge_count < 3):
                # can't triangulate less than 3 points
                continue

            # this doesn't work and it drives me abs.
            # crazy. but whatever it works if you don't do this
            # if(poly.edge_count == 3):
            #     d["triangles_poly"].append([[0, 1, 2]])
            #     d["triangles_verts_poly"].append(verts)
            #     continue

            if (poly.edge_count >= 3):
                points = [np.array([
                    self.points[x].x,
                    self.points[x].y,
                    self.points[x].z]) for x in verts]

                # grab two points from this poly to calculate the
                # plane this face is in
                p0 = np.array(points[0])
                p1 = np.array(points[1])
                p = [p0 - p1]
                u = p / np.linalg.norm(p)
                v = np.cross(u, normal)

                # "flatten" the 3d points into 2d points in the
                # plane of this face

                def flatten_3to2(vec3):
                    return np.array([np.dot(vec3 - p0, u[0]), np.dot(vec3 - p0, v[0])])
                # 2d face points
                face_points = np.array([flatten_3to2(x) for x in points])

                # create input for triangle library
                the_dict = dict(vertices=face_points, segments=edges)
                # do triangulation in planar straignt line graph mode
                result = triangle.triangulate(the_dict, 'p')

                if "triangles" not in result:
                    # the triangulator didn't work for some reason
                    # this is usually because there was just terrible
                    # shape data that made no sense
                    continue

                vert_diff = len(result["vertices"]) - len(the_dict["vertices"])
                if vert_diff > 0:
                    # we need to add more vertices to this shape, this can
                    # happen with self-intersecting polygons, which were
                    # A-OK in avara

                    for vert in result["vertices"]:
                        if vert in the_dict["vertices"]:
                            continue
                        # turn 2d point back into 3d point with previous constants
                        new_vert_3 = p0 + (vert[0] * u) + (vert[1] * v)
                        new_vert_3 = np.append(new_vert_3, [[1]])
                        new_vert_3 = new_vert_3.tolist()
                        # add to shape points
                        d["points"].append(new_vert_3)
                        # add index to this face
                        verts.append(len(d["points"]) - 1)

                # we'll use this to count how many shape edges we hit casting a
                # ray in an arbitrary direction from the center point of each triangle.
                # this allows us to remove triangles that are part of a "hole" in the
                # geometry
                def line_ray_intersection_point(rayOrigin, rayDirection, point1, point2):
                    v1 = rayOrigin - point1
                    v2 = point2 - point1
                    v3 = np.array([-rayDirection[1], rayDirection[0]])
                    divisor = np.dot(v2, v3)
                    # Sometimes divisor can be zero, infinity will work but
                    # numpy gives a warning so we just put a big number instead
                    t1 = (np.cross(v2, v1) / divisor) if divisor != 0 else 1000000
                    t2 = (np.dot(v1, v3) / divisor) if divisor != 0 else 1000000
                    if t1 >= 0.0 and t2 >= 0.0 and t2 <= 1.0:
                        return [rayOrigin + t1 * rayDirection]
                    return []

                # calculate which triangles to remove
                triangles_to_remove = []
                for tidx, t in enumerate(result["triangles"]):
                    tpoints = np.array([result["vertices"][x] for x in t])
                    tavg = np.mean(tpoints, axis=0)
                    total_intersects = 0
                    for edge in edges:
                        # check ray from center of all points
                        # out to an arbitrary +xy for each edge.
                        # this is in hopes we don't accidentally
                        # hit any points straight on, otherwise this
                        # won't work right.
                        intersects = line_ray_intersection_point(
                            tavg,
                            np.array([10, .1]), # very random positive xy ray
                            face_points[edge[0]],
                            face_points[edge[1]])
                        num_intersects = len(intersects)
                        if num_intersects == 0:
                            # ray doesn't intersect
                            continue
                        if num_intersects > 0:
                            total_intersects += num_intersects
                    # if we intersected an EVEN number of edges, remove that triangle
                    if total_intersects % 2 == 0:
                        # print("removing triangle inside hole: %d" % tidx)
                        triangles_to_remove.append(tidx)

                # remove hole triangles
                if len(triangles_to_remove) > 0:
                    result["triangles"] = np.delete(result["triangles"], triangles_to_remove, axis=0)

                new_tris = result["triangles"].tolist()

                if vert_diff > 0 and DEBUG_ADDED_VERTS:
                    the_dict["triangles"] = new_tris
                    the_dict["vertices"] = np.array([flatten_3to2(np.array(x[:3])) for x in d["points"]])
                    # shows a nice plot of the before and after
                    # you can throw the below lines pretty much anywhere to
                    # nonintrusively observe a graph of what is happening
                    triangle.compare(plt, the_dict, result)
                    plt.show()

                # add triangles and indexes to dictionary for use in avara_format
                d["triangles_poly"].append(new_tris)
                d["triangles_verts_poly"].append(verts)
        return d

    def avara_format(self):
        d = self.triangulate(int_colors=True)
        out = {
            'points': [p[:3] for p in d['points']],
            'bounds': {
                'min': d['min_bounds'][:3],
                'max': d['max_bounds'][:3],
            },
            'center': d['enclosure_point'][:3],
            'radius1': d['enclosure_point'][3],
            'radius2': d['enclosure_radius'],
            'polys': [],
        }
        try:
            for idx, (fe, ec, normal_idx, fp, bp, pvis, rs) in enumerate(d['polys']):
                # basepoint is not used, we are sending triangles instead of polygons.
                # nvis is the normal record visibility flag, which i can't determine
                # any use of
                vec_idx, basept, color_idx, nvis = d['normals'][normal_idx]

                # get values out from all the indexes
                normal = d['vectors'][vec_idx][:3]
                color = d['colors'][color_idx]

                # these were set in the triangulation step
                tris = d['triangles_poly'][idx]
                tri_points = d['triangles_verts_poly'][idx]

                # in avara, each poly has a flag to determine if the front,
                # back, or both sides should be drawn. in OpenGL, we use
                # back face culling with CCW winding.
                #
                # This means we both need to wind in the correct direction
                # to respect the front/back flag, but also duplicate faces
                # that need to be seen from both sides. The back face has
                # both its normal and triangles reversed.
                #
                # this first poly is the
                # 'front' face.
                the_poly = {
                    'normal': [x for x in normal],
                    'color': color,
                    'front': fp, # index of the poly in front of this one
                    'back': bp, # index of the poly in back of this one
                    'tris': [[tri_points[i] for i in t] for t in tris]
                }
                # check the poly visibility flags
                if pvis == 1 or pvis == 3:
                    # front or both
                    out['polys'].append(the_poly)

                if pvis == 2 or pvis == 3:
                    # back or both
                    # reverse triangles of front poly and reverse normal
                    reverse = the_poly.copy()
                    reverse['normal'] = [-x for x in normal]
                    reverse['tris'] = [[tri_points[i] for i in t][::-1] for t in tris]
                    out['polys'].append(reverse)


        except IndexError:
            raise
        return json.dumps(out, indent=2, sort_keys=True)


def serialize_list(the_list):
    return [x.serialize() for x in the_list]

def bsp2json(data):
    bsp = BSP(data)
    return bsp.avara_format()


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: bspt.py <BSPT.r>")
        sys.exit(1)
    else:
        with open(sys.argv[1], "rb") as input_file:
            print(BSP(input_file.read()).avara_format())
