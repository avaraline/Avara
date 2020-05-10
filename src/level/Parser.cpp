/*
    Copyright ©1993-1996, Juri Munkki
    All rights reserved.

    File: Parser.c
    Created: Wednesday, February 10, 1993, 19:18
    Modified: Wednesday, September 11, 1996, 00:32
*/

#include "Parser.h"

#include "CAbstractActor.h"
#include "CStringDictionary.h"
#include "CTagBase.h"
#include "InternalVars.h"
#include "LevelLoader.h"
#include "LinkLoose.h"
#include "RamFiles.h"

#include <cstring>
#include <math.h>
#include <stdio.h>
//#include "CommandList.h"
#include "CApplication.h"
#include "FastMat.h"
#include "Resource.h"
#include "Types.h"

//#define DEBUGPARSER 1
#ifdef DEBUGPARSER
#include "stdio.h"
#define DEBUGPAR(a) \
    { a }
#else
#define DEBUGPAR(a) \
    {}
#endif

typedef short tokentype;

ParserVariables parserVar;
long lastKeyword;
long lastVariable;
static short currentLevel;
static CAbstractActor *currentActor = NULL;
static short uniqueBase;

typedef struct {
    double value;
    short calcLevel;
} variableValue;

Handle theScript = 0;
double *stackP = 0;
CStringDictionary *symTable = 0;
CTagBase *variableBase = 0;
CTagBase *programBase = 0;

void FreshCalc() {
    currentLevel++;
}

void CreateTheObject() {
    Str255 nameBuf;

    if (currentActor) {
        currentActor->Dispose();
    }

    symTable->GetIndEntry(parserVar.lookahead.value.token, nameBuf);

    currentActor = (CAbstractActor *)CreateNamedObject(nameBuf);
    if (currentActor) {
        currentActor->IAbstractActor();
        currentActor->BeginScript();
        currentLevel++;
    }
}

void CreateTheAdjuster() {
    Str255 nameBuf;

    if (currentActor) {
        currentActor->Dispose();
    }

    symTable->GetIndEntry(parserVar.lookahead.value.token, nameBuf);

    currentActor = (CAbstractActor *)CreateNamedObject(nameBuf);
    if (currentActor) {
        currentActor->IAbstractActor();
        currentActor->BeginScript();
        currentLevel++;
    }
}

// TODO: NO! BAD!
static unsigned char *pstr(const char *s) {
    size_t len = std::strlen(s);
    unsigned char *p = new unsigned char[len];
    p[0] = len;
    memmove(p + 1, s, len);
    return p;
}

void InitSymbols() {
    symTable = new CStringDictionary;
    symTable->IStringDictionary();

    symTable->AddDictEntry(pstr("min"), -1);
    symTable->AddDictEntry(pstr("max"), -1);
    symTable->AddDictEntry(pstr("random"), -1);
    symTable->AddDictEntry(pstr("sin"), -1);
    symTable->AddDictEntry(pstr("cos"), -1);
    symTable->AddDictEntry(pstr("int"), -1);
    symTable->AddDictEntry(pstr("round"), -1);
    symTable->AddDictEntry(pstr("enum"), -1);
    symTable->AddDictEntry(pstr("unique"), -1);
    symTable->AddDictEntry(pstr("end"), -1);
    symTable->AddDictEntry(pstr("adjust"), -1);
    lastKeyword = symTable->AddDictEntry(pstr("object"), -1);
    lastVariable = lastKeyword;

    variableBase = new CTagBase;
    variableBase->ITagBase();

    programBase = new CTagBase;
    programBase->ITagBase();

    CreateVariable(-1);
}

void ResetVariables() {
    variableBase->Dispose();
    programBase->Dispose();
    symTable->Dispose();
    InitSymbols();
    uniqueBase = 0;
}
void CreateVariable(long token) {
    variableValue zeroValue;

    zeroValue.value = 0;
    zeroValue.calcLevel = -1;
    variableBase->WriteEntry(token, sizeof(variableValue), &zeroValue);
}

