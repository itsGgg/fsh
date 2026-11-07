#include "../include/parser.h"
#include "../include/debug.h"
#include "../include/utils.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int tokenize(const char *line, char ***tokens_out) {
    if (line == NULL || tokens_out == NULL) {
        return -1; // Vérification des arguments d'entrée
    }

    const char *delimiters = " ";
    size_t initial_capacity = 10;
    size_t token_count = 0;
    char **tokens = malloc(initial_capacity * sizeof(char *));
    if (tokens == NULL) {
        perror("Erreur d'allocation mémoire");
        return -1;
    }

    char *line_copy = strdup(line); // Créer une copie modifiable de la ligne
    if (line_copy == NULL) {
        perror("Erreur d'allocation mémoire");
        free(tokens);
        return -1;
    }

    char *token = strtok(line_copy, delimiters);
    while (token != NULL) {
        if (token_count >= initial_capacity) {
            initial_capacity *= 2;
            char **new_tokens = realloc(tokens, initial_capacity * sizeof(char *));
            if (new_tokens == NULL) {
                perror("Erreur d'allocation mémoire");
                free(line_copy);
                free(tokens);
                return -1;
            }
            tokens = new_tokens;
        }

        tokens[token_count] = strdup(token); // Allouer pour chaque token
        if (tokens[token_count] == NULL) {
            perror("Erreur d'allocation mémoire");
            free(line_copy);
            for (size_t i = 0; i < token_count; i++) {
                free(tokens[i]);
            }
            free(tokens);
            return -1;
        }

        token_count++;
        token = strtok(NULL, delimiters);
    }

    free(line_copy); // Libérer la copie globale, plus utilisée

    tokens = realloc(tokens, (token_count + 1) * sizeof(char *));
    if (tokens == NULL) {
        perror("Erreur d'allocation mémoire");
        for (size_t i = 0; i < token_count; i++) {
            free(tokens[i]);
        }
        free(tokens);
        return -1;
    }
    tokens[token_count] = NULL; // Ajouter un marqueur de fin

    *tokens_out = tokens;
    return token_count; // Retourne le nombre de tokens trouvés
}

// Fonction pour obtenir un sous-tableau (subtokens) d'un tableau de tokens, end inclus
int extract_subtokens(char **tokens, int start, int end, char ***subtokens) {
    if (start < 0 || end < start) {
        return -1; // Indices invalides
    }

    // Nombre de tokens dans l'intervalle
    int num_subtokens = end - start + 1;

    *subtokens = malloc((num_subtokens + 1) * sizeof(char *));
    if (*subtokens == NULL) {
        perror("allocation pour sous-tableau de tokens");
        return -1;
    }

    // Copier les tokens dans le sous-tableau
    for (int i = 0; i < num_subtokens; i++) {
        (*subtokens)[i] = strdup(tokens[start + i]);
        if ((*subtokens)[i] == NULL) {
            perror("extract_subtokens : Erreur d'allocation mémoire");
            // Libération des chaînes déjà allouées en cas d'erreur
            for (int j = 0; j < i; j++) {
                free((*subtokens)[j]);
            }
            free(*subtokens);
            return -1;
        }
    }

    (*subtokens)[num_subtokens] = NULL;

    return num_subtokens;
}

int free_tokens(char **tokens) {
    if (tokens == NULL) {
        return 0;
    }
    for (int i = 0; tokens[i] != NULL; i++) {
        free(tokens[i]);
    }
    free(tokens);
    return 0;
}

int print_tokens(char **tokens) {
    if (tokens == NULL) {
        return 0;
    }
    for (int i = 0; tokens[i] != NULL; i++) {
        printf("Token %d: %s\n", i, tokens[i]);
    }
    return 0;
}

