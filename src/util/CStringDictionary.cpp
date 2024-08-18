/*
    Copyright ©1992-1995, Juri Munkki
    All rights reserved.

    File: CStringDictionary.c
    Created: Friday, December 4, 1992, 11:45
    Modified: Tuesday, November 7, 1995, 18:21
*/

/*
**	See include file for more details on how this class works.
*/
#include "CStringDictionary.h"

#include "RamFiles.h"
#include "PascalStrings.h"

#include <cstring>

/*
**	Given a counted string of length len, HashString
**	will return a value from 0 to HASHTABLESIZE-1.
**	This value can be used as the hashing value for
**	that string.
*/
short HashString(char *string, short len) {
    if (len > 0) {
        return (string[0] + string[len - 1] + string[len >> 1]) & (HASHTABLESIZE - 1);
    } else
        return 0;
}

/*
**	Initialize a CStringDictionary. The directory will be empty.
*/
void CStringDictionary::IStringDictionary() {
    IBaseObject();
}

/*
**	Return count of items in dictionary.
*/
short CStringDictionary::GetDictionarySize() {
    return wordList.size();
}


std::string CStringDictionary::GetIndEntry(short index) {
    if (0 <= index && index < wordList.size()) return wordList.at(index);
    else return "";
}

/*
**	Copy the string for entry index to theEntry
*/
void CStringDictionary::GetIndEntry(short index, char *theEntry) {
    if (0 <= index && index < wordList.size()) {
        strcpy(theEntry, wordList.at(index).c_str());
    }
}

void CStringDictionary::GetIndEntry(short index, StringPtr theEntry) {
    if (0 <= index && index < wordList.size()) {
        std::string entry = wordList.at(index);
        theEntry[0] = entry.length();
        memcpy(theEntry + 1, entry.c_str(), entry.length());
    }
}

short CStringDictionary::GetIndEntrySize(short index) {
    if (index >= 0 && index < wordList.size()) {
        return wordList.at(index).length();
    } else {
        return 0;
    }
}


/*
**	Add a dictionary entry regardless
**	of wether it already is there or not. Normally you would
**	first look for a dictionary entry before calling this
**	routine. Call this routine if you are absolutely sure
**	that the entry isn't already in the dictionary.
**
**	NOTE: A negative return value signifies and error.
*/

tokentype CStringDictionary::AddDictEntry(const unsigned char *entry, short len) {
    if (len < 0) {
        len = entry[0];
    }
    if (len == 0) { return -1; }
    char *s = PascalStringtoCString(entry, len);
    tokentype tt = AddDictEntry(s, len);
    delete[] s;
    return tt;
}

tokentype CStringDictionary::AddDictEntry(const char *entry, short len) {
    std::string s;
    if (len >= 0) {
        s.append(entry, len);
    } else {
        s.append(entry);
    }

    auto idx = wordList.size();
    wordList.push_back(s);
    index.insert(std::pair<std::string, size_t>(s, idx));
    return idx;
}

/*
**	Look for an entry and return it if found. Return
**	-1 if the entry is not found.
*/
tokentype CStringDictionary::SearchForEntry(const unsigned char *entry, short len) {
    if (len < 0) {
        len = entry[0];
    }
    if (len == 0) { return 0; }
    char *s = PascalStringtoCString(entry, len);
    tokentype tt = SearchForEntry(s, len);
    delete[] s;
    return tt;
}
tokentype CStringDictionary::SearchForEntry(const char *entry, short len) {
    std::string s(entry, len);
    std::map<std::string, size_t>::iterator it;
    it = index.find(s);
    if (it != index.end()) {
        return index.find(s)->second;
    } else {
        return -1;
    }
}

/*
**	Look for an entry and if doesn't exist, create it.
**	This routine only returns an error if there is a
**	problem with memory management.
*/
tokentype CStringDictionary::FindEntry(const unsigned char *entry, short len) {
    tokentype theToken;

    theToken = SearchForEntry(entry, len);
    if (theToken >= 0)
        return theToken;
    else
        return AddDictEntry(entry, len);
}

/*
**	Unlock and dispose storage.
*/
void CStringDictionary::Dispose() {
    CBaseObject::Dispose();
}

/*
**	Lock everything.
*/
void CStringDictionary::Lock() {
    CBaseObject::Lock();
}
/*
**	Unlock everything.
*/
void CStringDictionary::Unlock() {
    CBaseObject::Unlock();
}

Handle CStringDictionary::WriteToHandle() {
    Handle result;
    long neededSize;
    short ind;
    short stringCount;
    size_t len;

    stringCount = GetDictionarySize();
    neededSize = sizeof(stringCount);

    for (ind = 0; ind < stringCount; ind++) {
        neededSize += sizeof(size_t) + GetIndEntrySize(ind);
    }

    result = NewHandle(neededSize);
    if (result) {
        char *p = (char *)result;
        HLock(result);
        memcpy(p, &stringCount, sizeof(stringCount));
        p += sizeof(stringCount);
        for (std::vector<std::string>::iterator it = wordList.begin(); it != wordList.end(); it++) {
            len = it->length();
            memcpy(p, &len, sizeof(size_t));
            p += sizeof(size_t);
            memcpy(p, it->c_str(), it->length());
            p += it->length();
        }
        HUnlock(result);
    }

    return result;
}

void CStringDictionary::ReadFromHandle(Handle source) {
    short ind;
    short stringCount;
    char cMemTags;
    size_t len;
    std::string s;

    if (source) {
        char *p = (char *)source;

        cMemTags = HGetState(source);
        HLock(source);

        memcpy(&stringCount, p, sizeof(stringCount));
        p += sizeof(stringCount);
        for (ind = 0; ind < stringCount; ind++) {
            memcpy(&len, p, sizeof(len));
            p += sizeof(len);
            AddDictEntry(p, len);
            p += len;
        }

        HSetState(source, cMemTags);
    }
}
