#ifndef SIGNAL_H
#define SIGNAL_H

#include <signal.h>

#define EXIT_SIGINT 100  // SIGUSR1 * 10 (Au hasard, "logique" mais pas trop)
#define EXIT_SIGTERM 120 // SIGUSR2 * 10 (Au hasard, "logique" mais pas trop)

// Variable globale indiquant si une interruption a été reçue
extern volatile sig_atomic_t interrupted;
extern volatile int last_status;

// Prototypes des fonctions
void handle_sigint(int signo);
void handle_sigterm(int signo);
void setup_signals(void);
void reset_signals_to_default(void);

#endif