void VariableToKeyword(LexSymbol *theSymbol) {
    switch (theSymbol->value.token) {
        case kMinKey:
            theSymbol->kind = kLexMin;
            break;
        case kMaxKey:
            theSymbol->kind = kLexMax;
            break;
        case kRandomKey:
            theSymbol->kind = kLexFun0;
            break;
        case kSinKey:
        case kCosKey:
        case kIntKey:
        case kRoundKey:
            theSymbol->kind = kLexFun1;
            break;
        case kEnumKey:
            theSymbol->kind = kLexEnum;
            break;
        case kUniqueKey:
            theSymbol->kind = kLexUnique;
            break;
        case kEndKey:
            theSymbol->kind = kLexEnd;
            break;
        case kObjectKey:
            theSymbol->kind = kLexObject;
            break;
        case kAdjustKey:
            theSymbol->kind = kLexAdjust;
            break;
    }
}

short MatchVariable(StringPtr theString) {
    unsigned char theChar;
    short matchCount = 0;
    short state = 1;

    theChar = theString[0];

    if ((theChar >= 'a' && theChar <= 'z') || (theChar >= 'A' && theChar <= 'Z')) {
        matchCount++;

        do {
            theChar = theString[matchCount];

            if ((theChar >= 'a' && theChar <= '}') || theChar == '_' || (theChar >= 'A' && theChar <= ']') ||
                theChar == '.' || (theChar >= '0' && theChar <= '9')) {
                matchCount++;
            } else {
                state = 0;
            }
        } while (state);
    }

    return matchCount;
}
/*
**	Match the exponent part of a floating point number.
**	The input is a null-terminated string. The output
**	tells how many characters could belong to the exponent
**	of a floating point number.
*/
short MatchExponent(StringPtr theString) {
    enum { initialState, integerState, endState };

    short matchCount = 0;
    char theChar;
    short state = initialState;

    while (state != endState) {
        theChar = theString[matchCount];

        if (theChar == 0)
            state = endState; //	String ends with null

        switch (state) {
            case initialState:
                if (theChar == '+' || theChar == '-' || (theChar >= '0' && theChar <= '9')) {
                    matchCount++;
                    state = integerState;
                } else
                    state = endState;
                break;
            case integerState:
                if (theChar >= '0' && theChar <= '9') {
                    matchCount++;
                } else
                    state = endState;
                break;
        }
    }

    return matchCount;
}

/*
**	Match the number part of a row index.
**	The input is a null-terminated string. The output
**	tells how many characters could belong to the row
**	index of a row specifier. Row specifiers are of the
**	format @n, where n is an integer. (@1, @2, @3, ...)
*/
short MatchRowIndex(StringPtr theString) {
    short matchCount = 0;
    char theChar;

    theChar = *theString++;

    while (theChar >= '0' && theChar <= '9') {
        matchCount++;
        theChar = *theString++;
    }

    return matchCount;
}

/*
**	Match a floating point number.
**	The input is a null-terminated string. The output
**	tells how many characters could belong to a floating
**	point number.
*/
short MatchFloat(StringPtr theString) {
    enum { initialState, integerPartState, decimalPartState, exponentMatchState, endState };

    short matchCount = 0;
    char theChar;
    short state = initialState;

    while (state != endState) {
        theChar = theString[matchCount];

        if (theChar == 0)
            state = endState; //	String ends with null

        switch (state) {
            case initialState:
                if (theChar == '+' || theChar == '-' || (theChar >= '0' && theChar <= '9')) {
                    matchCount++;
                    state = integerPartState;
                }
#define ALLOWSHORTHAND
#ifdef ALLOWSHORTHAND //	Are numbers like . and .0 and .e1 and .0e1 valid?
                else if (theChar == '.') {
                    matchCount++;
                    state = decimalPartState;
                }
#endif
                else
                    state = endState;
                break;

            case integerPartState:
                if (theChar >= '0' && theChar <= '9') {
                    matchCount++;
                } else if (theChar == 'e' || theChar == 'E') {
                    state = exponentMatchState;
                } else if (theChar == '.') {
                    matchCount++;
                    state = decimalPartState;
                } else
                    state = endState;
                break;

            case decimalPartState:
                if (theChar >= '0' && theChar <= '9') {
                    matchCount++;
                } else if (theChar == 'e' || theChar == 'E') {
                    state = exponentMatchState;
                } else
                    state = endState;
                break;

            case exponentMatchState: {
                short expLength = MatchExponent(theString + matchCount + 1);

                if (expLength > 0) {
                    matchCount += expLength + 1;
                }
                state = endState;
            } break;
        }
    }

    return matchCount;
}

