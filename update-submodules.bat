:: update-submodules.bat v1.1
@echo off

cd "%~dp0"
for %%a in ("%~dp0\.") do set "parentfolder=%%~nxa"

echo.
echo Updating submodules of %parentfolder%...
echo -^> git submodule update --remote --merge --recursive

git submodule update --remote --merge --recursive

echo Done

if [%~1] == [] (
    Pause
)