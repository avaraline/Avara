//
//  GameFilm.cpp
//  Avara
//
//  Created by Andy Halstead on 6/7/25.
//

#include "GameFilm.h"

GameFilm::GameFilm() {
    frames = new std::vector<PlayerFrameFunc>();
    playerTable = new std::vector<std::string>();
}

void GameFilm::SetFinalPlayerTable() {
    
}

void GameFilm::SetLevel(LevelInfo li) {
    
}

void GameFilm::RecordFrame(CPlayerManager *manager) {
    auto ff = manager->GetFunctions();
    auto id = manager->Slot();
    PlayerFrameFunc pff = { id, {ff} };
    frames->push_back(pff);
}

