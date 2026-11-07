#ifndef BUILTIN_H
#define BUILTIN_H

#define DEFAULT_BUFSIZE 512

// Structure pour les commandes internes
typedef struct builtin_func builtin_func;
struct builtin_func {
    char *name;                         // Nom de la commande
    int (*func)(int argc, char **args); // Pointeur vers la fonction qui exécute la commande
};

int builtin_pwd(int argc, char **args);
int builtin_cd(int argc, char **args);
int builtin_exit(int argc, char **args);
int builtin_ftype(int argc, char **args);

int execute_builtin(int argc, char **args); // Fonction pour exécuter une commande interne

#endif