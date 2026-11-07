#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stdlib.h>

// ------------------------------ Fonctions de utils ------------------------------ //
int write_output(const char *message);
int write_output_no_ln(const char *message);
int write_err(const char *message);
int write_err_no_ln(const char *message);

// ------------------------------ Fonctions de debug ------------------------------ //

int test_temp();

#endif