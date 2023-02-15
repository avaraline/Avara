/*
    Copyright ©1993-1996, Juri Munkki
    All rights reserved.

    File: Parser.h
    Created: Saturday, February 13, 1993, 13:20
    Modified: Wednesday, September 11, 1996, 00:22
*/

#pragma once
#include "ARGBColor.h"
#include "Types.h"

#ifdef __has_include
#  if __has_include(<optional>)                // Check for a standard library
#    include <optional>
#  elif __has_include(<experimental/optional>) // Check for an experimental version
#    include <experimental/optional>           // Check if __has_include is present
#  else                                        // Not found at all
#     error "Missing <optional>"
#  endif
#endif

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

// retrieve a variable by index as defined in LevelLoader.h or with the entry string
// Example: ReadLongVariable(iGrenades) OR ReadLongVariable("grenades")
short IndexForEntry(const char* entry);
double ReadVariable(short index);
double ReadVariable(const char *);
double ReadDoubleVar(const char *);
Fixed ReadFixedVar(short index);
Fixed ReadFixedVar(const char *);
long ReadLongVar(short index);
long ReadLongVar(const char *);
short ReadShortVar(short index);
short ReadShortVar(const char *s);
const std::optional<ARGBColor> ReadColorVar(short index);
const std::optional<ARGBColor> ReadColorVar(const char *);
std::string ReadStringVar(short index);
std::string ReadStringVar(const char *);

void ProgramVariable(short index, double value);
void ProgramFixedVar(short index, Fixed value);
void ProgramLongVar(short index, long value);
void ProgramReference(short index, short ref);
void ProgramOffsetAdd(short index, short ref, long addValue);
void ProgramOffsetMultiply(short index, short ref, long addValue);
void ProgramMessage(short index, long value);
