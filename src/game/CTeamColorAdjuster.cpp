#include "CTeamColorAdjuster.h"

#include "ARGBColor.h"
#include "AvaraDefines.h"
#include "ColorManager.h"

void CTeamColorAdjuster::BeginScript() {
    CAbstractActor::BeginScript();

    ProgramVariable(iMask, -1);
}

CAbstractActor *CTeamColorAdjuster::EndScript() {
    CAbstractActor::EndScript();

    colorMask = ReadLongVar(iMask);

    ARGBColor newColor = GetPixelColor();

    // Currently unused, but theoretically should be supported. We just don't do highlighting yet.
    // auto newHighlightColor = GetOtherPixelColor();

    for (uint8_t i = 1; i <= kMaxTeamColors; i++) {
        uint16_t teamMask = 1 << i;
        if (colorMask & teamMask) {
            ColorManager::overrideTeamColor(i, newColor);
        }
    }

    Dispose();
    return NULL;
}
