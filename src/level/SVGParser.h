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

typedef void SVGRectProc(Rect *r, int radius);

struct SVGProcs {
	SVGRectProc *rectProc;
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
	vector<string> split (const string &s, char delim);
};