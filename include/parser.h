#ifndef PARSER_H
#define PARSER_H

#include "command.h"
#include "utils.h"

#include <stdbool.h>
#include <stdlib.h>

// ------------------------------ Utils tokens ------------------------------ //

int tokenize(const char *line, char ***tokens_out);
int extract_subtokens(char **tokens, int start, int end, char ***subtokens);
int tokens_validator(char **tokens);
int free_tokens(char **tokens);
int print_tokens(char **tokens);

int split_tokens(char **tokens, const char *delimiter, char ****result);
int free_split_tokens(char ***split_tokens);
int print_split_tokens(char ***split_tokens);

// ------------------------------ Convertisseur utils ------------------------------ //

redirection_value_t tokens_get_redirection_value(char **tokens);
command_condition_t tokens_get_command_condition(char **tokens);
command_type_t tokens_get_command_type(char **tokens);

redirection_t *tokens_to_redirection(char **tokens);
simple_command_t *tokens_to_simple_command(char **tokens);
pipeline_command_t *tokens_to_pipeline_command(char **tokens);
structured_sequence_t *tokens_to_structured_sequence(char **tokens);
structured_if_t *tokens_to_structured_if(char **tokens);
structured_if_else_t *tokens_to_structured_if_else(char **tokens);
structured_for_option_t *tokens_to_structured_for_option(char **tokens);
structured_for_t *tokens_to_structured_for(char **tokens);
structured_command_t *tokens_to_structured_command_t(char **tokens);
command_t *tokens_to_command(char **tokens);

// ------------------------------ Convertisseur ------------------------------ //

int parse_line_to_command(const char *line, command_t **out);

#endif