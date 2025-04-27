//
//  CRUDsqlite.h
//  Avara
//
//  Created by Tom on 4/19/25.
//

#pragma once

#ifndef CRUDsqlite_h
#define CRUDsqlite_h

#include "CRUD.h"

#include "sqlite3.h"
#include <functional>

class CRUDsqlite : public CRUD {
private:
    sqlite3 *myDb;
    void MigrateTables();
    int InsertInto(const std::string clause, const char* returnField, std::function<void(sqlite3_stmt *)> bind_values);
protected:
    int InsertInto(const std::string clause, const std::vector<std::string> textBinds = {}, const char* returnField = NULL);
    int InsertInto(const std::string clause, const std::vector<int> intBinds, const char* returnField = NULL);
    int RecordLevelInfo(const LevelInfo &info);
    std::string RecentsView();

public:
    CRUDsqlite();
    ~CRUDsqlite();

    virtual void RecordGameStart(int gameId, const LevelInfo &info) override;

    virtual RecentLevelsList GetRecentLevels(int limit) override;
};

#endif /* CRUDsqlite_h */
