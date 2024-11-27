#include "database.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    if (createdb() != 0) {
        return -1;
    }

    int choose = 99;
    int key = 0;
    char tmp;
    char str_value[30];
    while (choose != 0) {
        system("clear");
        printf("Сhoose the command:\n");
        printf("1 - Add key\n");
        printf("2 - Remove key\n");
        printf("3 - Search key\n");
        printf("0 - Exit\n");


        scanf("%d", &choose);
        switch (choose) {
            case 1:
                printf("Enter key(int): ");
                scanf("%d", &key);

                printf("Enter value: ");
                scanf("%s", str_value);
                if (add(key, str_value) == 0) {
                    printf("Key added successfully!\n");
                }
                break;

            case 2:
                printf("Enter key(int): ");
                scanf("%d", &key);
                if (delete(key) == 0) {
                    printf("Key deleted successfully!\n");
                }
                break;

            case 3:
                printf("Enter key(int): ");
                scanf("%d", &key);
                const char* value = search(key);
                if (value)
                    printf("Found: %s\n", value);
                else
                    printf("Key not found\n");
                break;
        }
        if (choose == 0) {
            exit(0);
        }
        while ((tmp = getchar()) != '\n' && tmp != EOF) { }
        printf("Press Enter to continue...\n");
        tmp = getchar();
    }

    return 0;
}
