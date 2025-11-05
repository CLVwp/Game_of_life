#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
    #define msleep(milliseconds) Sleep(milliseconds)
    #define CLEAR_SCREEN() system("cls")
    #define KBHIT() _kbhit()
    #define GETCH() _getch()

#else
    #include <unistd.h>
    #include <termios.h>
    #include <fcntl.h>
    #include <sys/select.h>
    #define msleep(milliseconds) usleep(milliseconds * 1000)
    #define CLEAR_SCREEN() system("clear")
    
    // Fonction pour vérifier si une touche est pressée (non-bloquante)
    static int kbhit_linux(void) {
        struct timeval tv;
        fd_set rdfs;
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        FD_ZERO(&rdfs);
        FD_SET(STDIN_FILENO, &rdfs);
        select(STDIN_FILENO + 1, &rdfs, NULL, NULL, &tv);
        return FD_ISSET(STDIN_FILENO, &rdfs);
    }
    #define KBHIT() kbhit_linux()
    #define GETCH() getchar()
#endif

#include "args_parser.h"
#include "file.h"
#include "game.h"

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
    
    // fin lecture fichier glider.txt

    // Créer le jeu avec les dimensions des paramètres
    GameOfLife *game = create_game(params.width, params.height);
    if (game == NULL) {
        fprintf(stderr, "Erreur: Impossible de créer le jeu\n");
        free_bitgrid(grid);
        return 1;
    }
    
    // Charger le pattern au centre de la grille
    int offset_x = (params.width - grid->width) / 2;
    int offset_y = (params.height - grid->height) / 2;
    load_pattern(game, grid, offset_x, offset_y);
    
    free_bitgrid(grid);
    
    // Boucle interactive
    printf("\nAppuyez sur Entrée pour commencer la simulation...\n");
    getchar();
    
    // Choisir le mode de bordure
    CLEAR_SCREEN();
    printf("=== Choix du mode de bordure ===\n");
    printf("1. Edge (mort en dehors)\n");
    printf("2. Toroidal (périodique)\n");
    printf("3. Mirror (miroir)\n");
    printf("4. Alive rim (vivant en dehors)\n");
    printf("\nChoisissez un mode (1-4): ");
    
    int choice = 0;
    if (scanf("%d", &choice) != 1 || choice < 1 || choice > 4) {
        choice = 1; // Par défaut
    }
    getchar(); // Consommer le \n
    
    BoundaryMode boundary_mode = (BoundaryMode)(choice - 1);
    set_boundary_mode(game, boundary_mode);
    
    const char *mode_names[] = {"Edge", "Toroidal", "Mirror", "Alive rim"};
    printf("Mode sélectionné: %s\n", mode_names[boundary_mode]);
    printf("\nAppuyez sur Entrée pour continuer...\n");
    getchar();
    
    // Boucle principale de simulation
    int running = 0; // 0 = arrêté, 1 = en cours
    int generation = 0;
    
#ifdef _WIN32
    // Configurer la console pour la lecture non-bloquante
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD console_mode;
    GetConsoleMode(hStdin, &console_mode);
    SetConsoleMode(hStdin, console_mode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT));
#endif
    
    while (1) {
        CLEAR_SCREEN();
        
        printf("=== Game of Life ===\n");
        printf("Génération: %d\n", generation);
        printf("Mode bordure: %s\n", mode_names[boundary_mode]);
        printf("État: %s\n", running ? "EN COURS" : "ARRÊTÉ");
        printf("\n");
        
        display_grid(game);
        
        printf("\n");
        printf("=== Contrôles ===\n");
        printf("[L] Lancer la simulation\n");
        printf("[A] Arrêter la simulation\n");
        printf("[M] Mesurer les performances (1000 générations)\n");
        printf("[Q] Quitter\n");
        
        if (running) {
            next_generation(game);
            generation++;
            msleep(100); // Délai pour visualisation (10 FPS)
        }
        
        // Vérifier l'entrée clavier
        if (KBHIT()) {
            char key = GETCH();
            key = (key >= 'a' && key <= 'z') ? key - 32 : key; // Convertir en majuscule
            
            if (key == 'L') {
                running = 1;
            } else if (key == 'A') {
                running = 0;
            } else if (key == 'M') {
                running = 0;
                CLEAR_SCREEN();
                measure_performance(game, 1000);
                printf("\nAppuyez sur Entrée pour continuer...\n");
                getchar();
            } else if (key == 'Q') {
                break;
            }
        }
        
        // Si arrêté, attendre un peu pour ne pas surcharger le CPU
        if (!running) {
            msleep(50);
        }
    }
    
#ifdef _WIN32
    // Restaurer le mode de la console
    SetConsoleMode(hStdin, console_mode);
#endif
    
    free_game(game);
    return 0;
}