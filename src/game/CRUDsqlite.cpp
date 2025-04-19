//
//  CRUDsqlite.cpp
//  Avara
//
//  Created by Tom on 4/19/25.
//

#include "CRUDsqlite.h"

#include <SDL2/SDL.h>

CRUDsqlite::CRUDsqlite() {
    char *ppath = SDL_GetPrefPath("Avaraline", "Avara");
    auto path = std::string(ppath) + "avara.db";

    // creates database if it doesn't exist
    if (sqlite3_open(path.c_str(), &myDb) == SQLITE_OK) {
        SDL_Log("Database opened: %s\n", path.c_str());
        // foreign key enforcement must be enabled on each connection
        sqlite3_exec(myDb, "PRAGMA foreign_keys = ON", NULL, 0, NULL);
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
        "CREATE TABLE recents        (created timestamp default current_timestamp, "
                                     "level_id INTEGER NOT NULL, "
                                     "FOREIGN KEY(level_id) REFERENCES levels(id))",
    },
    // all subsequent migrations look something like this (DO NOT CHANGE PREVIOUS MIGRATIONS)
//        {
//            "CREATE TABLE new_table (...);",
//            "UPDATE TABLE existing_table (...);"
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
    SDL_Log("CRUDsqlite::InsertInto(%s, (...), %s)\n", clause.c_str(), returnField);

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
    if (rc != SQLITE_OK) {
        SDL_Log("ERROR inserting data: %s\n", sqlite3_errmsg(myDb));
        return 0;
    }

    sqlite3_step(stmt);
    int result = 0;
    if (returnField) {
        // get the requested returnField
        result = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    return result;
}

// an insert utility that also helps to avoid SQL injection and can return the id of the inserted/matching record
int CRUDsqlite::InsertInto(const std::string clause, const std::vector<std::string> textBinds, const char* returnField)
{
    SDL_Log("CRUDsqlite::InsertInto(%s, strings[..], %s)\n", clause.c_str(), returnField);

    return InsertInto(clause, returnField, [&](sqlite3_stmt * stmt) {
        for (int i = 0; i < textBinds.size(); i++) {
            sqlite3_bind_text(stmt, i+1, textBinds[i].c_str(), -1, SQLITE_TRANSIENT);
        }
    });
}

// same as above but for inserting ints
int CRUDsqlite::InsertInto(const std::string clause, const std::vector<int> intBinds, const char* returnField)
{
    SDL_Log("CRUDsqlite::InsertInto(%s, ints[..], %s)\n", clause.c_str(), returnField);

    return InsertInto(clause, returnField, [&](sqlite3_stmt * stmt) {
        for (int i = 0; i < intBinds.size(); i++) {
            sqlite3_bind_int(stmt, i+1, intBinds[i]);
        }
    });
}


int CRUDsqlite::RecordLevelInfo(const LevelInfo& info) {
    SDL_Log("CRUDsqlite::RecordLevelInfo(%s)\n", info.URL().c_str());

    int levelId = InsertInto("levels(url, set_name, level_name) VALUES (?,?,?)",
                             { info.URL(), info.setTag, info.levelName }, "id");

    for (auto author: info.Authors()) {
        int authorId = InsertInto("authors(name) VALUES (?)", { author }, "id");

        InsertInto("levels_authors(level_id, author_id) VALUES (?, ?)", { levelId, authorId });
    }

    return levelId;
}


// public/interface methods

void CRUDsqlite::RecordRecentLevel(const LevelInfo& info) {
    SDL_Log("CRUDsqlite::RecordRecentLevel(%s)\n", info.URL().c_str());

    int levelId = RecordLevelInfo(info);

    InsertInto("recents(level_id) VALUES (?)", { levelId });
}

CRUD::RecentLevelsList CRUDsqlite::GetRecentLevels(int count) {
    SDL_Log("CRUDsqlite::GetRecentLevels(%d)\n", count);
    CRUD::RecentLevelsList recents;

    auto query = "SELECT L.set_name, L.level_name "
                 "FROM (recents R LEFT JOIN levels L on R.level_id=L.id) "
                 "ORDER BY R.created DESC";

    sqlite3_exec(myDb, query, [](void* data, int argc, char** argv, char** colName) {
        CRUD::RecentLevelsList *list = (CRUD::RecentLevelsList *)data;
        list->push_back(LevelInfo(argv[0], argv[1]));
        return 0;
    }, &recents, NULL);

    return recents;
}
