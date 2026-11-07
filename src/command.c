#include "../include/command.h"
#include "../include/debug.h"
#include "../include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ------------------------------ Fonctions alloc ------------------------------ //

int alloc_simple_command(simple_command_t **cmd) {
    *cmd = malloc(sizeof(simple_command_t));
    if (*cmd == NULL) {
        perror("alloc_simple_command : Erreur d'allocation mémoire");
        return -1;
    }
    (*cmd)->argc = 0;
    (*cmd)->args = NULL;
    (*cmd)->nb_redir = 0;
    (*cmd)->redir = NULL;
    return 0;
}

int alloc_pipeline_command(pipeline_command_t **cmd) {
    *cmd = malloc(sizeof(pipeline_command_t));
    if (*cmd == NULL) {
        perror("alloc_pipeline_command : Erreur d'allocation mémoire");
        return -1;
    }
    (*cmd)->nb_commands = 0;
    (*cmd)->args = NULL;
    return 0;
}

int alloc_structured_sequence(structured_sequence_t **cmd) {
    *cmd = malloc(sizeof(structured_sequence_t));
    if (*cmd == NULL) {
        perror("alloc_structured_sequence : Erreur d'allocation mémoire");
        return -1;
    }
    (*cmd)->nb_commands = 0;
    (*cmd)->commands = NULL;
    return 0;
}

int alloc_structured_if(structured_if_t **cmd) {
    *cmd = malloc(sizeof(structured_if_t));
    if (*cmd == NULL) {
        perror("alloc_structured_if : Erreur d'allocation mémoire");
        return -1;
    }
    (*cmd)->test = NULL;
    (*cmd)->commands = NULL;
    return 0;
}

int alloc_structured_if_else(structured_if_else_t **cmd) {
    *cmd = malloc(sizeof(structured_if_else_t));
    if (*cmd == NULL) {
        perror("alloc_structured_if_else : Erreur d'allocation mémoire");
        return -1;
    }
    (*cmd)->test = NULL;
    (*cmd)->if_commands = NULL;
    (*cmd)->else_commands = NULL;
    return 0;
}

int alloc_structured_for_option(structured_for_option_t **cmd) {
    *cmd = malloc(sizeof(structured_for_option_t));
    if (*cmd == NULL) {
        perror("alloc_structured_for_option : Erreur d'allocation mémoire");
        return -1;
    }
    (*cmd)->option = '\0';
    (*cmd)->arg = NULL;
    return 0;
}

int alloc_structured_for(structured_for_t **cmd) {
    *cmd = malloc(sizeof(structured_for_t));
    if (*cmd == NULL) {
        perror("alloc_structured_for : Erreur d'allocation mémoire");
        return -1;
    }
    (*cmd)->var = '\0';
    (*cmd)->rep = NULL;
    (*cmd)->nb_options = 0;
    (*cmd)->options = NULL;
    (*cmd)->commands = NULL;
    return 0;
}

int alloc_structured_command(structured_command_t **cmd) {
    *cmd = malloc(sizeof(structured_command_t));
    if (*cmd == NULL) {
        perror("alloc_structured_command : Erreur d'allocation mémoire");
        return -1;
    }
    (*cmd)->condition = NO_CONDITION;
    (*cmd)->seq_cond = NULL; // au pif car mémoire partagée entre union
    return 0;
}

int alloc_command(command_t **cmd) {
    *cmd = malloc(sizeof(command_t));
    if (*cmd == NULL) {
        perror("alloc_command : Erreur d'allocation mémoire");
        return -1;
    }
    (*cmd)->type = NO_TYPE;
    (*cmd)->simple = NULL;
    return 0;
}

// ------------------------------ Fonctions free ------------------------------ //

int free_simple_command(simple_command_t *cmd) {
    if (cmd != NULL) {
        if (cmd->args != NULL) {
            for (int i = 0; cmd->args[i] != NULL; i++) {
                free(cmd->args[i]);
            }
            free(cmd->args);
        }
        if (cmd->redir != NULL) {
            for (int i = 0; cmd->redir[i] != NULL; i++) {
                free_redirection(cmd->redir[i]);
            }
            free(cmd->redir);
        }
        free(cmd);
    }
    return 0;
}

