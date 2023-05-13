#include <osalInput.h>
#include <osalTHread.h>
#include <assert.h>

inputEvtCallback _keyDownCallback;

DWORD WINAPI _receiveLoop(LPVOID thrdArg);
LRESULT CALLBACK _hookCallback(int nCode, WPARAM wParam, LPARAM lParam);

HANDLE _thrdHdl;
int _latestInput;
HHOOK _keyPrsHook;

void OsalInputKeyDownConnect(inputEvtCallback callback)
{
    _keyDownCallback = callback;
}

void OsalInputKeyDownDisconnect()
{
    assert(_keyDownCallback);
    _keyDownCallback = NULL;
}

void OsalInputStartReceiving()
{
    _thrdHdl = OsalDefaultThreadCreate(_receiveLoop, NULL);
}

void OsalInputStopReceiving()
{
    assert(_keyPrsHook);
    PostQuitMessage(0);     //break the msg loop
    UnhookWindowsHookEx(_keyPrsHook);
}

/*
 *  Internal functions
 */

DWORD WINAPI _receiveLoop(LPVOID thrdArg)
{
    //when msg comes, _hookCallback will be called
    _keyPrsHook = SetWindowsHookEx(WH_KEYBOARD_LL, _hookCallback, NULL, 0);       //registering the hook
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) != 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    assert(_thrdHdl);
    CloseHandle(_thrdHdl);

    return 0;       //thread exit
}

LRESULT CALLBACK _hookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
    PKBDLLHOOKSTRUCT key = (PKBDLLHOOKSTRUCT)lParam;

    if (nCode == HC_ACTION)
    {
        _latestInput = key->vkCode;
        if (wParam == WM_KEYDOWN)
            _keyDownCallback(&_latestInput);
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}