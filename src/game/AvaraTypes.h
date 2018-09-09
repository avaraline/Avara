/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: AvaraTypes.h
    Created: Saturday, June 24, 1995, 23:33
    Modified: Sunday, August 25, 1996, 08:52
*/

#pragma once

class CAbstractActor;

struct ActorLocator {
    struct ActorLocator *next;
    struct ActorLocator *prev;
    CAbstractActor *me;
};
typedef struct ActorLocator ActorLocator;

typedef unsigned short MsgType;

struct MessageRecord {
    struct MessageRecord *next;
    CAbstractActor *owner;
    MsgType messageId;
    unsigned short triggerCount;
};
typedef struct MessageRecord MessageRecord;

typedef long MaskType;

struct ActorAttachment {
    CAbstractActor *me;
    struct ActorAttachment *next;
};
typedef struct ActorAttachment ActorAttachment;
