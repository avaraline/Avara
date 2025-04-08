#include "CCompoundShape.h"

CCompoundShape::CCompoundShape() : CBSPPart()
{
    itsTransform[0][0] = invGlobTransform[0][0] = -FIX1;
    itsTransform[1][1] = invGlobTransform[1][1] = FIX1;
    itsTransform[2][2] = invGlobTransform[2][2] = -FIX1;
    itsTransform[3][3] = invGlobTransform[3][3] = FIX1;
}

CCompoundShape::~CCompoundShape()
{
    
}

void CCompoundShape::Append(CBSPPart &part)
{
    size_t startingColorCount = colorTable.size();
    size_t startingPointCount = pointTable.size();
    
    for (ColorRecord &color : part.colorTable) {
        colorTable.push_back(color);
    }
    
    for (FixedPoint &point : part.pointTable) {
        Vector p;
        p[0] = point.x;
        p[1] = point.y;
        p[2] = point.z;
        p[3] = point.w;
        
        Vector dest;
        VectorMatrixProduct(1, &p, &dest, &part.itsTransform);
        FixedPoint fp = FixedPoint(dest[0], dest[1], dest[2], dest[3]);
        pointTable.push_back(fp);
    }
    
    for (PolyRecord &poly : part.polyTable) {
        // Construct a new polygon, correcting color and point indices.
        PolyRecord newPoly = PolyRecord();
        newPoly.normal = FloatNormal(poly.normal);
        newPoly.triCount = poly.triCount;
        newPoly.triPoints = std::make_unique<uint16_t[]>(poly.triCount * 3);
        
        for (uint16_t i = 0; i < poly.triCount * 3; i++) {
            newPoly.triPoints[i] = poly.triPoints[i] + startingPointCount;
        }
        
        newPoly.front = poly.front;
        newPoly.back = poly.back;
        newPoly.colorIdx = poly.colorIdx + startingColorCount;
        newPoly.vis = poly.vis;
        
        polyTable.push_back(std::move(newPoly));
    }
    
    hasAlpha = hasAlpha || part.HasAlpha();
}

void CCompoundShape::Reserve(CBSPWorldImpl &world)
{
    size_t colorCount = 0;
    size_t pointCount = 0;
    size_t polyCount = 0;
    
    CBSPPart *part;
    uint16_t partCount = world.GetPartCount();
    for (uint16_t i = 0; i < partCount; i++) {
        part = world.GetIndPart(i);
        colorCount += part->colorTable.size();
        pointCount += part->pointTable.size();
        polyCount += part->polyTable.size();
    }
    
    colorTable.reserve(colorCount);
    pointTable.reserve(pointCount);
    polyTable.reserve(polyCount);
}