int split_tokens(char **tokens, const char *delimiter, char ****result) {
    if (!tokens || !delimiter || !result) // == NULL
    {
        return -1; // Entrées invalides
    }

    size_t token_count = 0; // Compter le nombre de tokens
    while (tokens[token_count] != NULL) {
        token_count++;
    }

    char ***temp_result = malloc(sizeof(char **) * (token_count + 2));
    if (!temp_result) {
        perror("split_tokens : Erreur d'allocation mémoire");
        return -1;
    }

    size_t result_index = 0;
    size_t start = 0;
    int count_braces = 0;

    for (size_t i = 0; i <= token_count; i++) {
        if (tokens[i] != NULL && strcmp(tokens[i], "{") == 0) {
            count_braces++;
        } else if (tokens[i] != NULL && strcmp(tokens[i], "}") == 0) {
            count_braces--;
        } else if ((tokens[i] == NULL || strcmp(tokens[i], delimiter) == 0) &&
                   count_braces == 0) {

            // Trouver un délimiteur ou la fin du tableau
            size_t segment_size = i - start;
            char **segment = malloc(sizeof(char *) * (segment_size + 1));
            if (!segment) {
                perror("split_tokens : Erreur d'allocation mémoire pour un segment");

                // Libérer les segments déjà alloués
                for (size_t j = 0; j < result_index; j++) {
                    free(temp_result[j]);
                }
                free(temp_result);
                return -1;
            }

            for (size_t j = 0; j < segment_size; j++) {
                segment[j] = strdup(tokens[start + j]);
                if (!segment[j]) {
                    perror("split_tokens : Erreur de duplication de chaîne");

                    // Libérer les segments déjà alloués
                    for (size_t k = 0; k < j; k++) {
                        free(segment[k]);
                    }
                    free(segment);

                    for (size_t k = 0; k < result_index; k++) {
                        free(temp_result[k]);
                    }
                    free(temp_result);
                    return -1;
                }
            }
            segment[segment_size] = NULL;

            temp_result[result_index++] = segment;
            start = i + 1; // Passer après le délimiteur
        }
    }

    temp_result[result_index] = NULL;
    *result = temp_result;
    return result_index;
}

int free_split_tokens(char ***split_tokens) {
    if (!split_tokens) {
        return -0;
    }

    for (size_t i = 0; split_tokens[i] != NULL; i++) {
        char **segment = split_tokens[i];
        for (size_t j = 0; segment[j] != NULL; j++) {
            free(segment[j]);
        }
        free(segment);
    }

    free(split_tokens);
    return 0;
}

int print_split_tokens(char ***split_tokens) {
    if (!split_tokens) {
        write_err("print_split_tokens : Le tableau est NULL");
        return -1;
    }

    for (size_t i = 0; split_tokens[i] != NULL; i++) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Segment %zu:", i);
        write_output(buffer);
        for (size_t j = 0; split_tokens[i][j] != NULL; j++) {
            write_output(split_tokens[i][j]);
        }
    }
    return 0;
}

redirection_value_t tokens_get_redirection_value(char **tokens) {
    redirection_value_t res = NO_REDIRECT;
    if (tokens != NULL) {
        for (size_t i = 0; tokens[i] != NULL; i++) {
            if (strcmp(tokens[i], "<") == 0) {
                res = REDIRECT_IN;
            } else if (strcmp(tokens[i], ">") == 0) {
                res = REDIRECT_OUT;
            } else if (strcmp(tokens[i], "2>") == 0) {
                res = REDIRECT_ERR;
            } else if (strcmp(tokens[i], ">>") == 0) {
                res = REDIRECT_OUT_APPEND;
            } else if (strcmp(tokens[i], "2>>") == 0) {
                res = REDIRECT_ERR_APPEND;
            } else if (strcmp(tokens[i], ">|") == 0) {
                res = REDIRECT_OUT_OVERWRITE;
            } else if (strcmp(tokens[i], "2>|") == 0) {
                res = REDIRECT_ERR_OVERWRITE;
            }
        }
    }
    return res;
}

