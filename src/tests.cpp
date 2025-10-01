#include "gtest/gtest.h"
#include <SDL2/SDL_log.h>
#include "BasePath.h"
#include "AssetManager.h"
#include "CAvaraApp.h"
#include "CBSPPart.h"
#include "CPlayerManager.h"
#include "CNetManager.h"
#include "CAvaraGame.h"
#include "CWalkerActor.h"
#include "CBSPWorld.h"
#include <nanogui/nanogui.h>
#include "FastMat.h"
#include "Parser.h"
#include "CSoundHub.h"
#include "CGrenade.h"
#include "CSmart.h"
#include "CScout.h"
#include "Messages.h"
#include "Preferences.h"
#include "System.h"
#include "CUDPConnection.h"
#include "NullRenderer.h"
#include "Material.h"

#include <iostream>
using namespace std;

// assuming 1 unit in Avara is roughly 1 meter (hector is 1.75 tall)
#define MILLIMETER (FIX1/1000)

class TestSoundHub : public CSoundHubImpl {
public:
    virtual Fixed* EarLocation() { return ear; }
    virtual bool AudioEnabled() { return false; }
private:
    Fixed ear[3];

};
class TestPlayerManager : public CPlayerManager {
public:
    TestPlayerManager(CAvaraGame* game) {
        itsGame = game;
        playa = 0;
        this->ft = new FunctionTable();
        FunctionTable &ft = *(this->ft);
        ft.down = ft.up = ft.held = ft.mouseDelta.h = ft.mouseDelta.v = ft.buttonStatus = ft.msgChar = 0;
    }
    virtual ~TestPlayerManager() {}
    virtual CAbstractPlayer* GetPlayer() { return playa; }
    virtual void SetPlayer(CAbstractPlayer* p) { playa = p; }
    virtual short Slot() { return 0; }
    virtual void SetLocal() {};
    virtual void AbortRequest() {}
    virtual void RemoveFromGame() {}
    virtual Boolean IsLocalPlayer() { return true; }
    virtual Boolean CalculateIsLocalPlayer() { return true; }
    virtual void GameKeyPress(char c) {}
    virtual FunctionTable *GetFunctions() { return ft; }
    virtual void DeadOrDone() {}
    virtual bool IsDeadOrDone() { return false; }
    virtual short Position() { return 0; }
    virtual Str255& PlayerName() { return str; }
    virtual std::string GetPlayerName() { return std::string((char *)str + 1, str[0]); }
    virtual std::deque<char>& LineBuffer() { return lineBuffer; }
    virtual void Dispose() {}
    virtual void NetDisconnect() {}
    virtual short IsRegistered() { return 0; }
    virtual void IsRegistered(short) {}
    virtual Str255& PlayerRegName() { return str; }
    virtual LoadingState LoadingStatus() { return kLNotConnected; }
    virtual PresenceType Presence() { return kzAvailable; }
    virtual void SetPresence(PresenceType pt) { }
    virtual void LoadStatusChange(short serverCRC, OSErr serverErr, std::string serverTag) {}
    virtual void SetPlayerStatus(LoadingState newStatus, PresenceType newPresence, FrameNumber theWin) {}
    virtual bool IsAway() { return false; };
    virtual bool IsSpectating() { return false; };
    virtual bool IsLoaded() { return true; };
    virtual bool IsReady() { return false; };
    virtual void ChangeName(StringPtr theName) {}
    virtual void SetPosition(short pos) {}

    virtual void RosterKeyPress(unsigned char c) {}
    virtual void RosterMessageText(short len, const char *c) {}
    virtual short LevelCRC() { return 0; }
    virtual OSErr LevelErr() { return noErr; }
    virtual std::string LevelTag() { return 0; }
    virtual void LevelCRC(short) {}
    virtual void LevelErr(OSErr) {}
    virtual void LevelTag(std::string) {}
    virtual void ResumeGame() {}
    virtual uint32_t DoMouseControl(Point *deltaMouse, Boolean doCenter) { return 0; }
    virtual void HandleEvent(SDL_Event &event) {}
    virtual void SendFrame() { itsGame->topSentFrame++; }
    virtual void ViewControl() {}
    virtual Fixed RandomKey() { return 0;}
    virtual void RandomKey(Fixed) {}
    virtual CAbstractPlayer *ChooseActor(CAbstractPlayer *actorList, short myTeamColor) { return 0; }
    virtual short GetStatusChar() { return 0; }
    virtual short GetMessageIndicator() { return 0; }
    virtual std::string GetChatString(int maxChars) { return ""; }
    virtual std::string GetChatLine() { return ""; }
    virtual void StoreMugShot(Handle mugHandle) {}
    virtual Handle GetMugShot() { return 0; }

    virtual CAbstractPlayer *TakeAnyActor(CAbstractPlayer *actorList) { return 0; }
    virtual short PlayerColor() { return 0; }
    virtual Boolean IncarnateInAnyColor() { return false; }
    virtual void ResendFrame(FrameNumber theFrame, short requesterId, short commandCode) {}
    virtual void SpecialColorControl() {}
    virtual PlayerConfigRecord& TheConfiguration() { return pcr; }
    virtual Handle MugPict() { return 0; }
    virtual void MugPict(Handle) {}
    virtual long MugSize() { return 0; }
    virtual long MugState() { return 0; }
    virtual void MugSize(long) {}
    virtual void MugState(long) {}
    virtual FrameNumber WinFrame() { return 0; }
    virtual void ProtocolHandler(struct PacketInfo *thePacket) {}
    virtual void IncrementAskAgainTime(int) {}
    virtual void SetShowScoreboard(bool b) {}
    virtual bool GetShowScoreboard() { return false; }

private:
    FunctionTable *ft;
    CAvaraGame *itsGame;
    CAbstractPlayer *playa;
    std::deque<char> lineBuffer;
    Str255 str;
    PlayerConfigRecord pcr;
};

class TestNetManager : public CNetManager {
public:
    std::shared_ptr<CPlayerManager> CreatePlayerManager(short id) {
        return std::make_shared<TestPlayerManager>(itsGame);
    }
};

class TestApp : public CAvaraApp {
public:
    TestApp() {
        AssetManager::Init();
        
        // Silence SDL logging with a no-op callback.
        SDL_LogSetOutputFunction([](void *d, int c, SDL_LogPriority p, const char *m){}, nullptr);
    }
    virtual bool DoCommand(int theCommand) {return false;}
    virtual void MessageLine(short index, MsgAlignment align) {}
    virtual void AddMessageLine(std::string lines, MsgAlignment align = MsgAlignment::Left, MsgCategory category = MsgCategory::System) {}
    virtual void RenderContents() {};
    virtual void DrawUserInfoPart(short i, short partList) {}
    virtual void ParamLine(short index, MsgAlignment align, StringPtr param1, StringPtr param2) {}
    virtual void StartFrame(FrameNumber frameNum) {}
    virtual void BrightBox(FrameNumber frameNum, short position) {}
    virtual void LevelReset() {}
    virtual long Number(const std::string name) { return 0; }
    virtual bool Boolean(const std::string name) { return false; }
    virtual OSErr LoadLevel(std::string set, std::string leveltag, CPlayerManager* sendingPlayer) { return noErr; }
    virtual void ComposeParamLine(StringPtr destStr, short index, StringPtr param1, StringPtr param2) {}
    virtual void NotifyUser() {}
    virtual json Get(const std::string name) { return ReadDefaultPrefs().at(name); }
    template <class T> T Get(const std::string name) {
        return static_cast<T>(ReadDefaultPrefs().at(name));
    }
    virtual void Set(const std::string name, const std::string value) {}
    virtual void Set(const std::string name, long value) {}
    virtual void Set(const std::string name, json value) {}
    virtual CNetManager* GetNet() { return itsNet; }
    virtual void SetNet(CNetManager* net) { itsNet = net; }
    virtual SDL_Window* sdlWindow() { return 0; }
    virtual void StringLine(std::string theString, MsgAlignment align) {}
    virtual CAvaraGame* GetGame() { return 0; }
    virtual void Done() {}
    virtual void BroadcastCommand(int) {}
    virtual void GameStarted(LevelInfo &loadedLevel) {};
    virtual std::deque<MsgLine>& MessageLines() { return msgLines; }
    virtual CommandManager* GetTui() { return 0; }
    virtual uint32_t ControllerEventType() { return 0; }
    virtual void Rumble(Fixed hitEnergy) {}
public:
    std::unique_ptr<CAvaraGame> itsGame;
private:
    CNetManager *itsNet;
    std::deque<MsgLine> msgLines;
};

class TestGame : public CAvaraGame {
public:
    TestGame(int frameTime) : CAvaraGame(frameTime) {}
    virtual std::unique_ptr<CNetManager> CreateNetManager() { return std::make_unique<TestNetManager>(); }
    virtual CSoundHub* CreateSoundHub() { TestSoundHub *t = new TestSoundHub(); t->ISoundHub(64,64); return t;}
    bool GameTick() {
        // force tick to happen by resetting nextScheduledFrame
        nextScheduledFrame = 0;
        itsNet->activePlayersDistribution = 1;
        nextPingTime = std::numeric_limits<uint32_t>::max(); // prevent pings
        return CAvaraGame::GameTick();
    }
};

class TestWalkerActor : public CWalkerActor {
    void ResetCamera() {

    }
};

class HectorTestScenario {
public:
    TestApp app;
    TestGame *game;
    CWalkerActor *hector;

    HectorTestScenario(int frameTime, Fixed hectorX, Fixed hectorY, Fixed hectorZ) {
        app.itsGame = std::make_unique<TestGame>(frameTime);
        game = (TestGame*)app.itsGame.get();
        gCurrentGame = game;
        gRenderer = new NullRenderer();

        InitParser();
        game->IAvaraGame(&app);
        game->EndScript();
        app.GetNet()->ChangeNet(kNullNet, "");
        game->LevelReset(false);
        hector = new TestWalkerActor();
        hector->BeginScript();
        hector->EndScript();
        game->itsNet->playerTable[0]->SetPlayer(hector);
        hector->itsManager = game->itsNet->playerTable[0];
        hector->location[0] = hectorX;
        hector->location[1] = hectorY;
        hector->location[2] = hectorZ;
        hector->location[3] = FIX1;
        game->AddActor(hector);
        game->freshPlayerList = 0;
        game->GameStart();
    }
};

class HectorEnergyReadings {
public:
    Fixed energy;
    Fixed shields;
    Fixed gunEnergy1;
    Fixed gunEnergy2;

