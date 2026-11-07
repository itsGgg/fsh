#include "../include/builtin.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {

    // Fichier régulier
    int regular_fd = open("test_file.txt", O_CREAT | O_WRONLY, 0644);
    if (regular_fd < 0) {
        perror("Erreur création fichier régulier");
        return 1;
    }
    close(regular_fd);

    // Répertoire
    if (mkdir("test_directory", 0755) < 0) {
        perror("Erreur création répertoire");
        return 1;
    }

    // Lien symbolique
    if (symlink("test_file.txt", "test_symlink") < 0) {
        perror("Erreur création lien symbolique");
        return 1;
    }

    // Tube nommé
    if (mkfifo("test_fifo", 0644) < 0) {
        perror("Erreur création FIFO");
        return 1;
    }

    // Tests
    printf("Test du fichier régulier:\n");
    builtin_ftype(2, (char *[]){"ftype", "test_file.txt"});

    printf("\nTest du répertoire:\n");
    builtin_ftype(2, (char *[]){"ftype", "test_directory"});

    printf("\nTest du lien symbolique:\n");
    builtin_ftype(2, (char *[]){"ftype", "test_symlink"});

    printf("\nTest du tube nommé:\n");
    builtin_ftype(2, (char *[]){"ftype", "test_fifo"});

    // Nettoyage après tests
    unlink("test_file.txt");
    unlink("test_symlink");
    unlink("test_fifo");
    rmdir("test_directory");

    return 0;
}
