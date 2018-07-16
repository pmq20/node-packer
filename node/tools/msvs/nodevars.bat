@echo off

rem Ensure this Node.js and npm are first in the PATH
set "PATH=%~dp0;%PATH%"

echo Your environment has been set up.

rem If we're in the Node.js directory, change to the user's home dir.
if "%CD%\"=="%~dp0" cd /d "%HOMEDRIVE%%HOMEPATH%"
