@echo off
g++ main.cpp -o game -I"C:\SFML\SFML-2.5.1\include" -L"C:\SFML\SFML-2.5.1\lib" -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
if %errorlevel%==0 (
    echo Compilation réussie !
    game.exe

) else (
    echo Erreur lors de la compilation.
)
pause
..