#include "../include/builtin.h"
#include "../include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef __linux__
#include <linux/limits.h>
#elif defined(__APPLE__) && defined(__MACH__)
#include <limits.h>
#else
#error "Unsupported platform"
#endif

#define MAX_PROMPT_LENGHT 30
#define RETURN_COLOR_SUCCESS "\033[32m"
#define RETURN_COLOR_FAILURE "\033[91m"
#define PATH_COLOR "\033[36m"
#define RESET_COLOR "\033[00m"

char *get_prompt(int last_status, int interrupted) {
    char cwd[PATH_MAX];
    if (getcwd(cwd, PATH_MAX) == NULL) {
        perror("getcwd");
        return NULL;
    }

    char status[12];
    if (last_status == 255 && interrupted)
        snprintf(status, sizeof(status), "SIG");
    else
        snprintf(status, sizeof(status), "%d", last_status);

    char cut_cwd[PATH_MAX];
    if (strlen(cwd) >
        MAX_PROMPT_LENGHT - strlen(status) - 7) // -7 : crochets(2) + "..." + "$ "
        snprintf(cut_cwd, sizeof(cut_cwd), "...%s",
                 cwd + strlen(cwd) - (MAX_PROMPT_LENGHT - strlen(status) - 7));
    else
        snprintf(cut_cwd, sizeof(cut_cwd), "%s", cwd);

    size_t prompt_lenght = strlen(RETURN_COLOR_SUCCESS) + strlen(RETURN_COLOR_FAILURE) +
                           strlen(PATH_COLOR) + strlen(RESET_COLOR) + strlen(status) +
                           strlen(cut_cwd) + 10;
    // 10 = crochet(2) + "& " + "\0" + marge pour éviter dépassement(5)

    char *prompt = malloc(prompt_lenght);
    if (prompt == NULL) {
        perror("malloc");
        return NULL;
    }

    snprintf(prompt, prompt_lenght, "%s[%s]%s%s%s$ ",
             last_status == 0 ? RETURN_COLOR_SUCCESS : RETURN_COLOR_FAILURE, status,
             PATH_COLOR, cut_cwd, RESET_COLOR);

    return prompt;
}