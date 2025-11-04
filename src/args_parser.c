#include "args_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void parse_args(int argc, char *argv[], GameParams *params) {
    // Initialisation des valeurs par défaut
    params->width = 0;
    params->height = 0;
    params->gens = 0;
    params->boundary[0] = '\0';
    params->in_file[0] = '\0';
    params->out_file[0] = '\0';
    params->target_hz = 0;

    // Parse les arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--width") == 0 && i + 1 < argc) {
            params->width = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--height") == 0 && i + 1 < argc) {
            params->height = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--gens") == 0 && i + 1 < argc) {
            params->gens = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--boundary") == 0 && i + 1 < argc) {
            strncpy(params->boundary, argv[++i], sizeof(params->boundary) - 1);
            params->boundary[sizeof(params->boundary) - 1] = '\0';
        } else if (strcmp(argv[i], "--in") == 0 && i + 1 < argc) {
            strncpy(params->in_file, argv[++i], sizeof(params->in_file) - 1);
            params->in_file[sizeof(params->in_file) - 1] = '\0';
        } else if (strcmp(argv[i], "--out") == 0 && i + 1 < argc) {
            strncpy(params->out_file, argv[++i], sizeof(params->out_file) - 1);
            params->out_file[sizeof(params->out_file) - 1] = '\0';
        } else if (strcmp(argv[i], "--target-hz") == 0 && i + 1 < argc) {
            params->target_hz = atoi(argv[++i]);
        }
    }
}

void print_params(const GameParams *params) {
    printf("=== Paramètres du jeu de la vie ===\n");
    printf("Width: %d\n", params->width);
    printf("Height: %d\n", params->height);
    printf("Générations: %d\n", params->gens);
    printf("Boundary: %s\n", params->boundary[0] != '\0' ? params->boundary : "(non défini)");
    printf("Fichier d'entrée: %s\n", params->in_file[0] != '\0' ? params->in_file : "(non défini)");
    printf("Fichier de sortie: %s\n", params->out_file[0] != '\0' ? params->out_file : "(non défini)");
    printf("Target Hz: %d\n", params->target_hz);
    printf("===================================\n");
}

