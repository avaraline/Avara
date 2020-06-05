/*
    Copyright ©1992-1999, Juri Munkki
    All rights reserved.

    File: LevelLoader.c
    Created: Saturday, December 5, 1992, 13:29
    Modified: Tuesday, October 19, 1999, 00:59
*/

#include "LevelLoader.h"
#include "AvaraGL.h"
#include "CAvaraGame.h"
#include "CWallActor.h"
#include "FastMat.h"
#include "Memory.h"
#include "PICTParser.h"
//#include "SVGParser.h"
#include "Parser.h"
#include "Resource.h"

#include <SDL2/SDL.h>

#define textBufferSize 4096

#define POINTTOUNIT(pt) (pt * 20480 / 9)

typedef struct {
    long v;
    long h;
} LongPoint;

static Ptr textBuffer;
static long textInBuffer;
static LongPoint lastArcPoint;
static short lastArcAngle;

static LongPoint lastOvalPoint;
static long lastOvalRadius;

Rect gLastBoxRect;
short gLastBoxRounding;

static Rect onePixelRect = {0, 0, 1, 1};
static Rect otherPixelRect = {0, 1, 1, 2};
static Rect bothPixelRect = {0, 0, 1, 2};

static LongPoint lastDomeCenter;
static short lastDomeAngle;
static short lastDomeSpan;
static long lastDomeRadius;

static RGBColor fillColor, frameColor;

static void TextBreak() {
    if (textInBuffer) {
        textBuffer[textInBuffer] = 0; //	null-terminate.
        RunThis((StringPtr)textBuffer);
        textInBuffer = 0;
    }
}

Fixed GetDome(Fixed *theLoc, Fixed *startAngle, Fixed *spanAngle) {
    theLoc[0] = POINTTOUNIT(lastDomeCenter.h);
    theLoc[2] = POINTTOUNIT(lastDomeCenter.v);
    theLoc[3] = 0;
    *startAngle = FDegToOne(((long)lastDomeAngle) << 16);
    *spanAngle = FDegToOne(((long)lastDomeSpan) << 16);

    return POINTTOUNIT(lastDomeRadius);
}

int GetPixelColor() {
    return ((((int)fillColor.red) << 8) & 0xFF0000) | (fillColor.green & 0xFF00) | (fillColor.blue >> 8);
}

int GetOtherPixelColor() {
    return ((((int)frameColor.red) << 8) & 0xFF0000) | (frameColor.green & 0xFF00) | (frameColor.blue >> 8);
}

void GetLastArcLocation(Fixed *theLoc) {
    theLoc[0] = POINTTOUNIT(lastArcPoint.h);
    theLoc[1] = 0;
    theLoc[2] = POINTTOUNIT(lastArcPoint.v);
    theLoc[3] = 0;
}

Fixed GetLastOval(Fixed *theLoc) {
    theLoc[0] = POINTTOUNIT(lastOvalPoint.h);
    theLoc[1] = 0;
    theLoc[2] = POINTTOUNIT(lastOvalPoint.v);
    theLoc[3] = 0;

    return POINTTOUNIT(lastOvalRadius);
}

