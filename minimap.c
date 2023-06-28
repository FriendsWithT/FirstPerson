#include <data.h>
#include <plMap.h>
#include <fpsCommon.h>
#include <osalProcess.h>

#include <windows.h>
#include <math.h>

#define SCREEN_HEIGHT (pMap->size.nRow + 10)
#define SCREEN_WIDTH 45

BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType);
void OutputInitialize();
void DisplayPosAndStat(PipePayloadT *payload, CoordT mergePos);
void OutputPrint();
void OutputFinalize();
void MinimapInitialize();

HANDLE hPipe = NULL;
WcMatrixT *screenBuff = NULL;
HANDLE hScreen;

extern WcMatrixT *pMap;

int main(int argc, char** argv)
{
    MinimapInitialize();

    hPipe = OsalPipeOpen(IPC_PIPE_NAME, TRUE);      //pipe to receive data from the main program 
    VERBOSE_ASSERT(hPipe != INVALID_HANDLE_VALUE, "Launch firstPerson.exe first");  //exit if it's a nonexisted pipe

    INT16 bRunning = TRUE;
    PipePayloadT *payload = (PipePayloadT*) malloc( sizeof(PipePayloadT) );

    while (bRunning)
    {
        OsalPipeMsgReceive(hPipe, payload, sizeof(PipePayloadT));

        CoordT mergePos = {0 ,0};
        MERGE_WCMTXES(screenBuff, pMap, mergePos);

        DisplayPosAndStat(payload, mergePos);

        OutputPrint();

        if (payload->bStop)
            bRunning = FALSE;
    }

    free(payload);
    OutputFinalize();
    OsalPipeClose(hPipe);
}

BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType)
{
    switch (dwCtrlType)
    {
        case CTRL_CLOSE_EVENT:
            OutputFinalize();
            OsalPipeClose(hPipe);
            return TRUE; // Indicate that the event has been handled
        default:
            return FALSE; // Indicate that the event has not been handled
    }
}

void MinimapInitialize()
{
    if (!SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE))   //handle finalizing when user click close button
    {
        VERBOSE_ASSERT(0, NULL);
    }

    UINT16 bFromFile = TRUE;
    plLoadMap(bFromFile);
    OutputInitialize();

    const wchar_t *creditInfo = L"--Made by psyke134, inspired by Javidx9--";
    const wchar_t *movementInfo = L"W,S,A,D to move.";
    const wchar_t *rotateInfo = L"Q,E to rotate.";
    const wchar_t *minimapInfo = L"TAB to open/close minimap.";
    memcpy(screenBuff->content[pMap->size.nRow + 1], creditInfo, wcslen(creditInfo) * sizeof(wchar_t));
    memcpy(screenBuff->content[pMap->size.nRow + 2], movementInfo, wcslen(movementInfo) * sizeof(wchar_t));
    memcpy(screenBuff->content[pMap->size.nRow + 3], rotateInfo, wcslen(rotateInfo) * sizeof(wchar_t));
    memcpy(screenBuff->content[pMap->size.nRow + 4], minimapInfo, wcslen(minimapInfo) * sizeof(wchar_t));
}

void OutputInitialize()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SMALL_RECT windowSize = {0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1};
    COORD bufferSize = {SCREEN_WIDTH, SCREEN_HEIGHT + 1};
    SetConsoleScreenBufferSize(hConsole, bufferSize);
    SetConsoleWindowInfo(hConsole, TRUE, &windowSize);  //adjust screen size

    ALLOC_WCMTX(screenBuff, SCREEN_HEIGHT, SCREEN_WIDTH);
    hScreen = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hScreen);
}

void DisplayPosAndStat(PipePayloadT *payload, CoordT mergePos)
{  
    PlayerInfoT playerInfo = payload->playerInfo;
    screenBuff->content[mergePos.Y + (int)playerInfo.playerPos.Y][mergePos.X + (int)playerInfo.playerPos.X] = 'P';    //indicate player's position

    swprintf(screenBuff->content[pMap->size.nRow + 5], L"X[%d], Y[%d]", (int)payload->playerInfo.playerPos.X, (int)payload->playerInfo.playerPos.Y);
    swprintf(screenBuff->content[pMap->size.nRow + 6], L"Angle(degree) [%d]", RADIAN_TO_DEGREE(payload->playerInfo.fPlayerAngle));

    long elapsedTime = labs(((double)(payload->frameStartTime.QuadPart - payload->frameEndTime.QuadPart)) * BILLION / payload->timeFreq.QuadPart);
    wmemset(screenBuff->content[pMap->size.nRow + 7], L' ', SCREEN_WIDTH);
    swprintf(screenBuff->content[pMap->size.nRow + 7], L"Frame duaration(nanosec) [%ld]", elapsedTime);
    wmemset(screenBuff->content[pMap->size.nRow + 8], L' ', SCREEN_WIDTH);
    swprintf(screenBuff->content[pMap->size.nRow + 8], L"Frame drawn per sec [%ld]", lround((double)BILLION / (double)elapsedTime));
}

void OutputPrint()
{
    DWORD dwLength = screenBuff->size.nRow * screenBuff->size.nCol;
    DWORD dwBytesWritten = 0;
    COORD writeCoord;
    writeCoord.X = 0;
    writeCoord.Y = 0;
    wchar_t *flattened = NULL;
    CONTENT_FLATTEN(screenBuff, flattened);

    WriteConsoleOutputCharacterW(hScreen, flattened, dwLength, writeCoord, &dwBytesWritten);
    FREE_FLATTENED(flattened);
}

void OutputFinalize()
{
    FREE_WCMTX(screenBuff);
    plCleanMap();
    CloseHandle(hScreen);
}