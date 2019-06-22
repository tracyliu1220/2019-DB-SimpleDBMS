#ifndef TABLE_H
#define TABLE_H
#include <stdlib.h>
#include <stdio.h>
#include <set>
#include <map>
#include <vector>
#include <string>
#include "User.h"
using namespace std;

#define INIT_TABLE_SIZE 100000
#define EXT_LEN 500

typedef struct Table {
    int capacity;
    int len;
    User_t *users;
    Like_t *likes;
    set<unsigned int> idx;
    set<unsigned int> idx1;
    map<unsigned int, unsigned int> idx2;
    size_t dirty;
    size_t triggered;
    vector<int> ages;
    vector<int> id_by_ages;
} Table_t;

Table_t *new_Table();
int add_User(Table_t *table, User_t *user);
User_t* get_User(Table_t *table, size_t idx);
int add_Like(Table_t *table, Like_t *like);
Like_t* get_Like(Table_t *table, size_t idx);

#endif