int free_pipeline_command(pipeline_command_t *cmd) {
    if (cmd != NULL) {
        if (cmd->args != NULL) {
            for (int i = 0; cmd->args[i] != NULL; i++) {
                free_simple_command(cmd->args[i]);
            }
            free(cmd->args);
        }
        free(cmd);
    }
    return 0;
}

int free_structured_sequence(structured_sequence_t *cmd) {
    if (cmd != NULL) {
        if (cmd->commands != NULL) {
            for (int i = 0; cmd->commands[i] != NULL; i++) {
                free_command(cmd->commands[i]);
            }
            free(cmd->commands);
        }
        free(cmd);
    }
    return 0;
}

int free_structured_if(structured_if_t *cmd) {
    if (cmd != NULL) {
        free_pipeline_command(cmd->test);
        free_structured_sequence(cmd->commands);
        free(cmd);
    }
    return 0;
}

int free_structured_if_else(structured_if_else_t *cmd) {
    if (cmd != NULL) {
        free_pipeline_command(cmd->test);
        free_structured_sequence(cmd->if_commands);
        free_structured_sequence(cmd->else_commands);
        free(cmd);
    }
    return 0;
}

int free_structured_for_option(structured_for_option_t *cmd) {
    if (cmd != NULL) {
        if (cmd->arg != NULL) {
            free(cmd->arg);
        }
        free(cmd);
    }
    return 0;
}

int free_structured_for(structured_for_t *cmd) {
    if (cmd != NULL) {
        if (cmd->rep != NULL) {
            free(cmd->rep);
        }
        if (cmd->options != NULL) {
            for (int i = 0; cmd->options[i] != NULL; i++) {
                free_structured_for_option(cmd->options[i]);
            }
            free(cmd->options);
        }
        free_structured_sequence(cmd->commands);
        free(cmd);
    }
    return 0;
}
int free_structured_command(structured_command_t *cmd) {
    if (cmd != NULL) {
        switch (cmd->condition) {
        case SEQUENCE:
            free_structured_sequence(cmd->seq_cond);
            break;

        case IF:
            free_structured_if(cmd->if_cond);
            break;

        case IF_ELSE:
            free_structured_if_else(cmd->if_else_cond);
            break;

        case FOR:
            free_structured_for(cmd->for_cond);
            break;

        default:
            // Vide
            break;
        }
        free(cmd);
    }
    return 0;
}

int free_command(command_t *cmd) {
    if (cmd != NULL) {
        switch (cmd->type) {
        case SIMPLE_COMMAND:
            free_simple_command(cmd->simple);
            break;

        case PIPELINE_COMMAND:
            free_pipeline_command(cmd->pipeline);
            break;

        case STRUCTURED_COMMAND:
            free_structured_command(cmd->structured);
            break;

        default:
            // Vide
            break;
        }
        free(cmd);
    }
    return 0;
}

// ------------------------------ Fonctions dup ------------------------------ //

int dup_simple_command(simple_command_t *cmd, simple_command_t **out) {
    if (cmd != NULL && out != NULL) {
        if (alloc_simple_command(out) < 0) {
            perror("dup_simple_command : Erreur d'allocation mémoire");
            return -1;
        }
        (*out)->argc = cmd->argc;
        (*out)->args = malloc((cmd->argc + 1) * sizeof(char *));
        if (!(*out)->args) {
            free_simple_command(*out);
            return -1;
        }
        for (int i = 0; i < cmd->argc; i++) {
            (*out)->args[i] = cmd->args[i] ? strdup(cmd->args[i]) : NULL;
            if (cmd->args[i] && !(*out)->args[i]) {
                free_simple_command(*out);
                return -1;
            }
        }
        (*out)->args[cmd->argc] = NULL;

        (*out)->nb_redir = cmd->nb_redir;
        (*out)->redir = malloc((cmd->nb_redir + 1) * sizeof(redirection_t *));
        if (!(*out)->redir) {
            free_simple_command(*out);
            return -1;
        }
        for (int i = 0; i < cmd->nb_redir; i++) {
            if (dup_redirection(cmd->redir[i], &((*out)->redir[i])) < 0) {
                free_simple_command(*out);
                return -1;
            }
        }
        (*out)->redir[cmd->nb_redir] = NULL;
    }
    return 0;
}