    HectorEnergyReadings(CWalkerActor *hector) {
        energy = hector->energy;
        shields = hector->shields;
        gunEnergy1 = hector->gunEnergy[0];
        gunEnergy2 = hector->gunEnergy[1];
    }
};

inline int GetTicksPerStep(int frameTime) {
    return CLASSICFRAMETIME / frameTime;
}

vector<Fixed> DropHector(int steps, Fixed fromHeight, int frameTime) {
    HectorTestScenario scenario(frameTime, 0, fromHeight, 0);
    vector<Fixed> altitudes;
    int ticksPerStep = GetTicksPerStep(frameTime);

    for (int i = 0; i < steps; i++) {
        for (int k = 0; k < ticksPerStep; k++) {
            scenario.game->GameTick();
        }
        altitudes.push_back(scenario.hector->location[1]);
    }
    return altitudes;
}

vector<VectorStruct> WalkHector(int settleSteps, int steps, int frameTime) {
    HectorTestScenario scenario(frameTime, 0, 0, 0);
    vector<VectorStruct> location;
    int ticksPerStep = GetTicksPerStep(frameTime);

    for (int i = 0; i < settleSteps; i++) {
        for (int k = 0; k < ticksPerStep; k++) {
            scenario.game->GameTick();
        }
    }

    for (int i = 0; i < steps + 1; i++) {
        for (int k = 0; k < ticksPerStep; k++) {
            scenario.hector->itsManager->GetFunctions()->held = (1 << kfuForward);
            scenario.game->GameTick();
        }
        location.push_back(*(VectorStruct*)scenario.hector->location);
    }

    return location;
}

vector<VectorStruct> JumpHector(int settleSteps, int jumpHoldSteps, int steps, int frameTime, int numHeldKeys) {
    HectorTestScenario scenario(frameTime, 0, 0, 0);
    vector<VectorStruct> location;
    int ticksPerStep = GetTicksPerStep(frameTime);

    for (int i = 0; i < settleSteps * ticksPerStep; i++) {
        scenario.game->GameTick();
    }

    scenario.hector->itsManager->GetFunctions()->down = (1 << kfuJump);
    scenario.game->GameTick();
    scenario.hector->itsManager->GetFunctions()->down = 0;

    for (int i = 0; i < jumpHoldSteps * ticksPerStep; i++) {
        scenario.hector->itsManager->GetFunctions()->held = (1 << kfuJump);
        scenario.game->GameTick();
    }

    int frame = 0;
    for (int i = 0; i < steps; i++) {

        for (int k = 0; k < ticksPerStep; k++, frame++) {
            // keep holding keys down for numHeldKeys-1 frames
            scenario.hector->itsManager->GetFunctions()->held = frame < (numHeldKeys - 1) ? (1 << kfuJump) : 0;
            // release a key "up" in each of the first numHeldKeys frames
            scenario.hector->itsManager->GetFunctions()->up = frame < numHeldKeys ? (1 << kfuJump) : 0;

            scenario.game->GameTick();
        }
        location.push_back(*(VectorStruct*)scenario.hector->location);
        // std::cout << "jump location[" << i << "] = " << FormatVector(scenario.hector->location, 3)
        //           << ", speed[" << i << "] = " << FormatVector(scenario.hector->speed, 3) << std::endl;
    }

    return location;
}

vector<VectorStruct> FireGrenade(int settleSteps, int steps, int frameTime) {
    HectorTestScenario scenario(frameTime, 0, 0, 0);
    vector<VectorStruct> trajectory;
    int ticksPerStep = GetTicksPerStep(frameTime);

    for (int i = 0; i < settleSteps; i++) {
        for (int k = 0; k < ticksPerStep; k++) {
            scenario.game->GameTick();
        }
    }

    scenario.hector->itsManager->GetFunctions()->down = (1 << kfuLoadGrenade) | (1 << kfuFireWeapon);
    scenario.game->GameTick();
    scenario.hector->itsManager->GetFunctions()->down = 0;

    CGrenade *grenade = 0;
    // hopefully there are two objects now. a hector and a grenade.
    for (CAbstractActor *aa = scenario.game->actorList; aa; aa = aa->nextActor) {
        if (typeid(*aa) == typeid(CGrenade)) {
            grenade = (CGrenade*)aa;
        }
    }

    for (int i = 0; i < steps && grenade != 0; i++) {
        trajectory.push_back(*(VectorStruct*)grenade->location);
        for (int k = 0; k < ticksPerStep; k++) {
            scenario.game->GameTick();
        }

        // this intends to figure out whether the grenade has exploded.
        grenade = 0;
        for (CAbstractActor *aa = scenario.game->actorList; aa; aa = aa->nextActor) {
            if (typeid(*aa) == typeid(CGrenade)) {
                grenade = (CGrenade*)aa;
            }
        }
    }

    return trajectory;
}

vector<VectorStruct> FireMissile(int hectorSettle, int scoutSettle, int steps, int frameTime) {
    HectorTestScenario scenario(frameTime, 0, 0, 0);
    vector<VectorStruct> trajectory;
    int ticksPerStep = GetTicksPerStep(frameTime);

    for (int i = 0; i < hectorSettle*ticksPerStep; i++) {
        scenario.game->GameTick();
    }

    // load missile
    scenario.hector->itsManager->GetFunctions()->down = (1 << kfuLoadMissile);
    scenario.game->GameTick();
    scenario.hector->itsManager->GetFunctions()->down = 0;

    // raise head: 20 degrees (negative delta.v is up)
    scenario.hector->itsManager->GetFunctions()->mouseDelta.v = -20 / 0.03125;
    scenario.game->GameTick();
    scenario.hector->itsManager->GetFunctions()->mouseDelta.v = 0;

    // scout forward (will be targeted on the way forward)
    scenario.hector->itsManager->GetFunctions()->held = (1 << kfuScoutControl);
    scenario.hector->itsManager->GetFunctions()->down = (1 << kfuForward);
    scenario.hector->itsManager->GetFunctions()->up = (1 << kfuScoutControl);
    scenario.game->GameTick();
    scenario.hector->itsManager->GetFunctions()->held = 0;
    scenario.hector->itsManager->GetFunctions()->down = 0;
    scenario.hector->itsManager->GetFunctions()->up = 0;

    // wait for scout to be in place
    for (int i = 0; i < scoutSettle*ticksPerStep; i++) {
        scenario.game->GameTick();
    }

    // Find the Missile in the actor list
    CSmart *missile = NULL;
    for (CAbstractActor *aa = scenario.game->actorList; aa; aa = aa->nextActor) {
        if (typeid(*aa) == typeid(CSmart)) {
            missile = (CSmart*)aa;
        }
    }

    // aim left: 90 degrees (so it curves towards the target)
    scenario.hector->itsManager->GetFunctions()->mouseDelta.h = 90/.0625;
    scenario.game->GameTick();
    scenario.hector->itsManager->GetFunctions()->mouseDelta.h = 0;

    // fire missile
    scenario.hector->itsManager->GetFunctions()->down = (1 << kfuFireWeapon);
    scenario.game->GameTick();
    scenario.hector->itsManager->GetFunctions()->down = 0;

    // see where the missile goes!
    for (int i = 0; i < steps && missile != NULL; i++) {
        trajectory.push_back(*(VectorStruct*)missile->location);
        // std::cout << "missile location[" << i << "] = " << FormatVector(missile->location, 3)
        //           << ", speed[" << i << "] = " << FormatVector(missile->speed, 3) << std::endl;
        for (int k = 0; k < ticksPerStep; k++) {
            scenario.game->GameTick();
        }

        // figure out whether the missile has exploded.
        missile = NULL;
        for (CAbstractActor *aa = scenario.game->actorList; aa; aa = aa->nextActor) {
            if (typeid(*aa) == typeid(CSmart)) {
                missile = (CSmart*)aa;
            }
        }
    }

    return trajectory;
}

vector<Fixed> TurnHector(int steps, int frameTime) {
    HectorTestScenario scenario(frameTime, 0, 0, 0);
    vector<Fixed> headings;
    int ticksPerStep = GetTicksPerStep(frameTime);

    for (int i = 0; i < steps; i++) {
        headings.push_back(scenario.hector->heading);
        for (int k = 0; k < ticksPerStep; k++) {
            scenario.hector->itsManager->GetFunctions()->held = (1 << kfuRight);
            scenario.game->GameTick();
        }
    }
    headings.push_back(scenario.hector->heading);
    return headings;
}

vector<HectorEnergyReadings> HectorEnergyRegen(int steps, bool useBoost, int frameTime) {
    HectorTestScenario scenario(frameTime, 0, 0, 0);
    vector<HectorEnergyReadings> energyValues;
    int ticksPerStep = GetTicksPerStep(frameTime);

    scenario.hector->energy = scenario.hector->maxEnergy * 0.5;

    for (int i = 0; i < steps; i++) {
        if (i == 1 && useBoost) {
            scenario.hector->itsManager->GetFunctions()->down = (1 << kfuBoostEnergy);
        }
        HectorEnergyReadings current(scenario.hector);
        energyValues.push_back(current);
        for (int k = 0; k < ticksPerStep; k++) {
            scenario.game->GameTick();
        }
    }

    return energyValues;
}

vector<HectorEnergyReadings> HectorPlasmaRegen(int steps, bool useBoost, int frameTime) {
    HectorTestScenario scenario(frameTime, 0, 0, 0);
    vector<HectorEnergyReadings> energyValues;
    int ticksPerStep = GetTicksPerStep(frameTime);

    scenario.hector->gunEnergy[0] = 0;
    scenario.hector->gunEnergy[1] = 0;

    for (int i = 0; i < steps; i++) {
        if (i == 1 && useBoost) {
            scenario.hector->itsManager->GetFunctions()->down = (1 << kfuBoostEnergy);
        }
        HectorEnergyReadings current(scenario.hector);
        energyValues.push_back(current);
        for (int k = 0; k < ticksPerStep; k++) {
            scenario.game->GameTick();
        }
    }

    return energyValues;
}

vector<HectorEnergyReadings> HectorShieldRegen(int steps, bool useBoost, int frameTime) {
    HectorTestScenario scenario(frameTime, 0, 0, 0);
    vector<HectorEnergyReadings> energyValues;
    int ticksPerStep = GetTicksPerStep(frameTime);

    scenario.hector->shields = scenario.hector->maxShields * 0.5;

    for (int i = 0; i < steps; i++) {
        if (i == 1 && useBoost) {
            scenario.hector->itsManager->GetFunctions()->down = (1 << kfuBoostEnergy);
        }
        HectorEnergyReadings current(scenario.hector);
        energyValues.push_back(current);
        for (int k = 0; k < ticksPerStep; k++) {
            scenario.game->GameTick();
        }
    }

    return energyValues;
}

