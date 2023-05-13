#include <windows.h>

HANDLE OsalDefaultThreadCreate(DWORD (*ThreadProc)(LPVOID), LPVOID lpParameter);