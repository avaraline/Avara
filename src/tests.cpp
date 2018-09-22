#include "gtest/gtest.h"
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

#include <iostream>
using namespace std;

class TestSoundHub : public CSoundHubImpl {
public:
    virtual Fixed* EarLocation() { return ear; }
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
    virtual CAbstractPlayer* GetPlayer() { return playa; }
    virtual void SetPlayer(CAbstractPlayer* p) { playa = p; }
    virtual short Slot() { return 0; }
    virtual void AbortRequest() {}
    virtual Boolean IsLocalPlayer() { return true; }
    virtual void GameKeyPress(char c) {}
    virtual FunctionTable *GetFunctions() { return ft; }
    virtual void DeadOrDone() {}
    virtual short Position() { return 0; }
    virtual Str255& PlayerName() { return str; }
    virtual std::deque<unsigned char>& LineBuffer() { return lineBuffer; }
    virtual void Dispose() {}
    virtual void NetDisconnect() {}
    virtual short IsRegistered() { return 0; }
    virtual void IsRegistered(short) {}
    virtual Str255& PlayerRegName() { return str; }
    virtual short LoadingStatus() { return 0; }
    virtual void SetPlayerStatus(short newStatus, long theWin) {}
    virtual void ChangeNameAndLocation(StringPtr theName, Point location) {}
    virtual void SetPosition(short pos) {}
    virtual void RosterKeyPress(unsigned char c) {}
    virtual void RosterMessageText(short len, char *c) {}
    virtual short LevelCRC() { return 0; }
    virtual OSErr LevelErr() { return noErr; }
    virtual OSType LevelTag() { return 0; }
    virtual void LevelCRC(short) {}
    virtual void LevelErr(OSErr) {}
    virtual void LevelTag(OSType) {}
    virtual void LoadStatusChange(short serverCRC, OSErr serverErr, OSType serverTag) {}
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

    virtual void StoreMugShot(Handle mugHandle) {}
    virtual Handle GetMugShot() { return 0; }

    virtual CAbstractPlayer *TakeAnyActor(CAbstractPlayer *actorList) { return 0; }
    virtual short PlayerColor() { return 0; }
    virtual Boolean IncarnateInAnyColor() { return false; }
    virtual void ResendFrame(long theFrame, short requesterId, short commandCode) {}
    virtual void SpecialColorControl() {}
    virtual PlayerConfigRecord& TheConfiguration() { return pcr; }
    virtual Handle MugPict() { return 0; }
    virtual void MugPict(Handle) {}
    virtual long MugSize() { return 0; }
    virtual long MugState() { return 0; }
    virtual void MugSize(long) {}
    virtual void MugState(long) {}
    virtual long WinFrame() { return 0; }
    virtual void ProtocolHandler(struct PacketInfo *thePacket) {}
    virtual void IncrementAskAgainTime(int) {}
private:
    FunctionTable *ft;
    CAvaraGame *itsGame;
    CAbstractPlayer *playa;
    std::deque<unsigned char> lineBuffer;
    Str255 str;
    PlayerConfigRecord pcr;
};

class TestNetManager : public CNetManager {
public:
    CPlayerManager* CreatePlayerManager(short id) {
        return new TestPlayerManager(itsGame);
    }
};

class TestApp : public CAvaraApp {
public:
    virtual bool DoCommand(int theCommand) {return false;}
    virtual void MessageLine(short index, short align) {}
    virtual void DrawUserInfoPart(short i, short partList) {}
    virtual void ParamLine(short index, short align, StringPtr param1, StringPtr param2) {}
    virtual void StartFrame(long frameNum) {}
    virtual void BrightBox(long frameNum, short position) {}
    virtual void LevelReset() {}
    virtual long Number(const std::string name) { return 0; }
    virtual OSErr LoadLevel(std::string set, OSType theLevel) { return noErr; }
    virtual void ComposeParamLine(StringPtr destStr, short index, StringPtr param1, StringPtr param2) {}
    virtual void NotifyUser() {}
    virtual json Get(const std::string name) {}
    virtual void Set(const std::string name, const std::string value) {}
    virtual void Set(const std::string name, long value) {}
    virtual void Set(const std::string name, json value) {}
    virtual CNetManager* GetNet() { return itsNet; }
    virtual void SetNet(CNetManager* net) { itsNet = net; }
    virtual SDL_Window* sdlWindow() { return 0; }
    virtual void StringLine(StringPtr theString, short align) {}
    virtual CAvaraGame* GetGame() { return 0; }
    virtual void Done() {}
    virtual void BroadcastCommand(int) {}
private:
    CNetManager *itsNet;
};

class TestGame : public CAvaraGame {
public:
    TestGame(int frameTime) : CAvaraGame(frameTime) {}
    virtual CNetManager* CreateNetManager() { return new TestNetManager(); }
    virtual CSoundHub* CreateSoundHub() { TestSoundHub *t = new TestSoundHub(); t->ISoundHub(64,64); return t;}
};