vector<HectorEnergyReadings> HectorComplexRegen(int steps, bool useBoost, int frameTime) {
    HectorTestScenario scenario(frameTime, 0, 0, 0);
    vector<HectorEnergyReadings> energyValues;
    int ticksPerStep = GetTicksPerStep(frameTime);

    scenario.hector->energy = scenario.hector->maxEnergy * 0.5;
    scenario.hector->shields = scenario.hector->maxShields * 0.2;
    scenario.hector->gunEnergy[0] = scenario.hector->gunEnergy[0] * 0.3;
    scenario.hector->gunEnergy[1] = scenario.hector->gunEnergy[1] * 0.1;
    if (useBoost) {
        scenario.hector->itsManager->GetFunctions()->down = (1 << kfuBoostEnergy);
    }

    for (int i = 0; i < steps; i++) {
        HectorEnergyReadings current(scenario.hector);
        energyValues.push_back(current);
        for (int k = 0; k < ticksPerStep; k++) {
            scenario.game->GameTick();
        }
    }

    return energyValues;
}

TEST(HECTOR, Gravity) {
    int dropSteps = 50;
    vector<Fixed> at64ms = DropHector(dropSteps, FIX(200), 64);
    vector<Fixed> at32ms = DropHector(dropSteps, FIX(200), 32);
    vector<Fixed> at16ms = DropHector(dropSteps, FIX(200), 16);
    vector<Fixed> at8ms = DropHector(dropSteps, FIX(200), 8);

    ASSERT_EQ(at64ms.size(), dropSteps) << "not enough steps recorded at 64ms";
    ASSERT_EQ(at32ms.size(), dropSteps) << "not enough steps recorded at 32ms";
    ASSERT_EQ(at16ms.size(), dropSteps) << "not enough steps recorded at 16ms";
    ASSERT_EQ(at8ms.size(), dropSteps) << "not enough steps recorded at 8ms";
    ASSERT_EQ(at64ms.back(), 6125961) << "64ms simulation fell wrong amount";

    for (int i = 0; i < dropSteps; i++) {
        double f64 = ToFloat(at64ms[i]);
        double f32 = ToFloat(at32ms[i]);
        double f16 = ToFloat(at16ms[i]);
        double f8 = ToFloat(at8ms[i]);
        ASSERT_LT(abs(f64-f32)/f64, 0.003) << "not close enough after " << i << " ticks.";
        ASSERT_LT(abs(f64-f16)/f64, 0.004) << "not close enough after " << i << " ticks.";
        ASSERT_LT(abs(f64-f8)/f64, 0.005) << "not close enough after " << i << " ticks.";
    }
}

double VecStructDist(const VectorStruct &one, const VectorStruct &two) {
    double answer = 0;
    double temp;
    for (int i = 0; i < 3; i++) {
        temp = ToFloat(one.theVec[i]) - ToFloat(two.theVec[i]);
        answer += temp * temp;
    }
    return sqrt(answer);

}

TEST(HECTOR, TurnSpeed) {
    int turnSteps = 50;
    vector<Fixed> at64ms = TurnHector(turnSteps, 64);
    vector<Fixed> at32ms = TurnHector(turnSteps, 32);
    vector<Fixed> at16ms = TurnHector(turnSteps, 16);
    vector<Fixed> at8ms = TurnHector(turnSteps, 8);

    // TurnHector adds an additional step!
    turnSteps += 1;

    ASSERT_EQ(at64ms.size(), turnSteps) << "not enough steps recorded at 64ms";
    ASSERT_EQ(at32ms.size(), turnSteps) << "not enough steps recorded at 32ms";
    ASSERT_EQ(at16ms.size(), turnSteps) << "not enough steps recorded at 16ms";
    ASSERT_EQ(at8ms.size(), turnSteps) << "not enough steps recorded at 8ms";
    // within 0.01 degrees
    ASSERT_NEAR(at64ms.back(), -30542, FDegToOne(FIX(0.01))) << "64ms simulation turned wrong amount";

    for (int i = 0; i < turnSteps; i++) {
        ASSERT_LE(at64ms[i] > at32ms[i] ? at64ms[i] - at32ms[i] : at32ms[i] - at64ms[i], 100) << "32ms not close enough after " << i << " ticks.";
        ASSERT_LE(at64ms[i] > at16ms[i] ? at64ms[i] - at16ms[i] : at16ms[i] - at64ms[i], 120) << "16ms not close enough after " << i << " ticks.";
        ASSERT_LE(at64ms[i] > at8ms[i] ? at64ms[i] - at8ms[i] : at8ms[i] - at64ms[i], 120) << "8ms not close enough after " << i << " ticks.";
    }
}

TEST(HECTOR, WalkForwardSpeed) {
    int walkSteps = 50;
    vector<VectorStruct> at64ms = WalkHector(20, walkSteps, 64);
    vector<VectorStruct> at32ms = WalkHector(20, walkSteps, 32);
    vector<VectorStruct> at16ms = WalkHector(20, walkSteps, 16);
    vector<VectorStruct> at8ms = WalkHector(20, walkSteps, 8);

    // WalkHector adds an additional step!
    walkSteps += 1;

    ASSERT_EQ(at64ms.size(), walkSteps) << "not enough steps recorded at 64ms";
    ASSERT_EQ(at32ms.size(), walkSteps) << "not enough steps recorded at 32ms";
    ASSERT_EQ(at16ms.size(), walkSteps) << "not enough steps recorded at 16ms";
    ASSERT_EQ(at8ms.size(), walkSteps) << "not enough steps recorded at 8ms";
    ASSERT_EQ(at64ms.back().theVec[0], 0) << "64ms simulation walked wrong amount";
    ASSERT_NEAR(at64ms.back().theVec[2], 1584235, 3*MILLIMETER) << "64ms simulation walked wrong amount";

    for (int i = 0; i < walkSteps; i++) {
        // std::cout << "dist32[" << i << "] = " << VecStructDist(at64ms[i], at32ms[i]) << std::endl;
        // std::cout << "dist16[" << i << "] = " << VecStructDist(at64ms[i], at16ms[i]) << std::endl;
        // std::cout << "dist8[" << i << "] = " << VecStructDist(at64ms[i], at8ms[i]) << std::endl;
        ASSERT_LT(VecStructDist(at64ms[i], at32ms[i]), 0.04) << "not close enough after " << i << " ticks.";
        ASSERT_LT(VecStructDist(at64ms[i], at16ms[i]), 0.08) << "not close enough after " << i << " ticks.";
        ASSERT_LT(VecStructDist(at64ms[i], at8ms[i]), 0.12) << "not close enough after " << i << " ticks.";
    }
}

void test_jump(int peakStep, int peakHeight, int numHeldKeys) {
    int jumpSteps = 40;
    vector<VectorStruct> at64ms = JumpHector(20, 20, jumpSteps, 64, numHeldKeys);
    vector<VectorStruct> at32ms = JumpHector(20, 20, jumpSteps, 32, numHeldKeys);
    vector<VectorStruct> at16ms = JumpHector(20, 20, jumpSteps, 16, numHeldKeys);
    vector<VectorStruct> at8ms = JumpHector(20, 20, jumpSteps, 8, numHeldKeys);

    // peak of jump is near frame 6
    ASSERT_EQ(at64ms.size(), jumpSteps) << "not enough steps recorded at 64ms";
    ASSERT_EQ(at32ms.size(), jumpSteps) << "not enough steps recorded at 32ms";
    ASSERT_EQ(at16ms.size(), jumpSteps) << "not enough steps recorded at 16ms";
    ASSERT_EQ(at8ms.size(), jumpSteps) << "not enough steps recorded at 8ms";

    ASSERT_NEAR(at64ms[peakStep].theVec[1], peakHeight, 3*MILLIMETER) << "64ms simulation peaked with wrong amount";
    ASSERT_NEAR(at32ms[peakStep].theVec[1], peakHeight, 3*MILLIMETER) << "32ms simulation peaked with wrong amount";
    ASSERT_NEAR(at16ms[peakStep].theVec[1], peakHeight, 5*MILLIMETER) << "16ms simulation peaked with wrong amount";

    for (int i = 0; i < jumpSteps; i++) {
        // std::cout << "dist32[" << i << "] = " << VecStructDist(at64ms[i], at32ms[i]) << "\n";
        // std::cout << "dist16[" << i << "] = " << VecStructDist(at64ms[i], at16ms[i]) << "\n\n";
        // std::cout << "dist8[" << i << "] = " << VecStructDist(at64ms[i], at8ms[i]) << "\n\n";
        ASSERT_LT(VecStructDist(at64ms[i], at32ms[i]), 0.1) << "not close enough after " << i << " ticks.";
        ASSERT_LT(VecStructDist(at64ms[i], at16ms[i]), 0.1) << "not close enough after " << i << " ticks.";
    }
}

TEST(HECTOR, JumpRegular) {
    test_jump(5, 116100, 1);
}

TEST(HECTOR, JumpSuper) {
    // doesn't matter how many jump keys you press/release, superjump is always the same
    test_jump(6, 181800, 2);
    test_jump(6, 181800, 3);
}

