/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: AvaraScoreInterface.h
    Created: Sunday, May 5, 1996, 12:28
    Modified: Monday, August 26, 1996, 10:03
*/

#pragma once

#include "Types.h"

#define kScoreInterfaceFileType 'AVSI'
#define kScoreInterfaceCodeType 'SCIF'
#define kScoreInterfaceCodeID 128

typedef enum {
    ksiInit, //	Called when the plug-in is loaded.
    ksiClose, //	Called before application quits or plug-in changes

    ksiScore, //	Some entity (player/computer) scored points in the game.
    ksiPlayerIntro, //	After start/resume, each player in the game is "introduced"

    ksiLevelLoaded, //	A new level was loaded.
    ksiLevelStarted, //	A level was started (frame 0)
    ksiLevelPaused, //	The game was paused
    ksiLevelRestarted, //	The game was resumed after a pause
    ksiLevelEnded, //	The game ends (for whatever reason)

    ksiResultsUpdate, //	The window needs an update...revise results text handle, if necessary
    ksiResultsDraw, //	Update-related draw. Only use this, if you do your own drawing
    ksiResultsShow, //	Custom results page is now visible...add controls & whatever
    ksiResultsHide, //	Custom results page needs to be hidden...hide/remove your controls.
    ksiResultsClick, //	A mouse click was detected in the custom results page

    ksiSave, //	User chose Save from a menu (not supported yet)
    ksiCopy, //	User chose Copy from a menu (not supported yet)
    ksiPrint, //	User chose Print from a menu (not supported yet)

    ksiConsoleText //	Called for console text messages...gives you a chance to "filter" them.

} ScoreInterfaceMessages;

//	Reasons why ksiScore call was made:
typedef enum {
    ksiNoReason = -1,

    ksiPlasmaHit,
    ksiGrenadeHit,
    ksiMissileHit,
    ksiMineBlast,
    ksiSelfDestructBlast,
    ksiObjectCollision, //	Running into a moving door for instance (not supported yet)
    ksiSecondaryDamage,

    ksiKillBonus,
    ksiExitBonus,
    ksiGoodyBonus,

    ksiHoldBall,
    ksiScoreGoal

} ScoreInterfaceReasons;

//	The following will be used for enabling menus. Set the capabilities bits accordingly.
typedef enum {
    ksiCanSave = 1, //	Receive "Save" commands
    ksiCanCopy = 2, //	Can copy to clipboard
    ksiCanPrint = 4 //	Can print (currently unsupported).

} ScoreInterfaceCapabilities;

typedef struct {
    long command; //	One of ScoreInterfaceMessages
    long result; //	Reserved for future calls that may return results.
    long capabilities; //	Your capabilities.
    Handle plugIn; //	Your plug-in code handle

    long maxPlayers; //	ksiInit sets this. (current default is 6)
    long maxTeams; //	ksiInit sets this. (current default is 6) Doesn't include neutral team!!

    FrameTime frameTime; //	Time for each frame in milliseconds.
    FrameNumber frameNumber; //	Number of current frame (starts at 0)

    /*
    **	The results are stored in a text handle called resultsHandle.
    **	It should always be a valid handle, but can be changed to point
    **	to a new handle, if necessary. If you do change it to a new Handle,
    **	 you are responsible for disposing the old contents.
    **
    **	When you change the contents of the handle (which can be at any
    **	time), please set resultsChanged to true. This will be detected
    **	by Avara and the area in the Mission window will be invalidated.
    **	If you change it in a ksiResultsUpdate call, the results will not
    **	be drawn immediately (a new update event will be generated) and
    **	you will receive another ksiResultsUpdate call. This can repeat
    **	forever unless you are carelessly setting the resultsChanged flag
    **	even when the text has not changed.
    */
    Handle resultsHandle;
    long resultsChanged;

    /*
    **	Alternatively, you can draw the results yourself when you receive a
    **	ksiResultsDraw call. If you add any controls to the window, you have
    **	to remove them when you are closed and you have to hide them when you
    **	receive a ksiResultsHide call.
    */
    /*
    WindowPtr		resultsWindow;	//	Guaranteed to be valid only for hide/draw/click
    Rect			resultsRect;	//	Valid only for draw/click calls
    EventRecord		*theEvent;
    */

    /*
    **	The following parameters are valid for all calls and usually change
    **	only at ksiLevelLoaded calls.
    */
    std::string levelName; //	Always pointer to current level name.
    std::string levelCreator;
    std::string levelDescription;
    std::string directory; //   mission collection identifier

    /*
    **	The following are valid only for ksiPlayerIntro, ksiPlayerNameChange
    **	ksiScore and ksiKilled calls. You can ignore ksiPlayerNameChange calls,
    **	if you want to use the names the the players had at the start of a level.
    */
    long playerID; //	From 0 to 6 (0 is computer team, 1-6 are player teams. Unless Avara is changed!)
    long playerTeam; //	From -1 to 5 (0 to 5 are players. Unless Avara is changed!)
    long playerLives; //	Not valid when playerID == -1 (computer)
    StringPtr playerName; //	Player name, when appropriate
    FrameNumber winFrame; //	Frame at which player exited (won) or -1.

    /*
    **	The following are only valid for ksiScore calls:
    */
    long scorePoints; //	How many points are awarded.
    Fixed scoreEnergy; //	Only when this makes sense. (Energy blast caused damage)
    long scoreReason; //	What happened, why were the points given...see enum.
    long scoreTeam; //	Which team was hit?
    long scoreID; //	Who was hit? (-1 for computers)

    /*
    **	Console text:
    */
    StringPtr consoleLine; //	Text content of console line
    long consoleJustify; //	Which way to justify text -1, 0, 1 (right, center, left)

} ScoreInterfaceRecord;

typedef struct {
    std::string player;                 // Player who caused the event
    std::string playerTarget;           // Player affected by the event

    short team;                         // Team that created the event
    short teamTarget;                   // Team affected by the event

    long damage;                        // Damage caused for the event
    FrameNumber frameNumber;            // Frame number the event occurred
    int gameId;                         // Frame number the event occurred
    ScoreInterfaceReasons scoreType;    // Weapon that caused the event
    ScoreInterfaceReasons weaponUsed;   // Weapon that caused the event

} ScoreInterfaceEvent;

typedef void ScoreInterfaceCallType(ScoreInterfaceRecord *rec);