class HectorTestScenario {
public:
    TestApp app;
    TestGame *game;
    CWalkerActor *hector;

    HectorTestScenario(int frameTime, Fixed hectorX, Fixed hectorY, Fixed hectorZ) {
        game = new TestGame(frameTime);
        gCurrentGame = game;
        InitParser();
        game->IAvaraGame(&app);
        game->EndScript();
        app.GetNet()->ChangeNet(kNullNet, "");
        hector = new CWalkerActor();
        hector->IAbstractActor();
        hector->BeginScript();
        hector->EndScript();
        game->itsNet->playerTable[0]->SetPlayer(hector);
        hector->itsManager = game->itsNet->playerTable[0];
        hector->location[0] = hectorX;
        hector->location[1] = hectorY;
        hector->location[2] = hectorZ;
        hector->location[3] = FIX1;
        game->AddActor(hector);
        game->GameStart();
    }
};

vector<Fixed> DropHector(int steps, int ticksPerStep, Fixed fromHeight, int frameTime) {
    HectorTestScenario scenario(frameTime, 0, fromHeight, 0);
    vector<Fixed> altitudes;
    for (int i = 0; i < steps; i++) {
        scenario.game->nextScheduledFrame = 0;
        scenario.game->itsNet->activePlayersDistribution = 1;
        altitudes.push_back(scenario.hector->location[1]);
        for (int k = 0; k < ticksPerStep; k++) {
            scenario.game->GameTick();
        }
    }
    altitudes.push_back(scenario.hector->location[1]);
    return altitudes;
}

vector<VectorStruct> WalkHector(int settleSteps, int steps, int ticksPerStep, int frameTime) {
    HectorTestScenario scenario(frameTime, 0, 0, 0);
    vector<VectorStruct> location;
    for (int i = 0; i < settleSteps; i++) {
        scenario.game->nextScheduledFrame = 0;
        scenario.game->itsNet->activePlayersDistribution = 1;
        for (int k = 0; k < ticksPerStep; k++) {
            scenario.game->GameTick();
        }
    }
    scenario.game->nextScheduledFrame = 0;
    scenario.game->itsNet->activePlayersDistribution = 1;
    scenario.hector->itsManager->GetFunctions()->held = (1 << kfuForward);
    scenario.game->GameTick();

    for (int i = 0; i < steps; i++) {
        scenario.game->nextScheduledFrame = 0;
        scenario.game->itsNet->activePlayersDistribution = 1;
        location.push_back(*(VectorStruct*)scenario.hector->location);
        for (int k = 0; k < ticksPerStep; k++) {
            scenario.hector->itsManager->GetFunctions()->held = (1 << kfuForward);
            scenario.game->GameTick();
        }
    }
    location.push_back(*(VectorStruct*)scenario.hector->location);

    return location;
}

vector<VectorStruct> FireGrenade(int settleSteps, int steps, int ticksPerStep, int frameTime) {
    HectorTestScenario scenario(frameTime, 0, 0, 0);
    vector<VectorStruct> trajectory;
    for (int i = 0; i < settleSteps; i++) {
        scenario.game->nextScheduledFrame = 0;
        scenario.game->itsNet->activePlayersDistribution = 1;
        for (int k = 0; k < ticksPerStep; k++) {
            scenario.game->GameTick();
        }
    }
    scenario.game->nextScheduledFrame = 0;
    scenario.game->itsNet->activePlayersDistribution = 1;
    scenario.hector->itsManager->GetFunctions()->down = (1 << kfuLoadGrenade) | (1 << kfuFireWeapon);
    scenario.game->GameTick();
    scenario.hector->itsManager->GetFunctions()->down = 0;
    CGrenade *grenade = 0;
    // hopefully there are two objects now. a hector and a grenade.
    CAbstractActor *aa = scenario.game->actorList;
    int count = 2;
    for (count = 0; aa; aa = aa->nextActor, count++) {
        if (aa != scenario.hector) {
            grenade = (CGrenade*)aa;
        }
    }

    for (int i = 0; i < steps && count == 2; i++) {
        scenario.game->nextScheduledFrame = 0;
        scenario.game->itsNet->activePlayersDistribution = 1;
        trajectory.push_back(*(VectorStruct*)grenade->location);
        for (int k = 0; k < ticksPerStep; k++) {
            scenario.game->GameTick();
        }
        // this intends to figure out whether the grenade has exploded.
        grenade = 0;
        aa = scenario.game->actorList;
        for (count = 0; aa; aa = aa->nextActor, count++) {
            if (aa != scenario.hector) {
                grenade = (CGrenade*)aa;
            }
        }
    }

    return trajectory;
}