TEST(HECTOR, EnergyRegen) {
    int regenSteps = 85;
    vector<HectorEnergyReadings> at64ms = HectorEnergyRegen(regenSteps, false, 64);
    vector<HectorEnergyReadings> at32ms = HectorEnergyRegen(regenSteps, false, 32);
    vector<HectorEnergyReadings> at16ms = HectorEnergyRegen(regenSteps, false, 16);
    vector<HectorEnergyReadings> at8ms = HectorEnergyRegen(regenSteps, false, 8);

    ASSERT_EQ(at64ms.size(), regenSteps) << "not enough steps recorded at 64ms";
    ASSERT_EQ(at32ms.size(), regenSteps) << "not enough steps recorded at 32ms";
    ASSERT_EQ(at16ms.size(), regenSteps) << "not enough steps recorded at 16ms";
    ASSERT_EQ(at8ms.size(), regenSteps) << "not enough steps recorded at 8ms";
    ASSERT_EQ(at64ms[0].energy, 163840) << "starting energy value incorrect at 64ms";
    ASSERT_EQ(at32ms[0].energy, 163840) << "starting energy value incorrect at 32ms";
    ASSERT_EQ(at16ms[0].energy, 163840) << "starting energy value incorrect at 16ms";
    ASSERT_EQ(at8ms[0].energy, 163840) << "starting energy value incorrect at 8ms";
    ASSERT_LT(at64ms[regenSteps - 2].energy, 327680) << "energy recharges too quickly at 64ms";
    ASSERT_EQ(at64ms[regenSteps - 1].energy, 327680) << "energy recharges too slowly at 64ms";
    ASSERT_EQ(at32ms[regenSteps - 1].energy, 327680) << "energy recharges too slowly at 32ms";
    ASSERT_EQ(at16ms[regenSteps - 1].energy, 327680) << "energy recharges too slowly at 16ms";
    ASSERT_EQ(at8ms[regenSteps - 1].energy, 327680) << "energy recharges too slowly at 8ms";

    for (int i = 0; i < regenSteps; i++) {
        // cout << at64ms[i] << "\t" << at32ms[i] << "\t" << at16ms[i] << "\t" << at8ms[i] << endl;
        ASSERT_NEAR(at32ms[i].energy, at64ms[i].energy, at64ms[i].energy * 0.001) << "32ms not close enough after " << i << " ticks.";
        ASSERT_NEAR(at16ms[i].energy, at64ms[i].energy, at64ms[i].energy * 0.001) << "16ms not close enough after " << i << " ticks.";
        ASSERT_NEAR(at8ms[i].energy, at64ms[i].energy, at64ms[i].energy * 0.001) << "8ms not close enough after " << i << " ticks.";

        ASSERT_EQ(at64ms[i].shields, 196608) << "64ms shields didn't remain full";
        ASSERT_EQ(at32ms[i].shields, 196608) << "32ms shields didn't remain full";
        ASSERT_EQ(at16ms[i].shields, 196608) << "16ms shields didn't remain full";
        ASSERT_EQ(at8ms[i].shields, 196608) << "8ms shields didn't remain full";

        ASSERT_EQ(at64ms[i].gunEnergy1, 52428) << "64ms plasma energy 1 didn't remain full";
        ASSERT_EQ(at32ms[i].gunEnergy1, 52428) << "32ms plasma energy 1 didn't remain full";
        ASSERT_EQ(at16ms[i].gunEnergy1, 52428) << "16ms plasma energy 1 didn't remain full";
        ASSERT_EQ(at8ms[i].gunEnergy1, 52428) << "8ms plasma energy 1 didn't remain full";

        ASSERT_EQ(at64ms[i].gunEnergy2, 52428) << "64ms plasma energy 2 didn't remain full";
        ASSERT_EQ(at32ms[i].gunEnergy2, 52428) << "32ms plasma energy 2 didn't remain full";
        ASSERT_EQ(at16ms[i].gunEnergy2, 52428) << "16ms plasma energy 2 didn't remain full";
        ASSERT_EQ(at8ms[i].gunEnergy2, 52428) << "8ms plasma energy 2 didn't remain full";
    }
}

TEST(HECTOR, BoostedEnergyRegen) {
    int regenSteps = 19;
    vector<HectorEnergyReadings> at64ms = HectorEnergyRegen(regenSteps, true, 64);
    vector<HectorEnergyReadings> at32ms = HectorEnergyRegen(regenSteps, true, 32);
    vector<HectorEnergyReadings> at16ms = HectorEnergyRegen(regenSteps, true, 16);
    vector<HectorEnergyReadings> at8ms = HectorEnergyRegen(regenSteps, true, 8);

    ASSERT_EQ(at64ms.size(), regenSteps) << "not enough steps recorded at 64ms";
    ASSERT_EQ(at32ms.size(), regenSteps) << "not enough steps recorded at 32ms";
    ASSERT_EQ(at16ms.size(), regenSteps) << "not enough steps recorded at 16ms";
    ASSERT_EQ(at8ms.size(), regenSteps) << "not enough steps recorded at 8ms";
    ASSERT_EQ(at64ms[0].energy, 163840) << "starting energy value incorrect at 64ms";
    ASSERT_EQ(at32ms[0].energy, 163840) << "starting energy value incorrect at 32ms";
    ASSERT_EQ(at16ms[0].energy, 163840) << "starting energy value incorrect at 16ms";
    ASSERT_EQ(at8ms[0].energy, 163840) << "starting energy value incorrect at 8ms";
    ASSERT_LT(at64ms[regenSteps - 2].energy, 327680) << "energy recharges too quickly at 64ms";
    ASSERT_EQ(at64ms[regenSteps - 1].energy, 327680) << "energy recharges too slowly at 64ms";
    ASSERT_EQ(at32ms[regenSteps - 1].energy, 327680) << "energy recharges too slowly at 32ms";
    ASSERT_EQ(at16ms[regenSteps - 1].energy, 327680) << "energy recharges too slowly at 16ms";
    ASSERT_EQ(at8ms[regenSteps - 1].energy, 327680) << "energy recharges too slowly at 8ms";

    for (int i = 0; i < regenSteps; i++) {
        // cout << at64ms[i] << "\t" << at32ms[i] << "\t" << at16ms[i] << "\t" << at8ms[i] << endl;
        ASSERT_NEAR(at32ms[i].energy, at64ms[i].energy, at64ms[i].energy * 0.045) << "32ms not close enough after " << i << " ticks.";
        ASSERT_NEAR(at16ms[i].energy, at64ms[i].energy, at64ms[i].energy * 0.045) << "16ms not close enough after " << i << " ticks.";
        ASSERT_NEAR(at8ms[i].energy, at64ms[i].energy, at64ms[i].energy * 0.045) << "8ms not close enough after " << i << " ticks.";

        ASSERT_EQ(at64ms[i].shields, 196608) << "64ms shields didn't remain full";
        ASSERT_EQ(at32ms[i].shields, 196608) << "32ms shields didn't remain full";
        ASSERT_EQ(at16ms[i].shields, 196608) << "16ms shields didn't remain full";
        ASSERT_EQ(at8ms[i].shields, 196608) << "8ms shields didn't remain full";

        ASSERT_EQ(at64ms[i].gunEnergy1, 52428) << "64ms plasma energy 1 didn't remain full";
        ASSERT_EQ(at32ms[i].gunEnergy1, 52428) << "32ms plasma energy 1 didn't remain full";
        ASSERT_EQ(at16ms[i].gunEnergy1, 52428) << "16ms plasma energy 1 didn't remain full";
        ASSERT_EQ(at8ms[i].gunEnergy1, 52428) << "8ms plasma energy 1 didn't remain full";

        ASSERT_EQ(at64ms[i].gunEnergy2, 52428) << "64ms plasma energy 2 didn't remain full";
        ASSERT_EQ(at32ms[i].gunEnergy2, 52428) << "32ms plasma energy 2 didn't remain full";
        ASSERT_EQ(at16ms[i].gunEnergy2, 52428) << "16ms plasma energy 2 didn't remain full";
        ASSERT_EQ(at8ms[i].gunEnergy2, 52428) << "8ms plasma energy 2 didn't remain full";
    }
}

TEST(HECTOR, PlasmaRegen) {
    int chargeSteps = 30;
    vector<HectorEnergyReadings> at64ms = HectorPlasmaRegen(chargeSteps, false, 64);
    vector<HectorEnergyReadings> at32ms = HectorPlasmaRegen(chargeSteps, false, 32);
    vector<HectorEnergyReadings> at16ms = HectorPlasmaRegen(chargeSteps, false, 16);
    vector<HectorEnergyReadings> at8ms = HectorPlasmaRegen(chargeSteps, false, 8);

    ASSERT_EQ(at64ms.size(), chargeSteps) << "not enough steps recorded at 64ms";
    ASSERT_EQ(at32ms.size(), chargeSteps) << "not enough steps recorded at 32ms";
    ASSERT_EQ(at16ms.size(), chargeSteps) << "not enough steps recorded at 16ms";
    ASSERT_EQ(at8ms.size(), chargeSteps) << "not enough steps recorded at 8ms";

    ASSERT_EQ(at64ms[0].gunEnergy1, 0) << "starting plasma energy value 1 incorrect at 64ms";
    ASSERT_EQ(at32ms[0].gunEnergy1, 0) << "starting plasma energy value 1 incorrect at 32ms";
    ASSERT_EQ(at16ms[0].gunEnergy1, 0) << "starting plasma energy value 1 incorrect at 16ms";
    ASSERT_EQ(at8ms[0].gunEnergy1, 0) << "starting plasma energy value 1 incorrect at 8ms";

    ASSERT_EQ(at64ms[0].gunEnergy2, 0) << "starting plasma energy value 2 incorrect at 64ms";
    ASSERT_EQ(at32ms[0].gunEnergy2, 0) << "starting plasma energy value 2 incorrect at 32ms";
    ASSERT_EQ(at16ms[0].gunEnergy2, 0) << "starting plasma energy value 2 incorrect at 16ms";
    ASSERT_EQ(at8ms[0].gunEnergy2, 0) << "starting plasma energy value 2 incorrect at 8ms";

    ASSERT_LT(at64ms[chargeSteps - 3].gunEnergy1, 52428) << "plasma 1 recharges too quickly at 64ms";
    ASSERT_EQ(at64ms[chargeSteps - 2].gunEnergy1, 52428) << "plasma 1 recharges too slowly at 64ms";
    ASSERT_EQ(at32ms[chargeSteps - 2].gunEnergy1, 52428) << "plasma 1 recharges too slowly at 32ms";
    ASSERT_EQ(at16ms[chargeSteps - 1].gunEnergy1, 52428) << "plasma 1 recharges too slowly at 16ms";
    ASSERT_EQ(at8ms[chargeSteps - 1].gunEnergy1, 52428) << "plasma 1 recharges too slowly at 8ms";

    ASSERT_LT(at64ms[chargeSteps - 3].gunEnergy2, 52428) << "plasma 2 recharges too quickly at 64ms";
    ASSERT_EQ(at64ms[chargeSteps - 2].gunEnergy2, 52428) << "plasma 2 recharges too slowly at 64ms";
    ASSERT_EQ(at32ms[chargeSteps - 2].gunEnergy2, 52428) << "plasma 2 recharges too slowly at 32ms";
    ASSERT_EQ(at16ms[chargeSteps - 1].gunEnergy2, 52428) << "plasma 2 recharges too slowly at 16ms";
    ASSERT_EQ(at8ms[chargeSteps - 1].gunEnergy2, 52428) << "plasma 2 recharges too slowly at 8ms";

    ASSERT_EQ(at64ms[0].energy, 327680) << "starting energy value incorrect at 64ms";
    ASSERT_EQ(at32ms[0].energy, 327680) << "starting energy value incorrect at 32ms";
    ASSERT_EQ(at16ms[0].energy, 327680) << "starting energy value incorrect at 16ms";
    ASSERT_EQ(at8ms[0].energy, 327680) << "starting energy value incorrect at 8ms";
    ASSERT_EQ(at64ms[chargeSteps - 1].energy, 267990) << "ending energy value incorrect at 64ms";

    for (int i = 0; i < chargeSteps; i++) {
        // cout << at64ms[i].gunEnergy1 << "\t" << at64ms[i].gunEnergy2 << endl;
        // cout << at64ms[i].gunEnergy1 << "\t" << at32ms[i].gunEnergy1 << "\t"
        //      << at16ms[i].gunEnergy1 << "\t" << at8ms[i].gunEnergy1 << endl;
        ASSERT_EQ(at64ms[i].gunEnergy1, at64ms[i].gunEnergy2) << "plasma energy values not equal at 64ms";
        ASSERT_EQ(at32ms[i].gunEnergy1, at32ms[i].gunEnergy2) << "plasma energy values not equal at 32ms";
        ASSERT_EQ(at16ms[i].gunEnergy1, at16ms[i].gunEnergy2) << "plasma energy values not equal at 16ms";
        ASSERT_EQ(at8ms[i].gunEnergy1, at8ms[i].gunEnergy2) << "plasma energy values not equal at 8ms";

        ASSERT_NEAR(at32ms[i].gunEnergy1, at64ms[i].gunEnergy1, at64ms[i].gunEnergy1 * 0.02) << "32ms not close enough after " << i << " ticks.";
        ASSERT_NEAR(at16ms[i].gunEnergy1, at64ms[i].gunEnergy1, at64ms[i].gunEnergy1 * 0.02) << "16ms not close enough after " << i << " ticks.";
        ASSERT_NEAR(at8ms[i].gunEnergy1, at64ms[i].gunEnergy1, at64ms[i].gunEnergy1 * 0.02) << "8ms not close enough after " << i << " ticks.";

        ASSERT_EQ(at64ms[i].shields, 196608) << "64ms shields didn't remain full";
        ASSERT_EQ(at32ms[i].shields, 196608) << "32ms shields didn't remain full";
        ASSERT_EQ(at16ms[i].shields, 196608) << "16ms shields didn't remain full";
        ASSERT_EQ(at8ms[i].shields, 196608) << "8ms shields didn't remain full";

        ASSERT_NEAR(at32ms[i].energy, at64ms[i].energy, at64ms[i].energy * 0.004) << "32ms energy not close enough after " << i << " ticks.";
        ASSERT_NEAR(at16ms[i].energy, at64ms[i].energy, at64ms[i].energy * 0.004) << "16ms energy not close enough after " << i << " ticks.";
        ASSERT_NEAR(at8ms[i].energy, at64ms[i].energy, at64ms[i].energy * 0.004) << "8ms energy not close enough after " << i << " ticks.";
    }
}

