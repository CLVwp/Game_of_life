#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <time.h>
#endif

// Fonction pour obtenir le temps en millisecondes
static double get_time_ms() {
#ifdef _WIN32
    LARGE_INTEGER frequency, counter;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / frequency.QuadPart * 1000.0;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
#endif
}

// Créer une nouvelle instance du jeu
GameOfLife* create_game(int width, int height) {
    GameOfLife *game = (GameOfLife*)malloc(sizeof(GameOfLife));
    if (game == NULL) return NULL;
    
    game->width = width;
    game->height = height;
    game->boundary = BOUNDARY_EDGE;
    
    // Allouer la grille actuelle
    game->current_grid = (BitGrid*)malloc(sizeof(BitGrid));
    if (game->current_grid == NULL) {
        free(game);
        return NULL;
    }
    game->current_grid->width = width;
    game->current_grid->height = height;
    game->current_grid->grid = (uint8_t*)calloc(width * height, sizeof(uint8_t));
    if (game->current_grid->grid == NULL) {
        free(game->current_grid);
        free(game);
        return NULL;
    }
    
    // Allouer la grille shadow
    game->shadow_grid = (BitGrid*)malloc(sizeof(BitGrid));
    if (game->shadow_grid == NULL) {
        free(game->current_grid->grid);
        free(game->current_grid);
        free(game);
        return NULL;
    }
    game->shadow_grid->width = width;
    game->shadow_grid->height = height;
    game->shadow_grid->grid = (uint8_t*)calloc(width * height, sizeof(uint8_t));
    if (game->shadow_grid->grid == NULL) {
        free(game->current_grid->grid);
        free(game->current_grid);
        free(game->shadow_grid);
        free(game);
        return NULL;
    }
    
    return game;
}

// Libérer la mémoire du jeu
void free_game(GameOfLife *game) {
    if (game == NULL) return;
    
    if (game->current_grid != NULL) {
        free_bitgrid(game->current_grid);
    }
    if (game->shadow_grid != NULL) {
        free_bitgrid(game->shadow_grid);
    }
    free(game);
}

// Définir le mode de bordure
void set_boundary_mode(GameOfLife *game, BoundaryMode mode) {
    if (game != NULL) {
        game->boundary = mode;
    }
}

// Charger un pattern dans la grille
void load_pattern(GameOfLife *game, BitGrid *pattern, int offset_x, int offset_y) {
    if (game == NULL || pattern == NULL) return;
    
    // Copier le pattern dans la grille actuelle à la position offset
    for (int y = 0; y < pattern->height; y++) {
        for (int x = 0; x < pattern->width; x++) {
            int target_x = offset_x + x;
            int target_y = offset_y + y;
            
            if (target_x >= 0 && target_x < game->width && 
                target_y >= 0 && target_y < game->height) {
                game->current_grid->grid[target_y * game->width + target_x] = 
                    pattern->grid[y * pattern->width + x];
            }
        }
    }
}

// Normaliser les coordonnées selon le mode de bordure
static void normalize_coords(GameOfLife *game, int *x, int *y) {
    switch (game->boundary) {
        case BOUNDARY_EDGE:
            // Les coordonnées hors limites restent hors limites
            break;
            
        case BOUNDARY_TOROIDAL:
            // Wrap around avec modulo
            if (*x < 0) *x = ((*x % game->width) + game->width) % game->width;
            else if (*x >= game->width) *x = *x % game->width;
            if (*y < 0) *y = ((*y % game->height) + game->height) % game->height;
            else if (*y >= game->height) *y = *y % game->height;
            break;
            
        case BOUNDARY_MIRROR:
            // Reflect: miroir autour des bords
            // Pour x
            if (*x < 0) {
                *x = -(*x) - 1;
            } else if (*x >= game->width) {
                *x = 2 * game->width - *x - 1;
            }
            // Si toujours hors limites, réfléchir à nouveau
            if (*x < 0 || *x >= game->width) {
                // Utiliser modulo pour les grandes valeurs
                int period = 2 * game->width;
                int temp = *x;
                if (temp < 0) {
                    temp = ((-temp - 1) % period);
                    if (temp < 0) temp += period;
                } else {
                    temp = (temp % period);
                }
                if (temp >= game->width) temp = period - temp - 1;
                *x = temp;
            }
            // Pour y
            if (*y < 0) {
                *y = -(*y) - 1;
            } else if (*y >= game->height) {
                *y = 2 * game->height - *y - 1;
            }
            // Si toujours hors limites, réfléchir à nouveau
            if (*y < 0 || *y >= game->height) {
                // Utiliser modulo pour les grandes valeurs
                int period = 2 * game->height;
                int temp = *y;
                if (temp < 0) {
                    temp = ((-temp - 1) % period);
                    if (temp < 0) temp += period;
                } else {
                    temp = (temp % period);
                }
                if (temp >= game->height) temp = period - temp - 1;
                *y = temp;
            }
            break;
            
        case BOUNDARY_ALIVE_RIM:
            // Les coordonnées hors limites sont considérées comme vivantes
            break;
    }
}