int dup_pipeline_command(pipeline_command_t *cmd, pipeline_command_t **out) {
    if (cmd != NULL && out != NULL) {
        if (alloc_pipeline_command(out) < 0) {
            perror("dup_pipeline_command : Erreur d'allocation mémoire");
            return -1;
        }

        (*out)->nb_commands = cmd->nb_commands;
        (*out)->args = malloc((cmd->nb_commands + 1) * sizeof(simple_command_t *));
        if (!(*out)->args) {
            free_pipeline_command(*out);
            return -1;
        }
        for (int i = 0; i < cmd->nb_commands; i++) {
            if (dup_simple_command(cmd->args[i], &((*out)->args[i])) < 0) {
                free_pipeline_command(*out);
                return -1;
            }
        }
        (*out)->args[cmd->nb_commands] = NULL;
    }
    return 0;
}

int dup_structured_sequence(structured_sequence_t *cmd, structured_sequence_t **out) {
    if (cmd != NULL && out != NULL) {
        if (alloc_structured_sequence(out) < 0) {
            perror("dup_structured_sequence : Erreur d'allocation mémoire");
            return -1;
        }

        (*out)->nb_commands = cmd->nb_commands;
        (*out)->commands = malloc((cmd->nb_commands + 1) * sizeof(command_t *));
        if (!(*out)->commands) {
            free_structured_sequence(*out);
            return -1;
        }
        for (int i = 0; i < cmd->nb_commands; i++) {
            if (dup_command(cmd->commands[i], &((*out)->commands[i])) < 0) {
                free_structured_sequence(*out);
                return -1;
            }
        }
        (*out)->commands[cmd->nb_commands] = NULL;
    }
    return 0;
}

int dup_structured_if(structured_if_t *cmd, structured_if_t **out) {
    if (cmd != NULL && out != NULL) {
        if (alloc_structured_if(out) < 0) {
            perror("dup_structured_if : Erreur d'allocation mémoire");
            return -1;
        }

        if (dup_pipeline_command(cmd->test, &((*out)->test)) < 0 ||
            dup_structured_sequence(cmd->commands, &((*out)->commands)) < 0) {
            free_structured_if(*out);
            return -1;
        }
    }
    return 0;
}

int dup_structured_if_else(structured_if_else_t *cmd, structured_if_else_t **out) {
    if (cmd != NULL && out != NULL) {
        if (alloc_structured_if_else(out) < 0) {
            perror("dup_structured_if_else : Erreur d'allocation mémoire");
            return -1;
        }

        if (dup_pipeline_command(cmd->test, &((*out)->test)) < 0 ||
            dup_structured_sequence(cmd->if_commands, &((*out)->if_commands)) < 0 ||
            dup_structured_sequence(cmd->else_commands, &((*out)->else_commands)) < 0) {
            free_structured_if_else(*out);
            return -1;
        }
    }
    return 0;
}

int dup_structured_for_option(structured_for_option_t *cmd,
                              structured_for_option_t **out) {
    if (cmd != NULL && out != NULL) {
        if (alloc_structured_for_option(out) < 0) {
            perror("dup_structured_for_option : Erreur d'allocation mémoire");
            return -1;
        }

        (*out)->option = cmd->option;
        (*out)->arg = cmd->arg ? strdup(cmd->arg) : NULL;
        if (cmd->arg && !(*out)->arg) {
            free_structured_for_option(*out);
            return -1;
        }
    }
    return 0;
}

int dup_structured_for(structured_for_t *cmd, structured_for_t **out) {
    if (cmd != NULL && out != NULL) {
        if (alloc_structured_for(out) < 0) {
            perror("dup_structured_for : Erreur d'allocation mémoire");
            return -1;
        }

        (*out)->var = cmd->var;
        (*out)->rep = cmd->rep ? strdup(cmd->rep) : NULL;
        if (cmd->rep && !(*out)->rep) {
            free_structured_for(*out);
            return -1;
        }

        (*out)->nb_options = cmd->nb_options;
        (*out)->options =
            malloc((cmd->nb_options + 1) * sizeof(structured_for_option_t *));
        if (!(*out)->options) {
            free_structured_for(*out);
            return -1;
        }
        for (int i = 0; i < cmd->nb_options; i++) {
            if (dup_structured_for_option(cmd->options[i], &((*out)->options[i])) < 0) {
                free_structured_for(*out);
                return -1;
            }
        }
        (*out)->options[cmd->nb_options] = NULL;

        if (dup_structured_sequence(cmd->commands, &((*out)->commands)) < 0) {
            free_structured_for(*out);
            return -1;
        }
    }
    return 0;
}

