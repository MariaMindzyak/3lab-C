#include "dialog.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "fileHelper.h"

int load(Table *table, char* fname){
    fopen_s(&(table->fd), fname, "r+b");
    if(table->fd == NULL) return 0;
    fread(&table->max_size, sizeof(int), 1, table->fd);
    table->ks = (KeySpace *) calloc(table->max_size, sizeof(KeySpace));
    fread(&table->size, sizeof(int), 1, table->fd);
    fread(table->ks, sizeof(KeySpace), table->size, table->fd);
    return 1;
}

int create(Table *table, char *fname, int sz){
    table->max_size = sz;
    table->size = 0;
    table->fd = fopen(fname, "r+b");
    if(table->fd == NULL){
        table->ks = NULL;
        return 0;
    }
    table->ks = (KeySpace *) calloc(table->max_size, sizeof(KeySpace));
    fwrite(&table->max_size, sizeof(int), 1, table->fd);
    fwrite(&table->size, sizeof(int), 1, table->fd);
    //fwrite(table->ks, sizeof(KeySpace), table->max_size, table->fd); /////////////////////
    return 1;
}

int save(Table *table){
    fseek(table->fd, sizeof(int), SEEK_SET);
    fwrite(&table->size, sizeof(int), 1, table->fd);
    fwrite(table->ks, sizeof(KeySpace), table->size, table->fd);
    fclose(table->fd);
    table->fd = NULL;
    return 1;
}
