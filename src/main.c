#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
    #define msleep(milliseconds) Sleep(milliseconds)

#else
    #include <unistd.h>
    #define msleep(milliseconds) usleep(milliseconds * 1000)
#endif

#include "args_parser.h"
#include "file.h"

#ifdef _WIN32
// Fonction pour obtenir le répertoire de l'exécutable sur Windows
static void get_exe_directory(char *buffer, size_t size) {
    GetModuleFileNameA(NULL, buffer, (DWORD)size);
    // Trouver le dernier backslash et le remplacer par \0
    char *last_slash = strrchr(buffer, '\\');
    if (last_slash != NULL) {
        *(last_slash + 1) = '\0';
    }
}
#endif

int main(int argc, char *argv[]) {
#ifdef _WIN32
    // Configurer la console Windows pour supporter UTF-8
    SetConsoleOutputCP(65001);  // UTF-8
#endif
    
    GameParams params;
    parse_args(argc, argv, &params);
    print_params(&params);

    uint64_t test = 10;
    printf("%llu\n", test);

    // lecture fichier glider.txt
    printf("lecture fichier glider.txt\n");
    
    // Construire la liste des chemins possibles
    char exe_path[512] = {0};
    char exe_config_path[512] = {0};
    
#ifdef _WIN32
    // Obtenir le répertoire de l'exécutable
    get_exe_directory(exe_path, sizeof(exe_path));
    snprintf(exe_config_path, sizeof(exe_config_path), "%sconfig\\glider.txt", exe_path);
#endif
    
    // Essayer plusieurs chemins possibles pour trouver le fichier
    const char *possible_paths[] = {
#ifdef _WIN32
        exe_config_path,            // Depuis le répertoire de l'exe
#endif
        "config/glider.txt",        // Depuis le répertoire de travail actuel
        "build/config/glider.txt",  // Depuis la racine du projet
        "src/config/glider.txt",    // Depuis la racine du projet (autre chemin)
        NULL
    };
    
    BitGrid *grid = NULL;
    const char *filename = NULL;
    
    for (int i = 0; possible_paths[i] != NULL; i++) {
        filename = possible_paths[i];
        printf("Tentative de lecture: %s\n", filename);
        grid = read_pattern_file(filename);
        if (grid != NULL) {
            printf("Fichier trouvé: %s\n", filename);
            break;
        }
    }
    
    if(grid == NULL) {
        fprintf(stderr, "\nErreur: Impossible de lire le fichier glider.txt\n");
        fprintf(stderr, "Tous les chemins possibles ont été essayés.\n");
#ifdef _WIN32
        fprintf(stderr, "\nAssurez-vous que le dossier 'config' contenant 'glider.txt'\n");
        fprintf(stderr, "est présent dans le même répertoire que l'exécutable.\n");
        fprintf(stderr, "Chemin de l'exe: %s\n", exe_path);
#endif
        printf("\nAppuyez sur Entrée pour fermer...\n");
        getchar();
        return 1;
    }
    printf("fichier glider.txt lu avec succès\n");
    printf("Dimensions: %d x %d\n", grid->width, grid->height);
    
    // affichage de la grille avec caractères Unicode
    printf("Affichage de la grille:\n");
    for(int i = 0; i < grid->height; i++) {
        for(int j = 0; j < grid->width; j++) {
            // Utiliser des caractères Unicode : █ (bloc plein) pour vivant, □ (carré vide) pour mort
            if (grid->grid[i * grid->width + j] == 1) {
                printf("█");  // Caractère bloc plein (U+2588)
            } else {
                printf("□");  // Caractère carré vide (U+25A1)
            }
        }
        printf("\n");
    }
    // fin affichage de la grille
    printf("Fin affichage de la grille\n");
    
    free_bitgrid(grid);
    // fin lecture fichier glider.txt

    // fermeture du programme
    printf("\nAppuyez sur Entrée pour fermer...\n");
    // Attendre une entrée utilisateur pour que la console reste ouverte
    getchar();
    return 0;
}