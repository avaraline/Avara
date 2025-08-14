//
//  GameFilm.cpp
//  Avara
//
//  Created by Andy Halstead on 6/7/25.
//

#include "GameFilm.h"
#include "CAvaraGame.h"
#include "CAbstractPlayer.h"
#include "CWalkerActor.h"
#include "GitVersion.h"

GameFilm::GameFilm() {
    frames = std::make_unique<FilmReel>();
    written = 0;
}

void GameFilm::RecordFrame(short slot, FunctionTable *ft) {
    FunctionTable ftc(*ft);
    PlayerFrameFunc pff = {slot, ftc};
    frames->push_back(pff);
}

std::unique_ptr<FilmReel> *GameFilm::GetReelRef() {
    return &frames;
}

bool GameFilm::HasUnflushedFrames() {
    return frames->size() > written;
}

const char FilmHeader[9] = {'A', 'V', 'A', 'R', 'A', 'F', 'I', 'L', 'M'};

#define oswrite(x) os.write(reinterpret_cast<const char *>(&x), sizeof(x))
#define isread(x) is.read(reinterpret_cast<char *>(&x), sizeof(x))

#if !defined(_WIN32) && !defined(__APPLE__)
unsigned long long htonll(unsigned long long v) {
    union { unsigned long lv[2]; unsigned long long llv; } u;
    u.lv[0] = htonl(v >> 32);
    u.lv[1] = htonl(v & 0xFFFFFFFFULL);
    return u.llv;
}

unsigned long long ntohll(unsigned long long v) {
    union { unsigned long lv[2]; unsigned long long llv; } u;
    u.llv = v;
    return ((unsigned long long)ntohl(u.lv[0]) << 32) | (unsigned long long)ntohl(u.lv[1]);
}
#endif

void GameFilm::Serialize(std::ostream &os, GamePointer &game) {
    auto linfo = game->loadedLevelInfo.get();
    uint64_t gameId = htonll(game->currentGameId);
    std::string lurl = linfo->URL();
    uint8_t frameTime = 0;
    uint8_t spawnOrder = 0;
    uint8_t totalPlayers = 0;
    bool hostInfo = false;
    for (int i = 0; i < kMaxAvaraPlayers; i++) {
        auto manager = (&game->itsNet->playerTable[i])->get();
        if (!manager) continue;
        auto thePlayer = manager->GetPlayer();
        if (!thePlayer) continue;
        if (!hostInfo) {
            CPlayerManager* host = manager->ServerPlayer();
            auto hostConfig = host->TheConfiguration();
            frameTime = (uint8_t)hostConfig.frameTime;
            spawnOrder = (uint8_t)hostConfig.spawnOrder;
            hostInfo = true;
        }
        totalPlayers++;
    }
    
    // header
    for (int i = 0; i < sizeof(FilmHeader); i++) {
        oswrite(FilmHeader[i]);
    }
    // level url
    uint16_t lurlSize = htons((uint16_t)lurl.size());
    oswrite(lurlSize);
    const char* lurlStr = lurl.c_str();
    for (int j = 0; j < lurl.size(); j++) {
        oswrite(lurlStr[j]);
    }
    // game id
    oswrite(gameId);
    // frame time
    oswrite(frameTime);
    // spawn order
    oswrite(spawnOrder);
    // total players
    oswrite(totalPlayers);
    // player list
    for (int i = 0; i < kMaxAvaraPlayers; i++) {
        auto manager = (&game->itsNet->playerTable[i])->get();
        if (!manager) continue;
        auto thePlayer = manager->GetPlayer();
        if (!thePlayer) continue;
        if (!hostInfo) {
            CPlayerManager* host = manager->ServerPlayer();
            auto hostConfig = host->TheConfiguration();
            frameTime = (uint8_t)hostConfig.frameTime;
            spawnOrder = (uint8_t)hostConfig.spawnOrder;
            hostInfo = true;
        }
        std::string name = manager->GetPlayerName();
        uint8_t slot = manager->Slot();
        uint8_t team = thePlayer->teamColor;
        auto walker = (CWalkerActor*) thePlayer;
        uint16_t hull = htons(walker->hullRes);
        uint16_t grenades = htons(thePlayer->grenadeCount);
        uint16_t missiles = htons(thePlayer->missileCount);
        uint16_t boosters = htons((int)thePlayer->boostsRemaining);
        auto playerConfig = manager->TheConfiguration();
        uint32_t hullColor = htonl(playerConfig.hullColor.GetRaw());
        uint32_t gunColor = htonl(playerConfig.gunColor.GetRaw());
        uint32_t cockpitColor = htonl(playerConfig.cockpitColor.GetRaw());
        uint32_t trimColor = htonl(playerConfig.trimColor.GetRaw());
        // player name
        uint16_t nameSize = htons((uint16_t)name.size());
        oswrite(nameSize);
        const char* nameStr = name.c_str();
        for (int k = 0; k < name.size(); k++) {
            oswrite(nameStr[k]);
        }
        // slot
        oswrite(slot);
        // team
        oswrite(team);
        // config
        oswrite(hull);
        oswrite(grenades);
        oswrite(missiles);
        oswrite(boosters);
        // colors
        oswrite(hullColor);
        oswrite(gunColor);
        oswrite(cockpitColor);
        oswrite(trimColor);
    }
    // number of frame funcs
    uint64_t frameCount = htonll((uint64_t)frames->size());
    oswrite(frameCount);
    uint64_t currentFrame = 0;
    for (auto it = frames->begin(); it < frames->end(); ++it, ++currentFrame) {
        uint8_t slot = it->slot;
        uint64_t frame = htonll(currentFrame);
        uint32_t down = htonl(it->ft.down);
        uint32_t up = htonl(it->ft.up);
        uint32_t held = htonl(it->ft.held);
        uint8_t buttonStatus = it->ft.buttonStatus;
        uint16_t mouseH = htons(it->ft.mouseDelta.h);
        uint16_t mouseV = htons(it->ft.mouseDelta.v);
        char msgChar = it->ft.msgChar;
        
        oswrite(slot);
        oswrite(frame);
        oswrite(down);
        oswrite(up);
        oswrite(held);
        oswrite(buttonStatus);
        oswrite(mouseH);
        oswrite(mouseV);
        oswrite(msgChar);
    }
}

