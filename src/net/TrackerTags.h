/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: TrackerTags.h
    Created: Friday, August 30, 1996, 14:09
    Modified: Saturday, August 31, 1996, 19:05
*/

#pragma once

/*
**	Simple tags use the positive range of numbers. Their format is the
**	char-sized tag, then the unsigned char length of the following data
**	and then the data itself.
*/
enum //	Simple tags.
{ ktsNull = 0, //	Used to enclose comments or skip data.

    ktsCommand, //	char -> See further ahead in this file for list of commands.
    ktsQueryID, //	long -> unique indentifier for query AND response.
    ktsQueryString, //	String -> tracker query contains a string for commands & matching

    //	Tracker sends these:
    ktsHostDomain, //	String -> domain name of server. Such as hyperion.ambrosiasw.com
    ktsResponseIndex, //	short -> index of response packet (from 0 to n-1)
    ktsResponseCount, //	short -> total number of response packets
                      //			 client can send this to request max ktsResponseCount packets.
    ktsStatusMessage, //	String -> optional short message to show in "Status" space.
    ktsInfoMessage, //	String -> optional message to show in "Info" box next to server list.

    //	Tracker & server use these:
    ktsInvitation, //	String -> Invitation string of server.
    ktsHasPassword, //	No data. Tag indicates that a password exists.
    ktsPlayerLimit, //	char -> max number of players allowed, including server.

    ktsGameStatus, //	char -> See further in this file.
    ktsLevelDirectory, //	String -> name of level directory used on server.
    ktsLevelName, //	String -> name of level currently loaded on server.

    ktsProtocolVersion, //	short -> number indicating network compatibility
    ktsSoftwareVersion, //	String -> short version number from 'vers' 1 resource.

    ktsCount };

/*
**	Indexed tags use the negative range of numbers. Their format is the
**	char-sized tag, then the unsigned char length of the following data,
**	including the index, then the char-sized index and then the data itself.
*/
enum //	Indexed tags
{ kisNull = 0, //	Used to enclose comments or skip data.

    kisPlayerIPPort, //	size -> 6 (IP, then port), index -> player slot number (0 is server)
    kisPlayerNick, //	string data containing player nickname
    kisPlayerLives, //	char -> Player lives remaining (if tag is missing, player is not active)
    kisPlayerLocation, //	Point (pair of shorts) containing high-order words of player longitude/latitude.

    kisCount };

enum //	ktsCommand list
{ ktcQuery = 0, //	Tracker client sends queries
    ktcResponse, //	Tracker sends a response to query

    ktcHello, //	Server reports that it exists (periodical)
    ktcGoodbye //	Server signs off.
};

enum //	Status of game
{ ktgsNotLoaded, //	Game aborted/ended or never started.
    ktgsLoaded, //	Level has been loaded on server.
    ktgsActive, //	Server host is playing.
    ktgsPaused //	Server has paused.
};
