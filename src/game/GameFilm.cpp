//
//  GameFilm.cpp
//  Avara
//
//  Created by Andy Halstead on 6/7/25.
//

#include "GameFilm.h"

GameFilm::GameFilm() {
    frames = std::make_unique<FilmReel>();
    written = 0;
}

void GameFilm::RecordFrame(short slot, FunctionTable *ft) {
    FunctionTable ftc(*ft);
    PlayerFrameFunc pff = {slot, ftc};
    frames->push_back(pff);
}

FilmReel *GameFilm::GetReelRef() {
    return frames.get();
}

bool GameFilm::HasUnflushedFrames() {
    return frames->size() > written;
}
