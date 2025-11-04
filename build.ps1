# Script de compilation pour Game of Life

# Créer le dossier build s'il n'existe pas
if (-not (Test-Path "build")) {
    New-Item -ItemType Directory -Path "build" | Out-Null
    Write-Host "Dossier build créé" -ForegroundColor Cyan
}

# Compiler l'exécutable dans le dossier build
Write-Host "Compilation en cours..." -ForegroundColor Cyan
gcc -Isrc .\src\main.c .\src\args_parser.c .\src\file.c -o .\build\game.exe

if ($LASTEXITCODE -eq 0) {
    Write-Host "Compilation réussie ! Exécutable créé: build\game.exe" -ForegroundColor Green
    
    # Copier le dossier config dans build s'il existe
    if (Test-Path "src\config") {
        if (-not (Test-Path "build\config")) {
            New-Item -ItemType Directory -Path "build\config" | Out-Null
        }
        Copy-Item -Path "src\config\*" -Destination "build\config\" -Force -Recurse
        Write-Host "Dossier config copié dans build\" -ForegroundColor Yellow
    }
    
    # Copier les fichiers d'entrée potentiels dans build s'ils existent
    $inputFiles = @("glider.txt", "*.txt")
    foreach ($pattern in $inputFiles) {
        $files = Get-ChildItem -Path . -Filter $pattern -ErrorAction SilentlyContinue
        foreach ($file in $files) {
            if ($file.DirectoryName -ne "build") {
                Copy-Item -Path $file.FullName -Destination "build\" -Force
                Write-Host "Fichier copié: $($file.Name) -> build\" -ForegroundColor Yellow
            }
        }
    }
} else {
    Write-Host "Erreur lors de la compilation" -ForegroundColor Red
    exit 1
}

