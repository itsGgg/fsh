#include "../include/builtin.h"
#include "../include/command.h"
#include "../include/debug.h"
#include "../include/executor.h"
#include "../include/parser.h"
#include "../include/prompt.h"
#include "../include/signal.h"
#include "../include/utils.h"

#include <readline/history.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    char *line;
    char *prompt;
    command_t *command = NULL;
    rl_outstream = stderr;
    loop_var_map_t *var_map;
    int valid = 0;

    setup_signals();

    while (1) {

        if (interrupted) {
            last_status = 255;
        }
        prompt = get_prompt(last_status, interrupted);
        interrupted = 0;

        line = readline(prompt);
        if (!line) {
            // Tableau d'args contenant la commande "exit"
            char *args[] = {"exit", NULL};
            builtin_exit(1, args);
            break;
        }
        add_history(line);

        valid = parse_line_to_command(line, &command);
        if (valid == 1) {
            if (command != NULL) {

                DEBUG_CALL(print_command(command));
                DEBUG_CALL(write_output(""));

                alloc_var_map(&var_map);
                last_status = execute_command(command, var_map);
                free_var_map(var_map);

                if (free_command(command) < 0) {
                    LOG_ERROR("main : ERREUR FREE");
                    last_status = 1;
                }
                command = NULL;
            }
        } else {
            if (valid == -2 || valid == -1) {
                last_status = valid * (-1);
            }
        }
        free(line);
        free(prompt);
    }

    return 0;
}