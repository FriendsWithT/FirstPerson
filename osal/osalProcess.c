#include <osalProcess.h>
#include <fpsCommon.h>

#define PIPE_TIMEOUT 5000

PROCESS_INFORMATION OsalSpawnChildProcess(const char *cmd)
{
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};

    // Set up the STARTUPINFO structure
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOW;

    // Create the child process
    CreateProcess(
        cmd, // Path to the child process executable
        NULL, // Command line arguments
        NULL, // Process handle not inheritable
        NULL, // Thread handle not inheritable
        FALSE, // Set handle inheritance to FALSE
        CREATE_NEW_CONSOLE, // Create a new console window for the child process
        NULL, // Use the same environment as the calling process
        NULL, // Use the same working directory as the calling process
        &si, // Pointer to STARTUPINFO structure
        &pi // Pointer to PROCESS_INFORMATION structure
    );
}

/*
 *  Functions used for IPC
 */

HANDLE OsalPipeOpen(const char *name, UINT16 pipeExisted)
{
    HANDLE hPipe;
    const char *pipeName = name;

    if (pipeExisted)
    {
        hPipe = CreateFile(
            pipeName, // Pipe name
            GENERIC_READ, // Access mode
            0, // Share mode
            NULL, // Security attributes
            OPEN_EXISTING, // Creation disposition
            0, // Flags and attributes
            NULL // Template file handle
        );
    }
    else
    {
        // Create the named pipe
        hPipe = CreateNamedPipe(
            pipeName, // Pipe name
            PIPE_ACCESS_OUTBOUND, // Pipe open mode
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, // Pipe mode
            1, // Maximum number of instances
            0, // Output buffer size
            0, // Input buffer size
            0, // Default timeout
            NULL // Security attributes
        );

        // Connect to the named pipe
        ConnectNamedPipe(hPipe, NULL);
    }

    return hPipe;
}

UINT16 OsalPipeMsgSend(HANDLE hPipe, void *data, size_t size)
{
    DWORD bytesWritten = 0;
    OVERLAPPED overlapped;
    ZeroMemory(&overlapped, sizeof(overlapped));
    overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    // Send the message through the named pipe
    INT16 writeResult = WriteFile(hPipe, data, size, &bytesWritten, &overlapped);

    if (!writeResult && GetLastError() == ERROR_IO_PENDING)
    {
        // Wait for the overlapped operation to complete with a timeout
        DWORD waitResult = WaitForSingleObject(overlapped.hEvent, PIPE_TIMEOUT);
        if (waitResult == WAIT_OBJECT_0)
        {
            // The overlapped operation completed successfully
            writeResult = TRUE;
        }
        else if (waitResult == WAIT_TIMEOUT)
        {
            // The overlapped operation timed out
            CancelIo(hPipe);
            writeResult = FALSE;
        }
        else
        {
            // An error occurred while waiting for the overlapped operation to complete
            VERBOSE_ASSERT(0, "Pipe error");
        }
    }

    return writeResult;
}

void OsalPipeMsgReceive(HANDLE hPipe, void *data, size_t size)
{
    DWORD bytesRead = 0;
    // Read the message from the named pipe
    ReadFile(hPipe, data, size, &bytesRead, NULL);
}

void OsalPipeClose(HANDLE hOpenedPipe)
{
    CloseHandle(hOpenedPipe);
}
