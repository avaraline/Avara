#include "OpenGLVertices.h"

#ifdef _WIN32
#include <Windows.h>
#endif
#include <glad/glad.h>

#include <algorithm>
#include <math.h>

struct Tri {
    GLData v1;
    GLData v2;
    GLData v3;
    float d;
};

inline float distSq(const GLData &g1, const GLData &g2) {
  return
    pow(g1.x - g2.x, 2) +
    pow(g1.y - g2.y, 2) +
    pow(g1.z - g2.z, 2);
}

void GLVertexCollection::SortFromCamera(float x, float y, float z)
{
    std::vector<Tri> newTris = {};
    newTris.reserve(glData.size() / 3);

    GLData camera = GLData(x, y, z);

    for (int i = 0; i < glData.size(); i += 3) {
        // put the center halfway up the longest edge of the triangle to emulate
        // calculating the circumcenter of a triangle.  For a right triangle this WILL be
        // the circumcenter, for other triangles, it's not quite but it seems good for sorting

        GLData longestEdge;
        float longestDist = -1;

        // tests whether an edge is the longest and, if so, keeps its midpoint
        auto testLongestEdge = [&](const GLData &endPoint1, const GLData &endPoint2) {
            float testDist = distSq(endPoint1, endPoint2);
            if (testDist > longestDist) {
                // save the center of the longest edge
                longestEdge.x = (endPoint1.x + endPoint2.x) / 2.0;
                longestEdge.y = (endPoint1.y + endPoint2.y) / 2.0;
                longestEdge.z = (endPoint1.z + endPoint2.z) / 2.0;
                longestDist = testDist;
            }
        };

        testLongestEdge(glData[i], glData[i+2]);    // often the longest
        testLongestEdge(glData[i+2], glData[i+1]);
        testLongestEdge(glData[i+1], glData[i]);

        // now compute distance from camera to the longest edge of the triangle
        float delta = distSq(longestEdge, camera);

        // Copy data to triangle.
        Tri t = Tri();
        t.v1 = glData[i];
        t.v2 = glData[i + 1];
        t.v3 = glData[i + 2];
        t.d = delta;
        newTris.push_back(t);
    }

    std::sort(newTris.begin(), newTris.end(), [](const Tri &a, const Tri &b) {
        return a.d > b.d;
    });

    glData.clear();
    for (auto &t : newTris) {
        glData.push_back(t.v1);
        glData.push_back(t.v2);
        glData.push_back(t.v3);
    }
}

OpenGLVertices::OpenGLVertices()
{
    opaque = GLVertexCollection();
    alpha = GLVertexCollection();
    glGenBuffers(1, &opaque.vertexBuffer);
    glGenVertexArrays(1, &opaque.vertexArray);
    glGenBuffers(1, &alpha.vertexBuffer);
    glGenVertexArrays(1, &alpha.vertexArray);
}

OpenGLVertices::~OpenGLVertices()
{
    if (opaque.glDataSize > 0) {
        glDeleteBuffers(1, &opaque.vertexBuffer);
        glDeleteVertexArrays(1, &opaque.vertexArray);
    }
    if (alpha.glDataSize > 0) {
        glDeleteBuffers(1, &alpha.vertexBuffer);
        glDeleteVertexArrays(1, &alpha.vertexArray);
    }
}

