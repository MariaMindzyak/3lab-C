#include <stddef.h>
#include <malloc.h>
#include "table.h"
#include <stdio.h>
#include <string.h>
#define MAX_TABLE_SIZE 4 

Table init_table() {
    Table table;
    table.size = 0;
    table.max_size = MAX_TABLE_SIZE;
    table.ks = (KeySpace*)calloc(MAX_TABLE_SIZE, sizeof(KeySpace));
    return table;
}

int bin_search(Table table, int key){
    int left = 0;
    int right = table.size - 1;
    int mid;

    while (left <= right) {
        mid = (left + right) / 2;
        if (table.ks[mid].key == key) return mid;
        if (table.ks[mid].key > key) {
            right = mid - 1;
            continue;
        }
        if (table.ks[mid].key < key) {
            left = mid + 1;
            continue;
        }
    }
    return -1;
}

/*nt search_by_key(Table table, int key, Table *table_copy) {
    int n = bin_search(table, key);
    if(n == -1) return NOT_FOUND;
    while(table.first[n].key == key) --n;
    fseek(table.fd, table.first[n].offset, SEEK_SET);
    int count_rel = 0;
    int idx = n;
    while(table.first[n].key == key){
        count_rel++;
        idx++;
    }
    fread(table_copy->first, sizeof(Node), count_rel, table.fd);
    return OK;
}

int search_by_key_and_release(Table table, int key, int release, Table* table_copy) {
    int n = bin_search(table, key);
    if(n == -1) return NOT_FOUND;

    while(table.first[n].release != release){
        if(table.first[n].release > release) n--;
        if(table.first[n].release < release) n++;
    }
    fread(table_copy->first, sizeof(Node), 1, table.fd);
    return OK;
}
*/
void reposition_info(Table* table, Node* node, int count, int reposition){ //таблица, с какого начинать, сколько перемещать, на сколько
    char* all_info = (char*) calloc(1, sizeof(char));
    fseek(table->fd, node->offset, SEEK_SET);
    fread(all_info, sizeof(char), node->len, table->fd);
    fseek(table->fd, node->offset + reposition, SEEK_SET);
    fwrite(all_info, sizeof(char), node->len, table->fd);
}

void change_offset_node(Table* table, Node* node, int position, int count, int reposition){ //таблица, ячейка, место ячейки, сколько менять, насколько двигать
    for (int j = 0; j < count; j++){
        fseek(table->fd, position + j * sizeof(Node), SEEK_SET);
        fread(node, sizeof(Node), 1, table->fd);
        node->offset += reposition;
        fseek(table->fd, position + j * sizeof(Node), SEEK_SET);
        fwrite(node, sizeof(Node), 1, table->fd);
    }
}

void reposition_node_array(Table* table, int position, int count_change, int reposition, int idx){
    Node* node_array = (Node*) calloc(1, sizeof(Node));
    fseek(table->fd, position, SEEK_SET);
    fread(node_array, sizeof(Node), count_change, table->fd);
    fseek(table->fd, position + reposition, SEEK_SET);
    fwrite(node_array, sizeof(Node), count_change, table->fd);

    for (int k = idx; k < table->size; k++){
        table->ks[k].offset_node += sizeof(Node);
    }
}

