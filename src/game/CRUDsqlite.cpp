//
//  CRUDsqlite.cpp
//  Avara
//
//  Created by Tom on 4/19/25.
//

#include "CRUDsqlite.h"
#include "CAbstractPlayer.h"
#include "GitVersion.h"
#include "CWalkerActor.h"
#include "CNetManager.h"
#include <SDL2/SDL.h>
#include <stdexcept>   // runtime_error
#include "Debug.h"

CRUDsqlite::CRUDsqlite() {
    char *ppath = SDL_GetPrefPath("Avaraline", "Avara");
    auto path = std::string(ppath) + "avara.db";

    // creates database if it doesn't exist
    if (sqlite3_open(path.c_str(), &myDb) == SQLITE_OK) {
        SDL_Log("Database opened: %s\n", path.c_str());
        // foreign key enforcement must be enabled on each connection
        sqlite3_exec(myDb, "PRAGMA foreign_keys = ON", NULL, 0, NULL);
        // max wait if the table is locked (helpful when testing with multiple clients on same computer)
        sqlite3_busy_timeout(myDb, 5 /* msec */);
        // migrate/create the database tables
        MigrateTables();
    } else {
        SDL_Log("COULD NOT OPEN DATABASE %s\n", path.c_str());
        exit(1);
    }
}

CRUDsqlite::~CRUDsqlite() {
    sqlite3_close(myDb);
}

// list of database migrations, add your block of changes to the bottom of this list
static std::vector<std::vector<std::string>> migrations = {
    // initial database configuration
    {
        "CREATE TABLE migrations     (created TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
                                     "version INTEGER UNIQUE)",
        "CREATE TABLE levels         (created TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
                                     "id INTEGER PRIMARY KEY ASC, "  // <-- becomes an alias for rowid
                                     "url TEXT UNIQUE NOT NULL, "
                                     "set_name TEXT NOT NULL, "
                                     "level_name TEXT NOT NULL)",
        "CREATE TABLE authors        (created TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
                                     "id INTEGER PRIMARY KEY ASC, "  // aka rowid
                                     "name TEXT UNIQUE NOT NULL)",
        "CREATE TABLE levels_authors (created TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
                                     "level_id INTEGER NOT NULL, "
                                     "author_id INTEGER NOT NULL, "
                                     "UNIQUE(level_id, author_id), "
                                     "FOREIGN KEY(level_id) REFERENCES levels(id), "
                                     "FOREIGN KEY(author_id) REFERENCES authors(id))",
        "CREATE TABLE games          (created timestamp default current_timestamp, "
                                     "id INTEGER PRIMARY KEY, "
                                     "level_id INTEGER NOT NULL, "
                                     "FOREIGN KEY(level_id) REFERENCES levels(id))"
    },
    {
        "CREATE TABLE players        (created timestamp default current_timestamp, "
                                     "id INTEGER PRIMARY KEY ASC, "
                                     "name TEXT NOT NULL, "
                                     "last_seen TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
                                     "elo INTEGER, "
                                     "colors TEXT, "
                                     "properties TEXT, "
                                     "UNIQUE(name))",

        "ALTER TABLE games ADD COLUMN total_frames INTEGER ",
        "ALTER TABLE games ADD COLUMN properties TEXT ",
        "ALTER TABLE games ADD COLUMN spawn_order INTEGER ",
        "ALTER TABLE games ADD COLUMN frame_time INTEGER ",
        "ALTER TABLE games ADD COLUMN game_version TEXT ",
        "ALTER TABLE games ADD COLUMN network_version INTEGER ",
        "ALTER TABLE games ADD COLUMN filmed INTEGER ",
        
        "CREATE TABLE games_players  (player_id INTEGER NOT NULL, "
                                     "game_id INTEGER NOT NULL, "
                                     "slot INTEGER NOT NULL, "
                                     "team INTEGER NOT NULL, "
                                     "grenades INTEGER NOT NULL DEFAULT 0, "
                                     "missiles INTEGER NOT NULL DEFAULT 0, "
                                     "boosters INTEGER NOT NULL DEFAULT 0, "
                                     "hull_rsrc_id INTEGER NOT NULL, "
                                     "hull_color INTEGER, "
                                     "gun_color INTEGER, "
                                     "cockpit_color INTEGER, "
                                     "trim_color INTEGER, "
                                     "UNIQUE(player_id, game_id), "
                                     "FOREIGN KEY(player_id) REFERENCES players(id), "
                                     "FOREIGN KEY(game_id) REFERENCES games(id))",

        "CREATE TABLE frame_funcs    (game_id INTEGER NOT NULL, "
                                     "frame_number INTEGER NOT NULL, "
                                     "slot INTEGER NOT NULL, "
                                     "down INTEGER, "
                                     "up INTEGER, "
                                     "held INTEGER, "
                                     "mouse_delta_h INTEGER, "
                                     "mouse_delta_v INTEGER, "
                                     "button_status INTEGER, "
                                     "msg_char VARCHAR(8), "
                                     "FOREIGN KEY(game_id) REFERENCES games(id))"
    }
    // all subsequent migrations look something like this (DO NOT CHANGE PREVIOUS MIGRATIONS)
    //        {
    //            "CREATE TABLE new_table (...);",
    //            "ALTER TABLE existing_table (...);"
    //        },
};

