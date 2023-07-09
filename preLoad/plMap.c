#include <plMap.h>
#include <stdio.h>
#include <windows.h>
#include <string.h>

#define FILE_NAME "map.txt"
#define MAX_SIZE 100

#define STR_TO_WSTR(text, wText)                        \
{                                                       \
    const size_t size = strlen(text) + 1;               \
    wText = (wchar_t *) malloc(size * sizeof(wchar_t)); \
    mbstowcs(wText, text, size);                        \
}

WcMatrixT *pMap = NULL;
CoordT *spawnPt = NULL;

void _plLoadDefault();
void _plLoadFromFile();

void plLoadMap(UINT16 fromFile)
{
    if (fromFile)
        _plLoadFromFile();
    else
        _plLoadDefault();
}

void plCleanMap()
{
    free(pMap);
    free(spawnPt);
}

void _plLoadDefault()
{
    UINT16 mapHeight = 16;
    UINT16 mapWidth = 16;

    ALLOC_WCMTX(pMap, mapHeight, mapWidth);

    INI_MAP_ROW(pMap, 0,    L"################");
    INI_MAP_ROW(pMap, 1,    L"#...............");
    INI_MAP_ROW(pMap, 2,    L"#...............");
    INI_MAP_ROW(pMap, 3,    L"#.........##...#");
    INI_MAP_ROW(pMap, 4,    L"#..............#");
    INI_MAP_ROW(pMap, 5,    L"#..............#");
    INI_MAP_ROW(pMap, 6,    L"#..............#");
    INI_MAP_ROW(pMap, 7,    L"#####..........#");
    INI_MAP_ROW(pMap, 8,    L"#..............#");
    INI_MAP_ROW(pMap, 9,    L"#..............#");
    INI_MAP_ROW(pMap, 10,   L"#...##.........#");
    INI_MAP_ROW(pMap, 11,   L"#..............#");
    INI_MAP_ROW(pMap, 12,   L"#.....##########");
    INI_MAP_ROW(pMap, 13,   L"#..............#");
    INI_MAP_ROW(pMap, 14,   L"#..............#");
    INI_MAP_ROW(pMap, 15,   L"################");
}

void _plLoadFromFile()
{
    FILE *pFile = fopen(FILE_NAME, "r");
    VERBOSE_ASSERT(pFile != NULL, "Missing map.txt file");

    char **buff = (char **) malloc(MAX_SIZE * sizeof(char *));
    *buff = (char *) malloc(MAX_SIZE * sizeof(char));
    
    char *currLine = *buff;
    UINT16 bFirstLine = TRUE;
    UINT16 mapWidth = 0;
    UINT16 mapHeight = 0;

    while (fgets(currLine, MAX_SIZE - 1, pFile) != NULL)
    {
        UINT16 lineLen = (UINT16) strlen(currLine);
        if (currLine[lineLen - 1] == '\n')
            currLine[lineLen - 1] = '\0';
        if (bFirstLine)
        {
            mapWidth = (UINT16) strlen(currLine);
            bFirstLine = FALSE;
        }
        else
        {
            lineLen = (UINT16) strlen(currLine);
            VERBOSE_ASSERT(lineLen == mapWidth, "Row length mismatch");
        }

        mapHeight++;
        VERBOSE_ASSERT(mapHeight <= MAX_SIZE, "Map too long");
        // currLine = (char *) malloc(MAX_SIZE * sizeof(char));
        *(buff + mapHeight) = (char *) malloc(MAX_SIZE * sizeof(char));
        currLine = *(buff + mapHeight);
    }

    ALLOC_WCMTX(pMap, mapHeight, mapWidth);
    UINT16 i = 0;
    for (i; i < mapHeight; i++)
    {
        currLine = *(buff + i);

        int len = strlen(currLine);
        int j = 0;
        for (j; j < len; j++)
        {
            VERBOSE_ASSERT(currLine[j] == '#' || currLine[j] == '.' || currLine[j] == 'x', "Map tokens should be: \"#\" for wall, \".\" for empty space, \"x\" for spawn point. Please also remove any newlines, spaces if any.");
            if (currLine[j] == 'x')
            {
                VERBOSE_ASSERT(!spawnPt, "Spawn point already existed");   //spawn point already exist
                spawnPt = (CoordT *) malloc(sizeof(CoordT));
                spawnPt->X = j;
                spawnPt->Y = i;
            }
        }

        wchar_t *wRowContent = NULL;
        STR_TO_WSTR(currLine, wRowContent);

        size_t rowSize = mapWidth * sizeof(wchar_t);
        memcpy(pMap->content[i], wRowContent, rowSize);

        free(wRowContent);
        free(currLine);
    }

    VERBOSE_ASSERT(spawnPt, "Missing spawn point");    //missing spawn point
    VERBOSE_ASSERT(mapHeight > 4 && mapWidth > 4, "Map too narrow");
    free(buff);
    fclose(pFile);
}