int dup_structured_command(structured_command_t *cmd, structured_command_t **out) {
    if (cmd != NULL && out != NULL) {

        if (alloc_structured_command(out) < 0) {
            perror("dup_structured_command : Erreur d'allocation mémoire");
            return -1;
        }

        (*out)->condition = cmd->condition;

        switch (cmd->condition) {
        case SEQUENCE:
            if (dup_structured_sequence(cmd->seq_cond, &(*out)->seq_cond) < 0) {
                free_structured_command(*out);
                return -1;
            }
            break;

        case IF:
            if (dup_structured_if(cmd->if_cond, &(*out)->if_cond) < 0) {
                free_structured_command(*out);
                return -1;
            }
            break;

        case IF_ELSE:
            if (dup_structured_if_else(cmd->if_else_cond, &(*out)->if_else_cond) < 0) {
                free_structured_command(*out);
                return -1;
            }
            break;

        case FOR:
            if (dup_structured_for(cmd->for_cond, &(*out)->for_cond) < 0) {
                free_structured_command(*out);
                return -1;
            }
            break;

        default:
            free_structured_command(*out);
            return -1; // Condition invalide
        }
    }
    return 0;
}

int dup_command(command_t *cmd, command_t **out) {
    if (cmd != NULL && out != NULL) {

        if (alloc_command(out) < 0) {
            perror("dup_command : Erreur d'allocation mémoire");
            return -1;
        }

        (*out)->type = cmd->type;

        switch (cmd->type) {
        case SIMPLE_COMMAND:
            if (dup_simple_command(cmd->simple, &(*out)->simple) < 0) {
                free_command(*out);
                return -1;
            }
            break;

        case PIPELINE_COMMAND:
            if (dup_pipeline_command(cmd->pipeline, &(*out)->pipeline) < 0) {
                free_command(*out);
                return -1;
            }
            break;

        case STRUCTURED_COMMAND:
            if (dup_structured_command(cmd->structured, &(*out)->structured) < 0) {
                free_command(*out);
                return -1;
            }
            break;

        default:
            free_command(*out);
            return -1; // Type invalide
        }
    }
    return 0;
}

// --------------------------- Fonctions print (debug) --------------------------- //

int print_simple_command(simple_command_t *cmd) {
    LOG_INFO("Print : Simple command");
    if (cmd == NULL) {
        LOG_WARNING("print_simple_command [NULL]");
        return 0;
    }
    if (cmd->args != NULL) {
        for (size_t i = 0; i < cmd->argc; i++) {
            write_output_no_ln(cmd->args[i]);
            if (i < cmd->argc - 1) {
                write_output_no_ln(" ");
            }
        }
    }
    if (cmd->redir != NULL) {
        for (size_t i = 0; i < cmd->nb_redir; i++) {
            if (print_redirection(cmd->redir[i]) < 0) {
                write_err("print_simple_command");
                return -1;
            }
        }
    }
    return 0;
}

int print_pipeline_command(pipeline_command_t *cmd) // facto
{
    LOG_INFO("Print : Pipeline command");
    if (cmd == NULL) {
        LOG_WARNING("print_pipeline_command [NULL]");
        return 0;
    }
    if (cmd->args != NULL) {
        for (size_t i = 0; i < cmd->nb_commands; i++) {
            if (print_simple_command(cmd->args[i]) < 0) {
                write_err("print_pipeline_command");
                return -1;
            }
            if (i < cmd->nb_commands - 1) {
                write_output_no_ln(" | ");
            }
        }
    }
    return 0;
}

int print_structured_sequence(structured_sequence_t *cmd) {
    LOG_INFO("Print : Structured sequence");
    if (cmd == NULL) {
        LOG_WARNING("print_structured_sequence [NULL]");
        return 0;
    }
    if (cmd->commands != NULL) {
        for (size_t i = 0; i < cmd->nb_commands; i++) {
            if (print_command(cmd->commands[i]) < 0) {
                write_err("print_structured_sequence");
                return -1;
            }
            if (i < cmd->nb_commands - 1) {
                write_output_no_ln(" ; ");
            }
        }
    }
    return 0;
}

