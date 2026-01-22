#ifndef LAB3B_FILEHELPER_H
#define LAB3B_FILEHELPER_H

int load(Table *table, char* fname);
int create(Table *table, char *fname, int sz);
int save(Table *table);

#endif //LAB3B_FILEHELPER_H
