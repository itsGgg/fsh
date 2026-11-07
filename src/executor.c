#define _POSIX_C_SOURCE 200809L
#include "../include/executor.h"
#include "../include/builtin.h"
#include "../include/parser.h"
#include "../include/signal.h"
#include "../include/utils.h"

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#ifdef __linux__
#include <linux/limits.h>
#elif defined(__APPLE__) && defined(__MACH__)
#include <limits.h>
#else
#error "Unsupported platform"
#endif

int alloc_var_map(loop_var_map_t **map) {
    *map = malloc(sizeof(loop_var_map_t));
    if (*map == NULL) {
        return -1;
    }

    (*map)->nb_var = 0;
    (*map)->variables = NULL;
    (*map)->values = NULL;

    return 0;
}

int free_var_map(loop_var_map_t *map) {
    if (map != NULL) {
        free(map->variables);
        if (map->values != NULL) {
            for (int i = 0; map->values[i] != NULL; i++) {
                free(map->values[i]);
            }
            free(map->values);
        }
        free(map);
    }
    return 0;
}

int print_var_map(loop_var_map_t *map) {
    if (map == NULL) {
        return 0;
    }
    for (int i = 0; i < map->nb_var; i++) {
        write_err_no_ln("Variable: ");
        write_err_no_ln((char[]){map->variables[i], '\0'});
        write_err_no_ln(", Value: ");
        write_output(map->values[i]);
    }
    return 0;
}

int add_var(loop_var_map_t *map, char var, char *value) {
    // Réallocation pour ajouter une nouvelle variable
    map->nb_var++;

    map->variables =
        realloc(map->variables,
                (map->nb_var + 1) *
                    sizeof(char)); // Ajout de place pour une nouvelle variable + NULL
    if (map->variables == NULL) {
        return -1;
    }

    map->values =
        realloc(map->values,
                (map->nb_var + 1) *
                    sizeof(char *)); // Ajout de place pour une nouvelle valeur + NULL
    if (map->values == NULL) {
        return -1;
    }

    // Ajout de la variable et de la valeur
    map->variables[map->nb_var - 1] = var;
    map->values[map->nb_var - 1] = strdup(value);

    if (map->values[map->nb_var - 1] == NULL) {
        return -1;
    }

    // Terminer les tableaux par NULL
    map->variables[map->nb_var] = '\0';
    map->values[map->nb_var] = NULL;
    return 0;
}

int dup_var_map(loop_var_map_t *map, loop_var_map_t **map_out) {

    if (alloc_var_map(map_out) != 0) {
        return -1;
    }

    // Copie des variables et des valeurs
    (*map_out)->nb_var = map->nb_var;
    (*map_out)->variables = malloc((map->nb_var + 1) * sizeof(char));
    (*map_out)->values = malloc((map->nb_var + 1) * sizeof(char *));
    if ((*map_out)->variables == NULL || (*map_out)->values == NULL) {
        return -1;
    }

    for (int i = 0; i < map->nb_var; i++) {
        (*map_out)->variables[i] = map->variables[i];
        (*map_out)->values[i] = strdup(map->values[i]);
        if ((*map_out)->values[i] == NULL) {
            return -1;
        }
    }

    // Terminer les tableaux par NULL
    (*map_out)->variables[(*map_out)->nb_var] = '\0';
    (*map_out)->values[(*map_out)->nb_var] = NULL;

    return 0;
}