Fixed GetLastArcDirection() {
    return FDegToOne(((long)lastArcAngle) << 16);
}
/*
static void SvgColor(unsigned short r, unsigned short g, unsigned short b, bool fg) {
    if (fg) {
        fillColor.red = r;
        fillColor.green = g;
        fillColor.blue = b;
    }
    else {
        frameColor.red = r;
        frameColor.green = g;
        frameColor.blue = b;
    }
}

static void SvgArc(float x, float y, short start, short angle, long largest_radius) {
    lastArcPoint.h = (long)roundf(x * 2);
    lastArcPoint.v = (long)roundf(y * 2);
    lastArcAngle = (630 - (start + angle / 2)) % 360;
    lastDomeCenter.h = (long)roundf(x * 2);
    lastDomeCenter.v = (long)roundf(y * 2);
    lastDomeAngle = 360 - start;
    lastDomeSpan = angle;
    lastDomeRadius = largest_radius;
}

static void SvgEllipse(float x, float y, long r) {
    lastOvalPoint.h = (long)roundf(x * 2);
    lastOvalPoint.v = (long)roundf(y * 2);
    lastOvalRadius = r * 2;

    lastDomeCenter.h = (long)roundf(x * 2);
    lastDomeCenter.v = (long)roundf(x * 2);

    lastDomeAngle = 0;
    lastDomeSpan = 360;
    lastDomeRadius = r * 2;
}

static void SvgRect(Rect *r, int radius, unsigned short thickness) {
    //SDL_Log("fillColor at time of rect: %d %d %d", fillColor.red, fillColor.blue, fillColor.green);
    //SDL_Log("frameColor at time of rect: %d %d %d", frameColor.red, frameColor.blue, frameColor.green);

    r->left += thickness >> 1;
    r->top += thickness >> 1;
    r->right -= (thickness + 1) >> 1;
    r->bottom -= (thickness + 1) >> 1;

    if(thickness == 1) {
        CWallActor *theWall;
        theWall = new CWallActor;
        theWall->IAbstractActor();
        theWall->MakeWallFromRect(r, (short)radius, 0, true);
    }
    else {
        gLastBoxRect = *r;
        gLastBoxRounding = radius;
    }
}

static void SvgText(unsigned char *script) {
    RunThis(script);
}
*/
static void PeepStdRRect(PICTContext *context, GrafVerb verb, Rect *r, short ovalWidth, short ovalHeight) {
    CWallActor *theWall;

    TextBreak();

    if (verb == kQDGrafVerbFrame) {
        short thickness;

        BlockMoveData(&context->fgColor, &frameColor, sizeof(RGBColor));
        // ClipRect(&bothPixelRect);
        // StdRect(paint, &otherPixelRect);

        r->left += (context->pnSize.h) >> 1;
        r->top += (context->pnSize.v) >> 1;
        r->right -= (context->pnSize.h + 1) >> 1;
        ;
        r->bottom -= (context->pnSize.v + 1) >> 1;

        thickness = context->pnSize.h;
        if (thickness < context->pnSize.v)
            thickness = context->pnSize.v;

        ovalHeight = context->ovSize.v;
        ovalWidth = context->ovSize.h;
        if (ovalHeight > ovalWidth)
            ovalHeight = ovalWidth;

        if (thickness == 1) {
            theWall = new CWallActor;
            theWall->IAbstractActor();
            theWall->MakeWallFromRect(r, ovalHeight, 0, true);
        } else {
            gLastBoxRect = *r;
            gLastBoxRounding = ovalHeight;
        }
    } else { // ClipRect(&bothPixelRect);
        // StdRect(verb, &onePixelRect);
        BlockMoveData(&context->fgColor, &fillColor, sizeof(RGBColor));
    }
}

static void PeepStdArc(PICTContext *context, GrafVerb verb, Rect *r, short startAngle, short arcAngle) {
    long r1, r2;

    TextBreak();

    // ClipRect(&bothPixelRect);
    // StdRect(verb, (verb == frame) ? &otherPixelRect : &onePixelRect);
    RGBColor *to = verb == kQDGrafVerbFrame ? &frameColor : &fillColor;
    BlockMoveData(&context->fgColor, to, sizeof(RGBColor));

    {
        r->left += (context->pnSize.h) >> 1;
        r->top += (context->pnSize.v) >> 1;
        r->right -= (context->pnSize.h + 1) >> 1;
        ;
        r->bottom -= (context->pnSize.v + 1) >> 1;

        r1 = r->right - r->left;
        r2 = r->bottom - r->top;
        if (r2 > r1)
            r1 = r2;

        lastArcPoint.h = r->left + r->right;
        lastArcPoint.v = r->top + r->bottom;
        lastArcAngle = (720 - (startAngle + arcAngle / 2)) % 360;

        lastDomeCenter.h = r->left + r->right;
        lastDomeCenter.v = r->top + r->bottom;
        lastDomeAngle = 360 - startAngle;
        lastDomeSpan = arcAngle;
        lastDomeRadius = r1;
    }
}

static void PeepStdOval(PICTContext *context, GrafVerb verb, Rect *r) {
    long r1, r2;

    TextBreak();

    // ClipRect(&bothPixelRect);
    // StdRect(verb, (verb == frame) ? &otherPixelRect : &onePixelRect);
    RGBColor *to = verb == kQDGrafVerbFrame ? &frameColor : &fillColor;
    BlockMoveData(&context->fgColor, to, sizeof(RGBColor));

    if (verb == kQDGrafVerbFrame) {
        r->right--;
        r->bottom--;
        r1 = r->right - r->left;
        r2 = r->bottom - r->top;

        if (r2 > r1)
            r1 = r2;

        lastOvalPoint.h = r->left + r->right;
        lastOvalPoint.v = r->top + r->bottom;
        lastOvalRadius = r1 + r1;

        lastDomeCenter.h = r->left + r->right;
        lastDomeCenter.v = r->top + r->bottom;

        lastDomeAngle = 0;
        lastDomeSpan = 360;
        lastDomeRadius = r1;
    }
}

static void PeepStdText(PICTContext *context, short byteCount, Ptr theText, Boolean prependNewline) {
    short i;
    char *p;
    static Point oldPoint;
    Point newPoint;

    // SDL_Log("PeepStdText(%d)\n", byteCount);

    if (byteCount + textInBuffer + 2 < textBufferSize) {
        p = textBuffer + textInBuffer;

        if (textInBuffer && p[-1] != 13 && prependNewline) {
            *p++ = 13;
            textInBuffer++;
        }

        /*
        TODO: not really sure why the position info matters here?
        GetPen(&newPoint);

        if(textInBuffer && p[-1] != 13)
        {	while(newPoint.v > oldPoint.v)
            {	FontInfo	fInfo;

                GetFontInfo(&fInfo);

                oldPoint.v += fInfo.ascent + fInfo.descent + fInfo.leading;
                *p++ = 13;
                textInBuffer++;
            }
        }
        oldPoint = newPoint;
        */
        textInBuffer += byteCount;

        i = byteCount;
        while (i--) {
            *p++ = *theText++;
        }
    }
}

