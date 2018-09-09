/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: ComputerVoice.h
    Created: Monday, December 4, 1995, 02:44
    Modified: Sunday, June 30, 1996, 15:48
*/

#pragma once

enum {
    kvWelcomeToAvara, //	Welcome to Avara
    kvGameOver, //	Game Over
    kvThankYou, //	Thank you for playing
    kvYouWin, //	You win!
    kvMissionCompleted, //	Mission completed
    kvTenSeconds, //	Ten seconds [to generator shutdown]
    kvSixtySeconds, //	Sixty seconds [to generator shutdown]
    kvToGenerator, //	to generator shutdown
    kvShieldsLow, //	Shields low

    kvChannelOpen, //	Channel open	(Message channel to other players)
    kvChannelClosed, //	Channel closed	(Message channel to other players)

    kvMissiles, //	Missiles armed
    kvGrenades, //	Grenades armed

    kvBoost, //	No boosterpacks available.

    kvScout, //	Scout
    kvReleased, //		released
    kvLeft, //		left
    kvRight, //		right
    kvForward, //		forward
    kvBehind, //		follow
    kvStation, //		station/stationary
    kvDestroyed, //		destroyed

    kvVoiceCount
};
