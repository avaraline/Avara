#include "OpenGLVertices.h"

#include <glad/glad.h>

OpenGLVertices::OpenGLVertices()
{
    glGenBuffers(1, &vertexBuffer);
    glGenVertexArrays(1, &vertexArray);
}

OpenGLVertices::~OpenGLVertices()
{
    if (glDataSize > 0) {
        glDeleteBuffers(1, &vertexBuffer);
        glDeleteVertexArrays(1, &vertexArray);
    }
}

void OpenGLVertices::Append(const CBSPPart &part)
{
    PolyRecord *poly;
    ARGBColor *color;
    pointCount = 0;
    uint8_t vis;
    int tris, points;

    // up front, calculate how many points we *actually* need
    // for faces that should be visible on both sides, double
    // the faces, triangles, and points to be reversed for the
    // back side
    for (size_t i = 0; i < part.polyCount; i++)
    {
        poly = &part.polyTable[i];
        color = &part.currColorTable[poly->colorIdx];
        vis = (part.HasAlpha() && poly->vis != 0) ? 3 : poly->vis;
        if (!vis) vis = 0;
        switch (vis) {
            case 0:
                tris = 0;
                break;
            case 3:
                tris = poly->triCount * 2;
                break;
            default:
                tris = poly->triCount;
                break;
        }
        points = tris * 3;
        pointCount += points;
    }

    glDataSize = pointCount * sizeof(GLData);
    glData.reserve(glData.size() + pointCount);

    // Count all the points we output so that we can make sure it matches what we just calculated.
    int p = 0;
    for (int i = 0; i < part.polyCount; i++) {
        poly = &part.polyTable[i];
        color = &part.currColorTable[poly->colorIdx];
        uint8_t vis = (part.HasAlpha() && poly->vis != 0) ? 3 : poly->vis;
        if (!vis) vis = 0; // default to 0 (none) if vis is empty

        // vis can ONLY be 0 for None, 1 for Front, 2 for Back, or 3 for Both
        assert (vis == 0 || vis == 1 || vis == 2 || vis == 3);

        // Wrap forwards (front side).
        if (vis == 1 || vis == 3) {
            for (int v = 0; v < poly->triCount * 3; v++) {
                Vector *pt = &part.pointTable[poly->triPoints[v]];
                GLData vertex = GLData();
                vertex.x = ToFloat((*pt)[0]);
                vertex.y = ToFloat((*pt)[1]);
                vertex.z = ToFloat((*pt)[2]);
                color->ExportGLFloats(&vertex.r, 4);
                vertex.nx = poly->normal[0];
                vertex.ny = poly->normal[1];
                vertex.nz = poly->normal[2];
                glData.push_back(std::move(vertex));
                p++;
            }
        }

        // Wrap backwards (back side).
        if (vis == 2 || vis == 3) {
            for (int v = (poly->triCount * 3) - 1; v >= 0; v--) {
                Vector *pt = &part.pointTable[poly->triPoints[v]];
                GLData vertex = GLData();
                vertex.x = ToFloat((*pt)[0]);
                vertex.y = ToFloat((*pt)[1]);
                vertex.z = ToFloat((*pt)[2]);
                color->ExportGLFloats(&vertex.r, 4);
                vertex.nx = -poly->normal[0];
                vertex.ny = -poly->normal[1];
                vertex.nz = -poly->normal[2];
                glData.push_back(std::move(vertex));
                p++;
            }
        }
    }

    // Make sure we filled in the array correctly.
    assert(p == pointCount);
}

void OpenGLVertices::Replace(const CBSPPart &part)
{
    if (glDataSize > 0) {
        glDeleteBuffers(1, &vertexBuffer);
        glDeleteVertexArrays(1, &vertexArray);
    }
    glDataSize = 0;
    glData.clear();

    glGenBuffers(1, &vertexBuffer);
    glGenVertexArrays(1, &vertexArray);
    Append(part);
}