static void PeepStdComment(PICTContext *context, short kind, short dataSize, Handle dataHandle) {
    // SDL_Log("PeepStdComment(%d,%d)\n", kind, dataSize);
    switch (kind) {
        case TextBegin:
            //		case StringBegin:
            textInBuffer = 0;
            break;

        case TextEnd:
            //		case StringEnd:
            textBuffer[textInBuffer] = 0; //	null-terminate.
            RunThis((StringPtr)textBuffer);
            textInBuffer = 0;
            break;
    }
}

static void PeepStdRect(PICTContext *context, GrafVerb verb, Rect *r) {
    CWallActor *theWall;

    TextBreak();

    if (verb == kQDGrafVerbFrame) {
        short thickness;

        // ClipRect(&bothPixelRect);
        // StdRect(paint, &otherPixelRect);
        BlockMoveData(&context->fgColor, &frameColor, sizeof(RGBColor));

        r->left += (context->pnSize.h) >> 1;
        r->top += (context->pnSize.v) >> 1;
        r->right -= (context->pnSize.h + 1) >> 1;
        ;
        r->bottom -= (context->pnSize.v + 1) >> 1;

        thickness = context->pnSize.h;
        if (thickness < context->pnSize.v)
            thickness = context->pnSize.v;

        if (thickness == 1) {
            theWall = new CWallActor;
            theWall->IAbstractActor();
            theWall->MakeWallFromRect(r, 0, 0, true);
        } else {
            gLastBoxRect = *r;
            gLastBoxRounding = 0;
        }
    } else { // ClipRect(&bothPixelRect);
        // StdRect(verb, &onePixelRect);
        BlockMoveData(&context->fgColor, &fillColor, sizeof(RGBColor));
    }
}
/*
void SVGConvertToLevelMap() {
    InitParser();
    SVGParser *parser = new SVGParser();
    parser->callbacks.rectProc = &SvgRect;
    parser->callbacks.colorProc = &SvgColor;
    parser->callbacks.textProc = &SvgText;
    parser->callbacks.arcProc = &SvgArc;
    parser->callbacks.ellipseProc = &SvgEllipse;

    parser->Parse();
    delete parser;
    TextBreak();
    FreshCalc();
    gCurrentGame->EndScript();
}
*/
void ConvertToLevelMap(Handle levelData) {
    InitParser();
    // TODO: Not a good place for this
    AvaraGLLightDefaults();

    textBuffer = NewPtr(textBufferSize);
    
    PICTParser *parser = new PICTParser();
    parser->callbacks.arcProc = &PeepStdArc;
    parser->callbacks.rRectProc = &PeepStdRRect;
    // parser->callbacks.polyProc = &PeepStdPoly;
    parser->callbacks.textProc = &PeepStdText;
    parser->callbacks.rectProc = &PeepStdRect;
    parser->callbacks.ovalProc = &PeepStdOval;
    parser->callbacks.commentProc = &PeepStdComment;
    // parser->callbacks.getPicProc = &PeekGetPic;
    parser->Parse(levelData);
    delete parser;
    
    /*
    TODO: replace this with a basic PICT parser with drawing function callbacks

    GetGWorld(&savedGrafPtr, &savedGD);
    NewGWorld(&utilGWorld, 32, &twoPixelRect, NULL, NULL, keepLocal);
    LockPixels(GetGWorldPixMap(utilGWorld));
    SetGWorld(utilGWorld, NULL);

    GetPort((GrafPtr *)&context);
    SetStdCProcs(&myQDProcs);
    ClipRect(&twoPixelRect);

    myQDProcs.arcProc = (void *)PeepStdArc;
    myQDProcs.rRectProc = (void *)PeepStdRRect;
    myQDProcs.polyProc = (void *)PeepStdPoly;
    myQDProcs.textProc = (void *)PeepStdText;
    myQDProcs.rectProc = (void *)PeepStdRect;
    myQDProcs.ovalProc = (void *)PeepStdOval;
    myQDProcs.commentProc = (void *)PeepStdComment;
    myQDProcs.getPicProc = (void *)PeekGetPic;
    context->grafProcs = (void *)&myQDProcs;

    state = HGetState((Handle) thePicture);
    HLock((Handle) thePicture);

    sPicDataPtr = (Ptr) (1+&((*thePicture)->picFrame));
    sPicCount = 0;

    DrawPicture(thePicture,&((*thePicture)->picFrame));
    */
    TextBreak();
    FreshCalc();
    gCurrentGame->EndScript();
}
