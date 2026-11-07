#include "../include/validator.h"
#include "../include/debug.h"
#include "../include/parser.h"
#include "../include/utils.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int is_valid_extension(char *token) {
    if (!token || token[0] == '\0')
        return 0;

    for (int i = 0; token[i] != '\0'; i++)
        if (!isalnum(token[i]))
            return 0;

    return 1;
}

int is_valid_syntax(char **tokens) {
    int prev_is_pipe = 0;
    int brace_count = 0;
    int i = 0;
    if (tokens[0] && strcmp(tokens[0], "|") == 0)
        return -2;

    while (tokens[i] != NULL) {
        if (!tokens[i])
            break;
        if (strcmp(tokens[i], "{") == 0) {
            brace_count++;
        } else if (strcmp(tokens[i], "}") == 0) {
            brace_count--;
            if (brace_count < 0)
                return -2;
        }

        if (strcmp(tokens[i], "|") == 0) {
            if (prev_is_pipe)
                return -2;
            prev_is_pipe = 1;
        } else
            prev_is_pipe = 0;
        i++;
    }

    if (brace_count != 0)
        return -2;

    return 1;
}

// Renvoie un -1 en cas d'échec.
// Chaque tableau contient une suite de tokens qui étaient contenu entre des ;
// CMD_1 ; CMD_N ->  [[token_1],[token_2]] , [[token_1],[token_2],[token_3]]
int extract_token_between_semicolon(char **tokens, char ****cmd) {
    if (!tokens || !cmd)
        return -1;

    size_t token_count = 0;
    while (tokens[token_count] != NULL) {
        token_count++;
    }

    char ***temp_result = malloc(sizeof(char **) * (token_count + 2));
    if (!temp_result) {
        perror("extract_token_between_semicolon : Erreur d'allocation mémoire");
        return -1;
    }

    int brace_count = 0;
    int res_index = 0;
    int res_index2 = 0;

    temp_result[res_index] = malloc(sizeof(char *) * (token_count + 1));
    if (!temp_result[res_index]) {
        perror("extract_token_between_semicolon : Erreur d'allocation mémoire");
        if (temp_result != NULL)
            free(temp_result);
        return -1;
    }

    for (int i = 0; tokens[i] != NULL; i++) {
        char *token = tokens[i];

        if (strcmp(token, "{") == 0)
            brace_count++;
        else if (strcmp(token, "}") == 0)
            brace_count--;

        if (strcmp(token, ";") == 0 && brace_count == 0) {
            temp_result[res_index][res_index2] = NULL;
            res_index++;
            res_index2 = 0;

            temp_result[res_index] = malloc(sizeof(char *) * (token_count + 1));
            if (!temp_result[res_index]) {
                perror("extract_token_between_semicolon : Erreur d'allocation mémoire");
                for (int j = 0; j < res_index; j++)
                    if (temp_result[j] != NULL)
                        free(temp_result[j]);
                if (temp_result != NULL)
                    free(temp_result);
                return -1;
            }
        } else {
            temp_result[res_index][res_index2] = token;
            res_index2++;
        }
    }

    if (res_index2 > 0) {
        temp_result[res_index][res_index2] = NULL;
        res_index++;
    } else if (res_index2 == 0 && res_index > 0)
        if (temp_result[res_index] != NULL)
            free(temp_result[res_index]);

    temp_result[res_index] = NULL;
    *cmd = temp_result;
    return 0;
}

int print_extracted_token_between_semicolon(char ***tokens) {
    if (!tokens) {
        write_err("Tableau vide");
        return -1;
    }

    for (int i = 0; tokens[i] != NULL; i++) {
        printf("T%d : ", i);
        if (tokens[i][0] == NULL) {
            printf("EMPTY");
        } else {
            for (int j = 0; tokens[i][j] != NULL; j++) {
                printf("%s ", tokens[i][j]);
            }
        }
        printf("\n");
    }

    return 0;
}