TEST(HECTOR, BoostedPlasmaRegen) {
    int chargeSteps = 27;
    vector<HectorEnergyReadings> at64ms = HectorPlasmaRegen(chargeSteps, true, 64);
    vector<HectorEnergyReadings> at32ms = HectorPlasmaRegen(chargeSteps, true, 32);
    vector<HectorEnergyReadings> at16ms = HectorPlasmaRegen(chargeSteps, true, 16);
    vector<HectorEnergyReadings> at8ms = HectorPlasmaRegen(chargeSteps, true, 8);

    ASSERT_EQ(at64ms.size(), chargeSteps) << "not enough steps recorded at 64ms";
    ASSERT_EQ(at32ms.size(), chargeSteps) << "not enough steps recorded at 32ms";
    ASSERT_EQ(at16ms.size(), chargeSteps) << "not enough steps recorded at 16ms";
    ASSERT_EQ(at8ms.size(), chargeSteps) << "not enough steps recorded at 8ms";

    ASSERT_EQ(at64ms[0].gunEnergy1, 0) << "starting plasma energy value 1 incorrect at 64ms";
    ASSERT_EQ(at32ms[0].gunEnergy1, 0) << "starting plasma energy value 1 incorrect at 32ms";
    ASSERT_EQ(at16ms[0].gunEnergy1, 0) << "starting plasma energy value 1 incorrect at 16ms";
    ASSERT_EQ(at8ms[0].gunEnergy1, 0) << "starting plasma energy value 1 incorrect at 8ms";

    ASSERT_EQ(at64ms[0].gunEnergy2, 0) << "starting plasma energy value 2 incorrect at 64ms";
    ASSERT_EQ(at32ms[0].gunEnergy2, 0) << "starting plasma energy value 2 incorrect at 32ms";
    ASSERT_EQ(at16ms[0].gunEnergy2, 0) << "starting plasma energy value 2 incorrect at 16ms";
    ASSERT_EQ(at8ms[0].gunEnergy2, 0) << "starting plasma energy value 2 incorrect at 8ms";

    ASSERT_LT(at64ms[chargeSteps - 2].gunEnergy1, 52428) << "plasma 1 recharges too quickly at 64ms";
    ASSERT_EQ(at64ms[chargeSteps - 1].gunEnergy1, 52428) << "plasma 1 recharges too slowly at 64ms";
    ASSERT_EQ(at32ms[chargeSteps - 1].gunEnergy1, 52428) << "plasma 1 recharges too slowly at 32ms";
    ASSERT_EQ(at16ms[chargeSteps - 1].gunEnergy1, 52428) << "plasma 1 recharges too slowly at 16ms";
    ASSERT_EQ(at8ms[chargeSteps - 1].gunEnergy1, 52428) << "plasma 1 recharges too slowly at 8ms";

    ASSERT_LT(at64ms[chargeSteps - 2].gunEnergy2, 52428) << "plasma 2 recharges too quickly at 64ms";
    ASSERT_EQ(at64ms[chargeSteps - 1].gunEnergy2, 52428) << "plasma 2 recharges too slowly at 64ms";
    ASSERT_EQ(at32ms[chargeSteps - 1].gunEnergy2, 52428) << "plasma 2 recharges too slowly at 32ms";
    ASSERT_EQ(at16ms[chargeSteps - 1].gunEnergy2, 52428) << "plasma 2 recharges too slowly at 16ms";
    ASSERT_EQ(at8ms[chargeSteps - 1].gunEnergy2, 52428) << "plasma 2 recharges too slowly at 8ms";

    ASSERT_EQ(at64ms[0].energy, 327680) << "starting energy value incorrect at 64ms";
    ASSERT_EQ(at32ms[0].energy, 327680) << "starting energy value incorrect at 32ms";
    ASSERT_EQ(at16ms[0].energy, 327680) << "starting energy value incorrect at 16ms";
    ASSERT_EQ(at8ms[0].energy, 327680) << "starting energy value incorrect at 8ms";

    for (int i = 0; i < chargeSteps; i++) {
        // cout << at64ms[i].gunEnergy1 << "\t" << at64ms[i].gunEnergy2 << endl;
        // cout << at64ms[i].gunEnergy1 << "\t" << at32ms[i].gunEnergy1 << "\t"
        //      << at16ms[i].gunEnergy1 << "\t" << at8ms[i].gunEnergy1 << endl;
        ASSERT_EQ(at64ms[i].gunEnergy1, at64ms[i].gunEnergy2) << "plasma energy values not equal at 64ms";
        ASSERT_EQ(at32ms[i].gunEnergy1, at32ms[i].gunEnergy2) << "plasma energy values not equal at 32ms";
        ASSERT_EQ(at16ms[i].gunEnergy1, at16ms[i].gunEnergy2) << "plasma energy values not equal at 16ms";
        ASSERT_EQ(at8ms[i].gunEnergy1, at8ms[i].gunEnergy2) << "plasma energy values not equal at 8ms";

        ASSERT_NEAR(at32ms[i].gunEnergy1, at64ms[i].gunEnergy1, at64ms[i].gunEnergy1 * 0.02) << "32ms not close enough after " << i << " ticks.";
        ASSERT_NEAR(at16ms[i].gunEnergy1, at64ms[i].gunEnergy1, at64ms[i].gunEnergy1 * 0.02) << "16ms not close enough after " << i << " ticks.";
        ASSERT_NEAR(at8ms[i].gunEnergy1, at64ms[i].gunEnergy1, at64ms[i].gunEnergy1 * 0.02) << "8ms not close enough after " << i << " ticks.";

        ASSERT_EQ(at64ms[i].shields, 196608) << "64ms shields didn't remain full";
        ASSERT_EQ(at32ms[i].shields, 196608) << "32ms shields didn't remain full";
        ASSERT_EQ(at16ms[i].shields, 196608) << "16ms shields didn't remain full";
        ASSERT_EQ(at8ms[i].shields, 196608) << "8ms shields didn't remain full";

        if (i == 1) {
            ASSERT_LT(at64ms[i].energy, 327680) << "64ms second energy value shouldn't be full";
        } else {
            ASSERT_EQ(at64ms[i].energy, 327680) << "64ms energy value should be full at " << i << " ticks";
            ASSERT_EQ(at32ms[i].energy, 327680) << "32ms energy value should be full at " << i << " ticks";
            ASSERT_EQ(at16ms[i].energy, 327680) << "16ms energy value should be full at " << i << " ticks";
            ASSERT_EQ(at8ms[i].energy, 327680) << "8ms energy value should be full at " << i << " ticks";
        }
    }
}