command_condition_t tokens_get_command_condition(char **tokens) {
    int count_brakets = 0;
    int have_semicolon = 0;
    int have_if = 0;
    int have_else = 0;
    int have_for = 0;

    if (tokens != NULL) {
        for (size_t i = 0; tokens[i] != NULL; i++) {
            if (strcmp(tokens[i], "{") == 0) {
                count_brakets++;
            } else if (strcmp(tokens[i], "}") == 0) {
                count_brakets--;
            } else if (count_brakets == 0) {
                if (strcmp(tokens[i], ";") == 0) {
                    have_semicolon = 1;
                } else if (strcmp(tokens[i], "if") == 0) {
                    have_if = 1;
                } else if (strcmp(tokens[i], "else") == 0) {
                    have_else = 1;
                } else if (strcmp(tokens[i], "for") == 0) {
                    have_for = 1;
                }
            }
        }
        if (have_semicolon) {
            return SEQUENCE;
        } else if (have_for) {
            return FOR;
        } else if (have_if) {
            if (have_else) {
                return IF_ELSE;
            } else {
                return IF;
            }
        }
    }
    return SEQUENCE; // Séquence vide
}

command_type_t tokens_get_command_type(char **tokens) {
    // Cette fonction marche si la ligne de commande est bien formée (condition vérifiée
    // avant l'appel), alors seule cette vérification suffit normalement
    int is_pipeline = 0;
    int count_brakets = 0;

    if (tokens != NULL && tokens[0] != NULL) {
        if (strcmp(tokens[0], "for") == 0) {
            return STRUCTURED_COMMAND;
        } else if (strcmp(tokens[0], "if") == 0) {
            return STRUCTURED_COMMAND;
        }

        for (size_t i = 0; tokens[i] != NULL; i++) {
            if (strcmp(tokens[i], "{") == 0) {
                count_brakets++;
            } else if (strcmp(tokens[i], "}") == 0) {
                count_brakets--;
            } else if (count_brakets == 0) {
                if (strcmp(tokens[i], "|") == 0) {
                    is_pipeline = 1;
                } else if (strcmp(tokens[i], ";") == 0) {
                    return STRUCTURED_COMMAND; // STRUCTURED_COMMAND a la priorité n°1
                }
            }
        }
        return (is_pipeline == 1) ? PIPELINE_COMMAND : SIMPLE_COMMAND;
    }
    return NO_TYPE;
}

redirection_t *tokens_to_redirection(char **tokens) {
    LOG_INFO("Tokens -> redirection");
    redirection_t *cmd = NULL;

    if (tokens == NULL) {
        goto cleanup_return;
    }

    if (alloc_redirection(&cmd) < 0)
        goto cleanup_return;

    if (tokens == NULL) {
        goto cleanup_return;
    }

    // Déterminer le type de redirection
    cmd->type = get_redirection_value(tokens[0]);

    // Dupliquer la chaîne de caractères pour le fichier de redirection
    cmd->file = strdup(tokens[1]);
    if (cmd->file == NULL) {
        perror("tokens_to_redirection : Erreur de duplication de chaîne");
        goto cleanup_return;
    }
cleanup_return:
    return cmd;
}

