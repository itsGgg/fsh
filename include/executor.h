#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "command.h"
#include "external.h"
#include "redirection.h"

#include <sys/stat.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct loop_var_map_t loop_var_map_t;

struct loop_var_map_t {
    int nb_var;
    char *variables;
    char **values;
};

int alloc_var_map(loop_var_map_t **map);
int free_var_map(loop_var_map_t *map);
int print_var_map(loop_var_map_t *map);
int add_var(loop_var_map_t *map, char var, char *value);
int dup_var_map(loop_var_map_t *map, loop_var_map_t **map_out);
char *replace_var(loop_var_map_t *map, char *arg);
char **replace_var_args(loop_var_map_t *map, char **args);
int process_for_options(structured_for_t *cmd, int *include_hidden, int *recursive,
                        int *max_parallel, char **ext_filter, char *type_filter);
int process_for_entry(char *full_path, char *ext_filter, char type_filter,
                      struct stat *sb);
int handle_for_parallel_execution(loop_var_map_t *map, int *active_processes,
                                  int max_parallel, structured_sequence_t *commands);

int execute_redirection(redirection_t *cmd, loop_var_map_t *map);
int execute_simple_command(simple_command_t *cmd, loop_var_map_t *map);
int execute_pipeline_command(pipeline_command_t *cmd, loop_var_map_t *map);
int execute_structured_sequence(structured_sequence_t *cmd, loop_var_map_t *map);
int execute_structured_if(structured_if_t *cmd, loop_var_map_t *map);
int execute_structured_if_else(structured_if_else_t *cmd, loop_var_map_t *map);
int execute_structured_for(structured_for_t *cmd, loop_var_map_t *map);
int execute_structured_command(structured_command_t *cmd, loop_var_map_t *map);
int execute_command(command_t *cmd, loop_var_map_t *map);

#endif