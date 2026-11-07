#include "../include/builtin.h"
#include "../include/utils.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int builtin_pwd(int argc, char **args) {
    int ret = 0;
    char *buffer = NULL;
    ssize_t bufsize = DEFAULT_BUFSIZE;

    if (argc > 1) {
        write_err_no_ln("pwd: ");
        write_err_no_ln(args[1]);
        write_err_no_ln(": invalid argument\n");
        return 1;
    }

    // Allocation initiale du buffer
    buffer = malloc(bufsize);
    if (buffer == NULL) {
        perror("Erreur lors l'allocation de mémoire");
        return 1;
    }

    // Obtenir le chemin courant
    while (getcwd(buffer, bufsize) == NULL) {
        // Taille du buffer trop petite
        if (errno == ERANGE) {
            bufsize *= 2;
            char *new_buffer = realloc(buffer, bufsize);
            if (new_buffer == NULL) {
                perror("realloc : Erreur lors de l'allocation de mémoire du nouveau buffer");
                ret = 1;
                goto cleanup_return;
            }
            buffer = new_buffer;
        }
        // Autre erreurs :
        else {
            perror("getcwd");
            ret = 1;
            goto cleanup_return;
        }
    }

    // Ajout d'un retour à la ligne
    ssize_t len_path = strlen(buffer);
    // Vérifier que on a assez d'espace pour le retour à la ligne
    if (len_path + 2 > bufsize) {
        char *new_buffer = realloc(buffer, len_path + 1);
        if (new_buffer == NULL) {
            perror("realloc : Erreur lors de l'expansion du buffer.");
            ret = 1;
            goto cleanup_return;
        }
        buffer = new_buffer;
    }
    buffer[len_path] = '\0';

    // Afficher le chemin
    if (write_output(buffer) == -1) {
        ret = 1;
    }

cleanup_return:
    free(buffer);
    return ret;
}

int builtin_cd(int argc, char **args) {
    char repertoire_courant[512];
    static char repertoire_precedent[512] = "";

    if (argc > 2) {
        write_err_no_ln("cd: too many arguments\n");
        return 1;
    }

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
    else if (strcmp(args[1], "-") == 0) {
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
        if (chdir(args[1]) != 0) {
            perror("cd: Impossible d'aller dans le répertoire choisi");
            return 1;
        }
    }

    strncpy(repertoire_precedent, repertoire_courant, sizeof(repertoire_precedent));
    return 0;
}

int builtin_exit(int argc, char **args) {
    extern int last_status; // Accès à la variable globale last_status

    // Initialisation du code de retour par défaut à 0
    int exit_code = last_status;
    // Vérification si plus de 2 arguments sont passés
    if (argc > 2) {
        write_err_no_ln("exit: too many arguments\n");
        return 1;
    }
    // Vérification si un argument est passé
    if (argc == 2) {
        // Pointer utilisé pour la détéction des caractères non valides après conversion
        // Pointe sur premier caractère non convertible si il y en a un
        char *end_pointer;
        errno = 0;
        // Conversion de la chaîne args[1] en entier
        exit_code = strtol(args[1], &end_pointer, 10);
        // Si args[1] contient des caractères non convertibles
        if (*end_pointer != '\0' || errno == ERANGE) {
            if (write_err("strtol : Argument passé doit être un entier valide.\n") == -1) {
                return 1;
            }
            return 1;
        }
    }
    // Quitte fsh avec error_code
    exit(exit_code);
}

int builtin_ftype(int argc, char **args) {
    struct stat st;
    int ret = 0;

    if (argc < 2) {
        perror("Usage: ftype <file_path>\n");
        ret = 1;
        goto cleanup_return;
    }

    if (lstat(args[1], &st) < 0) {
        ret = 1;
        goto cleanup_return;
    }
    if (S_ISDIR(st.st_mode)) {
        write_output("directory");
    } else if (S_ISREG(st.st_mode)) {
        write_output("regular file");
    } else if (S_ISLNK(st.st_mode)) {
        write_output("symbolic link");
    } else if (S_ISFIFO(st.st_mode)) {
        write_output("named pipe");
    } else {
        write_output("other");
    }

cleanup_return:
    if (ret == 1) {
        perror("ftype:");
    }
    return ret;
}

// Liste des commandes internes sous forme d'un tableau à parcourir
static builtin_func builtins[] = {

    {"pwd", builtin_pwd},
    {"cd", builtin_cd},
    {"exit", builtin_exit},
    {"ftype", builtin_ftype},
};
static int nb_builtins = sizeof(builtins) / sizeof(builtin_func);

// Fonction pour exécuter une commande interne
int execute_builtin(int argc, char **args) {
    for (int i = 0; i < nb_builtins; i++) {
        if (strcmp(args[0], builtins[i].name) == 0) {
            return builtins[i].func(argc, args); // Exécute la commande interne
        }
    }
    return -1;
    // Retourne -1 si la commande n'est pas interne
}