void OpenGLVertices::Append(const CBSPPart &part)
{
    ARGBColor color;
    uint8_t vis;
    int tris, points;

    // up front, calculate how many points we *actually* need
    // for faces that should be visible on both sides, double
    // the faces, triangles, and points to be reversed for the
    // back side
    int tmpOpaquePointCount = 0;
    int tmpAlphaPointCount = 0;
    for (auto &poly : part.polyTable)
    {
        color = part.colorTable[poly.colorIdx].current;
        vis = (part.HasAlpha() && poly.vis != 0) ? 3 : poly.vis;
        if (!vis) vis = 0;
        switch (vis) {
            case 0:
                tris = 0;
                break;
            case 3:
                tris = poly.triCount * 2;
                break;
            default:
                tris = poly.triCount;
                break;
        }
        points = tris * 3;
        if (color.HasAlpha()) {
            alpha.pointCount += points;
            tmpAlphaPointCount += points;
        } else {
            opaque.pointCount += points;
            tmpOpaquePointCount += points;
        }
    }

    opaque.glDataSize = opaque.glDataSize + (tmpOpaquePointCount * sizeof(GLData));
    opaque.glData.reserve(opaque.glData.size() + tmpOpaquePointCount);
    alpha.glDataSize = alpha.glDataSize + (tmpAlphaPointCount * sizeof(GLData));
    alpha.glData.reserve(alpha.glData.size() + tmpAlphaPointCount);

    // Count all the points we output so that we can make sure it matches what we just calculated.
    const FixedPoint *pt;
    int pOpaque = 0;
    int pAlpha = 0;
    for (auto &poly : part.polyTable) {
        color = part.colorTable[poly.colorIdx].current;
        uint8_t vis = (part.HasAlpha() && poly.vis != 0) ? 3 : poly.vis;
        if (!vis) vis = 0; // default to 0 (none) if vis is empty

        // vis can ONLY be 0 for None, 1 for Front, 2 for Back, or 3 for Both
        assert (vis == 0 || vis == 1 || vis == 2 || vis == 3);

        // Wrap forwards (front side).
        if (vis == 1 || vis == 3) {
            for (int v = 0; v < poly.triCount * 3; v++) {
                pt = &part.pointTable[poly.triPoints[v]];
                GLData vertex = GLData();
                vertex.x = ToFloat(pt->x);
                vertex.y = ToFloat(pt->y);
                vertex.z = ToFloat(pt->z);
                color.ExportGLFloats(&vertex.r, 4);
                vertex.nx = poly.normal.x;
                vertex.ny = poly.normal.y;
                vertex.nz = poly.normal.z;
                if (color.HasAlpha()) {
                    alpha.glData.push_back(vertex);
                    pAlpha++;
                } else {
                    opaque.glData.push_back(vertex);
                    pOpaque++;
                }
            }
        }

        // Wrap backwards (back side).
        if (vis == 2 || vis == 3) {
            for (int v = (poly.triCount * 3) - 1; v >= 0; v--) {
                pt = &part.pointTable[poly.triPoints[v]];
                GLData vertex = GLData();
                vertex.x = ToFloat(pt->x);
                vertex.y = ToFloat(pt->y);
                vertex.z = ToFloat(pt->z);
                color.ExportGLFloats(&vertex.r, 4);
                vertex.nx = -poly.normal.x;
                vertex.ny = -poly.normal.y;
                vertex.nz = -poly.normal.z;
                if (color.HasAlpha()) {
                    alpha.glData.push_back(vertex);
                    pAlpha++;
                } else {
                    opaque.glData.push_back(vertex);
                    pOpaque++;
                }
            }
        }
    }

    // Make sure we filled in the arrays correctly.
    assert(pOpaque == tmpOpaquePointCount);
    assert(pAlpha == tmpAlphaPointCount);
    
    // Upload to GPU.
    GLint currBuffer;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, opaque.vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, opaque.glDataSize, opaque.glData.data(), GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, alpha.vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, alpha.glDataSize, alpha.glData.data(), GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, currBuffer);
}

void OpenGLVertices::Replace(const CBSPPart &part)
{
    if (opaque.glDataSize > 0) {
        glDeleteBuffers(1, &opaque.vertexBuffer);
        glDeleteVertexArrays(1, &opaque.vertexArray);
    }
    opaque.glDataSize = 0;
    opaque.glData.clear();
    opaque.pointCount = 0;

    if (alpha.glDataSize > 0) {
        glDeleteBuffers(1, &alpha.vertexBuffer);
        glDeleteVertexArrays(1, &alpha.vertexArray);
    }
    alpha.glDataSize = 0;
    alpha.glData.clear();
    alpha.pointCount = 0;

    glGenBuffers(1, &opaque.vertexBuffer);
    glGenVertexArrays(1, &opaque.vertexArray);
    glGenBuffers(1, &alpha.vertexBuffer);
    glGenVertexArrays(1, &alpha.vertexArray);
    Append(part);
}