int add_el(Table *table, int key, char *info) {
    if (table->size == table->max_size){
    	return OVERFLOW;
    }
    int idx = bin_search(*table, key);

    if (idx != -1){

        int count = 0; // сколько всего node
        int count_prev = 0; // сколько node перед вставляемым
        for(int i = 0; i < table->size; i++){
            count += table->ks[i].num_release;
            if (i <= idx) count_prev++;
        }

        fseek(table->fd, table->ks[0].offset_node, SEEK_SET);
        Node* node = (Node*) calloc(1, sizeof (Node)); // первый node
        fread(node, sizeof(Node), 1, table->fd);


        reposition_info(table, node, count, sizeof(Node));

        change_offset_node(table, node, table->ks[0].offset_node, count, sizeof(Node));

        reposition_node_array(table, table->ks[idx + 1].offset_node, count - count_prev, sizeof(Node), idx + 1);


        Node* prev = (Node*) calloc(1, sizeof (Node));
        fseek(table->fd, table->ks[idx].offset_node + table->ks[idx].num_release * sizeof(Node), SEEK_SET);
        fread(prev, sizeof(Node), 1, table->fd);
        table->ks[idx].num_release++;
        Node* new_node = (Node*) calloc(1, sizeof (Node));
        new_node->release = table->ks[idx].num_release;
        new_node->offset = prev->offset + sizeof(char);
        new_node->len = strlen(info) + 1;
        fwrite(new_node, sizeof(Node), 1, table->fd);

        fseek(table->fd, new_node->offset, SEEK_SET);
        char *info2 = (char*) calloc(1, sizeof(char));
        fread(info2, sizeof(char), count - count_prev, table->fd);
        strcat(info, info2);
        fseek(table->fd, new_node->offset, SEEK_SET);
        fwrite(info, sizeof(char), count - count_prev + 1, table->fd);

        fseek(table->fd, table->ks[idx + 1].offset_node, SEEK_SET);
        fread(node, sizeof(Node), count - count_prev + 1, table->fd);
        change_offset_node(table, node, table->ks[idx + 1].offset_node, count - count_prev - 1, sizeof(char));
    }

    else{
        if (table->size == 0){
            table->ks[0].key = key;
            table->ks[0].num_release = 1;
            table->ks[0].offset_node = 2*sizeof(int) + sizeof(KeySpace);
            fseek(table->fd, 2*sizeof(int), SEEK_SET);
            fwrite(table->ks, sizeof(KeySpace), 1, table->fd);

            Node* new_node = (Node*) calloc(1, sizeof(Node));
            new_node->len = strlen(info);
            new_node->release = 1;
            new_node->offset = 2*sizeof(int) + sizeof(KeySpace) + sizeof(Node);
            fseek(table->fd, table->ks[0].offset_node, SEEK_SET);
            fwrite(new_node, sizeof(Node), 1, table->fd);

            fseek(table->fd, new_node->offset, SEEK_SET);
            fwrite(info, sizeof(char), new_node->len, table->fd);

            table->size++;
            fseek(table->fd, sizeof(int), SEEK_SET);
            fwrite(&table->size, sizeof(int), 1, table->fd);
            return OK;
        }
        while ((idx != table->size) && (table->ks[idx].key < key)) {
            idx++;
        }

        int count_prev = 0;
        int count = 0;
        for(int i = 0; i < table->size; i++){
            count += table->ks[i].num_release;
            if (i < idx) count_prev += table->ks[i].num_release;
        }

        fseek(table->fd, table->ks[0].offset_node, SEEK_SET);
        Node* node = (Node*) calloc(1, sizeof (Node));
        fread(node, sizeof(Node), 1, table->fd);

        reposition_info(table, node, count, sizeof(KeySpace) + sizeof(Node));

        change_offset_node(table, node, table->ks[0].offset_node, count, sizeof(KeySpace) + sizeof(Node));

        reposition_node_array(table, table->ks[0].offset_node, count, sizeof(KeySpace), 0);

        for (int j = table->size; j > idx; j--) {
            table->ks[j] = table->ks[j-1];
        }
        table->ks[idx].num_release = 1;
        table->ks[idx].key = key;
        table->size++;

        fseek(table->fd, sizeof(int), SEEK_SET);
        fwrite(&table->size, sizeof(int), 1, table->fd);
        fwrite(table->ks, sizeof(KeySpace), table->size, table->fd);

        reposition_node_array(table, table->ks[idx + 1].offset_node, count, sizeof(Node), idx + 1);

        Node* new_node = (Node*) calloc(1, sizeof (Node));
        new_node->len = strlen(info);
        new_node->release = 1;
        table->ks[idx].offset_node = 2*sizeof(int) + table->size * sizeof(KeySpace) + count_prev*sizeof(Node);
        fseek(table->fd, table->ks[idx].offset_node, SEEK_SET);
        fwrite(new_node, sizeof(Node), 1, table->fd);


        Node* next = (Node*) calloc(1, sizeof (Node));

        if (table->ks[idx+1].num_release != 0) {
            fseek(table->fd, table->ks[idx + 1].offset_node, SEEK_SET);
            fread(next, sizeof(Node), 1, table->fd);
            fseek(table->fd, next->offset, SEEK_SET);
            char *info2 = (char *) calloc(1, sizeof(char));
            fread(info2, sizeof(char), count - count_prev, table->fd);
            strcat(info, info2);
            new_node->offset = next->offset;
            change_offset_node(table, next, table->ks[idx + 1].offset_node, count - count_prev, sizeof(char));
        }
        else{
            fseek(table->fd, 0, SEEK_END);
            new_node->offset = ftell(table->fd);
        }
        fseek(table->fd, new_node->offset, SEEK_SET);
        fwrite(info, sizeof(char), new_node->len, table->fd);

        table->size++;
    }

    return OK;
}

/*int delete_el_by_key(Table* table, int key){
    int idx = bin_search(*table, key);
    if (idx == -1) return NOT_FOUND;
 
    	Node node = table->first[idx];
    	Node next = table->first[idx + 1];
    	while (next != NULL){
    	    node = next;
    	    //next = next->next;
    	}
        for (int i = idx; i<table->table_size - 1; i++){
            table->first[i].key = table->first[i+1].key;
            table->first[i] = table->first[i+1];
        }
        table->table_size -= 1;
    return OK;
}

int delete_el_by_key_and_release(Table* table, int key, int release){
    int idx = bin_search(*table, key);

    if (idx == -1) return NOT_FOUND;

    if(table->first[idx].release < release) return OUT_OF_TABLE;

    if (release == 1 && table->first[idx].release == 1) {
        int rc = delete_el_by_key(table, key);
        return rc;
    }
    Node head = table->first[idx];
    Node prev = table->first[idx - 1];
    Node this_node = table->first[idx];
    while (this_node->release != release){
        prev = this_node;
        //this_node = this_node->next;
    }
    Node sub_node = this_node->next;
    if (head == this_node){
        table->first[idx] = sub_node;
    }
    else {
        prev->next = sub_node;
    }
    return OK;
}

void freeTable(Table* table){
    for(int i = 0; i<table->table_size; i++){
    	if (table-> == NULL) continue;
    	Node* node = table->ks[i].node;
    	Node* next = table->ks[i].node->next;
    	while (next != NULL){
        	free(node->info);
        	free(node);
        	node = next;
      		next = next->next;
        }
        free(node->info);
        free(node);
    }
    free(table->ks);
}
	
*/