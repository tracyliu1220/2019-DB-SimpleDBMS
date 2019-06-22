#include <stdlib.h>
#include <string.h>
#include <set>
#include <map>
#include <sys/stat.h>
#include "Table.h"
using namespace std;

///
/// Allocate a Table_t struct, then initialize some attributes, and
/// load table if the `file_name` is given
///
Table_t *new_Table() {
    Table_t *table = (Table_t*)malloc(sizeof(Table_t));
    memset((void*)table, 0, sizeof(Table_t));
    table->capacity = INIT_TABLE_SIZE;
    table->len = 0;
    table->users = (User_t*)malloc(
                            sizeof(User_t) * INIT_TABLE_SIZE);
    table->likes = (Like_t*)malloc(
                            sizeof(Like_t) * INIT_TABLE_SIZE);
    table->dirty = 1;
    table->idx = set<unsigned int>();
    table->idx1 = set<unsigned int>();
    table->idx2 = map<unsigned int, unsigned int>();
    return table;
}

///
/// Add the `User_t` data to the given table
/// If the table is full, it will allocate new space to store more
/// user data
/// return 1 when the data successfully add to table
///
int add_User(Table_t *table, User_t *user) {
    size_t idx;
    if (!table || !user) {
        return 0;
    }
    // Check id doesn't exist in the table
    if (table->idx.count(user->id)) {
        return 0;
    }
    table->idx.insert(user->id);

    if (table->len == table->capacity) {
        User_t *new_user_buf = (User_t*)malloc(sizeof(User_t)*(table->len+EXT_LEN));

        memcpy(new_user_buf, table->users, sizeof(User_t)*table->len);

        table->users = new_user_buf;
        table->capacity += EXT_LEN;
    }
    idx = table->len;
    memcpy((table->users)+idx, user, sizeof(User_t));
    table->len++;
    table->dirty = 1;
    return 1;
}

int add_Like(Table_t *table, Like_t *like) {
    size_t idx;
    if (!table || !like) {
        return 0;
    }
    
    table->idx1.insert(like->id1);
    if (table->idx2.count(like->id2))
        table->idx2[like->id2] ++;
    else
        table->idx2[like->id2] = 1;

    if (table->len == table->capacity) {
        Like_t *new_like_buf = (Like_t*)malloc(sizeof(Like_t)*(table->len+EXT_LEN));
        memcpy(new_like_buf, table->likes, sizeof(Like_t)*table->len);

        table->likes = new_like_buf;
        table->capacity += EXT_LEN;
    }
    idx = table->len;
    memcpy((table->likes)+idx, like, sizeof(Like_t));
    table->len++;
    table->dirty = 1;
    return 1;
}

///
/// Return the user in table by the given index
///
User_t* get_User(Table_t *table, size_t idx) {
    return table->users+idx;
}

Like_t* get_Like(Table_t *table, size_t idx) {
    return table->likes+idx;
}