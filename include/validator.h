#ifndef VALIDATOR_H
#define VALIDATOR_H

int is_valid_extension(char *token);
int is_valid_syntax(char **tokens);
int extract_token_between_semicolon(char **tokens, char ****cmd);
int print_extracted_token_between_semicolon(char ***tokens);
int is_valid_content(char ***tokens);
int is_number(char *token);
int is_valid_for(char **tokens);
int is_valid_if(char **tokens);
int is_simple_command(char *token);
int is_valid_pipeline(char **tokens);
int tokens_validator(char **tokens);

#endif