void EmitInstruction(LexSymbol *theOperation, short stackChange) {
    long writeOffset = parserVar.logicalSize;

    parserVar.stackDepth += stackChange;
    if (parserVar.stackDepth > parserVar.stackMax)
        parserVar.stackMax = parserVar.stackDepth;

    if (IncreaseByClump(
            parserVar.output, &parserVar.realSize, &parserVar.logicalSize, sizeof(LexSymbol), sizeof(LexSymbol) * 10) ==
        noErr) {
        *(LexSymbol *)(*parserVar.output + writeOffset) = *theOperation;
    }
}

void SkipComment() {
    parserVar.input += 2;
    if (parserVar.input[0] != 0) {
        while ((parserVar.input[0] != 0) && !(parserVar.input[0] == '*' && parserVar.input[1] == '/')) {
            parserVar.input++;
        }

        if (parserVar.input[0] != 0) {
            parserVar.input += 2;
        }
    }
}
void SkipOneLineComment() {
    parserVar.input += 2;
    if (parserVar.input[0] != 0) {
        while ((parserVar.input[0] != 0) && !(parserVar.input[0] == 13)) {
            parserVar.input++;
        }

        if (parserVar.input[0] != 0) {
            parserVar.input += 1;
        }
    }
}

tokentype LexStringConstant();
#define STRINGSTARTQUOTE '"'
#define STRINGENDQUOTE '"'

tokentype LexStringConstant() {
    unsigned char theChar;
    unsigned char *input, *output;
    tokentype theResult;
    long len;

    input = parserVar.input + 1;
    output = input;

    do {
        theChar = *input++;
        if (theChar == STRINGENDQUOTE) {
            theChar = *input++;
            if (theChar != STRINGENDQUOTE) {
                theChar = 0;
            }
        }
        *output++ = theChar;
    } while (theChar);

    len = output - parserVar.input - 2;

    if (len > 255)
        len = 255;
    theResult = symTable->FindEntry(parserVar.input, len);
    parserVar.input = input - 1;

    return theResult;
}

void LexRead(LexSymbol *theSymbol) {
    unsigned char theChar;
    short matchCount;

    //	First, skip any whitespace like returns, tabs, spaces and control characters.

    theChar = *parserVar.input;

    while (theChar && (theChar <= 32 || theChar == 160)) //	Option space!!
    {
        theChar = *(++parserVar.input);
    }

    switch (theChar) {
        case '+':
            theSymbol->kind = kLexPlus;
            parserVar.input++;
            break;
        case '-':
            theSymbol->kind = kLexMinus;
            parserVar.input++;
            break;
        case '|':
            theSymbol->kind = kLexAbs;
            parserVar.input++;
            break;
        case '*':
            theSymbol->kind = kLexMultiply;
            parserVar.input++;
            break;
        case '/':
            if (parserVar.input[1] == '*') {
                SkipComment();
                LexRead(theSymbol);
            } else if (parserVar.input[1] == '/') {
                SkipOneLineComment();
                LexRead(theSymbol);
            } else {
                theSymbol->kind = kLexDivide;
                parserVar.input++;
            }
            break;
        case STRINGSTARTQUOTE:
            theSymbol->value.floating = LexStringConstant();
            theSymbol->kind = kLexConstant;
            break;
        case '%':
            theSymbol->kind = kLexModulo;
            parserVar.input++;
            break;
        case '^':
            theSymbol->kind = kLexPower;
            parserVar.input++;
            break;
        case '>':
            if (parserVar.input[1] == '=') {
                theSymbol->kind = kLexGtEq;
                parserVar.input += 2;
            } else {
                theSymbol->kind = kLexGt;
                parserVar.input++;
            }
            break;
        case '<':
            if (parserVar.input[1] == '=') {
                theSymbol->kind = kLexLtEq;
                parserVar.input += 2;
            } else {
                theSymbol->kind = kLexLt;
                parserVar.input++;
            }
            break;
        case '=':
            theSymbol->kind = kLexEq;
            parserVar.input++;
            break;
#ifdef TESTMINMAX
        case ']':
            theSymbol->kind = kLexMin;
            parserVar.input++;
            break;
        case '[':
            theSymbol->kind = kLexMax;
            parserVar.input++;
            break;
#endif
        case '(':
            theSymbol->kind = kLexOpenParen;
            parserVar.input++;
            break;
        case ')':
            theSymbol->kind = kLexCloseParen;
            parserVar.input++;
            break;
        case 0:
            theSymbol->kind = kLexEof;
            break;
        case '@':
            parserVar.input++;
            LexRead(theSymbol);
            if (theSymbol->kind == kLexVariable) {
                theSymbol->kind = kLexConstant;
                theSymbol->value.floating = theSymbol->value.token;
            } else {
                theSymbol->kind = kParseError;
            }
            break;

        default:
            matchCount = MatchVariable(parserVar.input);
            if (matchCount > 0) {
                theSymbol->value.token = symTable->FindEntry(parserVar.input - 1, matchCount);
                if (theSymbol->value.token > lastVariable) {
                    lastVariable = theSymbol->value.token;
                    CreateVariable(lastVariable);
                }
                theSymbol->kind = kLexVariable;
                if (theSymbol->value.token <= lastKeyword) {
                    VariableToKeyword(theSymbol);
                }
                parserVar.input += matchCount;
            } else {
                matchCount = MatchFloat(parserVar.input);
                if (matchCount > 0) {
                    char temp;
                    char tempString[256];
                    char *pstr = (char *)parserVar.input - 1;

                    // From StringToLongDouble
                    BlockMoveData(pstr + 1, tempString, pstr[0]); //<fp.h>
                    tempString[pstr[0]] = '\0';

                    temp = parserVar.input[-1];
                    parserVar.input[-1] = matchCount;
                    theSymbol->kind = kLexConstant;
                    // theSymbol->value.floating = StringToLongDouble(parserVar.input-1);
                    theSymbol->value.floating = atof(tempString);
                    // SDL_Log("\natof(%s) --> %f\n", tempString, theSymbol->value.floating);
                    parserVar.input[-1] = temp;
                    parserVar.input += matchCount;
                }
            }

            if (matchCount == 0) {
                theSymbol->kind = kParseError;
            }
            break;
    }
}

