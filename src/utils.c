// Fichier pour les fonctions utilitaires communes

#include "../include/utils.h"
#include "../include/command.h"
#include "../include/parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Fonction pour écrire une chaîne dans la sortie standard,
// à changer peut-être quand y'aura les redirections etc...
int write_output(const char *message) {
    if (write(STDOUT_FILENO, message, strlen(message)) == -1) {
        perror("write: Erreur d'écriture.");
        return -1;
    }

    if (write(STDOUT_FILENO, "\n", 1) == -1) {
        perror("write: Erreur d'écriture.");
        return 1;
    }
    return 0;
}

int write_output_no_ln(const char *message) {
    if (write(STDOUT_FILENO, message, strlen(message)) == -1) {
        perror("write: Erreur d'écriture.");
        return -1;
    }
    return 0;
}

// Fonction pour écrire une chaîne dans la sortie erreur standard
int write_err(const char *message) {
    if (write(STDERR_FILENO, message, strlen(message)) == -1) {
        perror("write : Erreur d'écriture.");
        return -1;
    }

    if (write(STDOUT_FILENO, "\n", 1) == -1) {
        perror("write: Erreur d'écriture.");
        return 1;
    }
    return 0;
}

int write_err_no_ln(const char *message) {
    if (write(STDERR_FILENO, message, strlen(message)) == -1) {
        perror("write : Erreur d'écriture.");
        return -1;
    }
    return 0;
}

int test_temp() {
    // char *args[] = {"cd", "Hello", "World"};

    // simple_command_t simple_cmd = {
    //     .argc = 3,
    //     .args = args,
    //     .redir = {
    //         .file = "output.txt",
    //         .type = 2 // ">"
    //     }};

    // command_t cmd = {
    //     .type = SIMPLE_COMMAND,
    //     .simple = simple_cmd};

    // // Test de l'affichage de la commande
    // printf("Test de print_command :\n");
    // print_command(&cmd); // Affiche : echo Hello World > output.txt

    return 0;
}