simple_command_t *tokens_to_simple_command(char **tokens) {
    LOG_INFO("Tokens -> simple");
    simple_command_t *cmd = NULL;

    if (tokens == NULL) {
        goto cleanup_return;
    }

    int count_args = 0;
    int start_redir = -1;
    int end_redir = -1;
    char **sub_tokens = NULL;
    int count_redir = 0;

    if (alloc_simple_command(&cmd) < 0)
        goto cleanup_return;

    // Parcourir les tokens jusqu'à la première redirection
    for (size_t i = 0; tokens[i] != NULL; i++) {
        if (is_redirection(tokens[i]) && start_redir == -1) {
            start_redir = i;
        }
        if (i != -1 && start_redir == -1) {
            count_args++;
        }
        end_redir++;
    }

    // Allocation des arguments
    cmd->args = malloc((count_args + 1) * sizeof(char *));
    if (cmd->args == NULL) {
        perror("tokens_to_simple_command : Erreur d'allocation cmd->args");
        goto cleanup_return;
    }
    // Copie des arguments
    for (int i = 0; i < count_args; i++) {
        cmd->args[i] = strdup(tokens[i]);
        if (cmd->args[i] == NULL) {
            perror("Erreur de duplication de token");
            goto cleanup_return;
        }
    }
    cmd->args[count_args] = NULL;
    cmd->argc = count_args;

    if (start_redir != -1) {
        if (extract_subtokens(tokens, start_redir, end_redir, &sub_tokens) < 0) {
            goto cleanup_return;
        }
        for (size_t i = 0; sub_tokens[i] != NULL; i++) {
            if (is_redirection(sub_tokens[i])) {
                count_redir++;
            }
        }
        cmd->redir = malloc((count_redir + 1) * sizeof(redirection_t *));
        for (size_t i = 0; i < count_redir; i++) {
            if (sub_tokens[2 * i] != NULL && sub_tokens[2 * i + 1] != NULL) {
                char *temp_redir_tokens[] = {sub_tokens[2 * i], sub_tokens[2 * i + 1],
                                             NULL};
                cmd->redir[i] = tokens_to_redirection(temp_redir_tokens);
            } else {
                perror("tokens_to_simple_command : sub_tokens count_redir");
                goto cleanup_return;
            }
        }
        cmd->redir[count_redir] = NULL;
        cmd->nb_redir = count_redir;
    }

cleanup_return:
    if (sub_tokens != NULL) {
        free_tokens(sub_tokens);
    }
    return cmd;
}

pipeline_command_t *tokens_to_pipeline_command(char **tokens) {
    LOG_INFO("Tokens -> pipeline");
    pipeline_command_t *cmd = NULL;

    if (tokens == NULL) {
        goto cleanup_return;
    }

    const char *tokens_delimiter = "|";
    char ***splited_tokens = NULL;
    int count_splited_tokens = 0;

    if (alloc_pipeline_command(&cmd) < 0)
        goto cleanup_return;

    count_splited_tokens = split_tokens(tokens, tokens_delimiter, &splited_tokens);
    if (count_splited_tokens < 0) {
        write_err("tokens_to_pipeline_command : split_tokens");
        goto cleanup_return;
    }

    cmd->args = malloc((count_splited_tokens + 1) * sizeof(simple_command_t *));
    if (cmd->args == NULL) {
        perror("tokens_to_pipeline_command : Erreur d'allocation cmd->args");
        goto cleanup_return;
    }

    for (size_t i = 0; i < count_splited_tokens; i++) {
        if ((cmd->args[i] = tokens_to_simple_command(splited_tokens[i])) < 0) {
            perror("tokens_to_pipeline_command : Erreur cmd->args[i]");
            goto cleanup_return;
        }
    }
    cmd->nb_commands = count_splited_tokens;
    cmd->args[count_splited_tokens] = NULL;

cleanup_return:
    if (splited_tokens != NULL) {
        free_split_tokens(splited_tokens);
    }
    return cmd;
}

structured_sequence_t *tokens_to_structured_sequence(char **tokens) {
    LOG_INFO("Tokens -> sequence");
    structured_sequence_t *cmd = NULL;

    if (tokens == NULL) {
        goto cleanup_return;
    }

    const char *tokens_delimiter = ";";
    char ***splited_tokens = NULL;
    int count_splited_tokens = 0;

    if (alloc_structured_sequence(&cmd) < 0)
        goto cleanup_return;

    count_splited_tokens = split_tokens(tokens, tokens_delimiter, &splited_tokens);
    if (count_splited_tokens < 0) {
        write_err("structured_sequence_t : split_tokens");
        goto cleanup_return;
    }

    cmd->commands = malloc((count_splited_tokens + 1) * sizeof(command_t *));
    if (cmd->commands == NULL) {
        perror("structured_sequence_t : Erreur d'allocation cmd->commands");
        goto cleanup_return;
    }
    for (size_t i = 0; i < count_splited_tokens; i++) {
        if (splited_tokens[i] == NULL) {
            cmd->commands[i] = NULL;
            continue;
        }
        if ((cmd->commands[i] = tokens_to_command(splited_tokens[i])) < 0) {
            perror("structured_sequence_t : Erreur cmd->commands[i]");
            goto cleanup_return;
        }
    }
    cmd->nb_commands = count_splited_tokens;
    cmd->commands[count_splited_tokens] = NULL;

cleanup_return:
    if (splited_tokens != NULL) {
        free_split_tokens(splited_tokens);
    }
    return cmd;
}