vector<Fixed> TurnHector(int steps, int ticksPerStep, int frameTime) {
    HectorTestScenario scenario(frameTime, 0, 0, 0);
    vector<Fixed> headings;
    for (int i = 0; i < steps; i++) {
        scenario.game->nextScheduledFrame = 0;
        scenario.game->itsNet->activePlayersDistribution = 1;
        headings.push_back(scenario.hector->heading);
        for (int k = 0; k < ticksPerStep; k++) {
            scenario.hector->itsManager->GetFunctions()->held = (1 << kfuRight);
            scenario.game->GameTick();
        }
    }
    headings.push_back(scenario.hector->heading);
    return headings;
}

TEST(HECTOR, Gravity) {
    vector<Fixed> at64ms = DropHector(50, 1, FIX(200), 64);
    vector<Fixed> at32ms = DropHector(50, 2, FIX(200), 32);
    vector<Fixed> at16ms = DropHector(50, 4, FIX(200), 16);
    ASSERT_EQ(at64ms.back(), 6126784) << "64ms simulation fell wrong amount";
    ASSERT_EQ(at64ms.size(), at32ms.size()) << "DropHector didn't do ticks right";
    for (int i = 0; i < at64ms.size(); i++) {
        double f64 = ToFloat(at64ms[i]);
        double f32 = ToFloat(at32ms[i]);
        double f16 = ToFloat(at32ms[i]);
        ASSERT_LT(abs(f64-f32)/f64, 0.18) << "not close enough after " << i << " ticks.";
        ASSERT_LT(abs(f64-f16)/f64, 0.18) << "not close enough after " << i << " ticks.";
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
    vector<Fixed> at64ms = TurnHector(50, 1, 64);
    vector<Fixed> at32ms = TurnHector(50, 2, 32);
    vector<Fixed> at16ms = TurnHector(50, 4, 16);
    ASSERT_EQ(at64ms.back(), -30592) << "64ms simulation turned wrong amount";
    ASSERT_EQ(at64ms.size(), at32ms.size()) << "TurnHector didn't do ticks right";
    for (int i = 0; i < at64ms.size(); i++) {
        EXPECT_LT(at64ms[i] > at32ms[i] ? at64ms[i] - at32ms[i] : at32ms[i] - at64ms[i], 0) << "not close enough after " << i << " ticks.";
    }
    for (int i = 0; i < at64ms.size(); i++) {
        EXPECT_LT(at64ms[i] > at16ms[i] ? at64ms[i] - at16ms[i] : at16ms[i] - at64ms[i], 0) << "not close enough after " << i << " ticks.";
    }
}

TEST(HECTOR, WalkForwardSpeed) {
    vector<VectorStruct> at64ms = WalkHector(20, 50, 1, 64);
    vector<VectorStruct> at32ms = WalkHector(20, 50, 2, 32);
    vector<VectorStruct> at16ms = WalkHector(20, 50, 4, 16);
    ASSERT_EQ(at64ms.back().theVec[0], 0) << "64ms simulation walked wrong amount";
    ASSERT_EQ(at64ms.back().theVec[1], -11284) << "64ms simulation walked wrong amount";
    ASSERT_EQ(at64ms.back().theVec[2], 1584235) << "64ms simulation walked wrong amount";
    ASSERT_EQ(at64ms.size(), at32ms.size()) << "WalkHector didn't do ticks right";
    for (int i = 0; i < min(at32ms.size(), at64ms.size()); i++) {
        EXPECT_LT(VecStructDist(at64ms[i], at32ms[i]), 0) << "not close enough after " << i << " ticks.";
    }
    for (int i = 0; i < min(at16ms.size(), at64ms.size()); i++) {
        EXPECT_LT(VecStructDist(at64ms[i], at16ms[i]), 0) << "not close enough after " << i << " ticks.";
    }
}

TEST(GRENADE, Trajectory) {
    vector<VectorStruct> at64ms = FireGrenade(20, 50, 1, 64);
    vector<VectorStruct> at32ms = FireGrenade(20, 50, 2, 32);
    vector<VectorStruct> at16ms = FireGrenade(20, 50, 4, 16);
    ASSERT_EQ(at64ms.back().theVec[1], 59420) << "64ms simulation is wrong";
    for (int i = 0; i < min(at32ms.size(), at64ms.size()); i++) {
        EXPECT_LT(VecStructDist(at64ms[i], at32ms[i]), 0.7) << "not close enough after " << i << " ticks.";
    }
    for (int i = 0; i < min(at16ms.size(), at64ms.size()); i++) {
        EXPECT_LT(VecStructDist(at64ms[i], at16ms[i]), 1) << "not close enough after " << i << " ticks.";
    }
}

int main(int argc, char **argv) {
    CBSPPart::actuallyRender = false;
    nanogui::init();
    InitMatrix();
    ::testing::InitGoogleTest(&argc, argv);
    int r = RUN_ALL_TESTS();
    nanogui::shutdown();
    return r;
}
