#define _POSIX_C_SOURCE 200809L

#include "../include/signal.h"
#include "../include/debug.h"
#include "../include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

volatile int last_status = 0;
volatile sig_atomic_t interrupted = 0;

// Gestionnaire pour SIGINT
void handle_sigint(int signo) {
    interrupted = 1;
    LOG_INFO("\n[Signal reçu: SIGINT]\n");
}

// Gestionnaire pour SIGTERM (ignoré)
void handle_sigterm(int signo) { LOG_INFO("[SIGTERM ignoré]\n"); }

// Initialisation des gestionnaires de signaux
void setup_signals(void) {
    struct sigaction sa_int, sa_term;

    // Gestion de SIGINT
    sa_int.sa_handler = handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    if (sigaction(SIGINT, &sa_int, NULL) < 0) {
        perror("Erreur sigaction(SIGINT)");
        exit(EXIT_FAILURE);
    }

    // Gestion de SIGTERM
    sa_term.sa_handler = handle_sigterm;
    sigemptyset(&sa_term.sa_mask);
    sa_term.sa_flags = 0; // Pas de comportement particulier
    if (sigaction(SIGTERM, &sa_term, NULL) < 0) {
        perror("Erreur sigaction(SIGTERM)");
        exit(EXIT_FAILURE);
    }
}

void reset_signals_to_default() {
    struct sigaction sa_default;

    // Rétablir le comportement par défaut pour les signaux
    sa_default.sa_handler = SIG_DFL;
    sigemptyset(&sa_default.sa_mask);
    sa_default.sa_flags = 0;

    // Liste des signaux à réinitialiser
    int signals[] = {SIGTERM}; // Ajouter d'autres si nécessaire
    size_t num_signals = sizeof(signals) / sizeof(signals[0]);

    for (size_t i = 0; i < num_signals; i++) {
        if (sigaction(signals[i], &sa_default, NULL) == -1) {
            perror("sigaction");
            exit(EXIT_FAILURE);
        }
    }
}