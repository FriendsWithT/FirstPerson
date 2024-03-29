#include <math.h>
#include <wchar.h>

#include <osalOutput.h>
#include <osalInput.h>
#include <osalThread.h>
#include <osalProcess.h>
#include <data.h>
#include <plMap.h>
#include <fpsCommon.h>
#include <w32Extras.h>

#define SCREEN_HEIGHT 160
#define SCREEN_WIDTH 480
#define RAY_MAX_DIS 16

PlayerInfoT playerInfo;
UINT16 bPlaying = TRUE;

float fFOV = PI / 4.0f;     //field of view

extern WcMatrixT *screenBuff;
extern WcMatrixT *pMap;
extern CoordT *spawnPt;

PROCESS_INFORMATION minimapProcInfo;
HANDLE hPipe = INVALID_HANDLE_VALUE;
HANDLE hPipeMsgThrd = INVALID_HANDLE_VALUE;
UINT16 bMinimapStop = FALSE;

/*
 *  Prototypes and macros
 */

void GameInitialize();
void GameFinalize();
float CastRay(int screenCol);
void DrawScreenCol(int col, float fDisToWall);
void OnKeyDown(void *data);
void LaunchMiniMap();
void CloseMiniMap();
DWORD WINAPI PipeMsgLoop(LPVOID thrdArg);
BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType);

#define WALL_SHADE_FROM_DIS(fDisToWall, wallShadingCh)                      \
{                                                                           \
    wallShadingCh = fDisToWall <= ((float)RAY_MAX_DIS / 4.0f) ? 0x2588 :    /*close to a wall, so we use brightest character*/    \
                    fDisToWall <= ((float)RAY_MAX_DIS / 3.0f) ? 0x2593 :    /*the further the wall, the less bright character we use*/\
                    fDisToWall <= ((float)RAY_MAX_DIS / 2.0f) ? 0x2592 :    \
                    fDisToWall <= ((float)RAY_MAX_DIS) ? 0x2591 : ' ';      \
}

#define FLOOR_SHADE_FROM_ROW(rowNo, floorShadingCh)                         \
{                                                                           \
    UINT32 halfScreenHeight = SCREEN_HEIGHT / 2;                            \
    UINT32 offsetFromMid = rowNo - halfScreenHeight;                        \
    float ratio = 1.0f - (float)offsetFromMid / (float)halfScreenHeight;    \
                                                                            \
    floorShadingCh = ratio < 0.25 ? '#' :                                   \
                     ratio < 0.5 ?  'x' :                                   \
                     ratio < 0.75 ? '.' :                                   \
                     ratio < 0.9 ?  '-' :                                   \
                     ' ';                                                   \
}

#define OUT_OF_MAP(coord)                   \
(                                           \
    ((int)coord.Y) >= pMap->size.nRow ||    \
    ((int)coord.Y) < 0 ||                   \
    ((int)coord.X) >= pMap->size.nCol ||    \
    ((int)coord.X) < 0                      \
    ? TRUE : FALSE                          \
)

#define MINIMAP_ON ((hPipe != INVALID_HANDLE_VALUE && hPipeMsgThrd != INVALID_HANDLE_VALUE) ? TRUE : FALSE)
#define MINIMAP_OFF ((hPipe == INVALID_HANDLE_VALUE && hPipeMsgThrd == INVALID_HANDLE_VALUE) ? TRUE : FALSE)

#define WALL_CH '#'

/*
 *  =====================================
 *  ========Program's entry point========
 *  =====================================
 */

int main(int argc, char** argv)
{
    OsalOutputInitialize(SCREEN_HEIGHT, SCREEN_WIDTH);  //set up console properties
    GameInitialize();   //map loading, set up player position

    OsalInputKeyDownConnect(OnKeyDown);     //register key down event behavior
    OsalInputStartReceiving();

    LaunchMiniMap();

    while(bPlaying)
    {
        UINT16 x = 0;
        for (x; x < SCREEN_WIDTH; x++)      //for every collumn cast a ray and draw that screen collumn base on ray's travelled distance
        {
            float fDisToWall = CastRay(x);

            DrawScreenCol(x, fDisToWall);
        }

        OsalOutputWrite();      //write actual data to console screen
    }

    if (MINIMAP_ON)
        CloseMiniMap();
    OsalInputStopReceiving();
    OsalOutputFinalize();
    GameFinalize();
}

void GameInitialize()
{
    if (!SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE))   //handle finalizing when user click close button
    {
        VERBOSE_ASSERT(0, NULL);
    }

    UINT16 bFromFile = TRUE;
    plLoadMap(bFromFile);

    playerInfo.playerPos.X = (float) spawnPt->X;
    playerInfo.playerPos.Y = (float) spawnPt->Y;
    playerInfo.fPlayerAngle = 0.0f;
}

void GameFinalize()
{
    plCleanMap();
}

/*
 * Cast a single ray for each screen collumn and return how far the ray has travelled
 */
float CastRay(int screenCol)
{
    float fRayAngle = (playerInfo.fPlayerAngle - fFOV / 2.0f) + ((float)screenCol / (float)SCREEN_WIDTH) * fFOV;
    float fDisToWall = 0;
    UINT16 bHitWall = FALSE;

    fCoordT eyeVector;      //indicates the direction of the ray, it's also a unit vector
    eyeVector.X = sin(fRayAngle);
    eyeVector.Y = cos(fRayAngle);

    float fMaxDis = (float)RAY_MAX_DIS;

    while (!bHitWall && fDisToWall < fMaxDis)
    {
        fDisToWall += 0.1f;

        CoordT rayEndPoint;
        rayEndPoint.X = (int)(playerInfo.playerPos.X + eyeVector.X * fDisToWall);
        rayEndPoint.Y = (int)(playerInfo.playerPos.Y + eyeVector.Y * fDisToWall);

        if (OUT_OF_MAP(rayEndPoint))
        {
            // Ray has travelled out of map
            bHitWall = TRUE;
            fDisToWall = fMaxDis;
        }
        else
        {
            if (pMap->content[rayEndPoint.Y][rayEndPoint.X] == WALL_CH)
                bHitWall = TRUE;
        }
    }

    return fDisToWall;
}

