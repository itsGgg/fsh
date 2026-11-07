#define _POSIX_C_SOURCE 200809L
#include "../include/external.h"
#include "../include/signal.h"
#include "../include/utils.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int execute_external(int argc, char **args) {
    // Vérifier si une commande a été fournie
    if (argc == 0 || args == NULL || args[0] == NULL) {
        write_err("Erreur : Aucune commande n'a été passée.");
        return 1;
    }
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }
    // Faire exécuter la commande par le processus fils
    if (pid == 0) {
        // Réinitialiser les signaux aux comportements par défaut
        reset_signals_to_default();

        // Exécuter la commande
        execvp(args[0], args);

        // Si execvp échoue
        if (errno == ENOENT) {
            perror("redirect_exec");
        } else if (errno == EACCES) {
            perror("redirect_exec");
        } else {
            perror("redirect_exec");
        }

        // Quitter avec un code d'erreur standard
        exit(1);
    } else {
        // Attendre la fin du processus fils
        int status;
        while (1) {
            int ret = waitpid(pid, &status, 0);
            if (ret == -1 && errno == EINTR) {
                // Si waitpid est interrompu par un signal, on réessaie
                continue;
            } else if (ret == -1) {
                // Si une autre erreur se produit, on affiche l'erreur
                perror("waitpid");
                return 1;
            }
            break; // Si waitpid réussit, on sort de la boucle
        }

        // Vérifier si le processus enfant a été interrompu par un signal
        if (WIFSIGNALED(status)) {
            int signal_number = WTERMSIG(status);
            if (signal_number == SIGINT) {
                interrupted = 1;    // Le processus a été interrompu par SIGINT
                return EXIT_SIGINT; // Retourner un code de sortie spécifique pour SIGINT
            }
            if (signal_number == SIGTERM) {
                interrupted = 1;
                return EXIT_SIGTERM; // Retourner un code de sortie spécifique pour
                                     // SIGTERM
            }
        }

        // Vérifier si l'enfant s'est terminé normalement
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status); // Code de sortie de l'enfant
        }

        return 0; // Si aucun signal ou sortie normale n'a eu lieu
    }
}
