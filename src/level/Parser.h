/*
    Copyright ©1993-1996, Juri Munkki
    All rights reserved.

    File: Parser.h
    Created: Saturday, February 13, 1993, 13:20
    Modified: Wednesday, September 11, 1996, 00:22
*/

#pragma once
#include "Types.h"

enum {
    kLexPlus,
    kLexMinus,
    kLexAbs,
    kLexMultiply,
    kLexDivide,
    kLexModulo,
    kLexPower,
    kLexLt,
    kLexGt,
    kLexEq,
    kLexLtEq,
    kLexGtEq,
    kLexStore,
    kLexMin,
    kLexMax,
    kLexOpenParen,
    kLexCloseParen,
    kLexRowOffset,
    kLexConstant,
    kUnaryMinus,
    kLexVariable,
    kLexFun0,
    kLexFun1,
    kLexEnum,
    kLexUnique,
    kLexEnd,
    kLexAdjust,
    kLexObject,
    kAssignment,
    kParseError,
    kLexEof
};

enum {
    kMinKey,
    kMaxKey,
    kRandomKey,
    kSinKey,
    kCosKey,
    kIntKey,
    kRoundKey,
    kEnumKey,
    kUniqueKey,
    kEndKey,
    kAdjustKey,
    kObjectKey,
    firstVariable
};

typedef struct {
    short kind; //	Determines what this lexical symbol is.

    union {
        double floating; //	Used for numeric constants.
        long offset; //	Used as offset into datacube.
        long token; //	Token for variable
        long level; //	Level number from level statement
    } value;
} LexSymbol;

typedef struct {
    StringPtr input;
    Handle output;
    long realSize;
    long logicalSize;

    LexSymbol lookahead;

    short stackDepth;
    short stackMax;
} ParserVariables;

void CreateVariable(long token);
void ParseCompare();
void ParseExpr();
void LexRead(LexSymbol *theSymbol);
void EmitInstruction(LexSymbol *theOperation, short stackChange);
void LoadLevel(short whichLevel);
double EvalVariable(long token, Boolean forceCalc);
void WriteVariable(long token, double value);

void RunThis(StringPtr theScript);
void AllocParser();
void DeallocParser();
void InitParser();
void FreshCalc();
