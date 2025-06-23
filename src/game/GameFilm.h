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
#include <memory>

struct PlayerFrameFunc {
    short slot;
    FunctionTable ft;
};

typedef std::vector<PlayerFrameFunc> FilmReel;
typedef std::unique_ptr<FilmReel> FilmReelPointer;
class CAvaraGame;
typedef std::unique_ptr<CAvaraGame> GamePointer;

class GameFilm {
public:
    GameFilm();
    void RecordFrame(short slot, FunctionTable *ft);
    bool HasUnflushedFrames();
    const size_t GetWritten() { return written; };
    void SetWritten(size_t count) { written = count; };
    std::unique_ptr<FilmReel> *GetReelRef();
    void Serialize(std::ostream &os, GamePointer &game);
    void DeserializeTest(std::istream &is, GamePointer &game);
    ~GameFilm() {};

private:
    std::unique_ptr<FilmReel> frames;
    size_t written = 0;
};
#endif
