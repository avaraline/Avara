/*
    Copyright ©1994-1997, Juri Munkki
    All rights reserved.

    File: CommandList.h
    Created: Saturday, November 19, 1994, 23:49
    Modified: Tuesday, December 23, 1997, 03:27
*/

#pragma once

enum {
    kNullCmd,
    kNewCmd,
    kOpenCmd,
    kCloseCmd,
    kSaveCmd,
    kSaveAsCmd,
    kQuitCmd,
    kOpenApplicationCmd,

    kAboutCmd = 10,

    kUndoCmd = 20,
    kCutCmd = 22,
    kCopyCmd,
    kPasteCmd,

    kShowKeyEditorCmd = 100,
    kChatAndPlayCmd,
    kKeyEditorClosed,
    kJoystickModeCmd = 110,
    kFlipAxisCmd,

    kLowSensitivityCmd,
    kNormalSensitivityCmd,
    kHighSensitivityCmd,

    kSimpleHorizonCmd = 120,
    kLowDetailHorizonCmd,
    kHighDetailHorizonCmd,

    kAmbientHologramsCmd,

    kGameOptionsDialogCmd,

    kStartGame = 200,
    kResumeGame,
    kShowGameWind,
    kShowRosterWind,
    kShowInfoPanel,
    kShowLevelListWind,
    kPrepareShowLevelListWind,

    kUseDefaultDirectory = 250,
    kAutoSaveScores,
    kAllowBackgroundProcessCmd,

    kMuteSoundCmd = 300,
    kMonoSoundCmd,
    kStereoHeadphonesCmd,
    kStereoSpeakersCmd,

    k16BitMixCmd = 310,

    kAmbientToggleCmd = 320,
    kTuijaToggleCmd,
    kSoundInterpolateCmd,
    kMissileSoundLoopCmd,
    kFootStepSoundCmd,
    kSoundOptionsDialogCmd,

    kPPCToolboxNetCmd = 330,
    kDDPNetCmd,
    kDDPBroadcastNetCmd,

    kMacTCPNetCmd,

    kServerOptionsDialogCmd = 340,
    kAutoLatencyCmd,
    kBellArrivalsCmd,
    kBellSelectCmd,
    kReconfigureNetCmd,

    kLatencyToleranceZero = 350,
    kLatencyToleranceMax = 358,

    kRetransmitMin = 360,
    kRetransmitMax = 363,

    kSlowestConnectionCmd = 370,
    kFastestConnectionCmd = 373,

    kConnectToServer = 400,
    kStartServer,
    kDisconnectFromServer,
    kStopServer,
    kNetChangedCmd,
    kReportNameCmd,
    kLevelDirectoryChangedCmd,
    kGetReadyToStartCmd,
    kGameResultAvailableCmd,

    kConfigurePlayerCmd,
    kGiveMugShotCmd,
    kPastePlayerPictCmd,

    kYesCmd = 1000,
    kCancelCmd,
    kNoCmd,

    kInternalCommandNumbers = 64000,
    kSubTargetCmd, //	Used internally for subpanes.

    kBusyStartCmd,
    kBusyTimeCmd,
    kBusyEndCmd,
    kBusyHideCmd
};