// Fonction pour remplacer toutes les occurrences de $c
char *replace_var(loop_var_map_t *map, char *arg) {
    size_t arg_len = 0;
    size_t new_len = 0;
    size_t count = 0;
    char *result = NULL;
    size_t result_index = 0;

    if (!arg || !map) {
        return NULL;
    }
    arg_len = strlen(arg);

    // Première phase : Calcul de la longueur nécessaire
    for (size_t i = 0; i < arg_len; i++) {
        if (arg[i] == '$' && i + 1 < arg_len) {
            char var = arg[i + 1];
            for (int j = 0; j < map->nb_var; j++) {
                if (map->variables[j] == var) {
                    new_len += strlen(map->values[j]);
                    count++;
                    i++; // Sauter le caractère suivant (c dans $c)
                    break;
                }
            }
        } else {
            new_len++;
        }
    }

    // Si aucune substitution, retourner une copie de la chaîne originale
    if (count == 0) {
        return strdup(arg);
    }

    // Deuxième phase : Allouer la nouvelle chaîne
    result = malloc(new_len + 1);
    if (!result) {
        perror("malloc");
        return NULL;
    }

    // Troisième phase : Remplissage de la nouvelle chaîne
    for (size_t i = 0; i < arg_len; i++) {
        if (arg[i] == '$' && i + 1 < arg_len) {
            char var = arg[i + 1];
            int found = 0;
            for (int j = 0; j < map->nb_var; j++) {
                if (map->variables[j] == var) {
                    size_t value_len = strlen(map->values[j]);
                    memcpy(result + result_index, map->values[j], value_len);
                    result_index += value_len;
                    i++; // Sauter le caractère suivant (c dans $c)
                    found = 1;
                    break;
                }
            }
            if (!found) {
                result[result_index++] = arg[i];
            }
        } else {
            result[result_index++] = arg[i];
        }
    }

    result[result_index] = '\0';
    return result;
}

// Fonction qui remplace les variables dans un tableau d'arguments
char **replace_var_args(loop_var_map_t *map, char **args) {
    if (args == NULL || map == NULL) {
        return NULL;
    }

    int i = 0;
    // Compter le nombre d'arguments
    while (args[i] != NULL) {
        i++;
    }

    // Allouer de la mémoire pour le nouveau tableau d'arguments
    char **new_args = malloc((i + 1) * sizeof(char *));
    if (new_args == NULL) {
        perror("malloc failed");
        return NULL;
    }

    // Remplacer chaque argument
    for (int j = 0; j < i; j++) {
        new_args[j] = replace_var(map, args[j]);
    }

    // Terminer le tableau par NULL
    new_args[i] = NULL;

    return new_args;
}

int process_for_options(structured_for_t *cmd, int *include_hidden, int *recursive,
                        int *max_parallel, char **ext_filter, char *type_filter) {
    for (int i = 0; i < cmd->nb_options; i++) {
        if (cmd->options[i]->option == 'A') {
            *include_hidden = 1;
        } else if (cmd->options[i]->option == 'r') {
            *recursive = 1;
        } else if (cmd->options[i]->option == 'e') {
            *ext_filter = cmd->options[i]->arg;
        } else if (cmd->options[i]->option == 't') {
            *type_filter = cmd->options[i]->arg[0];
        } else if (cmd->options[i]->option == 'p') {
            *max_parallel = atoi(cmd->options[i]->arg);
        }
    }
    return 0;
}

int process_for_entry(char *full_path, char *ext_filter, char type_filter,
                      struct stat *sb) {
    // Filtrage selon le type de fichier
    if (lstat(full_path, sb) == -1) {
        perror("lstat");
        return -1;
    }
    if (type_filter) {
        if ((type_filter == 'f' && !S_ISREG(sb->st_mode)) ||
            (type_filter == 'd' && !S_ISDIR(sb->st_mode)) ||
            (type_filter == 'l' && !S_ISLNK(sb->st_mode)) ||
            (type_filter == 'p' && !S_ISFIFO(sb->st_mode))) {
            return -1;
        }
    }
    // Filtrage selon l'extension
    if (ext_filter) {
        char *dot = strrchr(full_path, '.');
        if (!dot || strcmp(dot + 1, ext_filter) != 0) {
            return -1;
        }
        *dot = '\0'; // Retirer l'extension
    }
    return 0;
}