void LexMatch(short kind) {
    if (parserVar.lookahead.kind == kind)
        LexRead(&parserVar.lookahead);
    else
        parserVar.lookahead.kind = kParseError;
}

void ParseFactor() {
    LexSymbol savedSymbol;

    switch (parserVar.lookahead.kind) {
        case kLexFun0:
            EmitInstruction(&parserVar.lookahead, 1);
            DEBUGPAR(printf("function0 ");)
            LexMatch(parserVar.lookahead.kind);
            break;

        case kLexFun1:
            savedSymbol = parserVar.lookahead;
            LexMatch(parserVar.lookahead.kind);
            ParseFactor();
            EmitInstruction(&savedSymbol, 0);
            DEBUGPAR(printf("function1 ");)
            break;

        case kLexConstant:
            EmitInstruction(&parserVar.lookahead, 1);
            DEBUGPAR(printf("%3.2f ", parserVar.lookahead.value.floating);)
            LexMatch(parserVar.lookahead.kind);
            break;

        case kLexVariable:
            EmitInstruction(&parserVar.lookahead, 1);
            DEBUGPAR(printf("Variable #%ld ", parserVar.lookahead.value.token);)
            LexMatch(parserVar.lookahead.kind);
            break;

        case kLexRowOffset:
            EmitInstruction(&parserVar.lookahead, 1);
            DEBUGPAR(printf("@(%ld) ", parserVar.lookahead.value.offset);)
            LexMatch(parserVar.lookahead.kind);
            break;

        case kLexMinus:
            LexMatch(parserVar.lookahead.kind);
            ParseFactor();

            savedSymbol.kind = kUnaryMinus;
            EmitInstruction(&savedSymbol, 0);
            DEBUGPAR(printf("neg ");)
            break;

        case kLexAbs:
            LexMatch(parserVar.lookahead.kind);
            ParseFactor();

            savedSymbol.kind = kLexAbs;
            EmitInstruction(&savedSymbol, 0);
            DEBUGPAR(printf("abs ");)
            break;

        case kLexPlus: //	This is a no-op.
            LexMatch(parserVar.lookahead.kind);
            ParseFactor();
            break;

        case kLexOpenParen:
            LexMatch(parserVar.lookahead.kind);
            ParseCompare();
            if (parserVar.lookahead.kind == kLexCloseParen) {
                LexMatch(parserVar.lookahead.kind);
            } else {
                parserVar.lookahead.kind = kParseError;
            }
            break;

        default:
            parserVar.lookahead.kind = kParseError;
            break;
    }
}
void ParsePower() {
    LexSymbol mySymbol;

    ParseFactor();

    while (parserVar.lookahead.kind == kLexPower) {
        mySymbol.kind = parserVar.lookahead.kind;

        LexMatch(parserVar.lookahead.kind);
        ParseFactor();

        EmitInstruction(&mySymbol, -1);

        DEBUGPAR(printf("^ ");)
    }
}
void ParseTerm() {
    LexSymbol mySymbol;

    ParsePower();

    while (parserVar.lookahead.kind == kLexMultiply || parserVar.lookahead.kind == kLexDivide ||
           parserVar.lookahead.kind == kLexModulo) {
        mySymbol.kind = parserVar.lookahead.kind;

        LexMatch(parserVar.lookahead.kind);
        ParsePower();

        EmitInstruction(&mySymbol, -1);

#ifdef DEBUGPARSER
        if (mySymbol.kind == kLexMultiply)
            printf("* ");
        else if (mySymbol.kind == kLexModulo)
            printf("%% ");
        else
            printf("/ ");
#endif
    }
}

