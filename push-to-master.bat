@echo off

cd "%~dp0"

:: Call the script in all submodules recursively
for /D %%i in (modules/*) do (
    if exist "%~dp0modules/%%i/push-to-master.bat" (
        call "%~dp0modules/%%i/push-to-master.bat" nopause
    )
)

cd "%~dp0"
for %%a in ("%~dp0\.") do set "parentfolder=%%~nxa"

echo.
echo Pushing all changes of %parentfolder% to master...
git status
git add .
git commit -m "Automated commit"
git push origin HEAD:master
echo Done
echo.

if [%~1] == [] (
    Pause
)