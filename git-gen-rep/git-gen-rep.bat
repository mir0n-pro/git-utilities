@echo off

rem # Find bash
set "bpath="
IF EXIST "%BASH_HOME%\bash.exe" (
set "bpath=%BASH_HOME%"
goto Found
)
IF EXIST "D:\Program Files (Arm)\Git\bin\bash.exe" (
set "bpath=D:\Program Files (Arm)\Git\bin"
goto Found
)
IF EXIST "D:\Program Files\Git\bin\bash.exe" (
set "bpath=D:\Program Files\Git\bin"
goto Found
)
IF EXIST "C:\Program Files (x86)\Git\bin\bash.exe" (
set "bpath=C:\Program Files (x86)\Git\bin"
goto Found
)
IF EXIST "C:\Program Files (Arm)\Git\bin\bash.exe" (
set "bpath=C:\Program Files (Arm)\Git\bin"
goto Found
)
IF EXIST "C:\Program Files\Git\bin\bash.exe" (
set "bpath=C:\Program Files\Git\bin"
goto Found
)
echo "Bash.exe not found: define BASH_HOME."
pause 
goto End

rem # Set the number of commits as needed
:Found
set tag=HEAD~1
set till=HEAD
if not "%1"=="" set tag=%1
if not "%2"=="" set till=%2
IF "%tag%" == "NONE" goto End
set response=
set /p response= %tag% till %till%. Right (Y/N)?
if "%response%" == "" goto Start
if "%response%" == "y" goto Start
if "%response%" == "Y" goto Start
goto End

:Start
rem need to path to Unix stile and on 8.3 format
rem set dd=%~dp0
set dd=%~dps0
set "cc=%dd:\=/%"
rem @echo "%dd%" to "%cc%"

rem "%bpath%\bash.exe" -c "git_gen_rep.sh %tag% %till%"
"%bpath%\bash.exe" -c "%cc%git_gen_rep.sh %tag% %till%"

:End
pause
