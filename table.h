
#ifndef LAB3_TABLE_H
#define LAB3_TABLE_H

#include <stdio.h>
#include "table.h"

enum Errors{
    OK = 0,
    NOT_FOUND = 1,
    OUT_OF_TABLE = 2,
    OVERFLOW = 3,
};

typedef struct KeySpace{
    int key;
    int offset_node;
    int num_release;
}KeySpace;

typedef struct Node {
    int release;
    int offset; //cмещение в файле
    int len; //длина информации
} Node;


typedef struct Table {
    int max_size; //размер вектора
    int size; //текущий размер таблицы
    KeySpace* ks; //массив элементов
    FILE *fd;
} Table;

Table init_table();

int bin_search(Table table, int key);

int search_by_key(Table table, int key, Table *table_copy);

int search_by_key_and_release(Table table, int key, int release, Table* table_copy);

int add_el(Table *table, int key, char *info);
void reposition_info(Table* table, Node* node, int count, int reposition);
void change_offset_node(Table* table, Node* node, int position, int count, int reposition); //изменение offset в node
void reposition_node_array(Table* table, int position, int count_change, int reposition, int idx);

int delete_el_by_key(Table* table, int key);

int delete_el_by_key_and_release(Table* table, int key, int release);

void freeTable(Table* table);


#endif //LAB3_TABLE_H
