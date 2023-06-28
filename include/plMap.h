#include "data.h"
#include "fpsCommon.h"
#include <wchar.h>

#define DEFAULT_HEIGHT 16
#define DEFAULT_WIDTH 16

#define INI_MAP_ROW(pWcMatrixMap, row, wRowContent)             \
{                                                               \
    VERBOSE_ASSERT(wcslen(wRowContent) == DEFAULT_WIDTH, NULL); \
    size_t rowSize = DEFAULT_WIDTH * sizeof(wchar_t);           \
    memcpy((pWcMatrixMap)->content[row], wRowContent, rowSize); \
}

void plLoadMap(UINT16 fromFile);
void plCleanMap();
