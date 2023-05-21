@echo off
set CC_DIR=.\3rdParty\tcc
set CC=%CC_DIR%\tcc
set INCLUDE_DIR=./include
set BIN_DIR=./bin
set TARGET_DIR=./target
set OSAL_DIR=./osal

set CC_FLAGS=-c -I %INCLUDE_DIR% -I %CC_DIR%/include
set LINKER_FLAGS=-lkernel32 -luser32 -lmsvcrt

set SUCCESS=1

color 0f

echo Building firstPerson.exe
echo.

call :EXEC_WITH_INFO "%CC% -impdef kernel32.dll -o %CC_DIR%/lib/kernel32.def"
call :EXEC_WITH_INFO "%CC% -impdef user32.dll -o %CC_DIR%/lib/user32.def"

call :EXEC_WITH_INFO "%CC% %CC_FLAGS% %OSAL_DIR%/osalOutput.c -o %BIN_DIR%/osalOutput.o"
call :EXEC_WITH_INFO "%CC% %CC_FLAGS% %OSAL_DIR%/osalInput.c -o %BIN_DIR%/osalInput.o"
call :EXEC_WITH_INFO "%CC% %CC_FLAGS% %OSAL_DIR%/osalThread.c -o %BIN_DIR%/osalThread.o"
call :EXEC_WITH_INFO "%CC% %CC_FLAGS% %OSAL_DIR%/osalProcess.c -o %BIN_DIR%/osalProcess.o"
call :EXEC_WITH_INFO "%CC% %CC_FLAGS% main.c -o %BIN_DIR%/main.o"
call :EXEC_WITH_INFO "%CC% %CC_FLAGS% minimap.c -o %BIN_DIR%/minimap.o"

call :EXEC_WITH_INFO "%CC% %BIN_DIR%/main.o %BIN_DIR%/osalInput.o %BIN_DIR%/osalOutput.o %BIN_DIR%/osalThread.o %BIN_DIR%/osalProcess.o -o %TARGET_DIR%/firstPerson.exe %LINKER_FLAGS%"
call :EXEC_WITH_INFO "%CC% %BIN_DIR%/minimap.o %BIN_DIR%/osalProcess.o -o %TARGET_DIR%/minimap.exe"

echo.
echo Buiding finished.
if %SUCCESS% neq 1 (
	call :ECHO_WITH_COLOR "There were errors while executing this scipt. Building failed!" "Red"
) else (
	call :ECHO_WITH_COLOR "You can run firstPerson.exe inside target folder." "Green"
)
echo.
pause
goto :eof

:EXEC_WITH_INFO
@echo Executing command: %1
%~1
if %errorlevel% neq 0 (
	set SUCCESS=0
)
goto :eof

:ECHO_WITH_COLOR
%Windir%\System32\WindowsPowerShell\v1.0\Powershell.exe write-host -foregroundcolor %~2 %1
goto:eof
