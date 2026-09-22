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

#include <cstring>

std::string CStringDictionary::GetIndEntry(short index) {
    if (0 <= index && index < wordList.size()) return wordList.at(index);
    else return "";
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

tokentype CStringDictionary::AddDictEntry(std::string s) {
    auto idx = wordList.size();
    wordList.push_back(s);
    index.insert(std::pair<std::string, size_t>(s, idx));
    return idx;
}

/*
**	Look for an entry and return it if found. Return
**	-1 if the entry is not found.
*/
tokentype CStringDictionary::SearchForEntry(std::string s) {
    auto it = index.find(s);
    return it != index.end() ? it->second : -1;
}

/*
**	Look for an entry and if doesn't exist, create it.
**	This routine only returns an error if there is a
**	problem with memory management.
*/
tokentype CStringDictionary::FindEntry(std::string s) {
    tokentype theToken = SearchForEntry(s);
    if (theToken >= 0)
        return theToken;
    else
        return AddDictEntry(s);
}
