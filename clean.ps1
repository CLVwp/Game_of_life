# Script de nettoyage
if (Test-Path "build") {
    Remove-Item -Path "build" -Recurse -Force
    Write-Host "Dossier build supprimé" -ForegroundColor Green
} else {
    Write-Host "Le dossier build n'existe pas" -ForegroundColor Yellow
}

