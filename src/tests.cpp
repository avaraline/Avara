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

class TestPlayerManager : public CPlayerManager {
    virtual CAbstractPlayer* GetPlayer() { return 0; }
    virtual void SetPlayer(CAbstractPlayer*) {}
    virtual short Slot() { return 0; }
    virtual void AbortRequest() {}
    virtual Boolean IsLocalPlayer() { return true; }
    virtual void GameKeyPress(char c) {}
    virtual FunctionTable *GetFunctions() { return 0; }
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
    virtual void SendFrame() {}
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
    std::deque<unsigned char> lineBuffer;
    Str255 str;
    PlayerConfigRecord pcr;
};

class TestNetManager : public CNetManager {
    CPlayerManager* CreatePlayerManager(short id) {
        return new TestPlayerManager();
    }
};

class TestApp : public CAvaraApp {
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
    virtual CNetManager* GetNet() { return 0; }
    virtual void SetNet(CNetManager*) {}
    virtual SDL_Window* sdlWindow() { return 0; }
    virtual void StringLine(StringPtr theString, short align) {}
    virtual CAvaraGame* GetGame() { return 0; }
    virtual void Done() {}
};

class TestGame : public CAvaraGame {
    virtual CNetManager* CreateNetManager() { return new TestNetManager(); }
};

TEST(FAIL, ShouldFail) {
    TestApp app;
    TestGame game;
    gCurrentGame = &game;
    game.IAvaraGame(&app);
    CWalkerActor *hector = new CWalkerActor();
    hector->IAbstractActor();
    game.AddActor(hector);
    ASSERT_NE(game.actorList, nullptr);
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
