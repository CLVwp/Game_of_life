#ifndef FILE_H
#define FILE_H

#include <stdint.h>

// Structure pour stocker le tableau de bits et ses dimensions
typedef struct {
    uint8_t *grid;      // Tableau de bits (0 ou 1)
    int width;          // Largeur du tableau
    int height;         // Hauteur du tableau
} BitGrid;

// Fonction pour lire un fichier et créer un tableau de bits
// Retourne NULL en cas d'erreur
BitGrid* read_pattern_file(const char *filename);

// Fonction pour libérer la mémoire du BitGrid
void free_bitgrid(BitGrid *grid);

#endif // FILE_H