int execute_redirection(redirection_t *cmd, loop_var_map_t *map) {
    int fd, target_fd;
    int flags;
    char *resolved_file = replace_var(map, cmd->file);
    if (resolved_file == NULL) {
        perror("replace_var");
        return -1;
    }

    switch (cmd->type) {
    case NO_REDIRECT: // Pas de rédirection
        if (resolved_file != NULL) {
            free(resolved_file);
        }
        return 0;
    case REDIRECT_IN: // <
        // Vérifier que le fichier existe
        if (access(resolved_file, F_OK) != 0) {
            if (resolved_file != NULL) {
                free(resolved_file);
            }
            return 1;
        }

        flags = O_RDONLY;
        target_fd = STDIN_FILENO;
        break;
    case REDIRECT_OUT: // >
        flags = O_CREAT | O_WRONLY | O_EXCL;
        target_fd = STDOUT_FILENO;
        break;
    case REDIRECT_ERR: // 2>
        flags = O_CREAT | O_WRONLY | O_EXCL;
        target_fd = STDERR_FILENO;
        break;
    case REDIRECT_OUT_APPEND: // >>
        flags = O_CREAT | O_APPEND | O_WRONLY;
        target_fd = STDOUT_FILENO;
        break;
    case REDIRECT_ERR_APPEND: // 2>>
        flags = O_CREAT | O_APPEND | O_WRONLY;
        target_fd = STDERR_FILENO;
        break;
    case REDIRECT_OUT_OVERWRITE: // >|
        flags = O_CREAT | O_TRUNC | O_WRONLY;
        target_fd = STDOUT_FILENO;
        break;
    case REDIRECT_ERR_OVERWRITE: // 2>|
        flags = O_CREAT | O_TRUNC | O_WRONLY;
        target_fd = STDERR_FILENO;
        break;
    }

    // Vérifier que le fichier n'existe pas
    if (cmd->type == REDIRECT_OUT || cmd->type == REDIRECT_ERR) {
        if (access(resolved_file, F_OK) == 0) {
            perror("file exists");
            if (resolved_file != NULL) {
                free(resolved_file);
            }
            return 1;
        }
    }

    // Ouvrir le fichier (avec ou sans mode selon les flags)
    if (flags & O_CREAT) {
        fd = open(resolved_file, flags, 0666);
    } else {
        fd = open(resolved_file, flags);
    }

    if (resolved_file != NULL) {
        free(resolved_file); // Libérer resolved_file
    }

    if (fd == -1) {
        perror("open");
        return -1;
    }

    if (dup2(fd, target_fd) == -1) {
        perror("dup2");
        close(fd);
        return -1;
    }

    // Fermer fd
    if (close(fd) == -1) {
        perror("close");
        return -1;
    }
    return 0;
}

int execute_simple_command(simple_command_t *cmd, loop_var_map_t *map) {

    if (cmd == NULL) {
        return 0;
    }
    char **renamed_args = NULL;
    int status;
    int save_fd[3];

    renamed_args = replace_var_args(map, cmd->args);

    // Sauvegarder descripteurs
    save_fd[0] = dup(STDIN_FILENO);
    save_fd[1] = dup(STDOUT_FILENO);
    save_fd[2] = dup(STDERR_FILENO);

    for (ssize_t i = 0; i < cmd->nb_redir; i++) {
        if (execute_redirection(cmd->redir[i], map) == 1) {
            perror("execute_redirection ");
            status = 1;
            goto cleanup_return;
        }
    }

    status = execute_builtin(cmd->argc, renamed_args);

    if (status < 0) {
        status = execute_external(cmd->argc, renamed_args);
    }

cleanup_return:
    if (renamed_args != NULL) {
        for (int i = 0; renamed_args[i] != NULL; i++) {
            free(renamed_args[i]);
        }
        free(renamed_args);
    }

    // Restaurer les descripteur origine
    if ((dup2(save_fd[0], STDIN_FILENO) == -1) || dup2(save_fd[1], STDOUT_FILENO) == -1 ||
        dup2(save_fd[2], STDERR_FILENO) == -1) {
        perror("dup2: Erreur de restauration des descripteurs d'origine.");
        status = -1;
    }
    if (close(save_fd[0]) == -1 || close(save_fd[1]) == -1 || close(save_fd[2]) == -1) {
        perror("close : Erreur de fermetures des fd save");
        status = -1;
    }
    return status;
}