TEST(HECTOR, ShieldRegen) {
    int regenSteps = 411;
    vector<HectorEnergyReadings> at64ms = HectorShieldRegen(regenSteps, false, 64);
    vector<HectorEnergyReadings> at32ms = HectorShieldRegen(regenSteps, false, 32);
    vector<HectorEnergyReadings> at16ms = HectorShieldRegen(regenSteps, false, 16);
    vector<HectorEnergyReadings> at8ms = HectorShieldRegen(regenSteps, false, 8);

    ASSERT_EQ(at64ms.size(), regenSteps) << "not enough steps recorded at 64ms";
    ASSERT_EQ(at32ms.size(), regenSteps) << "not enough steps recorded at 32ms";
    ASSERT_EQ(at16ms.size(), regenSteps) << "not enough steps recorded at 16ms";
    ASSERT_EQ(at8ms.size(), regenSteps) << "not enough steps recorded at 8ms";
    ASSERT_EQ(at64ms[0].shields, 98304) << "starting shield value incorrect at 64ms";
    ASSERT_EQ(at32ms[0].shields, 98304) << "starting shield value incorrect at 32ms";
    ASSERT_EQ(at16ms[0].shields, 98304) << "starting shield value incorrect at 16ms";
    ASSERT_EQ(at8ms[0].shields, 98304) << "starting shield value incorrect at 8ms";

    // At 8ms, shields take a surprising amount of extra steps in comparison to other
    // frame times. That said, it still amounts to something like an extra half second
    // out of roughly 26 seconds to recharge from half shields.
    ASSERT_LT(at64ms[regenSteps - 10].shields, 196608) << "shields recharge too quickly at 64ms";
    ASSERT_EQ(at64ms[regenSteps - 9].shields, 196608) << "shields recharge too slowly at 64ms";
    ASSERT_EQ(at32ms[regenSteps - 8].shields, 196608) << "shields recharge too slowly at 32ms";
    ASSERT_EQ(at16ms[regenSteps - 8].shields, 196608) << "shields recharge too slowly at 16ms";
    ASSERT_EQ(at8ms[regenSteps - 1].shields, 196608) << "shields recharge too slowly at 8ms";

    for (int i = 0; i < regenSteps; i++) {
        // cout << at64ms[i] << "\t" << at32ms[i] << "\t" << at16ms[i] << "\t" << at8ms[i] << endl;
        ASSERT_NEAR(at32ms[i].shields, at64ms[i].shields, at64ms[i].shields * 0.003) << "32ms not close enough after " << i << " ticks.";
        ASSERT_NEAR(at16ms[i].shields, at64ms[i].shields, at64ms[i].shields * 0.003) << "16ms not close enough after " << i << " ticks.";
        ASSERT_NEAR(at8ms[i].shields, at64ms[i].shields, at64ms[i].shields * 0.011) << "8ms not close enough after " << i << " ticks.";

        ASSERT_EQ(at64ms[0].energy, 327680) << "64ms energy didn't remain full";
        ASSERT_EQ(at32ms[0].energy, 327680) << "32ms energy didn't remain full";
        ASSERT_EQ(at16ms[0].energy, 327680) << "16ms energy didn't remain full";
        ASSERT_EQ(at8ms[0].energy, 327680) << "8ms energy didn't remain full";

        ASSERT_EQ(at64ms[i].gunEnergy1, 52428) << "64ms plasma energy 1 didn't remain full";
        ASSERT_EQ(at32ms[i].gunEnergy1, 52428) << "32ms plasma energy 1 didn't remain full";
        ASSERT_EQ(at16ms[i].gunEnergy1, 52428) << "16ms plasma energy 1 didn't remain full";
        ASSERT_EQ(at8ms[i].gunEnergy1, 52428) << "8ms plasma energy 1 didn't remain full";

        ASSERT_EQ(at64ms[i].gunEnergy2, 52428) << "64ms plasma energy 2 didn't remain full";
        ASSERT_EQ(at32ms[i].gunEnergy2, 52428) << "32ms plasma energy 2 didn't remain full";
        ASSERT_EQ(at16ms[i].gunEnergy2, 52428) << "16ms plasma energy 2 didn't remain full";
        ASSERT_EQ(at8ms[i].gunEnergy2, 52428) << "8ms plasma energy 2 didn't remain full";
    }
}

TEST(HECTOR, BoostedShieldRegen) {
    int regenSteps = 47;
    vector<HectorEnergyReadings> at64ms = HectorShieldRegen(regenSteps, true, 64);
    vector<HectorEnergyReadings> at32ms = HectorShieldRegen(regenSteps, true, 32);
    vector<HectorEnergyReadings> at16ms = HectorShieldRegen(regenSteps, true, 16);
    vector<HectorEnergyReadings> at8ms = HectorShieldRegen(regenSteps, true, 8);

    ASSERT_EQ(at64ms.size(), regenSteps) << "not enough steps recorded at 64ms";
    ASSERT_EQ(at32ms.size(), regenSteps) << "not enough steps recorded at 32ms";
    ASSERT_EQ(at16ms.size(), regenSteps) << "not enough steps recorded at 16ms";
    ASSERT_EQ(at8ms.size(), regenSteps) << "not enough steps recorded at 8ms";
    ASSERT_EQ(at64ms[0].shields, 98304) << "starting shield value incorrect at 64ms";
    ASSERT_EQ(at32ms[0].shields, 98304) << "starting shield value incorrect at 32ms";
    ASSERT_EQ(at16ms[0].shields, 98304) << "starting shield value incorrect at 16ms";
    ASSERT_EQ(at8ms[0].shields, 98304) << "starting shield value incorrect at 8ms";
    ASSERT_LT(at64ms[regenSteps - 2].shields, 196608) << "shields recharge too quickly at 64ms";
    ASSERT_EQ(at64ms[regenSteps - 1].shields, 196608) << "shields recharge too slowly at 64ms";
    ASSERT_EQ(at32ms[regenSteps - 1].shields, 196608) << "shields recharge too slowly at 32ms";
    ASSERT_EQ(at16ms[regenSteps - 1].shields, 196608) << "shields recharge too slowly at 16ms";
    ASSERT_EQ(at8ms[regenSteps - 1].shields, 196608) << "shields recharge too slowly at 8ms";

    for (int i = 0; i < regenSteps; i++) {
        // cout << at64ms[i] << "\t" << at32ms[i] << "\t" << at16ms[i] << "\t" << at8ms[i] << endl;
        ASSERT_NEAR(at32ms[i].shields, at64ms[i].shields, at64ms[i].shields * 0.015) << "32ms not close enough after " << i << " ticks.";
        ASSERT_NEAR(at16ms[i].shields, at64ms[i].shields, at64ms[i].shields * 0.015) << "16ms not close enough after " << i << " ticks.";
        ASSERT_NEAR(at8ms[i].shields, at64ms[i].shields, at64ms[i].shields * 0.0175) << "8ms not close enough after " << i << " ticks.";

        ASSERT_EQ(at64ms[0].energy, 327680) << "64ms energy didn't remain full";
        ASSERT_EQ(at32ms[0].energy, 327680) << "32ms energy didn't remain full";
        ASSERT_EQ(at16ms[0].energy, 327680) << "16ms energy didn't remain full";
        ASSERT_EQ(at8ms[0].energy, 327680) << "8ms energy didn't remain full";

        ASSERT_EQ(at64ms[i].gunEnergy1, 52428) << "64ms plasma energy 1 didn't remain full";
        ASSERT_EQ(at32ms[i].gunEnergy1, 52428) << "32ms plasma energy 1 didn't remain full";
        ASSERT_EQ(at16ms[i].gunEnergy1, 52428) << "16ms plasma energy 1 didn't remain full";
        ASSERT_EQ(at8ms[i].gunEnergy1, 52428) << "8ms plasma energy 1 didn't remain full";

        ASSERT_EQ(at64ms[i].gunEnergy2, 52428) << "64ms plasma energy 2 didn't remain full";
        ASSERT_EQ(at32ms[i].gunEnergy2, 52428) << "32ms plasma energy 2 didn't remain full";
        ASSERT_EQ(at16ms[i].gunEnergy2, 52428) << "16ms plasma energy 2 didn't remain full";
        ASSERT_EQ(at8ms[i].gunEnergy2, 52428) << "8ms plasma energy 2 didn't remain full";
    }
}

