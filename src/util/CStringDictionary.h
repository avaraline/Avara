/*
    Copyright ©1993-1995, Juri Munkki
    All rights reserved.

    File: CStringDictionary.h
    Created: Sunday, July 25, 1993, 18:48
    Modified: Thursday, June 1, 1995, 01:23
*/

/*
**	This class maintains a sort of dictionary of words. The idea
**	is to have a fast way to find a dictionary word. A hashed
**	table is used to speed up the searches while still keeping
**	this class fairly simple.
**
**	NOTE:
**		While data is being added to the object, the
**		object can not be locked.
*/
#pragma once

#include <map>
#include <string>
#include <vector>

#include "CBaseObject.h"
#include "Types.h"

using namespace std;

#define DICTIONARYCLUMPSIZE (16 * sizeof(DictEntry))
#define WORDCLUMPSIZE 256
#define HASHTABLESIZE 128 /* Must be a power of 2	*/

/*
**	The amount of storage required for a single item is
**	determined by the token type. A tokentype of "short"
**	will allow 32767 dictionary entries, which should be
**	sufficient for the kinds of data that this class was
**	written for.
*/
typedef short tokentype;

/*
**	Dictionary entries are stored into two separate handles.
**	The other one is the list of words and the other one
**	a list of hash table links and offsets to the words.
*/
typedef struct {
    long nameOffset;
    short hashLink;
} DictEntry;

class CStringDictionary : public CBaseObject {
public:
    /*	Methods:			*/
    virtual void IStringDictionary();
    virtual tokentype AddDictEntry(const unsigned char *entry, short len);
    virtual tokentype AddDictEntry(const char *entry, short len);
    virtual tokentype FindEntry(const unsigned char *entry, short len);
    virtual tokentype SearchForEntry(const unsigned char *entry, short len);
    virtual tokentype SearchForEntry(const char *entry, short len);
    virtual void ReadFromStringList(short strListID);

    virtual short GetDictionarySize();
    virtual void GetIndEntry(short index, StringPtr theEntry);
    virtual void GetIndEntry(short index, char *theEntry);

    virtual void Dispose();
    virtual void Lock();
    virtual void Unlock();

    virtual short GetIndEntrySize(short index);
    virtual Handle WriteToHandle();
    virtual void ReadFromHandle(Handle source);

private:
    /*	Variables:			*/
    std::vector<std::string> wordList;
    std::map<std::string, size_t> index;
};
