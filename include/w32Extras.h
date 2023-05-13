#pragma once
#include <windows.h>
#include <time.h>

/*
 *  adding some win32 data type and function declarations that is not available in TCC
 */

typedef struct _CONSOLE_FONT_INFOEX {
  ULONG cbSize;
  DWORD nFont;
  COORD dwFontSize;
  UINT  FontFamily;
  UINT  FontWeight;
  WCHAR FaceName[LF_FACESIZE];
} CONSOLE_FONT_INFOEX, *PCONSOLE_FONT_INFOEX;

WINBASEAPI WINBOOL WINAPI GetCurrentConsoleFontEx(
  HANDLE hConsoleOutput,
  WINBOOL bMaximumWindow,
  PCONSOLE_FONT_INFOEX lpConsoleCurrentFontEx
);

WINBASEAPI WINBOOL WINAPI SetCurrentConsoleFontEx(
  HANDLE hConsoleOutput,
  WINBOOL bMaximumWindow,
  PCONSOLE_FONT_INFOEX lpConsoleCurrentFontEx
);