void CRUDsqlite::MigrateTables() {
    int version = 0;

    // get current version number from migrations table
    std::string query = "SELECT MAX(version) FROM migrations;";
    sqlite3_exec(myDb, query.c_str(), [](void* data, int argc, char** argv, char** colName) {
        int *ver = (int *)data;
        *ver = std::stoi(argv[0]);
        return 0;
    }, &version, NULL);

    try {
         sqlite3_exec(myDb, "BEGIN TRANSACTION", 0, 0, 0);

        // loop over all migrations newer than what has already been run
        for (int i = version; i < migrations.size(); i++) {
            SDL_Log("RUNNING DATABASE MIGRATION #%d\n", i+1);
            int rc = SQLITE_OK;
            char *errMsg;
            for (auto query : migrations[i]) {
                SDL_Log("  sqlite> %s\n", query.c_str());
                rc = sqlite3_exec(myDb, query.c_str(), NULL, 0, &errMsg);
                if (rc != SQLITE_OK) {
                    SDL_Log("Migration error message: %s\n", errMsg);
                    break;
                }
            }
            if (rc == SQLITE_OK) {
                std::string updateMigrationVersion = "INSERT INTO migrations (version) VALUES (" + std::to_string(i+1) + ")";
                SDL_Log("  sqlite> %s\n", updateMigrationVersion.c_str());
                rc = sqlite3_exec(myDb, updateMigrationVersion.c_str(), NULL, 0, NULL);
            } else {
                throw std::runtime_error("ERROR UPDATING DATABASE!!");
            }
        }
        sqlite3_exec(myDb, "COMMIT", 0, 0, 0);

    } catch (std::exception& e) {
        sqlite3_exec(myDb, "ROLLBACK", 0, 0, 0);
        exit(-1);
    }
}

// for internal use only, you should probably use the methods that call this one
int CRUDsqlite::InsertInto(const std::string clause, const char* returnField, std::function<void(sqlite3_stmt *)> bind_values) {
    DBG_Log("sql", "CRUDsqlite::InsertInto(%s, (...), %s)\n", clause.c_str(), returnField);

    sqlite3_stmt * stmt;
    std::string query = "INSERT INTO " + clause;
    if (returnField) {
        std::string field = returnField;
        // a trick to get it to return the ID even if the record has already been inserted
        query += " ON CONFLICT DO UPDATE SET " + field + "=" + field + " RETURNING " + field;
    } else {
        query += " ON CONFLICT DO NOTHING";
    }

    int rc = sqlite3_prepare_v2(myDb, query.c_str(), -1, &stmt, NULL);
    bind_values(stmt);

    rc = sqlite3_step(stmt);
    if ((returnField && rc != SQLITE_ROW) || (!returnField && rc != SQLITE_DONE)) {
        SDL_Log("possible ERROR inserting data: [%d] %s\n", rc, sqlite3_errmsg(myDb));
    }
    int result = 0;
    if (returnField) {
        // get the requested returnField
        result = sqlite3_column_int(stmt, 0);
    }
    rc = sqlite3_finalize(stmt);
    if (rc != SQLITE_OK) {
        SDL_Log("ERROR finalizing insert: [%d] %s\n", rc, sqlite3_errmsg(myDb));
    }

    return result;
}

// an insert utility that also helps to avoid SQL injection and can return the id of the inserted/matching record
int CRUDsqlite::InsertInto(const std::string clause, const std::vector<std::string> textBinds, const char* returnField)
{
    DBG_Log("sql", "CRUDsqlite::InsertInto(%s, strings[...], %s)\n", clause.c_str(), returnField);

    return InsertInto(clause, returnField, [&](sqlite3_stmt * stmt) {
        for (int i = 0; i < textBinds.size(); i++) {
            sqlite3_bind_text(stmt, i+1, textBinds[i].c_str(), -1, SQLITE_TRANSIENT);
        }
    });
}

