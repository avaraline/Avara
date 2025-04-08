#include "CCompoundShape.h"

CCompoundShape::CCompoundShape() : CBSPPart()
{

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
        // TODO: Apply transforms to points to convert to world space.
        pointTable.push_back(point);
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
