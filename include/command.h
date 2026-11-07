#ifndef COMMAND_H
#define COMMAND_H

#include "redirection.h"

#include <stdlib.h>

typedef enum command_condition_t command_condition_t;
typedef enum command_type_t command_type_t;

typedef struct simple_command_t simple_command_t;
typedef struct pipeline_command_t pipeline_command_t;
typedef struct structured_sequence_t structured_sequence_t;
typedef struct structured_if_t structured_if_t;
typedef struct structured_if_else_t structured_if_else_t;
typedef struct structured_for_option_t structured_for_option_t;
typedef struct structured_for_t structured_for_t;
typedef struct structured_command_t structured_command_t;
typedef struct command_t command_t;

enum command_condition_t {
    NO_CONDITION, // Val par defaut
    SEQUENCE,     // CMD_1 ; CMD_2 ; ... ; CMD_N
    IF,           // if TEST { CMD }
    IF_ELSE,      // if TEST { CMD_1 } else { CMD_2 }
    FOR           // for F in REP [-A] [-r] [-e EXT] [-t TYPE] [-p MAX] { CMD }
};

enum command_type_t {
    NO_TYPE, // Val par defaut
    SIMPLE_COMMAND,
    PIPELINE_COMMAND,
    STRUCTURED_COMMAND
};

struct simple_command_t {
    int argc;              // Nombre d'arguments
    char **args;           // Arguments de la commande
    int nb_redir;          // Nombre de redirections
    redirection_t **redir; // Tableaux de redirections
};

struct pipeline_command_t {
    int nb_commands;         // N commandes simples
    simple_command_t **args; // Tableaux de commandes simples
};

struct structured_sequence_t {
    int nb_commands;      // Nombre de commandes
    command_t **commands; // Effectuer dans l'ordre
};

struct structured_if_t {
    pipeline_command_t *test;        // Test
    structured_sequence_t *commands; // Effectuer si reussite de if
};

struct structured_if_else_t {
    pipeline_command_t *test;             // Test
    structured_sequence_t *if_commands;   // Effectuer si reussite de if
    structured_sequence_t *else_commands; // Sinon
};

struct structured_for_option_t {
    char option; // Option unique
    char *arg;   // Argument unique s'il exsite
};

struct structured_for_t {
    char var;                          // Variable de boucle de for
    char *rep;                         // Repertoire cible;
    int nb_options;                    // Nombre d'options
    structured_for_option_t **options; // Tableaux d'options spécifiques
    structured_sequence_t *commands;
};
struct structured_command_t {
    command_condition_t condition; // Condition de la structure, pipeline exclu
    union {
        structured_sequence_t *seq_cond;
        structured_if_t *if_cond;
        structured_if_else_t *if_else_cond;
        structured_for_t *for_cond;
    };
};

struct command_t {
    command_type_t type; // Type de la commande
    union {
        simple_command_t *simple;         // Commande simple
        pipeline_command_t *pipeline;     // Commande pipeline
        structured_command_t *structured; // Commande structurée
    };
};

// ------------------------------ Fonctions alloc ------------------------------ //
int alloc_simple_command(simple_command_t **cmd);
int alloc_pipeline_command(pipeline_command_t **cmd);
int alloc_structured_sequence(structured_sequence_t **cmd);
int alloc_structured_if(structured_if_t **cmd);
int alloc_structured_if_else(structured_if_else_t **cmd);
int alloc_structured_for_option(structured_for_option_t **cmd);
int alloc_structured_for(structured_for_t **cmd);
int alloc_structured_command(structured_command_t **cmd);
int alloc_command(command_t **cmd);

// ------------------------------ Fonctions free ------------------------------ //
int free_simple_command(simple_command_t *cmd);
int free_pipeline_command(pipeline_command_t *cmd);
int free_structured_sequence(structured_sequence_t *cmd);
int free_structured_if(structured_if_t *cmd);
int free_structured_if_else(structured_if_else_t *cmd);
int free_structured_for_option(structured_for_option_t *cmd);
int free_structured_for(structured_for_t *cmd);
int free_structured_command(structured_command_t *cmd);
int free_command(command_t *cmd);

// ------------------------------ Fonctions dup ------------------------------ //
int dup_simple_command(simple_command_t *cmd, simple_command_t **out);
int dup_pipeline_command(pipeline_command_t *cmd, pipeline_command_t **out);
int dup_structured_sequence(structured_sequence_t *cmd, structured_sequence_t **out);
int dup_structured_if(structured_if_t *cmd, structured_if_t **out);
int dup_structured_if_else(structured_if_else_t *cmd, structured_if_else_t **out);
int dup_structured_for_option(structured_for_option_t *cmd,
                              structured_for_option_t **out);
int dup_structured_for(structured_for_t *cmd, structured_for_t **out);
int dup_structured_command(structured_command_t *cmd, structured_command_t **out);
int dup_command(command_t *cmd, command_t **out);

// ---------------------------- Fonctions print (debug) ---------------------------- //
int print_simple_command(simple_command_t *cmd);
int print_pipeline_command(pipeline_command_t *cmd);
int print_structured_sequence(structured_sequence_t *cmd);
int print_structured_if(structured_if_t *cmd);
int print_structured_if_else(structured_if_else_t *cmd);
int print_structured_for_option(structured_for_option_t *cmd);
int print_structured_for(structured_for_t *cmd);
int print_structured_command(structured_command_t *cmd);
int print_command(command_t *cmd);

#endif