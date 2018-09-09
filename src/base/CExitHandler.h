#pragma once

#include "CDirectObject.h"
#include "Types.h"

typedef void ExitFuncType(long theData);

struct ExitRecord {
    ExitFuncType *exitFunc;
    long theData;
    struct ExitRecord *nextExit;
};

typedef struct ExitRecord ExitRecord;

class CExitHandler : public CDirectObject {
public:
    ExitRecord *firstExit;

    CExitHandler() {}
    virtual ~CExitHandler() {}

    virtual void IExitHandler();
    virtual void AddExit(ExitRecord *theExit);
    virtual void RemoveExit(ExitRecord *theExit);
    virtual void DoExit();
};

void StartExitHandler(void);

#ifndef MAINEXITHANDLER
extern CExitHandler *gExitHandler;
#else
CExitHandler *gExitHandler = NULL;
#endif
