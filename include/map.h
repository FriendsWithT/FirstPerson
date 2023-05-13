#include "data.h"
#include <wchar.h>

#define MAP_HEIGHT 16
#define MAP_WIDTH 16

#define INI_MAP_ROW(pWcMatrixMap, row, wRowContent)             \
{                                                               \
    assert(wcslen(wRowContent) == MAP_WIDTH);                   \
    size_t rowSize = MAP_WIDTH * sizeof(wchar_t);               \
    memcpy((pWcMatrixMap)->content[row], wRowContent, rowSize); \
}

#define LOAD_MAP(pWcMatrixMap)                              \
{                                                           \
    ALLOC_WCMTX(pWcMatrixMap, MAP_HEIGHT, MAP_WIDTH);       \
                                                            \
    INI_MAP_ROW(pWcMatrixMap, 0,    L"################");   \
    INI_MAP_ROW(pWcMatrixMap, 1,    L"#...............");   \
    INI_MAP_ROW(pWcMatrixMap, 2,    L"#...............");   \
    INI_MAP_ROW(pWcMatrixMap, 3,    L"#.........##...#");   \
    INI_MAP_ROW(pWcMatrixMap, 4,    L"#..............#");   \
    INI_MAP_ROW(pWcMatrixMap, 5,    L"#..............#");   \
    INI_MAP_ROW(pWcMatrixMap, 6,    L"#..............#");   \
    INI_MAP_ROW(pWcMatrixMap, 7,    L"#####..........#");   \
    INI_MAP_ROW(pWcMatrixMap, 8,    L"#..............#");   \
    INI_MAP_ROW(pWcMatrixMap, 9,    L"#..............#");   \
    INI_MAP_ROW(pWcMatrixMap, 10,   L"#...##.........#");   \
    INI_MAP_ROW(pWcMatrixMap, 11,   L"#..............#");   \
    INI_MAP_ROW(pWcMatrixMap, 12,   L"#.....##########");   \
    INI_MAP_ROW(pWcMatrixMap, 13,   L"#..............#");   \
    INI_MAP_ROW(pWcMatrixMap, 14,   L"#..............#");   \
    INI_MAP_ROW(pWcMatrixMap, 15,   L"################");   \
}

#define CLEAN_MAP(pWcMatrixMap) \
{                               \
    FREE_WCMTX(pWcMatrixMap);   \
}
