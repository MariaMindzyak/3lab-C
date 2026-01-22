#include <stdio.h>
#include <stdlib.h>
#include "dialog.h"
#include "menu.h"
#include "table.h"
#include "fileHelper.h"

int main() {
    /*Table table = init_table();
    int rc;

    if(D_load(&table) == 0) return 1;

    while(rc = dialog(msgs, N)){
        if(!fptr[rc](&table)) break;
    }

    D_save(&table);
    printf("That's all, bye!");
    freeTable(&table);*/
    Table table = init_table();
    create(&table, "C:/Users/mindz/CLionProjects/lab3b/input.bin", 3);
    add_el(&table, 2, "mama");
    add_el(&table, 3, "papa");
    fseek(table.fd, 0, SEEK_SET);
    int max_size = 0;
    fread(&max_size, sizeof(int), 1, table.fd);
    int size = 0;
    fread(&size, sizeof(int), 1, table.fd);


//    Node* node = NULL;
//    fread(node, sizeof(Node), 1, table.fd);
      printf("%d, %d", max_size, size);

    return 0;
};

