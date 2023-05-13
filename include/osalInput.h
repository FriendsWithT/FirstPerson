#include <windows.h>

typedef void (*inputEvtCallback)(void*);

void OsalInputStartReceiving();
void OsalInputStopReceiving();

void OsalInputKeyDownConnect(inputEvtCallback callback);
void OsalInputKeyDownDisconnect();