int is_valid_content(char ***tokens) {
    int is_for_if = 0;
    int result = 0;
    if (!tokens || !(*tokens) || !tokens[0][0])
        return 1;

    for (int i = 0; tokens[i] != NULL; i++) {
        if (tokens[i][0] == NULL)
            continue;

        if (tokens[i][0] && is_redirection(tokens[i][0]))
            return -2;

        if (strcmp(tokens[i][0], "for") == 0 || strcmp(tokens[i][0], "if") == 0) {
            is_for_if = 1;
        }

        int have_semicolon = 0;
        for (int j = 0; tokens[i][j] != NULL; j++) {
            if (strcmp(tokens[i][j], ";") == 0) {
                have_semicolon = 1;
                break;
            }
            if ((strcmp(tokens[i][j], "{") == 0) && !is_for_if)
                return -2;
        }

        if (have_semicolon && !is_for_if) {
            char ***sub_tokens = NULL;
            // printf("extract at %d\n", i);
            result = extract_token_between_semicolon(tokens[i], &sub_tokens);
            if (result != 0)
                return -1;

            // print_extracted_token_between_semicolon(sub_tokens);
            result = is_valid_content(sub_tokens);

            for (int k = 0; sub_tokens[k] != NULL; k++) {
                if (sub_tokens[k] != NULL)
                    free(sub_tokens[k]);
            }
            if (sub_tokens != NULL)
                free(sub_tokens);

            if (result != 1)
                return result;
        } else {
            // verification
            if (strcmp(tokens[i][0], "for") == 0) {
                // printf("checking for at T%d\n", i);
                result = is_valid_for(tokens[i]);
            } else if (strcmp(tokens[i][0], "if") == 0) {
                // printf("checking if at T%d\n", i);
                result = is_valid_if(tokens[i]);
            }
            if (result < 0) {
                if (result == -1) {
                    LOG_INFO("Option non reconnue\n");
                }
                if (result == -2) {
                    LOG_INFO("Syntaxe invalide\n");
                }
                return result;
            }

            for (int j = 0; tokens[i][j] != NULL; j++) {
                if (is_redirection(tokens[i][j])) {
                    if (!tokens[i][j + 1] || is_redirection(tokens[i][j + 1])) {
                        return -2;
                    }
                    char first = tokens[i][j + 1][0];
                    if (!isalnum(first) && first != '/' && first != '.' && first != '-' &&
                        first != '_' && first != ',' && first != '*' && first != '$') {
                        return -2;
                    }
                }
                if (tokens[i][j] && strcmp(tokens[i][j], "|") == 0 && !tokens[i][j + 1]) {
                    return -2;
                }
            }
        }
    }
    return 1;
}

int is_number(char *token) {
    if (!token)
        return 0;

    for (int i = 0; token[i] != '\0'; i++)
        if (!isdigit(token[i]))
            return 0;

    return 1;
}

int is_valid_for(char **tokens) {
    if (!tokens || !tokens[0])
        return 0;

    char *token = tokens[0];
    if (strcmp(token, "for") != 0)
        return -2;

    token = tokens[1];
    if (!token || strlen(token) != 1 || !isalpha(token[0]))
        return -2;

    token = tokens[2];
    if (!token || strcmp(token, "in") != 0)
        return -2;

    token = tokens[3];
    if (!token || strlen(token) == 0)
        return -2;

    int current_pos = 4;

    int option_p = 0;
    token = tokens[current_pos];
    while (token && strcmp(token, "{") != 0) {
        if (token && token[0] == '-' && token[1] != '\0') {
            if (token[1] == 'A' || token[1] == 'r') {
                current_pos++;
                token = tokens[current_pos];
                if (token && token[0] != '-' && strcmp(token, "{") != 0) {
                    return -1;
                }
            } else if (token[1] == 't') {
                current_pos++;
                token = tokens[current_pos];
                if (token && (token[0] == 'f' || token[0] == 'd' || token[0] == 'l' ||
                              token[0] == 'p')) {
                    current_pos++;
                } else {
                    return -1;
                }
            } else if (token[1] == 'p') {
                if (option_p)
                    return -1;
                current_pos++;
                token = tokens[current_pos];
                if (is_number(token)) {
                    current_pos++;
                } else {
                    return -1;
                }
                option_p = 1;
            } else if (token[1] == 'e') {
                current_pos++;
                token = tokens[current_pos];
                if (token && (is_valid_extension(token)))
                    current_pos++;
                else
                    return -1;
            } else
                return -1;
        } else if (token && token[0] == '-' && token[1] == '\0') {
            return -1;
        } else
            return -2;
        token = tokens[current_pos];
    }

    if (!token || strcmp(token, "{") != 0)
        return -2;

    int i = current_pos + 1;
    int start = i;
    int brace_count = 1;
    while (tokens[i] && brace_count > 0) {
        i++;
        if (tokens[i] && strcmp(tokens[i], "{") == 0)
            brace_count++;
        else if (tokens[i] && strcmp(tokens[i], "}") == 0)
            brace_count--;
    }

    if (!tokens[i] || strcmp(tokens[i], "}"))
        return 1;

    int end = i - 1;

    char **token_cmd;
    if (!extract_subtokens(tokens, start, end, &token_cmd))
        return 0;

    int result = 0;
    char ***sub_tokens = NULL;
    if (extract_token_between_semicolon(token_cmd, &sub_tokens) != 0)
        return -1;

    result = is_valid_content(sub_tokens);

    for (int k = 0; sub_tokens[k] != NULL; k++)
        if (sub_tokens[k] != NULL)
            free(sub_tokens[k]);
    if (sub_tokens != NULL)
        free(sub_tokens);
    if (token_cmd != NULL)
        free_tokens(token_cmd);

    if (result != 1)
        return result;

    return 1;
}