int execute_pipeline_command(pipeline_command_t *cmd, loop_var_map_t *map) {
    if (!cmd || cmd->nb_commands <= 0 || !cmd->args) {
        return 0;
    }
    if (cmd->nb_commands == 1) {
        // Ne devrait pas arriver que rarement
        return execute_simple_command(cmd->args[0], map);
    }

    int pipe_fds[2];  // Tube pour connecter les commandes
    int prev_fd = -1; // Pour stocker le descripteur de fichier de la sortie précédente
    int status = 0;
    pid_t pid;

    for (int i = 0; i < cmd->nb_commands; i++) {

        // Créer un tube pour toutes les commandes sauf la dernière
        if (i < cmd->nb_commands - 1) {
            if (pipe(pipe_fds) < 0) {
                perror("pipe");
                return -1;
            }
        }

        pid = fork();
        if (pid < 0) {
            perror("fork");
            return -1;
        }

        if (pid == 0) { // Processus fils
            // Redirection de l'entrée standard pour les commandes suivantes
            if (prev_fd != -1) {
                if (dup2(prev_fd, STDIN_FILENO) < 0) {
                    perror("dup2 prev_fd");
                    exit(1);
                }
                close(prev_fd);
            }

            // Redirection de la sortie standard pour toutes les commandes sauf la
            // dernière
            if (i < cmd->nb_commands - 1) {
                close(pipe_fds[0]); // Fermer la lecture du tube
                if (dup2(pipe_fds[1], STDOUT_FILENO) < 0) {
                    perror("dup2 pipe_fds[1]");
                    exit(1);
                }
                close(pipe_fds[1]);
            }

            // Exécuter la commande simple
            status = execute_simple_command(cmd->args[i], map);
            exit(status);
        } else { // Processus père
            // Fermer les descripteurs inutilisés
            if (prev_fd != -1) {
                close(prev_fd);
            }
            if (i < cmd->nb_commands - 1) {
                close(pipe_fds[1]);    // Fermer la sortie du tube
                prev_fd = pipe_fds[0]; // Garder l'entrée pour la prochaine commande
            }
        }
    }

    // Attendre que tous les enfants terminent
    for (int i = 0; i < cmd->nb_commands; i++) {
        waitpid(-1, &status, 0);
    }

    return WEXITSTATUS(status); // Retourner le statut de la dernière commande
}

int execute_structured_sequence(structured_sequence_t *cmd, loop_var_map_t *map) {
    if (cmd == NULL || cmd->nb_commands == 0) {
        return 0;
    }

    int status = 0;
    for (int i = 0; i < cmd->nb_commands; i++) {
        status = execute_command(cmd->commands[i], map);

        if (status == EXIT_SIGTERM) {
            continue; // Comme ne rien mettre
        }
        if (status == EXIT_SIGINT) {
            return EXIT_SIGINT;
        }
        interrupted = 0;
    }
    return status;
}

int execute_structured_if(structured_if_t *cmd, loop_var_map_t *map) {

    int status = 0;
    if (cmd != NULL) {
        status = execute_pipeline_command(cmd->test, map);
        if (status == EXIT_SIGINT) {
            return EXIT_SIGINT;
        }
        if (status == 0) {
            status = execute_structured_sequence(cmd->commands, map);
        } else {
            status = 0;
        }
    }
    return status;
}

int execute_structured_if_else(structured_if_else_t *cmd, loop_var_map_t *map) {

    int status = 0;
    if (cmd != NULL) {
        status = execute_pipeline_command(cmd->test, map);
        if (status == EXIT_SIGINT) {
            return EXIT_SIGINT;
        }
        if (status == 0) {
            status = execute_structured_sequence(cmd->if_commands, map);
        } else {
            status = execute_structured_sequence(cmd->else_commands, map);
        }
    }
    return status;
}

