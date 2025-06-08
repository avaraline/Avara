//
//  GameFilm.h
//  Avara
//
//  Created by Andy Halstead on 6/7/25.
//

#include "KeyFuncs.h"
#include "CNetManager.h"
#include "LevelInfo.h"

struct PlayerFrameFunc {
    short slot;
    FrameFunction ff;
};

class GameFilm {
public:
    GameFilm();
    void RecordFrame(CPlayerManager *manager);
    void SetFinalPlayerTable();
    void SetLevel(LevelInfo li);
    ~GameFilm();
private:
    std::vector<std::string> *playerTable;
    std::vector<PlayerFrameFunc> *frames;
};
