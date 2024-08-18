#pragma once

#include <string>

enum struct MsgAlignment {
    Right = -1,
    Left,
    Center
};

enum struct MsgCategory {
    System,
    Level,
    Error
};

struct MsgLine {
    std::string text;
    MsgAlignment align;
    MsgCategory category;
    int gameId;
};
