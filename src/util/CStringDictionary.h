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

#include "Memory.h"
#include "Types.h"

using namespace std;

/*
**	The amount of storage required for a single item is
**	determined by the token type. A tokentype of "short"
**	will allow 32767 dictionary entries, which should be
**	sufficient for the kinds of data that this class was
**	written for.
*/
typedef short tokentype;


class CStringDictionary {
public:
    tokentype AddDictEntry(std::string s);
    tokentype FindEntry(std::string s);
    tokentype SearchForEntry(std::string s);
    std::string GetIndEntry(short index);

private:
    std::vector<std::string> wordList;
    std::map<std::string, size_t> index;
};