structured_if_t *tokens_to_structured_if(char **tokens) {
    LOG_INFO("Tokens -> if");
    structured_if_t *cmd = NULL;

    if (tokens == NULL) {
        goto cleanup_return;
    }

    int start_index_test = -1; // car token[0] == if
    int end_index_test = -1;
    int start_index_commands = -1;
    int end_index_commands = -1;
    char **sub_tokens_test = NULL;
    char **sub_tokens_commands = NULL;
    int count_brackets = 0;

    if (alloc_structured_if(&cmd) < 0)
        goto cleanup_return;

    for (size_t i = 0; tokens[i] != NULL; i++) {
        if (strcmp(tokens[i], "if") != 0 && start_index_test == -1) {
            start_index_test = i;
        } else if (strcmp(tokens[i], "{") == 0) {
            if (end_index_test == -1) {
                end_index_test = i - 1;
            }

            if (count_brackets == 0) {
                start_index_commands = i + 1;
            }
            count_brackets++;
        } else if (strcmp(tokens[i], "}") == 0) {
            count_brackets--;
            if (count_brackets == 0) {
                end_index_commands = i - 1;
            }
        }
    }
    if (start_index_test == -1 || end_index_test == -1 || start_index_commands == -1 ||
        end_index_commands == -1) {
        write_err("tokens_to_structured_if : count_brackets");
        goto cleanup_return;
    }

    if (extract_subtokens(tokens, start_index_test, end_index_test, &sub_tokens_test) <
        0) {
        goto cleanup_return;
    }
    cmd->test = tokens_to_pipeline_command(sub_tokens_test);

    if (extract_subtokens(tokens, start_index_commands, end_index_commands,
                          &sub_tokens_commands) < 0) {
        goto cleanup_return;
    }
    cmd->commands = tokens_to_structured_sequence(sub_tokens_commands);

cleanup_return:

    if (sub_tokens_test != NULL) {
        free_tokens(sub_tokens_test);
    }

    if (sub_tokens_commands != NULL) {
        free_tokens(sub_tokens_commands);
    }
    return cmd;
}

structured_if_else_t *tokens_to_structured_if_else(char **tokens) {
    LOG_INFO("Tokens -> if else");
    structured_if_else_t *cmd = NULL;

    if (tokens == NULL) {
        goto cleanup_return;
    }

    int start_index_test = -1; // car token[0] == if
    int end_index_test = -1;
    int start_index_if_commands = -1;
    int end_index_if_commands = -1;
    int start_index_else_commands = -1;
    int end_index_else_commands = -1;
    char **sub_tokens_test = NULL;
    char **sub_tokens_if_commands = NULL;
    char **sub_tokens_else_commands = NULL;
    int count_brackets = 0;

    if (alloc_structured_if_else(&cmd) < 0)
        goto cleanup_return;

    for (size_t i = 0; tokens[i] != NULL; i++) {
        if (strcmp(tokens[i], "if") == 0 && start_index_test == -1) {
            start_index_test = i + 1;
        } else if (strcmp(tokens[i], "{") == 0) {
            if (end_index_test == -1) {
                end_index_test = i - 1;
            }
            if (count_brackets == 0) {
                if (start_index_if_commands == -1) {
                    start_index_if_commands = i + 1;
                } else if (start_index_else_commands == -1) {
                    start_index_else_commands = i + 1;
                }
            }
            count_brackets++;
        } else if (strcmp(tokens[i], "}") == 0) {
            count_brackets--;
            if (count_brackets == 0) {
                if (end_index_if_commands == -1) {
                    end_index_if_commands = i - 1;
                } else if (end_index_else_commands == -1) {
                    end_index_else_commands = i - 1;
                }
            }
        }
    }
    if (start_index_test == -1 || end_index_test == -1 || start_index_if_commands == -1 ||
        end_index_if_commands == -1 || start_index_else_commands == -1 ||
        end_index_else_commands == -1) {
        write_err("tokens_to_structured_if_else : count_brackets");
        goto cleanup_return;
    }

    if (extract_subtokens(tokens, start_index_test, end_index_test, &sub_tokens_test) <
        0) {
        goto cleanup_return;
    }
    cmd->test = tokens_to_pipeline_command(sub_tokens_test);

    if (extract_subtokens(tokens, start_index_if_commands, end_index_if_commands,
                          &sub_tokens_if_commands) < 0) {
        goto cleanup_return;
    }
    cmd->if_commands = tokens_to_structured_sequence(sub_tokens_if_commands);

    if (extract_subtokens(tokens, start_index_else_commands, end_index_else_commands,
                          &sub_tokens_else_commands) < 0) {
        goto cleanup_return;
    }
    cmd->else_commands = tokens_to_structured_sequence(sub_tokens_else_commands);

cleanup_return:

    if (sub_tokens_test != NULL) {
        free_tokens(sub_tokens_test);
    }
    if (sub_tokens_if_commands != NULL) {
        free_tokens(sub_tokens_if_commands);
    }
    if (sub_tokens_else_commands != NULL) {
        free_tokens(sub_tokens_else_commands);
    }
    return cmd;
}

