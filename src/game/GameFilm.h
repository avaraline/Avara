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

struct PlayerInfo {
    std::string name;
    uint8_t slot;
    uint8_t team;
    uint16_t hull;
    uint16_t grenades;
    uint16_t missiles;
    uint16_t boosters;
    uint32_t hullColor;
    uint32_t gunColor;
    uint32_t cockpitColor;
    uint32_t trimColor;
    uint16_t nameSize;
};

typedef std::vector<PlayerInfo> FilmCast;
typedef std::vector<PlayerFrameFunc> FilmReel;
typedef std::unique_ptr<FilmReel> FilmReelPointer;
class CAvaraGame;
typedef std::unique_ptr<CAvaraGame> GamePointer;

class GameFilm {
public:

    ~GameFilm() {};

    GameFilm() {
        frames = std::make_unique<FilmReel>();
        filmCast = std::make_unique<FilmCast>();
        written = 0;
    }

    void RecordFrame(short slot, FunctionTable *ft) {
        FunctionTable ftc(*ft);
        PlayerFrameFunc pff = {slot, ftc};
        frames->push_back(pff);
    }

    std::unique_ptr<FilmReel> *GetReelRef() {
        return &frames;
    }

    bool HasUnflushedFrames() {
        return frames->size() > written;
    }
    const short FilmHeaderLength = 9;
    const char FilmHeader[9] = {'A', 'V', 'A', 'R', 'A', 'F', 'I', 'L', 'M'};

    const size_t GetWritten() { return written; };
    void SetWritten(size_t count) { written = count; };
    void SaveGameInfo(GamePointer &game);
    void LoadGameInfo(GamePointer &game);
    void Serialize(std::ostream &os);
    void Deserialize(std::istream &is);

private:
    std::unique_ptr<FilmCast> filmCast;
    std::unique_ptr<FilmReel> frames;
    size_t written = 0;
    uint64_t gameId = 0;
    std::string lurl;
    uint8_t frameTime = 0;
    uint8_t spawnOrder = 0;
    uint8_t totalPlayers = 0;
};
#endif
