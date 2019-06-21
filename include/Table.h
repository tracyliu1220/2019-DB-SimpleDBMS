#ifndef TABLE_H
#define TABLE_H
#include <stdlib.h>
#include <stdio.h>
#include <set>
#include "User.h"
using namespace std;

#define INIT_TABLE_SIZE 10000
#define EXT_LEN 500

typedef struct Table {
    size_t capacity;
    size_t len;
    User_t *users;
    Like_t *likes;
    unsigned char *cache_map;
    FILE *fp;
    char *file_name;
    set<unsigned int> idx;
    size_t dirty;
} Table_t;

Table_t *new_Table(char *file_name);
int add_User(Table_t *table, User_t *user);
User_t* get_User(Table_t *table, size_t idx);
int add_Like(Table_t *table, Like_t *like);
Like_t* get_Like(Table_t *table, size_t idx);

#endif