// Obtenir l'état d'une cellule (avec gestion des bords)
uint8_t get_cell_state(GameOfLife *game, int x, int y) {
    if (game == NULL) return 0;
    
    int orig_x = x, orig_y = y;
    
    // Si hors limites avec mode EDGE, retourner 0 (mort)
    if (game->boundary == BOUNDARY_EDGE) {
        if (x < 0 || x >= game->width || 
            y < 0 || y >= game->height) {
            return 0;
        }
        // Coordonnées valides, lire directement
        return game->current_grid->grid[y * game->width + x];
    }
    
    // Si hors limites avec mode ALIVE_RIM, retourner 1 (vivant)
    if (game->boundary == BOUNDARY_ALIVE_RIM) {
        if (x < 0 || x >= game->width || 
            y < 0 || y >= game->height) {
            return 1;
        }
        // Coordonnées valides, lire directement
        return game->current_grid->grid[y * game->width + x];
    }
    
    // Pour TOROIDAL et MIRROR, normaliser les coordonnées
    normalize_coords(game, &x, &y);
    
    // Coordonnées normalisées (devraient toujours être valides maintenant)
    if (x >= 0 && x < game->width && y >= 0 && y < game->height) {
        return game->current_grid->grid[y * game->width + x];
    }
    
    return 0;
}

// Compter le nombre de voisins vivants
int num_neighbors(GameOfLife *game, int x, int y) {
    if (game == NULL) return 0;
    
    int count = 0;
    
    // Vérifier les 8 voisins
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue; // Ignorer la cellule elle-même
            
            if (get_cell_state(game, x + dx, y + dy) == 1) {
                count++;
            }
        }
    }
    
    return count;
}

// Obtenir l'état suivant d'une cellule selon les règles du Game of Life
uint8_t get_next_state(GameOfLife *game, int x, int y) {
    if (game == NULL) return 0;
    
    uint8_t current = get_cell_state(game, x, y);
    int neighbors = num_neighbors(game, x, y);
    
    // Règles du Game of Life:
    // 1. Une cellule vivante avec 2 ou 3 voisins survit
    // 2. Une cellule morte avec exactement 3 voisins devient vivante
    // 3. Sinon, la cellule meurt ou reste morte
    
    if (current == 1) {
        // Cellule vivante
        if (neighbors == 2 || neighbors == 3) {
            return 1; // Survit
        } else {
            return 0; // Meurt
        }
    } else {
        // Cellule morte
        if (neighbors == 3) {
            return 1; // Naît
        } else {
            return 0; // Reste morte
        }
    }
}

// Calculer la prochaine génération
void next_generation(GameOfLife *game) {
    if (game == NULL) return;
    
    // Calculer la prochaine génération dans la grille shadow
    for (int y = 0; y < game->height; y++) {
        for (int x = 0; x < game->width; x++) {
            game->shadow_grid->grid[y * game->width + x] = 
                get_next_state(game, x, y);
        }
    }
    
    // Échanger les pointeurs des grilles
    BitGrid *temp = game->current_grid;
    game->current_grid = game->shadow_grid;
    game->shadow_grid = temp;
}

// Afficher la grille
void display_grid(GameOfLife *game) {
    if (game == NULL) return;
    
    for (int y = 0; y < game->height; y++) {
        for (int x = 0; x < game->width; x++) {
            if (game->current_grid->grid[y * game->width + x] == 1) {
                printf("█");  // Caractère bloc plein (U+2588)
            } else {
                printf("□");  // Caractère carré vide (U+25A1)
            }
        }
        printf("\n");
    }
}

// Mesurer les performances
void measure_performance(GameOfLife *game, int num_generations) {
    if (game == NULL) return;
    
    double *times = (double*)malloc(num_generations * sizeof(double));
    if (times == NULL) {
        fprintf(stderr, "Erreur: Impossible d'allouer la mémoire pour les mesures\n");
        return;
    }
    
    double total_time = 0.0;
    double max_time = 0.0;
    double min_time = 1e9;
    
    printf("Mesure des performances sur %d générations...\n", num_generations);
    
    for (int i = 0; i < num_generations; i++) {
        double start = get_time_ms();
        next_generation(game);
        double end = get_time_ms();
        
        double elapsed = end - start;
        times[i] = elapsed;
        total_time += elapsed;
        
        if (elapsed > max_time) max_time = elapsed;
        if (elapsed < min_time) min_time = elapsed;
    }
    
    double avg_time = total_time / num_generations;
    double jitter = max_time - avg_time;
    
    printf("\n=== Résultats de performance ===\n");
    printf("Temps moyen par génération: %.3f ms\n", avg_time);
    printf("Temps le plus long: %.3f ms\n", max_time);
    printf("Temps le plus court: %.3f ms\n", min_time);
    printf("Jitter (différence): %.3f ms\n", jitter);
    printf("Fréquence moyenne: %.1f Hz\n", 1000.0 / avg_time);
    printf("===============================\n");
}

