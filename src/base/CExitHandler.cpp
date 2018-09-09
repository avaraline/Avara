#define MAINEXITHANDLER

#include "CExitHandler.h"

void CExitHandler::IExitHandler() {
    firstExit = NULL;
}

void CExitHandler::AddExit(ExitRecord *theExit) {
    theExit->nextExit = firstExit;
    firstExit = theExit;
}

void CExitHandler::RemoveExit(ExitRecord *theExit) {
    ExitRecord *ep, **px;

    ep = firstExit;
    px = &firstExit;

    while (ep != NULL) {
        if (ep == theExit) {
            *px = theExit->nextExit;
            ep = NULL;
        } else {
            px = &ep->nextExit;
            ep = ep->nextExit;
        }
    }

    theExit->exitFunc = NULL;
}

void CExitHandler::DoExit() {
    ExitRecord *nextOne;
    ExitRecord *newList;

    newList = NULL;

    while (firstExit) {
        nextOne = firstExit->nextExit;
        firstExit->nextExit = newList;
        newList = firstExit;
        firstExit = nextOne;
    }

    while (newList) {
        nextOne = newList->nextExit;
        if (newList->exitFunc) {
            newList->exitFunc(newList->theData);
        }
        newList = nextOne;
    }
}

void StartExitHandler() {
    if (!gExitHandler) {
        gExitHandler = new CExitHandler;
        gExitHandler->IExitHandler();
    }
}
