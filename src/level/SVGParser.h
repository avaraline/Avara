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

typedef void SVGRectProc(Rect *r, int radius, unsigned short thiccness);
typedef void SVGColorProc(unsigned short r, unsigned short g, unsigned short b, bool fg);
typedef void SVGTextProc(unsigned char *script);
typedef void SVGArcProc(long x, long y, short start, short angle, long longest_radius);


struct SVGProcs {
	SVGRectProc *rectProc;
	SVGColorProc *colorProc;
	SVGTextProc *textProc;
	SVGArcProc *arcProc;
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