void ParseExpr() {
    LexSymbol mySymbol;

    ParseTerm();

    while (parserVar.lookahead.kind == kLexPlus || parserVar.lookahead.kind == kLexMinus) {
        mySymbol.kind = parserVar.lookahead.kind;

        LexMatch(parserVar.lookahead.kind);
        ParseTerm();
        EmitInstruction(&mySymbol, -1);

#ifdef DEBUGPARSER
        if (mySymbol.kind == kLexPlus)
            printf("+ ");
        else
            printf("- ");
#endif
    }
}
void ParseMinMax() {
    LexSymbol mySymbol;

    ParseExpr();

    while (parserVar.lookahead.kind == kLexMin || parserVar.lookahead.kind == kLexMax) {
        mySymbol.kind = parserVar.lookahead.kind;

        LexMatch(parserVar.lookahead.kind);
        ParseExpr();
        EmitInstruction(&mySymbol, -1);

#ifdef DEBUGPARSER
        if (mySymbol.kind == kLexMin)
            printf("min ");
        else
            printf("max ");
#endif
    }
}

void ParseCompare() {
    LexSymbol mySymbol;

    ParseMinMax();

    while (parserVar.lookahead.kind == kLexGt || parserVar.lookahead.kind == kLexEq ||
           parserVar.lookahead.kind == kLexLt || parserVar.lookahead.kind == kLexLtEq ||
           parserVar.lookahead.kind == kLexGtEq) {
        mySymbol.kind = parserVar.lookahead.kind;

        LexMatch(parserVar.lookahead.kind);
        ParseMinMax();
        EmitInstruction(&mySymbol, -1);

#ifdef DEBUGOUTPUT
        if (mySymbol.kind == kLexGt)
            printf("> ");
        else if (mySymbol.kind == kLexLt)
            printf("< ");
        else
            printf("= ");
#endif
    }
}