structured_for_option_t *tokens_to_structured_for_option(char **tokens) {
    LOG_INFO("Tokens -> for option");
    structured_for_option_t *cmd = NULL;

    if (tokens == NULL) {
        goto cleanup_return;
    }

    if (alloc_structured_for_option(&cmd) < 0)
        goto cleanup_return;

    if (tokens[0] != NULL) {
        cmd->option = tokens[0][1]; // car tokens[0][0] == '-'
    }

    if (tokens[1] != NULL) {
        cmd->arg = strdup(tokens[1]);
    }

cleanup_return:
    return cmd;
}

structured_for_t *tokens_to_structured_for(char **tokens) {
    LOG_INFO("Tokens -> for");
    structured_for_t *cmd = NULL;

    if (tokens == NULL) {
        goto cleanup_return;
    }

    int start_index_commands = -1;
    int end_index_commands = -1;
    int count_dash = 0; // tiret des options
    int count_curr_options = 0;
    char **temp_sub_tokens = NULL;
    int count_brackets = 0;
    char **sub_tokens_commands = NULL;

    if (alloc_structured_for(&cmd) < 0)
        goto cleanup_return;

    for (size_t i = 0; tokens[i] != NULL; i++) {
        if (strcmp(tokens[i], "{") == 0) {
            break;
        }
        if (tokens[i][0] == '-') {
            count_dash++;
        }
    }
    cmd->options = malloc((count_dash + 1) * sizeof(char *));
    if (cmd->options == NULL) {
        perror("tokens_to_structured_for : Erreur d'allocation cmd->options");
        goto cleanup_return;
    }

    cmd->var = tokens[1][0];
    cmd->rep = strdup(tokens[3]);
    cmd->nb_options = count_dash;

    for (size_t i = 4; tokens[i] != NULL; i++) {
        if (strcmp(tokens[i], "{") == 0) {
            if (count_brackets == 0) {
                start_index_commands = i + 1;
            }
            count_brackets++;
        } else if (strcmp(tokens[i], "}") == 0) {
            count_brackets--;
            if (count_brackets == 0) {
                end_index_commands = i - 1;
            }
        } else if (start_index_commands == -1 && tokens[i][0] == '-') {

            if (tokens[i + 1][0] != '-' && strcmp(tokens[i + 1], "{") != 0) {
                extract_subtokens(tokens, i, i + 1, &temp_sub_tokens);
                i++;
            } else {
                extract_subtokens(tokens, i, i, &temp_sub_tokens);
            }
            if (temp_sub_tokens == NULL) {
                goto cleanup_return;
            }
            cmd->options[count_curr_options] =
                tokens_to_structured_for_option(temp_sub_tokens);
            count_curr_options++;
            free_tokens(temp_sub_tokens);
            temp_sub_tokens = NULL;
        }
    }

    cmd->options[count_dash] = NULL;
    if (extract_subtokens(tokens, start_index_commands, end_index_commands,
                          &sub_tokens_commands) < 0) {
        goto cleanup_return;
    }
    cmd->commands = tokens_to_structured_sequence(sub_tokens_commands);

cleanup_return:

    if (temp_sub_tokens != NULL) {
        free_tokens(temp_sub_tokens);
    }
    if (sub_tokens_commands != NULL) {
        free_tokens(sub_tokens_commands);
    }

    return cmd;
}

