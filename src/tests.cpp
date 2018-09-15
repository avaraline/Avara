#include "gtest/gtest.h"
#include "CAvaraApp.h"
#include "CAvaraGame.h"
#include "CWalkerActor.h"
#include <nanogui/nanogui.h>
#include "FastMat.h"


TEST(FAIL, ShouldFail) {
    // ASSERT_EQ(1, 2);
    CCAvaraApp app;
    CWalkerActor *hector = new CWalkerActor();
    hector->IAbstractActor();
    //app.itsGame->AddActor(hector);
    ASSERT_NE(app.itsGame->actorList, nullptr);

    app.Done();
}

int main(int argc, char **argv) {
    nanogui::init();
    InitMatrix();
    ::testing::InitGoogleTest(&argc, argv);
    int r = RUN_ALL_TESTS();
    nanogui::shutdown();
    return r;
}