void ParseStatement(LexSymbol *statement) {
    *statement = parserVar.lookahead;
    switch (parserVar.lookahead.kind) {
        case kLexVariable:
            LexMatch(parserVar.lookahead.kind);
            if (parserVar.lookahead.kind == kLexLt) {
                LexMatch(parserVar.lookahead.kind);
                if (parserVar.lookahead.kind == kLexMinus) {
                    parserVar.lookahead.kind = kLexStore;
                }
            }

            if (parserVar.lookahead.kind == kLexEq || parserVar.lookahead.kind == kLexStore) {
                Boolean doStore = (parserVar.lookahead.kind == kLexStore);

                LexMatch(parserVar.lookahead.kind);

                if (statement->value.token > lastVariable) {
                    lastVariable = statement->value.token;
                    CreateVariable(lastVariable);
                }

                ParseCompare();
                statement->kind = kAssignment;
                EmitInstruction(statement, -1);

                if (doStore) {
                    SetHandleSize(parserVar.output, parserVar.logicalSize);
                    parserVar.realSize = parserVar.logicalSize;
                    programBase->WriteHandle(-1, parserVar.output);
                    ProgramVariable(statement->value.token - firstVariable, EvalVariable(-1, true));
                    statement->kind = kLexStore;
                }
                DEBUGPAR(printf("Store to var #%ld ", statement->value.token);)
            } else {
                parserVar.lookahead.kind = kParseError;
            }
            break;
        case kLexObject:
        case kLexAdjust: {
            short savedKind;

            savedKind = parserVar.lookahead.kind;
            LexMatch(parserVar.lookahead.kind);
            if (parserVar.lookahead.kind == kLexVariable) {
                if (savedKind == kLexObject)
                    CreateTheObject();
                else
                    CreateTheAdjuster();

                LexMatch(parserVar.lookahead.kind);
            } else {
                parserVar.lookahead.kind = kParseError;
            }
            break;
        }
        case kLexEnd:
            if (currentActor) {
                if (currentActor->EndScript()) {
                    currentActor->AddToGame();
                }
                currentActor = NULL;
            }
            LexMatch(parserVar.lookahead.kind);
            break;
        case kLexEnum:
            LexMatch(parserVar.lookahead.kind);
            ParseCompare();
            if (parserVar.lookahead.kind == kLexVariable) {
                LexSymbol enumStatement;
                double enumValue;

                enumStatement = parserVar.lookahead;
                enumStatement.kind = kAssignment;
                EmitInstruction(&enumStatement, -1);

                programBase->WriteHandle(enumStatement.value.token, parserVar.output);
                enumValue = EvalVariable(parserVar.lookahead.value.token, false);

                while (parserVar.lookahead.kind == kLexVariable) {
                    ProgramVariable(parserVar.lookahead.value.token - firstVariable, enumValue);
                    enumValue += 1;
                    LexMatch(parserVar.lookahead.kind);
                }

                LexMatch(kLexEnd);
            }
            break;
        case kLexUnique:
            LexMatch(parserVar.lookahead.kind);
            if (parserVar.lookahead.kind == kLexConstant) {
                uniqueBase = parserVar.lookahead.value.floating;
                LexMatch(parserVar.lookahead.kind);
            }

            while (parserVar.lookahead.kind == kLexVariable) {
                ProgramVariable(parserVar.lookahead.value.token - firstVariable, uniqueBase++);
                LexMatch(parserVar.lookahead.kind);
            }

            LexMatch(kLexEnd);
            break;

        default:
            parserVar.lookahead.kind = kParseError;
            break;
    }
}

void SetupCompiler(StringPtr theInput) {
    parserVar.input = theInput;
    parserVar.output = NewHandle(128);
    parserVar.realSize = 128;
    parserVar.logicalSize = 0;
    parserVar.stackDepth = 0;
    parserVar.stackMax = 0;
}

