//
//  GameFilm.h
//  Avara
//
//  Created by Andy Halstead on 6/7/25.
//
#pragma once
#ifndef GAMEFILM_H
#define GAMEFILM_H
#include "KeyFuncs.h"
#include "LevelInfo.h"

struct PlayerFrameFunc {
    short slot;
    FunctionTable ft;
};

typedef std::vector<PlayerFrameFunc> FilmReel;
typedef std::unique_ptr<FilmReel> FilmReelPointer;

class GameFilm {
public:
    GameFilm();
    void RecordFrame(short slot, FunctionTable *ft);
    bool HasUnflushedFrames();
    size_t Written() { return written; };
    void UpdateWritten(size_t count) { written = count; };
    FilmReel *GetReelRef();
    ~GameFilm() {};

private:
    std::unique_ptr<FilmReel> frames;
    size_t written = 0;
};
#endif