void GameFilm::DeserializeTest(std::istream &is, GamePointer &game) {
    // header
    is.ignore(sizeof(FilmHeader));
    // level url
    uint16_t lurlLength;
    isread(lurlLength);
    lurlLength = ntohs(lurlLength);
    std::string lurl = "";
    for (uint16_t i = 0; i < lurlLength; i++) {
        char theByte = '\0';
        isread(theByte);
        lurl += theByte;
    }
    uint64_t gameId;
    uint8_t frameTime;
    uint8_t spawnOrder;
    uint8_t totalPlayers;
    // game id
    isread(gameId);
    gameId = ntohll(gameId);
    // frame time
    isread(frameTime);
    // spawn order
    isread(spawnOrder);
    // total players
    isread(totalPlayers);
    
    SDL_Log("deserialize game %0llxd with %i players", gameId, totalPlayers);
    
    // player list
    for (int j = 0; j < totalPlayers; j++) {
        uint16_t nameLength;
        std::string name = "";
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
        // player name
        isread(nameLength);
        nameLength = ntohs(nameLength);
        for(uint16_t k = 0; k < nameLength; k++) {
            char theByte = '\0';
            isread(theByte);
            name += theByte;
        }
        // slot
        isread(slot);
        // team
        isread(team);
        // config
        isread(hull);
        hull = ntohs(hull);
        isread(grenades);
        grenades = ntohs(grenades);
        isread(missiles);
        missiles = ntohs(missiles);
        isread(boosters);
        boosters = ntohs(boosters);
        // colors
        isread(hullColor);
        hullColor = ntohl(hullColor);
        isread(gunColor);
        gunColor = ntohl(gunColor);
        isread(cockpitColor);
        cockpitColor = ntohl(cockpitColor);
        isread(trimColor);
        trimColor = ntohl(trimColor);
        SDL_Log("deserialize player %s in slot %i team %i hull %i", name.c_str(), slot, team, hull);
    }
    // frame total
    uint64_t frameCount;
    isread(frameCount);
    frameCount = ntohll(frameCount);
    SDL_Log("try to deserialize %llu frames", frameCount);
    for (size_t m = 0; m < frameCount; m++) {
        uint8_t slot;
        uint64_t frame;
        uint32_t down;
        uint32_t up;
        uint32_t held;
        uint8_t buttonStatus;
        uint16_t mouseH;
        uint16_t mouseV;
        char msgChar;

        isread(slot);
        isread(frame);
        frame = ntohll(frame);
        isread(down);
        down = ntohl(down);
        isread(up);
        up = ntohl(up);
        isread(held);
        held = ntohl(held);
        isread(buttonStatus);
        isread(mouseH);
        mouseH = ntohs(mouseH);
        isread(mouseV);
        mouseV = ntohs(mouseV);
        isread(msgChar);
        if (msgChar > 0) SDL_Log("MsgChar %c", msgChar);
    }
    SDL_Log("No problem!!");
    
}