structured_command_t *tokens_to_structured_command_t(char **tokens) {
    LOG_INFO("Tokens -> structured");
    structured_command_t *cmd = NULL;

    if (tokens == NULL) {
        goto cleanup_return;
    }

    if (alloc_structured_command(&cmd) < 0)
        goto cleanup_return;

    // Détermination du type de commande
    cmd->condition = tokens_get_command_condition(tokens);

    switch (cmd->condition) {
    case SEQUENCE:
        LOG_INFO("Detecte tokens : sequence");
        cmd->seq_cond = tokens_to_structured_sequence(tokens);
        break;

    case IF:
        LOG_INFO("Detecte tokens : if");
        cmd->if_cond = tokens_to_structured_if(tokens);
        break;

    case IF_ELSE:
        LOG_INFO("Detecte tokens : if else");
        cmd->if_else_cond = tokens_to_structured_if_else(tokens);
        break;

    case FOR:
        LOG_INFO("Detecte tokens : for");
        cmd->for_cond = tokens_to_structured_for(tokens);
        break;

    default:
        LOG_INFO("Detecte tokens : Vide");
        break;
    }

cleanup_return:
    return cmd;
}

command_t *tokens_to_command(char **tokens) {
    LOG_INFO("Tokens -> command");
    command_t *cmd = NULL;

    if (tokens == NULL) {
        goto cleanup_return;
    }

    if (alloc_command(&cmd) < 0)
        goto cleanup_return;

    // Détermination du type de commande
    cmd->type = tokens_get_command_type(tokens);

    switch (cmd->type) {
    case SIMPLE_COMMAND:
        LOG_INFO("Detecte tokens : simple");
        cmd->simple = tokens_to_simple_command(tokens);
        break;

    case PIPELINE_COMMAND:
        LOG_INFO("Detecte tokens : pipeline");
        cmd->pipeline = tokens_to_pipeline_command(tokens);
        break;

    case STRUCTURED_COMMAND:
        LOG_INFO("Detecte tokens : structured");
        cmd->structured = tokens_to_structured_command_t(tokens);
        break;

    default: // NO TYPE
        LOG_INFO("Detecte tokens : Vide");
        break;
    }

cleanup_return:
    return cmd;
}

int parse_line_to_command(const char *line, command_t **out) {
    LOG_INFO("Pile d'appel lors du parsing :");
    char **tokens = NULL;
    char log_message_buf[100];

    if (line == NULL || line[0] == '\0') {
        LOG_WARNING("parse_line_to_command : Ligne vide");
        goto cleanup_return;
    }

    int valid = 0;

    // Tokenisation de la ligne
    if (tokenize(line, &tokens) < 0) {
        LOG_ERROR("parse_line_to_command : Erreur de tokenisation");
        goto cleanup_return;
    }

    valid = tokens_validator(tokens);
    sprintf(log_message_buf, "VALIDATOR : %d", valid);
    LOG_INFO(log_message_buf);
    if (valid <= 0) {
        if (valid == -1) {
            LOG_INFO("[Mauvais argument]");
        } else {
            LOG_INFO("[Mauvaise syntaxe]");
        }
        goto cleanup_return;
    } else {
        LOG_INFO("[Ligne correcte]");
    }

    *out = tokens_to_command(tokens);

cleanup_return:
    if (tokens != NULL) {
        free_tokens(tokens);
    }
    return valid;
}
