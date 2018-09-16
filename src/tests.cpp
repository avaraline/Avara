#include "gtest/gtest.h"
#include "CAvaraApp.h"
#include "CAvaraGame.h"
#include "CWalkerActor.h"
#include <nanogui/nanogui.h>
#include "FastMat.h"

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

TEST(FAIL, ShouldFail) {
    TestApp app;
    CAvaraGame game;
    game.IAvaraGame(&app);
    CWalkerActor *hector = new CWalkerActor();
    hector->IAbstractActor();
    //app.itsGame->AddActor(hector);
    ASSERT_NE(game.actorList, nullptr);
}

int main(int argc, char **argv) {
    nanogui::init();
    InitMatrix();
    ::testing::InitGoogleTest(&argc, argv);
    int r = RUN_ALL_TESTS();
    nanogui::shutdown();
    return r;
}