int print_structured_if(structured_if_t *cmd) {
    LOG_INFO("Print : Structured if");
    if (cmd == NULL) {
        LOG_WARNING("print_structured_if [NULL]");
        return 0;
    }
    write_output_no_ln("if ");
    if (print_pipeline_command(cmd->test)) {
        write_err("print_structured_if");
        return -1;
    }
    write_output_no_ln(" { ");
    if (print_structured_sequence(cmd->commands) < 0) {
        write_err("print_structured_if");
        return -1;
    }
    write_output_no_ln(" }");
    return 0;
}

int print_structured_if_else(structured_if_else_t *cmd) {
    LOG_INFO("Print : Structured if else");
    if (cmd == NULL) {
        LOG_WARNING("print_structured_if_else [NULL]");
        return 0;
    }
    write_output_no_ln("if ");
    if (print_pipeline_command(cmd->test)) {
        write_err("print_structured_if_else");
        return -1;
    }
    write_output_no_ln(" { ");
    if (print_structured_sequence(cmd->if_commands) < 0) {
        write_err("print_structured_if_else");
        return -1;
    }
    write_output_no_ln(" } else { ");
    if (print_structured_sequence(cmd->else_commands) < 0) {
        write_err("print_structured_if_else");
        return -1;
    }
    write_output_no_ln(" }");
    return 0;
}

int print_structured_for_option(structured_for_option_t *cmd) {
    LOG_INFO("Print : Structured for option");
    if (cmd == NULL) {
        LOG_WARNING("print_structured_for_option [NULL]");
        return 0;
    }
    write_output_no_ln("-");
    write_output_no_ln((char[]){cmd->option, '\0'});
    if (cmd->arg != NULL) {
        write_output_no_ln(" ");
        write_output_no_ln(cmd->arg);
    }
    return 0;
}

int print_structured_for(structured_for_t *cmd) {
    LOG_INFO("Print : Structured for");
    if (cmd == NULL) {
        LOG_WARNING("print_structured_for [NULL]");
        return 0;
    }
    write_output_no_ln("for ");
    write_output_no_ln((char[]){cmd->var, '\0'});
    write_output_no_ln(" in ");
    if (cmd->rep != NULL) {
        write_output_no_ln(cmd->rep);
    } else {
        write_err("print_structured_for");
        return -1;
    }
    write_output_no_ln(" ");
    if (cmd->options != NULL) {
        for (size_t i = 0; i < cmd->nb_options; i++) {
            if (print_structured_for_option(cmd->options[i]) < 0) {
                write_err("print_structured_for");
                return -1;
            }
            write_output_no_ln(" ");
        }
    }
    write_output_no_ln("{ ");
    if (print_structured_sequence(cmd->commands) < 0) {
        write_err("print_structured_for");
        return -1;
    }
    write_output_no_ln(" }");
    return 0;
}

int print_structured_command(structured_command_t *cmd) {
    LOG_INFO("Print : Structured command");
    if (cmd == NULL) {
        LOG_WARNING("print_structured_command [NULL]");
        return 0;
    }
    switch (cmd->condition) {
    case NO_CONDITION:
        break;

    case SEQUENCE:
        if (print_structured_sequence(cmd->seq_cond) < 0) {
            write_err("print_structured_command");
            return -1;
        }
        break;

    case IF:
        if (print_structured_if(cmd->if_cond) < 0) {
            write_err("print_structured_command");
            return -1;
        }
        break;

    case IF_ELSE:
        if (print_structured_if_else(cmd->if_else_cond) < 0) {
            write_err("print_structured_command");
            return -1;
        }
        break;

    case FOR:
        if (print_structured_for(cmd->for_cond) < 0) {
            write_err("print_structured_command");
            return -1;
        }
        break;

    default:
        write_err("print_structured_command");
        return -1;
    }
    return 0;
}

int print_command(command_t *cmd) {
    LOG_INFO("Print : command");
    if (cmd == NULL) {
        LOG_WARNING("print_command [NULL]");
        return 0;
    }
    switch (cmd->type) {
    case NO_TYPE:
        break;

    case SIMPLE_COMMAND:
        if (print_simple_command(cmd->simple) < 0) {
            write_err("print_command");
            return -1;
        }
        break;

    case PIPELINE_COMMAND:
        if (print_pipeline_command(cmd->pipeline) < 0) {
            write_err("print_command");
            return -1;
        }
        break;

    case STRUCTURED_COMMAND:
        if (print_structured_command(cmd->structured) < 0) {
            write_err("print_command");
            return -1;
        }
        break;

    default:
        write_err("print_command");
        return -1;
    }
    return 0;
}
