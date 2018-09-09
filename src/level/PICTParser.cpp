#include "PICTParser.h"

#include "CDataBuffer.h"
#include "Memory.h"

#include <SDL2/SDL.h>

PICTParser::PICTParser() {
    context.pnLoc.v = 0;
    context.pnLoc.h = 0;
    context.pnSize.v = 1;
    context.pnSize.h = 1;
    context.ovSize.v = 0;
    context.ovSize.h = 0;
    context.fgColor.red = 0;
    context.fgColor.green = 0;
    context.fgColor.blue = 0;
    context.bgColor.red = 0;
    context.bgColor.green = 0;
    context.bgColor.blue = 0;

    callbacks.arcProc = NULL;
    callbacks.rRectProc = NULL;
    callbacks.rectProc = NULL;
    callbacks.textProc = NULL;
    callbacks.ovalProc = NULL;
    callbacks.commentProc = NULL;
}

void PICTParser::CopyAdjustRect(Rect *src, Rect *dst) {
    dst->top = src->top - context.pnLoc.v;
    dst->left = src->left - context.pnLoc.h;
    dst->bottom = src->bottom - context.pnLoc.v;
    dst->right = src->right - context.pnLoc.h;
}

void PICTParser::Parse(Handle data) {
    CDataBuffer *buf = new CDataBuffer(data);
    Rect frame;
    Point pt;
    short size, start, angle, kind;
    Boolean doneDrawing = false;

    size = buf->Short();
    buf->ReadRect(&frame);
    // context.pnLoc.v = frame.top;
    // context.pnLoc.h = frame.left;
    while (!doneDrawing && buf->More()) {
        short opcode = buf->Short();
        switch (opcode) {
            case 0x0: // NOOP
                break;
            case 0x1: // ClipRegion
                size = buf->Short();
                buf->ReadRect(&frame);
                buf->Skip(size - 10);
                break;
            case 0x3: // TextFont
                buf->Skip(2);
                break;
            case 0x4: // TextFace
                buf->Skip(1);
                break;
            case 0x7: // PenSize
                buf->ReadPoint(&context.pnSize);
                break;
            case 0x8: // PenMode
                buf->Skip(2);
                break;
            case 0x9: // PenPattern
                buf->Skip(8);
                break;
            case 0xb: // OvalSize
                buf->ReadPoint(&context.ovSize);
                break;
            case 0xc: // Origin
                context.pnLoc.h += buf->Short();
                context.pnLoc.v += buf->Short();
                break;
            case 0xd: // TextSize
                buf->Skip(2);
                break;
            case 0x10: // TextRatio
                buf->Skip(8);
                break;
            case 0x11: // Version
                break;
            case 0x15: // PenLocationHFractional
                buf->Skip(2);
                break;
            case 0x1a: // RGBForegroundColor
                buf->ReadColor(&context.fgColor);
                break;
            case 0x1b: // RGBBackgroundColor
                buf->ReadColor(&context.bgColor);
                break;
            case 0x1e: // DefaultHighlight
                break;
            case 0x1f: // OpColor
                buf->Skip(6);
                break;
            case 0x22: // ShortLine
                buf->Skip(6);
                break;
            case 0x23: // ShortLineFrom
                buf->Skip(2);
                break;
            case 0x28: // LongText
                buf->ReadPoint(&pt);
                size = buf->UByte();
                if (callbacks.textProc) {
                    Ptr data = NewPtr(size);
                    buf->Read(data, size);
                    callbacks.textProc(&context, size, data, NULL, NULL);
                    DisposePtr(data);
                } else {
                    buf->Skip(size);
                }
                break;
            case 0x29: // DHText
                buf->Skip(1); // dh
                size = buf->UByte();
                if (callbacks.textProc) {
                    Ptr data = NewPtr(size);
                    buf->Read(data, size);
                    callbacks.textProc(&context, size, data, NULL, NULL);
                    DisposePtr(data);
                } else {
                    buf->Skip(size);
                }
                break;
            case 0x2a: // DVText
                buf->Skip(1); // dv
                size = buf->UByte();
                if (callbacks.textProc) {
                    Ptr data = NewPtr(size);
                    buf->Read(data, size);
                    callbacks.textProc(&context, size, data, NULL, NULL);
                    DisposePtr(data);
                } else {
                    buf->Skip(size);
                }
                break;
            case 0x2b: // DHDVText
                buf->Skip(2); // dh, dv
                size = buf->UByte();
                if (callbacks.textProc) {
                    Ptr data = NewPtr(size);
                    buf->Read(data, size);
                    callbacks.textProc(&context, size, data, NULL, NULL);
                    DisposePtr(data);
                } else {
                    buf->Skip(size);
                }
                break;
            case 0x2c: // VariableReserved
                size = buf->Short();
                buf->Skip(size);
                break;
            case 0x2e: // VariableReserved
                size = buf->Short();
                buf->Skip(size);
                break;
            case 0x30: // FrameRectangle
                buf->ReadRect(&lastRect);
                CopyAdjustRect(&lastRect, &frame);
                if (callbacks.rectProc) {
                    callbacks.rectProc(&context, kQDGrafVerbFrame, &frame);
                }
                break;
            case 0x31: // PaintRectangle
                buf->ReadRect(&lastRect);
                CopyAdjustRect(&lastRect, &frame);
                if (callbacks.rectProc) {
                    callbacks.rectProc(&context, kQDGrafVerbPaint, &frame);
                }
                break;
            case 0x38: // FrameSameRectangle
                CopyAdjustRect(&lastRect, &frame);
                if (callbacks.rectProc) {
                    callbacks.rectProc(&context, kQDGrafVerbFrame, &frame);
                }
                break;
            case 0x39: // PaintSameRectangle
                CopyAdjustRect(&lastRect, &frame);
                if (callbacks.rectProc) {
                    callbacks.rectProc(&context, kQDGrafVerbPaint, &frame);
                }
                break;
            case 0x40: // FrameRoundedRectangle
                buf->ReadRect(&lastRRect);
                CopyAdjustRect(&lastRRect, &frame);
                if (callbacks.rRectProc) {
                    callbacks.rRectProc(&context, kQDGrafVerbFrame, &frame, context.pnSize.h, context.pnSize.v);
                }
                break;
            case 0x41: // PaintRoundedRectangle
                buf->ReadRect(&lastRRect);
                CopyAdjustRect(&lastRRect, &frame);
                if (callbacks.rRectProc) {
                    callbacks.rRectProc(&context, kQDGrafVerbPaint, &frame, context.pnSize.h, context.pnSize.v);
                }
                break;
            case 0x48: // FrameSameRoundedRectangle
                CopyAdjustRect(&lastRRect, &frame);
                if (callbacks.rRectProc) {
                    callbacks.rRectProc(&context, kQDGrafVerbFrame, &frame, context.pnSize.h, context.pnSize.v);
                }
                break;
            case 0x49: // PaintSameRoundedRectangle
                CopyAdjustRect(&lastRRect, &frame);
                if (callbacks.rRectProc) {
                    callbacks.rRectProc(&context, kQDGrafVerbPaint, &frame, context.pnSize.h, context.pnSize.v);
                }
                break;
            case 0x50: // FrameOval
                buf->ReadRect(&lastOval);
                CopyAdjustRect(&lastOval, &frame);
                if (callbacks.ovalProc) {
                    callbacks.ovalProc(&context, kQDGrafVerbFrame, &frame);
                }
                break;
            case 0x51: // PaintOval
                buf->ReadRect(&lastOval);
                CopyAdjustRect(&lastOval, &frame);
                if (callbacks.ovalProc) {
                    callbacks.ovalProc(&context, kQDGrafVerbPaint, &frame);
                }
                break;
            case 0x58: // FrameSameOval
                CopyAdjustRect(&lastOval, &frame);
                if (callbacks.ovalProc) {
                    callbacks.ovalProc(&context, kQDGrafVerbFrame, &frame);
                }
                break;
            case 0x59: // PaintSameOval
                CopyAdjustRect(&lastOval, &frame);
                if (callbacks.ovalProc) {
                    callbacks.ovalProc(&context, kQDGrafVerbPaint, &frame);
                }
                break;
            case 0x60: // FrameArc
                buf->ReadRect(&lastArc);
                CopyAdjustRect(&lastArc, &frame);
                start = buf->Short();
                angle = buf->Short();
                if (callbacks.arcProc) {
                    callbacks.arcProc(&context, kQDGrafVerbFrame, &frame, start, angle);
                }
                break;
            case 0x61: // PaintArc
                buf->ReadRect(&lastArc);
                CopyAdjustRect(&lastArc, &frame);
                start = buf->Short();
                angle = buf->Short();
                if (callbacks.arcProc) {
                    callbacks.arcProc(&context, kQDGrafVerbPaint, &frame, start, angle);
                }
                break;
            case 0x68: // FrameSameArc
                CopyAdjustRect(&lastArc, &frame);
                start = buf->Short();
                angle = buf->Short();
                if (callbacks.arcProc) {
                    callbacks.arcProc(&context, kQDGrafVerbFrame, &frame, start, angle);
                }
                break;
            case 0x69: // PaintSameArc
                CopyAdjustRect(&lastArc, &frame);
                start = buf->Short();
                angle = buf->Short();
                if (callbacks.arcProc) {
                    callbacks.arcProc(&context, kQDGrafVerbPaint, &frame, start, angle);
                }
                break;
            case 0x71: // PaintPoly
                // TODO: poly stuff?
                break;
            case 0x84: // FillRegion
                size = buf->Short();
                buf->ReadRect(&frame);
                buf->Skip(size - 10);
                break;
            case 0xa0: // ShortComment
                kind = buf->Short();
                if (callbacks.commentProc) {
                    callbacks.commentProc(&context, kind, 0, NULL);
                }
                break;
            case 0xa1: // LongComment
                kind = buf->Short();
                size = buf->Short();
                if (callbacks.commentProc) {
                    Handle data = NewHandle(size);
                    buf->Read(*data, size);
                    callbacks.commentProc(&context, kind, size, data);
                    DisposeHandle(data);
                } else {
                    buf->Skip(size);
                }
                break;
            case 0xff: // EndPict
                doneDrawing = true;
                break;
            case 0x02ff: // Version
                break;
            case 0x0c00: // Header
                buf->Skip(24);
                break;
            default:
                SDL_Log("UNKNOWN OPCODE: %x\n", opcode);
                break;
        }
        // Every PICT opcode data section is word-aligned, so if we end on an odd byte, skip one.
        buf->Align();
    }

    delete buf;
}