TEST(HECTOR, ComplexRegen) {
    int regenSteps = 784;
    vector<HectorEnergyReadings> at64ms = HectorComplexRegen(regenSteps, false, 64);
    vector<HectorEnergyReadings> at32ms = HectorComplexRegen(regenSteps, false, 32);
    vector<HectorEnergyReadings> at16ms = HectorComplexRegen(regenSteps, false, 16);
    vector<HectorEnergyReadings> at8ms = HectorComplexRegen(regenSteps, false, 8);

    ASSERT_EQ(at64ms.size(), regenSteps) << "not enough steps recorded at 64ms";
    ASSERT_EQ(at32ms.size(), regenSteps) << "not enough steps recorded at 32ms";
    ASSERT_EQ(at16ms.size(), regenSteps) << "not enough steps recorded at 16ms";
    ASSERT_EQ(at8ms.size(), regenSteps) << "not enough steps recorded at 8ms";

    ASSERT_EQ(at64ms[0].shields, 39321) << "starting shield value incorrect at 64ms";
    ASSERT_EQ(at32ms[0].shields, 39321) << "starting shield value incorrect at 32ms";
    ASSERT_EQ(at16ms[0].shields, 39321) << "starting shield value incorrect at 16ms";
    ASSERT_EQ(at8ms[0].shields, 39321) << "starting shield value incorrect at 8ms";

    ASSERT_EQ(at64ms[0].energy, 163840) << "starting energy value incorrect at 64ms";
    ASSERT_EQ(at32ms[0].energy, 163840) << "starting energy value incorrect at 32ms";
    ASSERT_EQ(at16ms[0].energy, 163840) << "starting energy value incorrect at 16ms";
    ASSERT_EQ(at8ms[0].energy, 163840) << "starting energy value incorrect at 8ms";

    ASSERT_EQ(at64ms[0].gunEnergy1, 15728) << "starting plasma energy value 1 incorrect at 64ms";
    ASSERT_EQ(at32ms[0].gunEnergy1, 15728) << "starting plasma energy value 1 incorrect at 32ms";
    ASSERT_EQ(at16ms[0].gunEnergy1, 15728) << "starting plasma energy value 1 incorrect at 16ms";
    ASSERT_EQ(at8ms[0].gunEnergy1, 15728) << "starting plasma energy value 1 incorrect at 8ms";

    ASSERT_EQ(at64ms[0].gunEnergy2, 5242) << "starting plasma energy value 1 incorrect at 64ms";
    ASSERT_EQ(at32ms[0].gunEnergy2, 5242) << "starting plasma energy value 1 incorrect at 32ms";
    ASSERT_EQ(at16ms[0].gunEnergy2, 5242) << "starting plasma energy value 1 incorrect at 16ms";
    ASSERT_EQ(at8ms[0].gunEnergy2, 5242) << "starting plasma energy value 1 incorrect at 8ms";

    ASSERT_LT(at64ms[regenSteps - 12].shields, 196608) << "shields recharge too quickly at 64ms";
    ASSERT_EQ(at64ms[regenSteps - 11].shields, 196608) << "shields recharge too slowly at 64ms";
    ASSERT_LT(at32ms[regenSteps - 11].shields, 196608) << "shields recharge too quickly at 32ms";
    ASSERT_EQ(at32ms[regenSteps - 10].shields, 196608) << "shields recharge too slowly at 32ms";
    ASSERT_LT(at16ms[regenSteps - 8].shields, 196608) << "shields recharge too quickly at 16ms";
    ASSERT_EQ(at16ms[regenSteps - 7].shields, 196608) << "shields recharge too slowly at 16ms";
    ASSERT_LT(at8ms[regenSteps - 3].shields, 196608) << "shields recharge too quickly at 8ms";
    ASSERT_EQ(at8ms[regenSteps - 2].shields, 196608) << "shields recharge too slowly at 8ms";

    ASSERT_LT(at64ms[regenSteps - 10].energy, 327680) << "energy recharges too quickly at 64ms";
    ASSERT_EQ(at64ms[regenSteps - 9].energy, 327680) << "energy recharges too slowly at 64ms";
    ASSERT_LT(at32ms[regenSteps - 9].energy, 327680) << "energy recharges too quickly at 32ms";
    ASSERT_EQ(at32ms[regenSteps - 8].energy, 327680) << "energy recharges too slowly at 32ms";
    ASSERT_LT(at16ms[regenSteps - 7].energy, 327680) << "energy recharges too quickly at 16ms";
    ASSERT_EQ(at16ms[regenSteps - 6].energy, 327680) << "energy recharges too slowly at 16ms";
    ASSERT_LT(at8ms[regenSteps - 2].energy, 327680) << "energy recharges too quickly at 8ms";
    ASSERT_EQ(at8ms[regenSteps - 1].energy, 327680) << "energy recharges too slowly at 8ms";

    ASSERT_LT(at64ms[42].gunEnergy1, 52428) << "plasma energy 1 recharges too quickly at 64ms";
    ASSERT_EQ(at64ms[43].gunEnergy1, 52428) << "plasma energy 1 recharges too slowly at 64ms";
    ASSERT_LT(at32ms[42].gunEnergy1, 52428) << "plasma energy 1 recharges too quickly at 32ms";
    ASSERT_EQ(at32ms[43].gunEnergy1, 52428) << "plasma energy 1 recharges too slowly at 32ms";
    ASSERT_LT(at16ms[43].gunEnergy1, 52428) << "plasma energy 1 recharges too quickly at 16ms";
    ASSERT_EQ(at16ms[44].gunEnergy1, 52428) << "plasma energy 1 recharges too slowly at 16ms";
    ASSERT_LT(at8ms[43].gunEnergy1, 52428) << "plasma energy 1 recharges too quickly at 8ms";
    ASSERT_EQ(at8ms[44].gunEnergy1, 52428) << "plasma energy 1 recharges too slowly at 8ms";

    ASSERT_LT(at64ms[53].gunEnergy2, 52428) << "plasma energy 2 recharges too quickly at 64ms";
    ASSERT_EQ(at64ms[54].gunEnergy2, 52428) << "plasma energy 2 recharges too slowly at 64ms";
    ASSERT_LT(at32ms[54].gunEnergy2, 52428) << "plasma energy 2 recharges too quickly at 32ms";
    ASSERT_EQ(at32ms[55].gunEnergy2, 52428) << "plasma energy 2 recharges too slowly at 32ms";
    ASSERT_LT(at16ms[54].gunEnergy2, 52428) << "plasma energy 2 recharges too quickly at 16ms";
    ASSERT_EQ(at16ms[55].gunEnergy2, 52428) << "plasma energy 2 recharges too slowly at 16ms";
    ASSERT_LT(at8ms[54].gunEnergy2, 52428) << "plasma energy 2 recharges too quickly at 8ms";
    ASSERT_EQ(at8ms[55].gunEnergy2, 52428) << "plasma energy 2 recharges too slowly at 8ms";

    for (int i = 0; i < regenSteps; i++) {
        // cout << at64ms[i].shields << "\t" << at64ms[i].energy << "\t"
        //      << at64ms[i].gunEnergy1 << "\t" << at64ms[i].gunEnergy2 << endl;
        // cout << at32ms[i].shields << "\t" << at32ms[i].energy << "\t"
        //      << at32ms[i].gunEnergy1 << "\t" << at32ms[i].gunEnergy2 << endl;
        // cout << at16ms[i].shields << "\t" << at16ms[i].energy << "\t"
        //      << at16ms[i].gunEnergy1 << "\t" << at16ms[i].gunEnergy2 << endl;
        // cout << at8ms[i].shields << "\t" << at8ms[i].energy << "\t"
        //      << at8ms[i].gunEnergy1 << "\t" << at8ms[i].gunEnergy2 << endl;
        ASSERT_NEAR(at32ms[i].shields, at64ms[i].shields, at64ms[i].shields * 0.006) << "32ms shields not close enough after " << i << " ticks.";
        ASSERT_NEAR(at16ms[i].shields, at64ms[i].shields, at64ms[i].shields * 0.006) << "16ms shields not close enough after " << i << " ticks.";
        ASSERT_NEAR(at8ms[i].shields, at64ms[i].shields, at64ms[i].shields * 0.018) << "8ms shields not close enough after " << i << " ticks.";

        ASSERT_NEAR(at32ms[i].energy, at64ms[i].energy, at64ms[i].energy * 0.0075) << "32ms energy not close enough after " << i << " ticks.";
        ASSERT_NEAR(at16ms[i].energy, at64ms[i].energy, at64ms[i].energy * 0.0075) << "16ms energy not close enough after " << i << " ticks.";
        ASSERT_NEAR(at8ms[i].energy, at64ms[i].energy, at64ms[i].energy * 0.01) << "8ms energy not close enough after " << i << " ticks.";

        ASSERT_NEAR(at32ms[i].gunEnergy1, at64ms[i].gunEnergy1, at64ms[i].gunEnergy1 * 0.008) << "32ms plasma energy 1 not close enough after " << i << " ticks.";
        ASSERT_NEAR(at16ms[i].gunEnergy1, at64ms[i].gunEnergy1, at64ms[i].gunEnergy1 * 0.008) << "16ms plasma energy 1 not close enough after " << i << " ticks.";
        ASSERT_NEAR(at8ms[i].gunEnergy1, at64ms[i].gunEnergy1, at64ms[i].gunEnergy1 * 0.018) << "8ms plasma energy 1 not close enough after " << i << " ticks.";

        ASSERT_NEAR(at32ms[i].gunEnergy2, at64ms[i].gunEnergy2, at64ms[i].gunEnergy2 * 0.01) << "32ms plasma energy 2 not close enough after " << i << " ticks.";
        ASSERT_NEAR(at16ms[i].gunEnergy2, at64ms[i].gunEnergy2, at64ms[i].gunEnergy2 * 0.01) << "16ms plasma energy 2 not close enough after " << i << " ticks.";
        ASSERT_NEAR(at8ms[i].gunEnergy2, at64ms[i].gunEnergy2, at64ms[i].gunEnergy2 * 0.018) << "8ms plasma energy 2 not close enough after " << i << " ticks.";
    }
}

TEST(GRENADE, Trajectory) {
    vector<VectorStruct> at64ms = FireGrenade(20, 50, 64);
    vector<VectorStruct> at32ms = FireGrenade(20, 50, 32);
    vector<VectorStruct> at16ms = FireGrenade(20, 50, 16);
    vector<VectorStruct> at8ms = FireGrenade(20, 50, 8);

    ASSERT_NEAR(at64ms.back().theVec[1], 59384, 3*MILLIMETER) << "64ms simulation is wrong";

    for (int i = 0; i < min(at32ms.size(), at64ms.size()); i++) {
        // std::cout << "delY32[" << i << "] = " << ToFloat(at32ms[i].theVec[1] - at64ms[i].theVec[1]) << std::endl;
        // std::cout << "dist32[" << i << "] = " << VecStructDist(at64ms[i], at32ms[i]) << std::endl;
        ASSERT_LT(VecStructDist(at64ms[i], at32ms[i]), 0.2) << "not close enough after " << i << " ticks.";
    }
    for (int i = 0; i < min(at16ms.size(), at64ms.size()); i++) {
        // std::cout << "delY16[" << i << "] = " << ToFloat(at16ms[i].theVec[1] - at64ms[i].theVec[1]) << std::endl;
        // std::cout << "dist16[" << i << "] = " << VecStructDist(at64ms[i], at16ms[i]) << std::endl;
        // std::cout << "loc64[" << i << "] = " << FormatVector(at64ms[i].theVec, 3) << std::endl;
        // std::cout << "loc16[" << i << "] = " << FormatVector(at16ms[i].theVec, 3) << std::endl;
        ASSERT_LT(VecStructDist(at64ms[i], at16ms[i]), 0.2) << "not close enough after " << i << " ticks.";
    }
    for (int i = 0; i < min(at8ms.size(), at64ms.size()); i++) {
        // std::cout << "delY8[" << i << "] = " << ToFloat(at8ms[i].theVec[1] - at64ms[i].theVec[1]) << std::endl;
        // std::cout << "dist8[" << i << "] = " << VecStructDist(at64ms[i], at8ms[i]) << std::endl;
        // std::cout << "loc64[" << i << "] = " << FormatVector(at64ms[i].theVec, 3) << std::endl;
        // std::cout << "loc8[" << i << "] = " << FormatVector(at8ms[i].theVec, 3) << std::endl;
        ASSERT_LT(VecStructDist(at64ms[i], at8ms[i]), 0.2) << "not close enough after " << i << " ticks.";
    }
}

