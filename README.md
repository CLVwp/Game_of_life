# gcc => cygwin

# cygwin terminal 
cd "/cygdrive/c/Users/viell/Desktop/Game_of_Life/Game_of_life"

./game.exe --width 320 --height 240 --gens 500 --boundary torus --in glider.txt --out result.txt --target-hz 60

# compilation : 
gcc .\src\main.c -o .\game.exe
ou build.ps1


# run :

.\game.exe --width 320 --height 240 --gens 500 --boundary torus --in glider.txt --out result.txt --target-hz 60