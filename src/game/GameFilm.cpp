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
#include "Debug.h"
#include "portable_endian.h"


#define oswrite(x) os.write(reinterpret_cast<const char *>(&x), sizeof(x))
#define isread(x) is.read(reinterpret_cast<char *>(&x), sizeof(x))


void GameFilm::SaveGameInfo(GamePointer &game) {
    filmCast->clear();
    auto linfo = game->loadedLevelInfo.get();
    gameId = game->currentGameId;
    lurl = linfo->URL();
    bool hostInfo = false;
    for (int i = 0; i < kMaxAvaraPlayers; i++) {
        auto manager = (&game->itsNet->playerTable[i])->get();
        if (!manager) continue;
        if (!hostInfo) {
            CPlayerManager* host = manager->ServerPlayer();
            auto hostConfig = host->TheConfiguration();
            frameTime = (uint8_t)hostConfig.frameTime;
            spawnOrder = (uint8_t)hostConfig.spawnOrder;
            hostInfo = true;
        }
        auto thePlayer = manager->GetPlayer();
        if (!thePlayer) continue;
        auto p = PlayerInfo();
        p.name = manager->GetPlayerName();
        p.slot = manager->Slot();
        p.team = thePlayer->teamColor;
        auto walker = (CWalkerActor*) thePlayer;
        p.hull = walker->hullRes;
        p.grenades = thePlayer->grenadeCount;
        p.missiles = thePlayer->missileCount;
        p.boosters = (int)thePlayer->boostsRemaining;
        auto playerConfig = manager->TheConfiguration();
        p.hullColor = playerConfig.hullColor.GetRaw();
        p.gunColor = playerConfig.gunColor.GetRaw();
        p.cockpitColor = playerConfig.cockpitColor.GetRaw();
        p.trimColor = playerConfig.trimColor.GetRaw();
        // player name
        p.nameSize = (uint16_t)p.name.size();
        filmCast->push_back(p);
    }
    totalPlayers = filmCast->size();
}

void GameFilm::LoadGameInfo(GamePointer &game) {

}

void GameFilm::Serialize(std::ostream &os) {
    // header
    for (int i = 0; i < FilmHeaderLength; i++) {
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
    uint64_t gid = htobe64(this->gameId);
    oswrite(gid);
    // frame time
    uint16_t ft = htons(this->frameTime);
    oswrite(ft);
    // spawn order
    uint16_t so = htons(this->spawnOrder);
    oswrite(so);
    // total players
    uint16_t tp = htons(this->filmCast->size());
    oswrite(tp);
    // player list
    for (const PlayerInfo &p : *filmCast) {
        std::string name = p.name;
        uint16_t nameSize = htons(name.length());
        oswrite(nameSize);
        const char* nameStr = p.name.c_str();
        for (int k = 0; k < p.name.size(); k++) {
            oswrite(nameStr[k]);
        }

        uint16_t slot = htons(p.slot);
        uint16_t team = htons(p.team);
        uint16_t hull = htons(p.hull);
        uint16_t grenades = htons(p.grenades);
        uint16_t missiles = htons(p.missiles);
        uint16_t boosters = htons(p.boosters);
        uint32_t hullColor = htonl(p.hullColor);
        uint32_t gunColor = htonl(p.gunColor);
        uint32_t cockpitColor = htonl(p.cockpitColor);
        uint32_t trimColor = htonl(p.trimColor);
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
    uint64_t total = (uint64_t)frames->size();
    uint64_t frameCount = htobe64(total);
    oswrite(frameCount);
    uint64_t currentFrame = 0;
    for (const PlayerFrameFunc &it : *frames) {
        uint8_t slot = htons(it.slot);
        uint64_t frame = htobe64(currentFrame);
        uint32_t down = htonl(it.ft.down);
        uint32_t up = htonl(it.ft.up);
        uint32_t held = htonl(it.ft.held);
        uint8_t buttonStatus = it.ft.buttonStatus;
        uint16_t mouseH = htons(it.ft.mouseDelta.h);
        uint16_t mouseV = htons(it.ft.mouseDelta.v);
        char msgChar = it.ft.msgChar;

        oswrite(slot);
        oswrite(frame);
        oswrite(down);
        oswrite(up);
        oswrite(held);
        oswrite(buttonStatus);
        oswrite(mouseH);
        oswrite(mouseV);
        oswrite(msgChar);
        currentFrame++;
    }
}

void GameFilm::Deserialize(std::istream &is) {
    // header
    is.ignore(FilmHeaderLength);
    // level url
    uint16_t lurlLength;
    isread(lurlLength);
    lurlLength = ntohs(lurlLength);
    lurl = "";
    for (uint16_t i = 0; i < lurlLength; i++) {
        char theByte = '\0';
        isread(theByte);
        lurl += theByte;
    }
    DBG_Log("films", "deserialize lurl %s", lurl.c_str());
    // game id
    uint64_t gid;
    isread(gid);
    gameId = be64toh(gid);
    // frame time
    uint16_t ft;
    isread(ft);
    frameTime = ntohs(ft);
    // spawn order
    uint16_t so;
    isread(so);
    spawnOrder = ntohs(so);
    // total players
    uint16_t tp;
    isread(tp);
    totalPlayers = ntohs(tp);

    DBG_Log("films", "deserialize game %0llxd with %i players", gameId, totalPlayers);

    filmCast->clear();

    // player list
    for (int j = 0; j < totalPlayers; j++) {
        std::string name = "";
        uint16_t nameSize;
        uint16_t slot;
        uint16_t team;
        uint16_t hull;
        uint16_t grenades;
        uint16_t missiles;
        uint16_t boosters;
        uint32_t hullColor;
        uint32_t gunColor;
        uint32_t cockpitColor;
        uint32_t trimColor;

        PlayerInfo p;
        // player name
        isread(nameSize);
        p.name = "";
        p.nameSize = ntohs(nameSize);
        for(uint16_t k = 0; k < p.nameSize; k++) {
            char theByte = '\0';
            isread(theByte);
            p.name += theByte;
        }
        // slot
        isread(slot);
        slot = ntohs(slot);
        // team
        isread(team);
        team = ntohs(team);
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
        DBG_Log("films", "deserialize player %s in slot %i team %i hull %i", p.name.c_str(), slot, team, hull);
        p.slot = slot;
        p.hull = hull;
        p.grenades = grenades;
        p.missiles = missiles;
        p.boosters = boosters;
        p.hullColor = hullColor;
        p.gunColor = gunColor;
        p.cockpitColor = cockpitColor;
        p.trimColor = trimColor;
        filmCast->push_back(p);
    }

    frames->clear();
    // frame total
    uint64_t fc;
    isread(fc);
    uint64_t frameCount = be64toh(fc);
    DBG_Log("films", "try to deserialize %llu frames", frameCount);
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
        frame = be64toh(frame);
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
        PlayerFrameFunc f;
        f.slot = slot;
        f.ft = FunctionTable();
        f.ft.down = down;
        f.ft.up = up;
        f.ft.held = held;
        f.ft.buttonStatus = buttonStatus;
        f.ft.msgChar = msgChar;
        frames->push_back(f);
    }
    DBG_Log("films", "got %i frames!", (int)frames->size());
}