void WriteVariable(long token, double value) {
    variableValue *theVar;
    double temp;

    theVar = (variableValue *)variableBase->GetEntryPointer(token);
    if (theVar) {
        theVar->calcLevel = currentLevel;
        theVar->value = value;
    }
}
double EvalVariable(long token, Boolean forceCalc) {
    variableValue *theVar;
    double temp;

    theVar = (variableValue *)variableBase->GetEntryPointer(token);
    if (theVar->calcLevel != currentLevel || forceCalc) {
        LexSymbol *theProgram;

        theVar->calcLevel = currentLevel;
        theProgram = (LexSymbol *)programBase->GetEntryPointer(token);
        if (theProgram) {
            while (theProgram->kind != kAssignment) {
                switch (theProgram->kind) {
                    case kLexConstant:
                        *(++stackP) = theProgram->value.floating;
                        break;
                    case kLexVariable:
                        temp = EvalVariable(theProgram->value.token, false);
                        *(++stackP) = temp;
                        break;
                    case kLexPlus:
                        stackP[-1] += stackP[0];
                        stackP--;
                        break;
                    case kLexMinus:
                        stackP[-1] -= stackP[0];
                        stackP--;
                        break;
                    case kLexMultiply:
                        stackP[-1] *= stackP[0];
                        stackP--;
                        break;
                    case kLexDivide:
                        stackP[-1] /= stackP[0];
                        stackP--;
                        break;

                    case kLexGt:
                        stackP[-1] = stackP[-1] > stackP[0];
                        stackP--;
                        break;
                    case kLexLt:
                        stackP[-1] = stackP[-1] < stackP[0];
                        stackP--;
                        break;
                    case kLexGtEq:
                        stackP[-1] = stackP[-1] >= stackP[0];
                        stackP--;
                        break;
                    case kLexLtEq:
                        stackP[-1] = stackP[-1] <= stackP[0];
                        stackP--;
                        break;
                    case kLexEq:
                        stackP[-1] = stackP[-1] == stackP[0];
                        stackP--;
                        break;

                    case kLexPower:
                        stackP[-1] = pow(stackP[-1], stackP[0]);
                        stackP--;
                        break;

                    case kLexMin:
                        if (stackP[-1] < stackP[0])
                            stackP[-1] = stackP[0];
                        stackP--;
                        break;
                    case kLexMax:
                        if (stackP[-1] > stackP[0])
                            stackP[-1] = stackP[0];
                        stackP--;
                        break;

                    case kLexModulo:
                        stackP[-1] = fmod(stackP[-1], stackP[0]);
                        stackP--;
                        break;
                    case kUnaryMinus:
                        stackP[0] = -stackP[0];
                        break;
                    case kLexAbs:
                        if (stackP[0] < 0)
                            stackP[0] = -stackP[0];
                        break;
                    case kLexFun0:
                        *(++stackP) = FRandom() / 65536.0;
                        break;
                    case kLexFun1:
                        switch (theProgram->value.token) {
                            case kSinKey:
                                *stackP = sin(*stackP * (3.14159265359 / 180));
                                break;
                            case kCosKey:
                                *stackP = cos(*stackP * (3.14159265359 / 180));
                                break;
                            case kRoundKey:
                                *stackP = floor(*stackP + 0.5);
                                break;
                            case kIntKey:
                                *stackP = floor(*stackP);
                                break;
                        }
                        break;
                }
                theProgram++;
            }
            theVar->value = *(stackP--);
        }
    }

    return theVar->value;
}

void LoadProgram() {
    OSErr err;
    short ref;
    long len;
    Handle mainScript;
    Handle privateScript;

    theScript = GetResource('TEXT', 2128);
    DetachResource(theScript);
    privateScript = GetResource('TEXT', 1000);
    if (privateScript) {
        HandAndHand(privateScript, theScript);
        ReleaseResource(privateScript);
    }

    len = GetHandleSize(theScript);
    HUnlock(theScript);
    SetHandleSize(theScript, len + 1);
    HLock(theScript);
    (*theScript)[len] = 0; //	Append a NULL to terminate the script.
}

static unsigned long oldTicks = 0;

char *fixedString(unsigned char *s) {
    int len = std::strlen((char *)s);
    char *fixed = new char[len + 1];
    memcpy(fixed, s, len);
    fixed[len] = 0;
    for (int i = 0; i < len; i++) {
        if (fixed[i] == 13)
            fixed[i] = 10;
    }
    return fixed;
}

void RunThis(unsigned char *script) {
    LexSymbol statement;
    unsigned long newTicks;

#ifdef DEBUGPARSER
    char *formattedScript = fixedString(script);
    SDL_Log("Running script:\n%s\n", formattedScript);
    std::free(formattedScript);
#endif

    /*
    newTicks = TickCount();
    if(newTicks - oldTicks > 10)
    {
        GDHandle	savedGD;
        CGrafPtr	savedGrafPtr;

        GetGWorld(&savedGrafPtr, &savedGD);
        SetGWorld((CGrafPtr)FrontWindow(), GetMainDevice());

        oldTicks = newTicks;
        gApplication->BroadcastCommand(kBusyTimeCmd);

        SetGWorld(savedGrafPtr, savedGD);
    }
    */

    SetupCompiler(script);
    LexRead(&parserVar.lookahead);

    do {
        ParseStatement(&statement);

        if (parserVar.lookahead.kind == kParseError) {
            DEBUGPAR(printf(" ** Parse Error ** on input: %s\n", parserVar.input);)
        } else {
            if (statement.kind == kAssignment) {
                SetHandleSize(parserVar.output, parserVar.logicalSize);
                programBase->WriteHandle(statement.value.token, parserVar.output);
                programBase->Lock();
                DEBUGPAR(printf(" ==> %lf\n", EvalVariable(statement.value.token, false));)
                programBase->Unlock();
            }
        }
        DisposHandle(parserVar.output);
        SetupCompiler(parserVar.input);

        // On parse error, skip ahead a line to see if we can read past the garbage
        if (parserVar.lookahead.kind == kParseError) {
          DEBUGPAR(printf(" ** trying to skip past parse error **\n");)
          SkipOneLineComment();
          LexRead(&parserVar.lookahead);
        }
    } while (!(parserVar.lookahead.kind == kLexEof));

    DisposeHandle(parserVar.output);
}