// same as above but for inserting ints
int CRUDsqlite::InsertInto(const std::string clause, const std::vector<int> intBinds, const char* returnField)
{
    DBG_Log("sql", "CRUDsqlite::InsertInto(%s, ints[..], %s)\n", clause.c_str(), returnField);

    return InsertInto(clause, returnField, [&](sqlite3_stmt * stmt) {
        for (int i = 0; i < intBinds.size(); i++) {
            sqlite3_bind_int(stmt, i+1, intBinds[i]);
        }
    });
}


int CRUDsqlite::RecordLevelInfo(const LevelInfo& info) {
    DBG_Log("sql", "CRUDsqlite::RecordLevelInfo(%s)\n", info.URL().c_str());

    int levelId = InsertInto("levels(url, set_name, level_name) VALUES (?,?,?)",
                             { info.URL(), info.setTag, info.levelName }, "id");

    for (auto author: info.Authors()) {
        int authorId = InsertInto("authors(name) VALUES (?)", { author }, "id");

        InsertInto("levels_authors(level_id, author_id) VALUES (?, ?)", { levelId, authorId });
    }

    return levelId;
}

std::string CRUDsqlite::RecentsView() {
    // static so it's only created once
    static std::string viewName;
    // create the temporary view and return the view name
    if (viewName.empty()) {
        viewName = "recent_levels";
        // because it's temporary you can modify this view as needed (no migration needed)
        auto query = "CREATE TEMPORARY VIEW " + viewName + " AS "
                     "SELECT MAX(G.created) AS created, L.set_name, L.level_name "
                     "FROM (games G LEFT JOIN levels L on G.level_id=L.id)"
                     "GROUP BY G.level_id "
                     "ORDER BY G.created DESC";
        char *errMsg = nullptr;
        if (sqlite3_exec(myDb, query.c_str(), NULL, NULL, &errMsg) != SQLITE_OK) {
            SDL_Log("ERROR CREATING RecentsView: %s\n", errMsg);
            // fall back to pulling from levels?
            viewName = "levels";
        }
    }
    return viewName;
}

// public/interface methods

void CRUDsqlite::RecordGameStart(GamePointer &game) {
    auto url = game->loadedLevelInfo->URL();
    auto gameId = game->currentGameId;
    
    DBG_Log("sql", "CRUDsqlite::RecordGameStart({%0xd, %s})\n", gameId, url.c_str());
    int levelId = RecordLevelInfo(*game->loadedLevelInfo);
    
    int frameTime = -1;
    int spawnOrder = -1;
    bool wroteGame = false;
    
    for (int i = 0; i < kMaxAvaraPlayers; i++) {
        auto manager = (&game->itsNet->playerTable[i])->get();
        if (!manager) continue;
        auto thePlayer = manager->GetPlayer();
        if (!thePlayer) continue;
        //auto manager = thePlayer->itsManager.get();
        if (!wroteGame) {
            CPlayerManager* host = manager->ServerPlayer();
            auto hostConfig = host->TheConfiguration();
            frameTime = hostConfig.frameTime;
            spawnOrder = hostConfig.spawnOrder;
            DBG_Log("sql", "CRUDsqlite::RecordGameStart(%0xd) params %i %i %i %s %i",
                    gameId, levelId, frameTime, spawnOrder, GIT_VERSION, kAvaraNetVersion);
            InsertInto("games(id, level_id, frame_time, spawn_order, network_version, game_version) VALUES (?, ?, ?, ?, ?, ?)", "id", [&](sqlite3_stmt * stmt) {
                sqlite3_bind_int(stmt, 1, gameId);
                sqlite3_bind_int(stmt, 2, levelId);
                sqlite3_bind_int(stmt, 3, frameTime);
                sqlite3_bind_int(stmt, 4, spawnOrder);
                sqlite3_bind_int(stmt, 6, kAvaraNetVersion);
                sqlite3_bind_text(stmt, 5, GIT_VERSION, -1, SQLITE_STATIC);
            });
            wroteGame = true;
        }
        auto name = manager->GetPlayerName();
        auto slot = manager->Slot();
        auto team = thePlayer->teamColor;
        auto walker = (CWalkerActor*) thePlayer;
        int hull = walker->hullRes;
        int grenades = thePlayer->grenadeCount;
        int missiles = thePlayer->missileCount;
        int boosters = (int)thePlayer->boostsRemaining;
        
        auto playerConfig = manager->TheConfiguration();
        int hullColor = playerConfig.hullColor.GetRaw();
        int gunColor = playerConfig.gunColor.GetRaw();
        int cockpitColor = playerConfig.cockpitColor.GetRaw();
        int trimColor = playerConfig.trimColor.GetRaw();
        int playerId = InsertInto("players(name) VALUES (?)", { name }, "id");
        DBG_Log("sql", "CRUDsqlite::RecordGameStart(%0xd) games_players params %i %i %i %i %i %i %i %i %i %i %i",
                gameId, playerId, slot, team, hull, grenades, missiles, boosters, hullColor, gunColor, cockpitColor, trimColor);
        InsertInto("games_players(game_id, player_id, slot, team, hull_rsrc_id, grenades, missiles, boosters, hull_color, gun_color, cockpit_color, trim_color) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", {gameId, playerId, slot, team, hull, grenades, missiles, boosters, hullColor, gunColor, cockpitColor, trimColor});
    }

    
}

