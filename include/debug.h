#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

// Activer ou désactiver le mode débogage ici
#ifndef DEBUG
// #define DEBUG  // Décommenter pour activer
#endif

// Niveaux de log
typedef enum {
    NO_LOG,      // 0 (Uniquement DEBUG_CALL) (DEFAUT)
    LOG_INFO,    // 1
    LOG_WARNING, // 2
    LOG_ERROR,   // 3
} log_level_t;

// Niveau de log
#ifndef LOG_LEVEL
// Niveau de log par défaut
#define LOG_LEVEL NO_LOG
#endif

typedef enum {
    LOG_UNIQ, // 0
    LOG_MAX   // 1
} log_trigger_t;

// Uniquement un niveau de log précis
#ifndef LOG_TRIGGER
// Niveau de log par défaut
#define LOG_TRIGGER LOG_UNIQ
#endif

// Macro pour gérer les logs selon le niveau
#ifdef DEBUG
#define LOG(level, msg) log_message(level, msg, __FILE__, __LINE__)
#define DEBUG_CALL(func_call)                                                                                          \
    do {                                                                                                               \
        if (LOG_LEVEL == NO_LOG)                                                                                       \
            func_call;                                                                                                 \
    } while (0)
#else
#define LOG(level, msg)                                                                                                \
    do {                                                                                                               \
    } while (0)
#define DEBUG_CALL(func_call) ((void)0)
#endif

// Fonctions spécifiques pour chaque type de log
#define LOG_INFO(msg) LOG(LOG_INFO, msg)
#define LOG_WARNING(msg) LOG(LOG_WARNING, msg)
#define LOG_ERROR(msg) LOG(LOG_ERROR, msg)

// Fonction de gestion des messages de log
void log_message(log_level_t level, const char *msg, const char *file, int line);

#endif