TEST(MISSILE, Trajectory) {
    vector<VectorStruct> at64ms = FireMissile(20, 60, 50, 64);
    vector<VectorStruct> at32ms = FireMissile(20, 60, 50, 32);
    vector<VectorStruct> at16ms = FireMissile(20, 60, 50, 16);
    vector<VectorStruct> at8ms = FireMissile(20, 60, 50, 8);

    // index 15 is the furthest left the missile goes, and index 26 is the furthest forward
    ASSERT_NEAR(at64ms[15].theVec[0], -591030, 3*MILLIMETER) << "64ms simulation is wrong on min X";
    ASSERT_NEAR(at64ms[26].theVec[2], 1306446, 3*MILLIMETER) << "64ms simulation is wrong on max Z";
    ASSERT_EQ(at64ms.size(), 37) << "64ms simulation blows up in the wrong frame";
    ASSERT_NEAR(at64ms.size(), at32ms.size(), 1) << "32ms simulation should blows up in wrong frame";
    ASSERT_NEAR(at64ms.size(), at16ms.size(), 1) << "16ms simulation should blows up at wrong frame";
    ASSERT_NEAR(at64ms.size(), at8ms.size(), 1) << "8ms simulation should blows up at wrong frame";

    for (int i = 0; i < min(at32ms.size(), at64ms.size()); i++) {
        // std::cout << "loc32[" << i << "] = " << FormatVector(at32ms[i].theVec, 3) << std::endl;
        // std::cout << "loc64[" << i << "] = " << FormatVector(at64ms[i].theVec, 3) << std::endl;
        // std::cout << "dist32[" << i << "] = " << VecStructDist(at64ms[i], at32ms[i]) << std::endl;
        ASSERT_LT(VecStructDist(at64ms[i], at32ms[i]), 0.1) << "not close enough after " << i << " ticks.";
    }
    for (int i = 0; i < min(at16ms.size(), at64ms.size()); i++) {
        // std::cout << "loc16[" << i << "] = " << FormatVector(at16ms[i].theVec, 3) << std::endl;
        // std::cout << "loc64[" << i << "] = " << FormatVector(at64ms[i].theVec, 3) << std::endl;
        // std::cout << "dist16[" << i << "] = " << VecStructDist(at64ms[i], at16ms[i]) << std::endl;
        ASSERT_LT(VecStructDist(at64ms[i], at16ms[i]), 0.1) << "not close enough after " << i << " ticks.";
    }
    for (int i = 0; i < min(at8ms.size(), at64ms.size()); i++) {
        // std::cout << "loc8[" << i << "] = " << FormatVector(at8ms[i].theVec, 3) << std::endl;
        // std::cout << "loc64[" << i << "] = " << FormatVector(at64ms[i].theVec, 3) << std::endl;
        // std::cout << "dist8[" << i << "] = " << VecStructDist(at64ms[i], at8ms[i]) << std::endl;
        ASSERT_LT(VecStructDist(at64ms[i], at8ms[i]), 0.1) << "not close enough after " << i << " ticks.";
    }
}

template<typename T, typename TMember>
void test_rollover(string counterName, T x, T y, TMember counter) {
    // quick test using signed int16_t max value in case somebody changes the type...
    x.*counter = std::numeric_limits<int16_t>::max();
    y.*counter = x.*counter + 2;
    EXPECT_LT(x.*counter, y.*counter)     << counterName << " failed (x < y) test for INT16_MAX";
    EXPECT_EQ(y.*counter - x.*counter, 2) << counterName << " failed (y - x) test for INT16_MAX";

    // ...but most tests should be for uint16_t
    x.*counter = std::numeric_limits<uint16_t>::max();
    y.*counter = x.*counter + 2;
    // Don't use googletest operator macros such as EXPECT_LT because we are testing the operators themselves
    EXPECT_TRUE(x.*counter < y.*counter)   << counterName << " failed (x < y) test for UINT16_MAX";
    EXPECT_TRUE(y.*counter > x.*counter)   << counterName << " failed (y > x) test for UINT16_MAX";
    EXPECT_TRUE(x.*counter <= y.*counter)  << counterName << " failed (x <= y) test for UINT16_MAX";
    EXPECT_TRUE(y.*counter >= x.*counter)  << counterName << " failed (y >= x) test for UINT16_MAX";
    EXPECT_TRUE(x.*counter != y.*counter)  << counterName << " failed (x != y) test for UINT16_MAX";
    EXPECT_EQ(y.*counter - x.*counter, 2)  << counterName << " failed (y - x) test for UINT16_MAX";
    EXPECT_EQ(x.*counter - y.*counter, -2) << counterName << " failed (x - y) test for UINT16_MAX";

    y.*counter = x.*counter;
    EXPECT_TRUE(x.*counter == y.*counter)  << counterName << " failed (x == y) test for equal values";
    EXPECT_TRUE(x.*counter <= y.*counter)  << counterName << " failed (x <= y) test for equal values";
    EXPECT_TRUE(y.*counter >= x.*counter)  << counterName << " failed (x >= y) test for equal values";
}

TEST(SERIAL_NUMBER, Rollover) {
    // tests all instance of variable serialNumber and any field used as a serial number proxy
    UDPPacketInfo packet1, packet2;
    test_rollover("UDPPacketInfo::serialNumber", packet1, packet2, &UDPPacketInfo::serialNumber);

    CUDPConnection conn1, conn2;
    test_rollover("CUDPConnection::serialNumber", conn1, conn2, &CUDPConnection::serialNumber);
    test_rollover("CUDPConnection::receiveSerial", conn1, conn2, &CUDPConnection::receiveSerial);
    test_rollover("CUDPConnection::maxValid", conn1, conn2, &CUDPConnection::maxValid);
    test_rollover("CUDPConnection::ackBase", conn1, conn2, &CUDPConnection::ackBase);
}

TEST(MATERIALS, Manipulation) {
    Material defaultMaterial = Material().WithColor(0xff000000).WithSpecular(0x00000000).WithShininess(0x00);
    ASSERT_EQ(defaultMaterial.GetColor().GetRaw(), 0xff000000) << "defaultMaterial color not black";
    ASSERT_EQ(defaultMaterial.GetSpecular().GetRaw(), 0x00000000) << "defaultMaterial specular not invisible black";
    ASSERT_EQ(defaultMaterial.GetShininess(), 0x00) << "defaultMaterial shininess non-zero";
    
    Material alteredMaterial = defaultMaterial;
    ASSERT_EQ(alteredMaterial.GetColor().GetRaw(), 0xff000000) << "alteredMaterial color not initialized to black";
    ASSERT_EQ(alteredMaterial.GetSpecular().GetRaw(), 0x00000000) << "alteredMaterial specular not initialized to invisible black";
    ASSERT_EQ(alteredMaterial.GetShininess(), 0x00) << "alteredMaterial shininess initialized to non-zero";
    
    alteredMaterial = alteredMaterial.WithColor(0xff3a404d);
    ASSERT_EQ(alteredMaterial.GetColor().GetRaw(), 0xff3a404d) << "failed to update material color to 0xff3a404d";
    alteredMaterial = alteredMaterial.WithSpecular(0xff33322b); // alpha bits will be dropped!
    ASSERT_EQ(alteredMaterial.GetSpecular().GetRaw(), 0x0033322b) << "failed to update material specular color to 0x0033322b";
    ASSERT_EQ(alteredMaterial.GetColor().GetRaw(), 0xff3a404d) << "failed to retain material color after setting specular to 0x0033322b";
    alteredMaterial = alteredMaterial.WithShininess(0x01);
    ASSERT_EQ(alteredMaterial.GetShininess(), 0x01) << "failed to update material shininess to 0x01";
    ASSERT_EQ(alteredMaterial.GetColor().GetRaw(), 0xff3a404d) << "failed to retain material color after setting shininess to 0x01";
    ASSERT_EQ(alteredMaterial.GetSpecular().GetRaw(), 0x0033322b) << "failed to retain material specular color after setting shininess to 0x01";
    
    alteredMaterial = alteredMaterial.WithSpecular(0xccff0000); // alpha bits will be dropped, again
    ASSERT_EQ(alteredMaterial.GetSpecular().GetRaw(), 0x00ff0000) << "failed to update material specular color to 0x00ff0000";
    ASSERT_EQ(alteredMaterial.GetColor().GetRaw(), 0xff3a404d) << "failed to retain material color after setting specular to 0x00ff0000";
    ASSERT_EQ(alteredMaterial.GetShininess(), 0x01) << "failed to retain material shininess after setting specular to 0x00ff0000";
    
    alteredMaterial = alteredMaterial.WithSpecB(0xff);
    ASSERT_EQ(alteredMaterial.GetSpecular().GetRaw(), 0x00ff00ff) << "failed to update material specular B component to 0xff";
    ASSERT_EQ(alteredMaterial.GetColor().GetRaw(), 0xff3a404d) << "failed to retain material color after setting specular B component to 0xff";
    ASSERT_EQ(alteredMaterial.GetShininess(), 0x01) << "failed to retain material shininess after setting specular B component to 0xff";
    
    alteredMaterial = alteredMaterial.WithSpecG(0xdd);
    ASSERT_EQ(alteredMaterial.GetSpecular().GetRaw(), 0x00ffddff) << "failed to update material specular G component to 0xdd";
    ASSERT_EQ(alteredMaterial.GetColor().GetRaw(), 0xff3a404d) << "failed to retain material color after setting specular G component to 0xdd";
    ASSERT_EQ(alteredMaterial.GetShininess(), 0x01) << "failed to retain material shininess after setting specular G component to 0xdd";
    
    alteredMaterial = alteredMaterial.WithSpecR(0xaa);
    ASSERT_EQ(alteredMaterial.GetSpecular().GetRaw(), 0x00aaddff) << "failed to update material specular R component to 0xaa";
    ASSERT_EQ(alteredMaterial.GetColor().GetRaw(), 0xff3a404d) << "failed to retain material color after setting specular R component to 0xaa";
    ASSERT_EQ(alteredMaterial.GetShininess(), 0x01) << "failed to retain material shininess after setting specular R component to 0xaa";
    
    alteredMaterial = defaultMaterial;
    ASSERT_EQ(alteredMaterial.GetColor().GetRaw(), 0xff000000) << "alteredMaterial color not reset to black";
    ASSERT_EQ(alteredMaterial.GetSpecular().GetRaw(), 0x00000000) << "alteredMaterial specular not reset to invisible black";
    ASSERT_EQ(alteredMaterial.GetShininess(), 0x00) << "alteredMaterial shininess not reset to zero";
    
    // Chained
    alteredMaterial = defaultMaterial
        .WithColor(0xff111111)
        .WithSpecular(0xff00cccc) // alpha bits dropped
        .WithShininess(0x10);
    ASSERT_EQ(alteredMaterial.GetColor().GetRaw(), 0xff111111) << "failed to update material color to 0xff111111";
    ASSERT_EQ(alteredMaterial.GetSpecular().GetRaw(), 0x0000cccc) << "failed to update material specular color to 0x0000cccc";
    ASSERT_EQ(alteredMaterial.GetShininess(), 0x10) << "failed to update material shininess to 0x10";
}

int main(int argc, char **argv) {
    // Check basepath override.
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--basepath") == 0) {
            SetBasePath(argv[++i]);
        }
    }
    nanogui::init();
    InitMatrix();
    ::testing::InitGoogleTest(&argc, argv);
    int r = RUN_ALL_TESTS();
    nanogui::shutdown();
    return r;
}