void CRUDsqlite::RecordFrames(GamePointer &game) {
    auto gameId = game->currentGameId;
    auto &film = game->itsFilm;
    size_t frameCount = film->GetWritten();
    
    sqlite3_exec(myDb, "BEGIN TRANSACTION", 0, 0, 0);
    auto reel = film->GetReelRef()->get();
    DBG_Log("sql", "CRUDsqlite::RecordFrames({%0xd, %i frames})\n", gameId, (int)reel->size());
    for(auto pff = reel->begin() + frameCount; pff != reel->end(); ++pff, ++frameCount) {
        //DBG_Log("sql", "CRUDsqlite::RecordFrames(): ff %i for slot %i", (int)frameCount, pff->slot);
        InsertInto("frame_funcs(game_id, frame_number, slot, down, up, held, mouse_delta_h, mouse_delta_v, button_status, msg_char) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", nullptr, [&](sqlite3_stmt * stmt) {
            sqlite3_bind_int(stmt, 1, gameId);
            sqlite3_bind_int(stmt, 2, (int)frameCount);
            sqlite3_bind_int(stmt, 3, pff->slot);
            sqlite3_bind_int(stmt, 4, pff->ft.down);
            sqlite3_bind_int(stmt, 5, pff->ft.up);
            sqlite3_bind_int(stmt, 6, pff->ft.held);
            sqlite3_bind_int(stmt, 7, pff->ft.mouseDelta.h);
            sqlite3_bind_int(stmt, 8, pff->ft.mouseDelta.v);
            sqlite3_bind_int(stmt, 9, pff->ft.buttonStatus);
            sqlite3_bind_text(stmt, 10, &pff->ft.msgChar, -1, SQLITE_TRANSIENT);
        });
    }
    film->SetWritten(frameCount);
    // mark game as recorded
    sqlite3_stmt * updateGameStmt;
    const char * updateGameSql = "UPDATE games SET total_frames = ?, filmed = 1 WHERE id = ?;";
    if (sqlite3_prepare_v2(myDb, updateGameSql, -1, &updateGameStmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(updateGameStmt, 1, (int)frameCount);
        sqlite3_bind_int(updateGameStmt, 2, gameId);
        if (sqlite3_step(updateGameStmt) != SQLITE_DONE) {
            SDL_Log("ERROR updating game %0xd %s\n", gameId, sqlite3_errmsg(myDb));
        }
        sqlite3_finalize(updateGameStmt);
    }
    else {
        SDL_Log("ERROR updating game %0xd %s\n", gameId, sqlite3_errmsg(myDb));
    }
    
    sqlite3_exec(myDb, "COMMIT", 0, 0, 0);
}

CRUD::RecentLevelsList CRUDsqlite::GetRecentLevels(int limit) {
    DBG_Log("sql", "CRUDsqlite::GetRecentLevels(%d)\n", limit);
    CRUD::RecentLevelsList recents;

    std::string query = "SELECT set_name, level_name FROM " + RecentsView() + " LIMIT " + std::to_string(limit);

    sqlite3_exec(myDb, query.c_str(), [](void* data, int argc, char** argv, char** colName) {
        CRUD::RecentLevelsList *list = (CRUD::RecentLevelsList *)data;
        list->push_back(LevelInfo(argv[0], argv[1]));
        return 0;
    }, &recents, nullptr);

    return recents;
}
