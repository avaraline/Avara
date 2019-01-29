from Converter.helpers import *
from Converter.bspt.datatypes import *
from itertools import zip_longest
import json

class BSP(object):
    # https://github.com/jmunkki/Avara/blob/master/src/Libraries/BSP/BSPResStructures.h
    def __init__(self, bsp_dict):
        self.res_id = ""
        self.name = bsp_dict['name']
        raw_data = bsp_dict['data']

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
        d["name"] = self.name
        d["res_id"] = self.res_id
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

        try:
            import triangle
            # import triangle.plot
            # import matplotlib.pyplot as plt
            import numpy as np
        except ImportError:
            print("Triangle library or numpy not found, will not output triangulations")
            return d

        d["triangles_poly"] = list()
        d["triangles_verts_poly"] = list()
        for poly in self.polys:
            # print(poly)
            normal_rec = self.normals[poly.normal_index]
            normal_idx = normal_rec.normal_index
            normal = np.array(self.vectors[normal_idx].as_list_3())
            verts = []
            edges = []
            # last = first + poly.edge_count
            for idx in range(0, poly.edge_count):
                e = self.unique_edges[self.edges[poly.first_edge + idx]]

                if e.a not in verts:
                    verts.append(e.a)
                if e.b not in verts:
                    verts.append(e.b)

                myidx_a = verts.index(e.a)
                myidx_b = verts.index(e.b)

                edges.append([myidx_a, myidx_b])

            points = [np.array([
                self.points[x].x,
                self.points[x].y,
                self.points[x].z]) for x in verts]

            p0 = points[0]
            p1 = points[1]
            u = np.linalg.norm([p0 - p1], axis=0)
            v = np.cross(u, normal)  # u.cross(normal)

            def flatten_3to2(vec3):
                return np.array([
                    np.dot(vec3 - p0, u),
                    np.dot(vec3 - p0, v)
                ])

            centroid3 = np.mean(points, axis=0)

            centroid2 = flatten_3to2(centroid3)

            face_points = np.array([flatten_3to2(x) for x in points])

            if (self.name == "Subway"):
                # uhh yeah.
                # this is for a shape
                # that crashed the triangulator
                continue



            the_dict = dict(vertices=face_points, segments=edges)
            result = triangle.triangulate(the_dict, 'p')
            the_triangles = result["triangles"]

            if not len(the_dict["vertices"]) == len(result["vertices"]):
                print("We need to add some vertices to this shape.")
                print("This can happen because of self-intersecting polygons.")
                for zthing in zip_longest(face_points, result["vertices"]):
                    if zthing[0] is None:
                        # the triangulator added points
                        # probably because of a  
                        # self-intersecting polygon 
                        new_vert = zthing[1]
                        new_vert_3 = new_vert[0] * u + new_vert[1] * v + p0
                        new_vert_3 = np.append(new_vert_3, [[1]])
                        print("vert %s added" % new_vert_3)
                        # put the point at the end of the list
                        d["points"].append(new_vert_3.tolist())
                        # add a pointer to this new point to the
                        # end of this list of verts
                        verts.append(len(d["points"]) - 1)
                        # TODO: Fix the above

            # we'll use this to count how many shape edges we hit casting a
            # ray in an arbitrary direction from the center point of each triangle.
            # this allows us to remove triangles that are part of a "hole" in the 
            # geometry
            def line_ray_intersection_point(rayOrigin, rayDirection, point1, point2):
                # rayDirection = np.linalg.norm(rayDirection, axis=0)
                v1 = rayOrigin - point1
                v2 = point2 - point1
                v3 = np.array([-rayDirection[1], rayDirection[0]])
                t1 = np.cross(v2, v1) / np.dot(v2, v3)
                t2 = np.dot(v1, v3) / np.dot(v2, v3)
                if t1 >= 0.0 and t2 >= 0.0 and t2 <= 1.0:
                    return [rayOrigin + t1 * rayDirection]
                return []

            triangles_to_remove = []
            for tidx, t in enumerate(the_triangles):
                tpoints = np.array([result["vertices"][x] for x in t])
                tavg = np.mean(tpoints, axis=0)
                total_intersects = 0
                for edge in edges:
                    intersects = line_ray_intersection_point(
                        tavg,
                        np.array([1, 0]), # positive x ray
                        face_points[edge[0]],
                        face_points[edge[1]])
                    num_intersects = len(intersects)
                    if num_intersects == 0:
                        # doesn't intersect
                        continue
                    if num_intersects > 0:
                        total_intersects += num_intersects
                # if we hit an EVEN number of edges, remove that triangle
                if total_intersects % 2 == 0:
                    print("removing triangle inside hole: %d" % tidx)
                    triangles_to_remove.append(tidx)

            # remove hole triangles
            result["triangles"] = np.delete(result["triangles"], triangles_to_remove, axis=0)

            # below used for debug, shows a face before and after triangularization
            # if (self.name == "MComplete"): # or (self.name == "SmartHairs"):
            #     triangle.plot.compare(plt, the_dict, result)
            #     plt.show()
            d["triangles_poly"].append(result["triangles"].tolist())
            d["triangles_verts_poly"].append(verts)

        return d


def serialize_list(the_list):
    return [x.serialize() for x in the_list]



def parse(resource):
    bsps = []
    for res_id in resource.keys():
        bsp = BSP(resource[res_id])
        bsp.res_id = res_id
        bsps.append(bsp)
    return bsps


def bsp2json(data):
    bsp = BSP({'data': data, 'name': ''})
    d = bsp.serialize(int_colors=True)
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
    for idx, (fe, ec, normal_idx, fp, bp, pvis, rs) in enumerate(d['polys']):
        vec_idx, basept, color_idx, nvis = d['normals'][normal_idx]
        normal = d['vectors'][vec_idx][:3]
        color = d['colors'][color_idx]
        tris = d['triangles_poly'][idx]
        tri_points = d['triangles_verts_poly'][idx]
        out['polys'].append({
            'normal': normal,
            'color': color,
            'tris': [[tri_points[i] for i in t] for t in tris],
        })
    return json.dumps(out, indent=2, sort_keys=True)