int execute_structured_for(structured_for_t *cmd, loop_var_map_t *map) {
    DIR *dir = NULL;
    loop_var_map_t *temp_map = NULL;
    int max_return = 0, temp_return = 0, wstatus = 0;
    char *replaced_rep = NULL;
    struct dirent *entry = NULL;
    int include_hidden = 0, recursive = 0, max_parallel = 1, active_processes = 0;
    char *ext_filter = NULL;
    char type_filter = 0;
    pid_t pid = -1;

    // Validation du répertoire
    if (cmd != NULL) {
        // Traitement des options
        process_for_options(cmd, &include_hidden, &recursive, &max_parallel, &ext_filter,
                            &type_filter);

        replaced_rep = replace_var(map, cmd->rep);
        dir = opendir(replaced_rep);
        if (dir == NULL) {
            max_return = 1;
            goto cleanup_return;
        }

        while ((entry = readdir(dir)) != NULL) {

            // Ignorer fichiers cachés et entrées . et ..
            if ((!include_hidden && entry->d_name[0] == '.') ||
                strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            if (active_processes == max_parallel) {
                wait(&wstatus);
                max_return = MAX(max_return, WEXITSTATUS(wstatus));
                active_processes--;
                if (interrupted) {
                    break;
                }
                if (WEXITSTATUS(wstatus) == EXIT_SIGINT) {
                    interrupted = 1;
                    break;
                }
            }

            pid = fork();
            if (pid < 0) {
                perror("fork");
                max_return = -1;
                break;
            } else if (pid == 0) {
                char full_path[PATH_MAX];
                snprintf(full_path, sizeof(full_path), "%s/%s", replaced_rep,
                         entry->d_name);

                struct stat stat_buf;

                if (process_for_entry(full_path, ext_filter, type_filter, &stat_buf) ==
                    0) {
                    if (dup_var_map(map, &temp_map) < 0 ||
                        add_var(temp_map, cmd->var, full_path) < 0) {
                        exit(-1);
                    }

                    temp_return = execute_structured_sequence(cmd->commands, temp_map);
                    free_var_map(temp_map);
                    temp_map = NULL;
                    if (temp_return < 0) {
                        exit(-1);
                    }
                    if (temp_return == EXIT_SIGINT) {
                        exit(EXIT_SIGINT);
                    }
                    max_return = MAX(max_return, temp_return);
                }

                // Récursion si option -r et est dossier
                if (recursive && S_ISDIR(stat_buf.st_mode)) {
                    structured_for_t *dup_cmd = NULL;
                    dup_structured_for(cmd, &dup_cmd);
                    if (dup_cmd->rep != NULL) {
                        free(dup_cmd->rep);
                    }
                    dup_cmd->rep = strdup(full_path);
                    temp_return = execute_structured_for(dup_cmd, map);
                    free_structured_for(dup_cmd);
                    max_return = MAX(max_return, temp_return);
                }
                exit(max_return);
            } else {
                active_processes++;
            }
        }

        while (active_processes > 0) {
            wait(&wstatus);
            max_return = MAX(max_return, WEXITSTATUS(wstatus));
            active_processes--;
        }
    }

cleanup_return:
    if (dir != NULL)
        closedir(dir);
    if (temp_map != NULL)
        free_var_map(temp_map);
    if (replaced_rep != NULL)
        free(replaced_rep);
    if (interrupted) {
        return EXIT_SIGINT;
    }
    return max_return;
}

int execute_structured_command(structured_command_t *cmd, loop_var_map_t *map) {
    if (cmd == NULL) {
        return 0;
    }

    switch (cmd->condition) {
    case SEQUENCE:
        return execute_structured_sequence(cmd->seq_cond, map);

    case IF:
        return execute_structured_if(cmd->if_cond, map);

    case IF_ELSE:
        return execute_structured_if_else(cmd->if_else_cond, map);

    case FOR:
        return execute_structured_for(cmd->for_cond, map);

    default:
        return 0;
    }
}

int execute_command(command_t *cmd, loop_var_map_t *map) {
    if (cmd == NULL) {
        return 0;
    }

    switch (cmd->type) {
    case SIMPLE_COMMAND:
        return execute_simple_command(cmd->simple, map);

    case PIPELINE_COMMAND:
        return execute_pipeline_command(cmd->pipeline, map);

    case STRUCTURED_COMMAND:
        return execute_structured_command(cmd->structured, map);

    default:
        return 0;
    }
}
