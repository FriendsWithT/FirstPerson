#include <windows.h>
#include <stdlib.h>
#include <wchar.h>

#include <osalOutput.h>
#include <data.h>
#include <w32Extras.h>

WcMatrixT *screenBuff;
HANDLE _hScreen;
DWORD _dwBytesWritten;

void OsalOutputInitialize(UINT16 nScreenHeight, UINT16 nScreenWidth)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    

    CONSOLE_FONT_INFOEX fontInfo = { sizeof(CONSOLE_FONT_INFOEX) };
    GetCurrentConsoleFontEx(hConsole, FALSE, &fontInfo);
    fontInfo.dwFontSize.X = 2;
    fontInfo.dwFontSize.Y = 4;
    SetCurrentConsoleFontEx(hConsole, FALSE, &fontInfo);    //adjust the font

    SMALL_RECT windowSize = {0, 0, nScreenWidth - 1, nScreenHeight - 1};
    COORD bufferSize = {nScreenWidth, nScreenHeight + 1};
    SetConsoleScreenBufferSize(hConsole, bufferSize);
    SetConsoleWindowInfo(hConsole, TRUE, &windowSize);  //adjust screen size

    ALLOC_WCMTX(screenBuff, nScreenHeight, nScreenWidth);
    _hScreen = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);

    SetConsoleActiveScreenBuffer(_hScreen);
    _dwBytesWritten = 0;
}

/*
 * OsalOutputWrite: write the content of global variable screenBuff to console
 */
void OsalOutputWrite()
{
    DWORD dwLength = screenBuff->size.nRow * screenBuff->size.nCol;
    COORD writeCoord;
    writeCoord.X = 0;
    writeCoord.Y = 0;
    wchar_t *flattened = NULL;
    CONTENT_FLATTEN(screenBuff, flattened);

    WriteConsoleOutputCharacterW(_hScreen, flattened, dwLength, writeCoord, &_dwBytesWritten);
    FREE_FLATTENED(flattened);
}

void OsalOutputFinalize()
{
    FREE_WCMTX(screenBuff);
    CloseHandle(_hScreen);
}
