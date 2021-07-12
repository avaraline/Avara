#include "CColorManager.h"

uint32_t CColorManager::teamColors[kMaxTeamColors + 1] = {
    0xffffffff,
    0xff006600,
    0xffcccc00,
    0xffcc0000,
    0xffcc0099,
    0xff9900cc,
    0xff0099cc,
    0xffffa500,
    0xff89c4c7
};

uint32_t CColorManager::teamTextColors[kMaxTeamColors + 1] = {
    0xff000000,
    0xff000000,
    0xff000000,
    0xff000000,
    0xff000000,
    0xff000000,
    0xff000000,
    0xff000000,
    0xff000000
};

void CColorManager::setColorBlind(ColorBlindMode mode) {
    switch (mode) {
        case Off:
            CColorManager::teamColors[0] = 0xffffffff;
            CColorManager::teamColors[1] = 0xff006600;
            CColorManager::teamColors[2] = 0xffcccc00;
            CColorManager::teamColors[3] = 0xffcc0000;
            CColorManager::teamColors[4] = 0xffcc0099;
            CColorManager::teamColors[5] = 0xff9900cc;
            CColorManager::teamColors[6] = 0xff0099cc;
            CColorManager::teamColors[7] = 0xffffa500;
            CColorManager::teamColors[8] = 0xff89c4c7;
            CColorManager::teamTextColors[0] = 0xff000000;
            CColorManager::teamTextColors[1] = 0xff000000;
            CColorManager::teamTextColors[2] = 0xff000000;
            CColorManager::teamTextColors[3] = 0xff000000;
            CColorManager::teamTextColors[4] = 0xff000000;
            CColorManager::teamTextColors[5] = 0xff000000;
            CColorManager::teamTextColors[6] = 0xff000000;
            CColorManager::teamTextColors[7] = 0xff000000;
            CColorManager::teamTextColors[8] = 0xff000000;
            break;
        case Deuteranopia:
            CColorManager::teamColors[0] = 0xffffffff;
            CColorManager::teamColors[1] = 0xff006600;
            CColorManager::teamColors[2] = 0xffcccc00;
            CColorManager::teamColors[3] = 0xffcc0000;
            CColorManager::teamColors[4] = 0xffcc0099;
            CColorManager::teamColors[5] = 0xff9900cc;
            CColorManager::teamColors[6] = 0xff0099cc;
            CColorManager::teamColors[7] = 0xffffa500;
            CColorManager::teamColors[8] = 0xff89c4c7;
            CColorManager::teamTextColors[0] = 0xff000000;
            CColorManager::teamTextColors[1] = 0xff000000;
            CColorManager::teamTextColors[2] = 0xff000000;
            CColorManager::teamTextColors[3] = 0xff000000;
            CColorManager::teamTextColors[4] = 0xff000000;
            CColorManager::teamTextColors[5] = 0xff000000;
            CColorManager::teamTextColors[6] = 0xff000000;
            CColorManager::teamTextColors[7] = 0xff000000;
            CColorManager::teamTextColors[8] = 0xff000000;
            break;
        case Protanopia:
            CColorManager::teamColors[0] = 0xffffffff;
            CColorManager::teamColors[1] = 0xff006600;
            CColorManager::teamColors[2] = 0xffcccc00;
            CColorManager::teamColors[3] = 0xffcc0000;
            CColorManager::teamColors[4] = 0xffcc0099;
            CColorManager::teamColors[5] = 0xff9900cc;
            CColorManager::teamColors[6] = 0xff0099cc;
            CColorManager::teamColors[7] = 0xffffa500;
            CColorManager::teamColors[8] = 0xff89c4c7;
            CColorManager::teamTextColors[0] = 0xff000000;
            CColorManager::teamTextColors[1] = 0xff000000;
            CColorManager::teamTextColors[2] = 0xff000000;
            CColorManager::teamTextColors[3] = 0xff000000;
            CColorManager::teamTextColors[4] = 0xff000000;
            CColorManager::teamTextColors[5] = 0xff000000;
            CColorManager::teamTextColors[6] = 0xff000000;
            CColorManager::teamTextColors[7] = 0xff000000;
            CColorManager::teamTextColors[8] = 0xff000000;
            break;
        case Tritanopia:
            CColorManager::teamColors[0] = 0xffffffff;
            CColorManager::teamColors[1] = 0xff006600;
            CColorManager::teamColors[2] = 0xffcccc00;
            CColorManager::teamColors[3] = 0xffcc0000;
            CColorManager::teamColors[4] = 0xffcc0099;
            CColorManager::teamColors[5] = 0xff9900cc;
            CColorManager::teamColors[6] = 0xff0099cc;
            CColorManager::teamColors[7] = 0xffffa500;
            CColorManager::teamColors[8] = 0xff89c4c7;
            CColorManager::teamTextColors[0] = 0xff000000;
            CColorManager::teamTextColors[1] = 0xff000000;
            CColorManager::teamTextColors[2] = 0xff000000;
            CColorManager::teamTextColors[3] = 0xff000000;
            CColorManager::teamTextColors[4] = 0xff000000;
            CColorManager::teamTextColors[5] = 0xff000000;
            CColorManager::teamTextColors[6] = 0xff000000;
            CColorManager::teamTextColors[7] = 0xff000000;
            CColorManager::teamTextColors[8] = 0xff000000;
            break;
    }
}
