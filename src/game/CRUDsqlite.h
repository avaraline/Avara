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
    int RecordLevelInfo(LevelInfo &info);

public:
    CRUDsqlite();
    ~CRUDsqlite();

    virtual void RecordRecentLevel(LevelInfo &info) override;

    virtual RecentLevelsList GetRecentLevels(int count = 50) override;
};

#endif /* CRUDsqlite_h */
