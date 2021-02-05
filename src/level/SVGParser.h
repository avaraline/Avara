#pragma once

#include "Types.h"
#include <string>
#include <vector>
using std::vector;
using std::string;

typedef void SVGRectProc(Rect *r, int radius, unsigned short thiccness);
typedef void SVGColorProc(unsigned short r, unsigned short g, unsigned short b, bool fg);
typedef void SVGTextProc(unsigned char *script);
typedef void SVGArcProc(float x, float y, short start, short angle, long longest_radius);
typedef void SVGEllipseProc(float x, float y, long longest_radius);

struct SVGProcs {
	SVGRectProc *rectProc;
	SVGColorProc *colorProc;
	SVGTextProc *textProc;
	SVGArcProc *arcProc;
	SVGEllipseProc *ellipseProc;
};

class SVGParser {
public:
	SVGProcs callbacks;
	Rect lastRect, lastOval, lastArc, lastRRect;

	SVGParser();
	virtual ~SVGParser() {}

	void Parse(std::string path);
protected:
	static vector<string> split (const string &s, char delim);
};