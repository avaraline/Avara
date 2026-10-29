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

typedef std::function<void(sqlite3_stmt *)> SqlFunc;

class CRUDsqlite : public CRUD {
private:
    sqlite3 *myDb;
    void MigrateTables();
    int InsertInto(const std::string clause, const char* returnField, SqlFunc bind_values);
protected:
    int InsertInto(const std::string clause, const std::vector<std::string> textBinds = {}, const char* returnField = NULL);
    int InsertInto(const std::string clause, const std::vector<int> intBinds, const char* returnField = NULL);
    int RecordLevelInfo(const LevelInfo &info);
    std::string RecentsView();

public:
    CRUDsqlite();
    ~CRUDsqlite();

    virtual void RecordGameStart(GamePointer &game) override;
    virtual void RecordFrames(GamePointer &game) override;
    virtual RecentLevelsList GetRecentLevels(int limit) override;
};

#endif /* CRUDsqlite_h */