/*
 * Base on how far the ray has travelled, draw one collumn into the screen buffer
 * A collumn consists of: ceiling part, wall part, floor part 
 */
void DrawScreenCol(int col, float fDisToWall)
{
    float fMaxDis = (float)RAY_MAX_DIS;
    int wallPortion = fDisToWall < fMaxDis ? ((float)SCREEN_HEIGHT / fDisToWall) : 0;    //no wall drawing if too far away.
    int nCeiling = ((float)SCREEN_HEIGHT / 2.0f) - wallPortion;
    int nFloor = SCREEN_HEIGHT - nCeiling;

    int y = 0;
    for (y; y < SCREEN_HEIGHT; y++)
    {
        if (y < nCeiling)
            screenBuff->content[y][col] = ' ';
        else if (y < nFloor)
        {
            wchar_t wallShadingCh;
            WALL_SHADE_FROM_DIS(fDisToWall, wallShadingCh);
            screenBuff->content[y][col] = wallShadingCh;
        }
        else
        {
            wchar_t floorShadeCh;
            FLOOR_SHADE_FROM_ROW(y, floorShadeCh);
            screenBuff->content[y][col] = floorShadeCh;
        }
    }
}

void OnKeyDown(void *data)
{
    int *chCode = (int*)data;

    PlayerInfoT playerInfoBak = playerInfo;
    switch (*chCode)
    {
    case 'Q':
        playerInfo.fPlayerAngle -= 0.05f;
        break;

    case 'E':
        playerInfo.fPlayerAngle += 0.05f;
        break;

    case 'W':
        playerInfo.playerPos.X += sin(playerInfo.fPlayerAngle) * 0.1f;
        playerInfo.playerPos.Y += cos(playerInfo.fPlayerAngle) * 0.1f;
        break;

    case 'S':
        playerInfo.playerPos.X -= sin(playerInfo.fPlayerAngle) * 0.1f;
        playerInfo.playerPos.Y -= cos(playerInfo.fPlayerAngle) * 0.1f;
        break;

    case 'D':
        playerInfo.playerPos.X += cos(playerInfo.fPlayerAngle) * 0.1f;
        playerInfo.playerPos.Y += - (sin(playerInfo.fPlayerAngle) * 0.1f);
        break;

    case 'A':
        playerInfo.playerPos.X += - (cos(playerInfo.fPlayerAngle) * 0.1f);
        playerInfo.playerPos.Y += sin(playerInfo.fPlayerAngle) * 0.1f;
        break;

    case '\t':
        if (MINIMAP_OFF)
            LaunchMiniMap();
        else if (MINIMAP_ON)
            CloseMiniMap();
        else
            VERBOSE_ASSERT(0, "Bad operation");
        break;

    case '0':
        bPlaying = FALSE;
        break;
    }

    if (pMap->content[(int)playerInfo.playerPos.Y][(int)playerInfo.playerPos.X] == WALL_CH || OUT_OF_MAP(playerInfo.playerPos))
        playerInfo = playerInfoBak;     //if hits a wall or OOM, then undo the move
}

void LaunchMiniMap()
{
    VERBOSE_ASSERT(MINIMAP_OFF, "Minimap already opened");

    bMinimapStop = FALSE;
    minimapProcInfo = OsalSpawnChildProcess("./minimap.exe");
    hPipe = OsalPipeOpen(IPC_PIPE_NAME, FALSE);
    hPipeMsgThrd = OsalDefaultThreadCreate(PipeMsgLoop, NULL);
}

void CloseMiniMap()
{
    VERBOSE_ASSERT(MINIMAP_ON, "Minimap hasn't been launched yet");

    bMinimapStop = TRUE;    //this will be sent to minimap process via pipe
    WaitForSingleObject(minimapProcInfo.hProcess, INFINITE);     //waits for the process to exit
    CloseHandle(minimapProcInfo.hProcess);
    CloseHandle(minimapProcInfo.hThread);

    WaitForSingleObject(hPipeMsgThrd, INFINITE);
    CloseHandle(hPipeMsgThrd);
    OsalPipeClose(hPipe);

    hPipe = INVALID_HANDLE_VALUE;
    hPipeMsgThrd = INVALID_HANDLE_VALUE;
}

DWORD WINAPI PipeMsgLoop(LPVOID thrdArg)
{
    UINT16 bSending = TRUE;
    while (bSending)
    {
        VERBOSE_ASSERT(hPipe, "Invalid pipe");

        PipePayloadT payload;
        payload.playerInfo = playerInfo;
        payload.bStop = bMinimapStop;

        UINT16 writeResult = OsalPipeMsgSend(hPipe, &payload, sizeof(payload));
        if (writeResult == FALSE)
            bSending = FALSE;
    }
}

BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType)
{
    switch (dwCtrlType)
    {
        case CTRL_CLOSE_EVENT:
            if (MINIMAP_ON)
                CloseMiniMap();
            OsalInputStopReceiving();
            OsalOutputFinalize();
            GameFinalize();

            return TRUE; // Indicate that the event has been handled
        default:
            return FALSE; // Indicate that the event has not been handled
    }
}