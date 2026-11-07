#ifndef REDIRECTION_H
#define REDIRECTION_H

#include <stdlib.h>

typedef enum redirection_value_t redirection_value_t;

typedef struct redirection_t redirection_t;

enum redirection_value_t {
    NO_REDIRECT,            // 0 // Pas de redirection, par defaut
    REDIRECT_IN,            // 1 // "<"
    REDIRECT_OUT,           // 2 // ">"
    REDIRECT_ERR,           // 3 // "2>"
    REDIRECT_OUT_APPEND,    // 4 // ">>"
    REDIRECT_ERR_APPEND,    // 5 // "2>>"
    REDIRECT_OUT_OVERWRITE, // 6 // ">|"
    REDIRECT_ERR_OVERWRITE, // 7 // "2>|"
};

struct redirection_t {
    int type;   // Type de redirection (entrée, sortie, erreur)
    char *file; // Fichier de redirection
};

// ------------------------------ Fonctions utils ------------------------------ //
int is_redirection(char *string);
int get_redirection_value(char *redir_string);

int alloc_redirection(redirection_t **cmd);
int free_redirection(redirection_t *cmd);
int dup_redirection(redirection_t *cmd, redirection_t **out);
int print_redirection(redirection_t *cmd);

#endif