#pragma once
#include "fpsCommon.h"
#include <stdlib.h>
#include <assert.h>
#include <windows.h>

typedef struct Size
{
    UINT16 nRow;
    UINT16 nCol;
}SizeT;

typedef struct Coord
{
    UINT16 X;
    UINT16 Y;
}CoordT;

typedef struct fCoord
{
    float X;
    float Y;
}fCoordT;

typedef struct PlayerInfo
{
    float fPlayerX;
    float fPlayerY;
    float fPlayerAngle;
}PlayerInfoT;

typedef struct WcMatrix
{
    SizeT size;
    wchar_t **content;
}WcMatrixT;


typedef struct PipePayload
{
    PlayerInfoT playerInfo;
    LARGE_INTEGER frameStartTime;
    LARGE_INTEGER frameEndTime;
    LARGE_INTEGER timeFreq;
    UINT16 bStop;
}PipePayloadT;

#define ALLOC_WCMTX(pWcMatrix, row, col)                                        \
{                                                                               \
    pWcMatrix = (WcMatrixT*) malloc( sizeof(WcMatrixT) );                       \
    (pWcMatrix)->content = (wchar_t **) malloc( row * sizeof(wchar_t *) );      \
    int i = 0;                                                                  \
    for (i; i < row; i++)                                                       \
    {                                                                           \
        (pWcMatrix)->content[i] = (wchar_t *) malloc( col * sizeof(wchar_t ) ); \
        wmemset((pWcMatrix)->content[i], L' ', col);                            \
    }                                                                           \
    (pWcMatrix)->size.nRow = row;                                               \
    (pWcMatrix)->size.nCol = col;                                               \
}

#define FREE_WCMTX(pWcMatrix)                   \
{                                               \
    int i = 0;                                  \
    for (i; i < (pWcMatrix)->size.nRow; i++)    \
    {                                           \
        free((pWcMatrix)->content[i]);          \
    }                                           \
    free((pWcMatrix)->content);                 \
    free(pWcMatrix);                            \
}

#define MERGE_WCMTXES(pWcBiggerMatrix, pWcSmallerMatrix, coord) \
{                                                               \
    /*invalid position check*/                                  \
    assert(coord.X + (pWcSmallerMatrix)->size.nCol < (pWcBiggerMatrix)->size.nCol && coord.Y + (pWcSmallerMatrix)->size.nRow < (pWcBiggerMatrix)->size.nRow);    \
                                                                \
    int i = 0;                                                  \
    for (i; i < (pWcSmallerMatrix)->size.nRow; i++)             \
        memcpy((pWcBiggerMatrix)->content[coord.Y + i] + coord.X, (pWcSmallerMatrix)->content[i], (pWcSmallerMatrix)->size.nCol * sizeof(wchar_t));  \
}

#define CONTENT_FLATTEN(pWcMatrix, pFlattenedBuff)                              \
{                                                                               \
    UINT32 n = (pWcMatrix)->size.nRow * (pWcMatrix)->size.nCol + 1;             \
    pFlattenedBuff = (wchar_t *) malloc( n * sizeof(wchar_t) );                 \
    wchar_t *whereToCpy = pFlattenedBuff;                                       \
    int i = 0;                                                                  \
    size_t rowSize = (pWcMatrix)->size.nCol * sizeof(wchar_t);                  \
    for (i; i < (pWcMatrix)->size.nRow; i++)                                    \
    {                                                                           \
        memcpy(whereToCpy, (pWcMatrix)->content[i], rowSize);                   \
        whereToCpy += (pWcMatrix)->size.nCol;                                   \
    }                                                                           \
    pFlattenedBuff[n - 1] = '\0';                                               \
}

#define FREE_FLATTENED(pFlattenedBuff)  \
{                                       \
    free(pFlattenedBuff);               \
}
