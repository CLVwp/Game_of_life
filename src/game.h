#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include "file.h"

// Modes de bordure
typedef enum {
    BOUNDARY_EDGE = 0,      // Dead outside
    BOUNDARY_TOROIDAL = 1,  // Wrap around
    BOUNDARY_MIRROR = 2,    // Reflect
    BOUNDARY_ALIVE_RIM = 3  // Alive outside
} BoundaryMode;

// Structure pour gérer le Game of Life
typedef struct {
    BitGrid *current_grid;  // Grille actuelle
    BitGrid *shadow_grid;   // Grille temporaire pour calcul
    BoundaryMode boundary;  // Mode de bordure
    int width;              // Largeur de la grille
    int height;             // Hauteur de la grille
} GameOfLife;

// Fonctions de création et destruction
GameOfLife* create_game(int width, int height);
void free_game(GameOfLife *game);
void set_boundary_mode(GameOfLife *game, BoundaryMode mode);
void load_pattern(GameOfLife *game, BitGrid *pattern, int offset_x, int offset_y);

// Fonctions de simulation
uint8_t get_cell_state(GameOfLife *game, int x, int y);
int num_neighbors(GameOfLife *game, int x, int y);
uint8_t get_next_state(GameOfLife *game, int x, int y);
void next_generation(GameOfLife *game);

// Fonction d'affichage
void display_grid(GameOfLife *game);

// Fonction pour mesurer les performances
void measure_performance(GameOfLife *game, int num_generations);

#endif // GAME_H

