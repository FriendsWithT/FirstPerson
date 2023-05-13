#include <windows.h>
#include "fpsCommon.h"

PROCESS_INFORMATION OsalSpawnChildProcess(const char *cmd);
HANDLE OsalPipeOpen(const char *name, UINT16 pipeExisted);
UINT16 OsalPipeMsgSend(HANDLE hPipe, void *data, size_t size);
void OsalPipeMsgReceive(HANDLE hPipe, void *data, size_t size);
void OsalPipeClose(HANDLE hOpenedPipe);