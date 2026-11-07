#include "../include/command.h"
#include "../include/debug.h"
#include "../include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ------------------------------ Fonctions utils ------------------------------ //
const char *redir_list[] = {"<", ">", "2>", ">>", "2>>", ">|", "2>|"};

int is_redirection(char *string) {

    const int list_size = 7;
    for (int i = 0; i < list_size; i++) {
        if (strcmp(string, redir_list[i]) == 0) {
            return 1; // Si la chaîne correspond à l'une des chaînes de la liste
        }
    }
    return 0;
}

int get_redirection_value(char *redir_string) {

    const int list_size = 7;
    for (int i = 0; i < list_size; i++) {
        if (strcmp(redir_string, redir_list[i]) == 0) {
            return i + 1;
        }
    }
    return 0;
}

int alloc_redirection(redirection_t **cmd) {
    *cmd = malloc(sizeof(redirection_t));
    if (*cmd == NULL) {
        perror("alloc_redirection : Erreur d'allocation mémoire");
        return -1;
    }
    (*cmd)->type = NO_REDIRECT;
    (*cmd)->file = NULL;
    return 0;
}

int free_redirection(redirection_t *cmd) {
    if (cmd != NULL) {
        if (cmd->file != NULL) {
            free(cmd->file);
        }
        free(cmd);
    }
    return 0;
}

int dup_redirection(redirection_t *cmd, redirection_t **out) {
    if (cmd != NULL && out != NULL) {

        if (alloc_redirection(out) < 0) {
            perror("dup_redirection : Erreur d'allocation mémoire");
            return -1;
        }

        (*out)->type = cmd->type;
        (*out)->file = cmd->file ? strdup(cmd->file) : NULL;
        if (cmd->file && !(*out)->file) {
            free_redirection(*out);
            return -1;
        }
    }
    return 0;
}

int print_redirection(redirection_t *cmd) // TODO : facto
{
    LOG_INFO("Print : Redirection");
    if (cmd == NULL) {
        LOG_WARNING("print_redirection [NULL]");
        return 0;
    }

    char *redir_char = "";
    switch (cmd->type) {
    case 0:
        redir_char = "\0";
        break;
    case 1:
        redir_char = " < ";
        break;
    case 2:
        redir_char = " > ";
        break;
    case 3:
        redir_char = " 2> ";
        break;
    case 4:
        redir_char = " >> ";
        break;
    case 5:
        redir_char = " 2>> ";
        break;
    case 6:
        redir_char = " >| ";
        break;
    case 7:
        redir_char = " 2>| ";
        break;
    default:
        LOG_INFO("Erreur : print_redirection");
        return -1;
    }
    if (cmd->type > 0) {
        write_output_no_ln(redir_char);
        write_output_no_ln(cmd->file);
    }
    return 0;
}