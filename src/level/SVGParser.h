#pragma once

#include "Types.h"
#include <string>
#include <vector>
using std::vector;
using std::string;

typedef struct {
	Point pnLoc;
	Point pnSize;

	RGBColor fgColor;
	RGBColor bgColor;
} SVGContext;

typedef void SVGRectProc(SVGContext *context, Rect *r, int radius);
typedef void SVGColorProc(unsigned short r, unsigned short g, unsigned short b, bool fg);


struct SVGProcs {
	SVGRectProc *rectProc;
	SVGColorProc *colorProc;
};

class SVGParser {
public:
	SVGProcs callbacks;
	SVGContext context;
	Rect lastRect, lastOval, lastArc, lastRRect;

	SVGParser();
	virtual ~SVGParser() {}

	void Parse();
protected:
	static vector<string> split (const string &s, char delim);
};