void AllocParser() {
    currentLevel = 0;
    InitSymbols();
    stackP = (double *)NewPtr(sizeof(double) * 256);

    currentActor = NULL;
    LoadProgram();
    RunThis((unsigned char *)*theScript);
}

void DeallocParser() {
    if (theScript)
        DisposeHandle(theScript);
    if (stackP)
        DisposePtr((Ptr)stackP);

    if (symTable)
        symTable->Dispose();
    if (variableBase)
        variableBase->Dispose();
    if (programBase)
        programBase->Dispose();

    theScript = NULL;
    stackP = NULL;
    symTable = NULL;
    variableBase = NULL;
    programBase = NULL;

    if (currentActor) {
        currentActor->Dispose();
        currentActor = NULL;
    }
}

double ReadVariable(short index) {
    return EvalVariable(index + firstVariable, false);
}

Fixed ReadFixedVar(short index) {
    return 65536 * EvalVariable(index + firstVariable, false);
}

long ReadLongVar(short index) {
    return EvalVariable(index + firstVariable, false);
}

long ReadColorVar(short index) {
    long theColor;

    theColor = ReadVariable(index);

    return (((theColor / 1000000) << 16) | (((theColor / 1000) % 1000) << 8) | (theColor % 1000));
}

void ReadStringVar(short index, StringPtr dest) {
    short len;
    Handle result;

    index = EvalVariable(index + firstVariable, false);
    if (index) {
        symTable->GetIndEntry(index, dest);
    } else {
        dest[0] = 0;
    }
}

void ProgramVariable(short index, double value) {
    LexSymbol steps[2];

    index += firstVariable;
    steps[0].kind = kLexConstant;
    steps[0].value.floating = value;
    steps[1].kind = kAssignment;
    steps[1].value.token = index;
    programBase->WriteEntry(index, sizeof(LexSymbol) * 2, (Ptr)steps);
}

void ProgramReference(short index, short ref) {
    LexSymbol steps[2];

    index += firstVariable;
    ref += firstVariable;
    steps[0].kind = kLexVariable;
    steps[0].value.token = ref;
    steps[1].kind = kAssignment;
    steps[1].value.token = index;
    programBase->WriteEntry(index, sizeof(LexSymbol) * 2, (Ptr)steps);
}

void ProgramOffsetAdd(short index, short ref, long addValue) {
    LexSymbol steps[4];

    index += firstVariable;
    ref += firstVariable;
    steps[0].kind = kLexVariable;
    steps[0].value.token = ref;
    steps[1].kind = kLexConstant;
    steps[1].value.floating = addValue;
    steps[2].kind = kLexPlus;
    steps[3].kind = kAssignment;
    steps[3].value.token = index;

    programBase->WriteEntry(index, sizeof(LexSymbol) * 4, (Ptr)steps);
}

void ProgramOffsetMultiply(short index, short ref, long multValue) {
    LexSymbol steps[4];

    index += firstVariable;
    ref += firstVariable;
    steps[0].kind = kLexVariable;
    steps[0].value.token = ref;
    steps[1].kind = kLexConstant;
    steps[1].value.floating = multValue;
    steps[2].kind = kLexMultiply;
    steps[3].kind = kAssignment;
    steps[3].value.token = index;

    programBase->WriteEntry(index, sizeof(LexSymbol) * 4, (Ptr)steps);
}

void ProgramFixedVar(short index, Fixed value) {
    ProgramVariable(index, value / 65536.0);
}

void ProgramLongVar(short index, long value) {
    ProgramVariable(index, value);
}

void ProgramMessage(short index, long value) {
    ProgramLongVar(index, value + firstVariable);
}

void InitParser() {
    static Boolean isAllocated = false;

    if (isAllocated) {
        DeallocParser();
    }

    AllocParser();
    isAllocated = true;
}
