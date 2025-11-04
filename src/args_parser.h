#ifndef ARGS_PARSER_H
#define ARGS_PARSER_H

#include <stdbool.h>

// Structure pour stocker les paramètres du jeu
typedef struct {
    int width;
    int height;
    int gens;
    char boundary[20];
    char in_file[256];
    char out_file[256];
    int target_hz;
} GameParams;

// Fonction pour parser les arguments de ligne de commande
void parse_args(int argc, char *argv[], GameParams *params);

// Fonction pour afficher les paramètres
void print_params(const GameParams *params);

#endif // ARGS_PARSER_H

