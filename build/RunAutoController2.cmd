cd %~dp0

if not exist Binaries\ (
    echo.
    echo Please unzip the folder before you can use it.
    echo.
    pause
    exit
)

if not exist Binaries\AutoController2.exe (
    echo.
    echo Binaries\AutoController2.exe not found. Did your anti-virus delete it?
    echo.
    pause
    exit
)

cd Binaries
start "" AutoController2.exe
