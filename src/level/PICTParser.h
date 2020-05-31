#pragma once

#include "Types.h"
#include "CDataBuffer.h"

typedef struct {
    Point pnLoc;
    Point pnSize;
    Point ovSize;
    RGBColor fgColor;
    RGBColor bgColor;
} PICTContext;

typedef void QDArcProc(PICTContext *context, GrafVerb verb, Rect *r, short startAngle, short arcAngle);
typedef void QDRectProc(PICTContext *context, GrafVerb verb, Rect *r);
typedef void QDOvalProc(PICTContext *context, GrafVerb verb, Rect *r);
typedef void QDRRectProc(PICTContext *context, GrafVerb verb, Rect *r, short ovalWidth, short ovalHeight);
typedef void QDCommentProc(PICTContext *context, short kind, short dataSize, Handle dataHandle);
typedef void QDTextProc(PICTContext *context, short byteCount, Ptr theText, Boolean prependNewline);

struct QDProcs {
    QDArcProc *arcProc;
    QDRRectProc *rRectProc;
    QDRectProc *rectProc;
    QDTextProc *textProc;
    QDOvalProc *ovalProc;
    QDCommentProc *commentProc;
};

class PICTParser {
public:
    QDProcs callbacks;
    PICTContext context;
    Rect lastRect, lastOval, lastArc, lastRRect;

    PICTParser();
    virtual ~PICTParser() {}

    void Parse(Handle data);

protected:
    void CopyAdjustRect(Rect *src, Rect *dst);
    void DirectBitsRgn(CDataBuffer *buf);
};
