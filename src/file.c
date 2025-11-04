#include "file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BitGrid* read_pattern_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Erreur: Impossible d'ouvrir le fichier '%s'\n", filename);
        return NULL;
    }

    // Allouer la structure
    BitGrid *grid = (BitGrid*)malloc(sizeof(BitGrid));
    if (grid == NULL) {
        fclose(file);
        fprintf(stderr, "Erreur: Échec d'allocation mémoire pour BitGrid\n");
        return NULL;
    }

    char line[1024];
    int height = 0;
    int width = 0;
    int first_line = 1;
    int *line_lengths = NULL;
    int capacity = 1000; // Capacité initiale
    int max_lines = 100000; // Limite maximale de lignes

    // Allouer un tableau pour stocker les longueurs de lignes
    line_lengths = (int*)malloc(capacity * sizeof(int));
    if (line_lengths == NULL) {
        fprintf(stderr, "Erreur: Échec d'allocation mémoire pour line_lengths\n");
        free(grid);
        fclose(file);
        return NULL;
    }

    // Première passe : déterminer les dimensions
    // La largeur est déterminée par la première ligne
    // La hauteur est déterminée par le nombre de lignes (première colonne)
    while (fgets(line, sizeof(line), file) && height < max_lines) {
        // Supprimer le retour à la ligne
        size_t len = strcspn(line, "\r\n");
        line[len] = '\0';
        
        if (first_line) {
            // La largeur est fixée par la première ligne
            width = len;
            first_line = 0;
            if (width == 0) {
                fprintf(stderr, "Erreur: La première ligne est vide\n");
                free(line_lengths);
                free(grid);
                fclose(file);
                return NULL;
            }
        }
        
        // Agrandir le tableau si nécessaire
        if (height >= capacity) {
            capacity *= 2;
            int *new_line_lengths = (int*)realloc(line_lengths, capacity * sizeof(int));
            if (new_line_lengths == NULL) {
                fprintf(stderr, "Erreur: Échec de réallocation mémoire\n");
                free(line_lengths);
                free(grid);
                fclose(file);
                return NULL;
            }
            line_lengths = new_line_lengths;
        }
        
        // Stocker la longueur de cette ligne
        line_lengths[height] = len;
        height++;
    }

    if (width == 0 || height == 0) {
        fprintf(stderr, "Erreur: Fichier vide ou aucune donnée valide\n");
        free(line_lengths);
        free(grid);
        fclose(file);
        return NULL;
    }

    // Vérifier les incohérences de longueur et afficher des warnings
    int has_warnings = 0;
    for (int i = 0; i < height; i++) {
        if (line_lengths[i] != width) {
            if (!has_warnings) {
                fprintf(stderr, "Warning: Incohérences de largeur détectées:\n");
                has_warnings = 1;
            }
            fprintf(stderr, "  Ligne %d: attendu %d caractères, trouvé %d. ", 
                    i + 1, width, line_lengths[i]);
            if (line_lengths[i] < width) {
                fprintf(stderr, "Cases manquantes remplies avec 0.\n");
            } else {
                fprintf(stderr, "Caractères supplémentaires ignorés.\n");
            }
        }
    }
    
    // Libérer le tableau des longueurs maintenant qu'on n'en a plus besoin
    free(line_lengths);

    // Allouer le tableau de bits (initialisé à 0 par calloc)
    grid->grid = (uint8_t*)calloc(width * height, sizeof(uint8_t));
    if (grid->grid == NULL) {
        fprintf(stderr, "Erreur: Échec d'allocation mémoire pour le tableau de bits\n");
        free(grid);
        fclose(file);
        return NULL;
    }

    grid->width = width;
    grid->height = height;

    // Réinitialiser le fichier pour la lecture
    rewind(file);

    // Deuxième passe : remplir le tableau
    int row = 0;
    while (fgets(line, sizeof(line), file) && row < height) {
        size_t len = strcspn(line, "\r\n");
        
        // Lire les caractères de la ligne (jusqu'à la largeur attendue)
        for (int col = 0; col < width; col++) {
            if (col < len) {
                // Caractère présent dans le fichier
                if (line[col] == '*') {
                    grid->grid[row * width + col] = 1;
                } else if (line[col] == '.') {
                    grid->grid[row * width + col] = 0;
                } else {
                    // Caractère invalide, reste à 0 (déjà initialisé par calloc)
                    fprintf(stderr, "Warning: Ligne %d, colonne %d: caractère invalide '%c' (attendu '.' ou '*')\n", 
                            row + 1, col + 1, line[col]);
                }
            }
            // Si col >= len, la case reste à 0 (déjà initialisé par calloc)
            // C'est le cas où la ligne est plus courte que la largeur attendue
        }
        
        row++;
    }

    fclose(file);
    return grid;
}

void free_bitgrid(BitGrid *grid) {
    if (grid != NULL) {
        if (grid->grid != NULL) {
            free(grid->grid);
        }
        free(grid);
    }
}

