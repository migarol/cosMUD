/* Quick password generator using MUD's sha256_crypt */
#include <stdio.h>
#include <string.h>
#include "sha256.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <password>\n", argv[0]);
        return 1;
    }

    if (strlen(argv[1]) < 5) {
        printf("Password must be at least 5 characters long.\n");
        return 1;
    }

    char *encrypted = sha256_crypt(argv[1]);
    printf("%s\n", encrypted);

    return 0;
}
