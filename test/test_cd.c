#include "../include/builtin.h"
#include "../include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
    char repertoire_courant[512];
    char repertoire_precedent[512];

    if (getcwd(repertoire_courant, sizeof(repertoire_courant)) == NULL) {
        perror("getcwd: Répertoire courant introuvable");
        return 1;
    }

    // Abscence de paramètres, on va dans $HOME
    if (argc == 1) {
        char *home = getenv("HOME");
        if (home == NULL) {
            write_err("cd: HOME non défini\n");
            return 1;
        }
        if (chdir(home) != 0) {
            perror("cd: Impossible d'aller dans HOME");
            return 1;
        }
    }
    // Paramètre "-", on va dans le répertoire précédent
    else if (strcmp(argv[1], "-") == 0) {
        if (strlen(repertoire_precedent) == 0) {
            write_err("cd: Pas de répertoire précédent\n");
            return 1;
        }
        if (chdir(repertoire_precedent) != 0) {
            perror("cd: Impossible d'aller dans le répertoire précédent");
            return 1;
        }
    }
    // On va dans le répertoire donné en argument
    else {
        if (chdir(argv[1]) != 0) {
            perror("cd: Impossible d'aller dans le répertoire choisi");
            return 1;
        }
    }

    strncpy(repertoire_precedent, repertoire_courant, sizeof(repertoire_precedent));
    return 0;
}