int is_valid_if(char **tokens) {
    if (!tokens || !tokens[0] || !tokens[1] || !tokens[2] || !tokens[3])
        return -2;

    char *token = tokens[0];
    if (strcmp(token, "if") != 0)
        return -2;

    token = tokens[1];
    if (token && strcmp(token, "{") == 0)
        return -2;

    int i = 1;
    int cond_count = 0;
    while (tokens[i] && strcmp(tokens[i], "{") != 0) {
        cond_count++;
        i++;
    }

    if (!tokens[i] || strcmp(tokens[i], "{"))
        return -2;

    char **token_cond;
    if (!extract_subtokens(tokens, 1, cond_count, &token_cond))
        return 0;

    if (is_valid_pipeline(token_cond) < 0)
        return -2;

    free_tokens(token_cond);

    i++;
    int cmd_count = 0;
    int brace_count = 1;
    while (tokens[i] && brace_count > 0) {
        cmd_count++;
        i++;
        if (tokens[i] && strcmp(tokens[i], "{") == 0)
            brace_count++;
        else if (tokens[i] && strcmp(tokens[i], "}") == 0)
            brace_count--;
    }

    if (!tokens[i] || strcmp(tokens[i], "}"))
        return 1;

    char **token_if;
    if (!extract_subtokens(tokens, 1 + 1 + cond_count, 1 + cond_count + cmd_count,
                           &token_if))
        return 0;

    int result = 0;
    char ***sub_tokens = NULL;
    if (extract_token_between_semicolon(token_if, &sub_tokens) != 0)
        return -1;

    result = is_valid_content(sub_tokens);

    for (int k = 0; sub_tokens[k] != NULL; k++)
        if (sub_tokens[k] != NULL)
            free(sub_tokens[k]);
    if (sub_tokens != NULL)
        free(sub_tokens);
    if (token_if != NULL)
        free_tokens(token_if);

    if (result != 1)
        return result;

    int else_count = 0;
    if (tokens[i + 1] && strcmp(tokens[i + 1], "else") == 0) {
        i += 2;

        if (!tokens[i] || strcmp(tokens[i], "{") != 0)
            return -2;

        i++;
        int else_start = i;
        brace_count = 1;
        while (tokens[i] && brace_count > 0) {
            else_count++;
            i++;
            if (tokens[i] && strcmp(tokens[i], "{") == 0)
                brace_count++;
            else if (tokens[i] && strcmp(tokens[i], "}") == 0)
                brace_count--;
        }

        if (!tokens[i] || strcmp(tokens[i], "}") != 0)
            return 1;

        char **token_else;
        if (!extract_subtokens(tokens, else_start, else_start + else_count - 1,
                               &token_else))
            return 0;

        sub_tokens = NULL;
        if (extract_token_between_semicolon(token_else, &sub_tokens) != 0)
            return -1;

        if (tokens[i + 1] && (strcmp(tokens[i + 1], ";") || strcmp(tokens[i + 1], "}")))
            return -2;

        result = is_valid_content(sub_tokens);

        for (int k = 0; sub_tokens[k] != NULL; k++)
            if (sub_tokens[k] != NULL)
                free(sub_tokens[k]);
        if (sub_tokens != NULL)
            free(sub_tokens);
        if (token_else != NULL)
            free_tokens(token_else);

        if (result != 1)
            return result;
    } else if (tokens[i + 1] &&
               (strcmp(tokens[i + 1], ";") || strcmp(tokens[i + 1], "else")))
        return -2;
    return 1;
}

int is_simple_command(char *token) {
    if (!token || (token[0] == '\0') ||
        (!isalpha(token[0]) && token[0] != '_' && token[0] != '[' && token[0] != ']'))
        return 0;

    for (int i = 1; token[i] != '\0'; i++) {
        if (!isalpha(token[i]) && token[i] != '_' && token[i] != '[' && token[i] != ']')
            return 0;
    }

    return 1;
}

int is_valid_pipeline(char **tokens) {
    int i = 0;
    int expecting_cmd = 1;
    int expecting_file = 0;

    while (tokens[i]) {
        if (strcmp(tokens[i], ";") == 0)
            return -2;

        if (strcmp(tokens[i], "for") == 0)
            return -2;

        if (strcmp(tokens[i], "if") == 0)
            return -2;

        if (strcmp(tokens[i], "|") == 0) {
            if (expecting_cmd || !tokens[i + 1])
                return -2;
            expecting_cmd = 1;
        } else if (is_redirection(tokens[i])) {
            if (expecting_file || !tokens[i + 1])
                return -2;
            expecting_file = 1;
        } else if (expecting_file) {
            if (!is_simple_command(tokens[i]))
                return -2;
            expecting_file = 0;
        } else {
            if (expecting_cmd) {
                if (!is_simple_command(tokens[i]))
                    return -2;
                expecting_cmd = 0;
            }
        }
        i++;
    }

    if (expecting_cmd || expecting_file)
        return -2;

    return 1;
}

int tokens_validator(char **tokens)
// -2 si mauvaise syntaxe
// -1 si mauvaise option
// 1 si ligne correct
// 0 si erreur du validator
{
    int result = 0;
    result = is_valid_syntax(tokens);
    if (result <= 0)
        return -2;

    char ***cmd = NULL;
    if (extract_token_between_semicolon(tokens, &cmd) < 0)
        return 0;

    result = is_valid_content(cmd);
    if (result <= 0)
        return result;

    for (int i = 0; cmd[i] != NULL; i++) {
        if (cmd[i] != NULL)
            free(cmd[i]);
    }
    if (cmd != NULL)
        free(cmd);

    result = 1;
    return result;
}