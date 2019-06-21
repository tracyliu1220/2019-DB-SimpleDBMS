#include <stdlib.h>
#include <string.h>
#include <set>
#include <sys/stat.h>
#include "Table.h"
using namespace std;

///
/// Allocate a Table_t struct, then initialize some attributes, and
/// load table if the `file_name` is given
///
Table_t *new_Table(char *file_name) {
    Table_t *table = (Table_t*)malloc(sizeof(Table_t));
    memset((void*)table, 0, sizeof(Table_t));
    table->capacity = INIT_TABLE_SIZE;
    table->len = 0;
    table->users = (User_t*)malloc(
                            sizeof(User_t) * INIT_TABLE_SIZE);
    table->users = (User_t*)malloc(
                            sizeof(Like_t) * INIT_TABLE_SIZE);
    table->cache_map = (unsigned char*)malloc(sizeof(char)*INIT_TABLE_SIZE);
    memset(table->cache_map, 0, sizeof(char)*INIT_TABLE_SIZE);
    table->fp = NULL;
    table->file_name = NULL;
    table->dirty = 1;
    table->idx = set<unsigned int>();
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
    User_t *usr_ptr;
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
        unsigned char *new_cache_buf = (unsigned char *)malloc(sizeof(unsigned char)*(table->len+EXT_LEN));

        memcpy(new_user_buf, table->users, sizeof(User_t)*table->len);

        memset(new_cache_buf, 0, sizeof(unsigned char)*(table->len+EXT_LEN));
        memcpy(new_cache_buf, table->cache_map, sizeof(unsigned char)*table->len);


        free(table->users);
        free(table->cache_map);
        table->users = new_user_buf;
        table->cache_map = new_cache_buf;
        table->capacity += EXT_LEN;
    }
    idx = table->len;
    memcpy((table->users)+idx, user, sizeof(User_t));
    table->cache_map[idx] = 1;
    table->len++;
    table->dirty = 1;
    return 1;
}

///
/// Return the user in table by the given index
///
User_t* get_User(Table_t *table, size_t idx) {
    size_t archived_len;
    struct stat st;
    if (!table->cache_map[idx]) {
        if (idx > INIT_TABLE_SIZE) {
            goto error;
        }
        if (stat(table->file_name, &st) != 0) {
            goto error;
        }
        archived_len = st.st_size / sizeof(User_t);
        if (idx >= archived_len) {
            //neither in file, nor in memory
            goto error;
        }

        fseek(table->fp, idx*sizeof(User_t), SEEK_SET);
        fread(table->users+idx, sizeof(User_t), 1, table->fp);
        table->cache_map[idx] = 1;
    }
    return table->users+idx;

error:
    return NULL;
}

int add_Like(Table_t *table, Like_t *like) {
    size_t idx;
    Like_t *like_ptr;
    if (!table || !like) {
        return 0;
    }
    if (table->len == table->capacity) {
        Like_t *new_like_buf = (Like_t*)malloc(sizeof(Like_t)*(table->len+EXT_LEN));
        unsigned char *new_cache_buf = (unsigned char *)malloc(sizeof(unsigned char)*(table->len+EXT_LEN));

        memcpy(new_like_buf, table->likes, sizeof(Like_t)*table->len);

        memset(new_cache_buf, 0, sizeof(unsigned char)*(table->len+EXT_LEN));
        memcpy(new_cache_buf, table->cache_map, sizeof(unsigned char)*table->len);


        free(table->likes);
        free(table->cache_map);
        table->likes = new_like_buf;
        table->cache_map = new_cache_buf;
        table->capacity += EXT_LEN;
    }
    idx = table->len;
    memcpy((table->likes)+idx, like, sizeof(Like_t));
    table->cache_map[idx] = 1;
    table->len++;
    table->dirty = 1;
    return 1;
}

Like_t* get_Like(Table_t *table, size_t idx) {
    size_t archived_len;
    struct stat st;
    if (!table->cache_map[idx]) {
        if (idx > INIT_TABLE_SIZE) {
            goto error;
        }
        if (stat(table->file_name, &st) != 0) {
            goto error;
        }
        archived_len = st.st_size / sizeof(Like_t);
        if (idx >= archived_len) {
            //neither in file, nor in memory
            goto error;
        }

        fseek(table->fp, idx*sizeof(Like_t), SEEK_SET);
        fread(table->likes+idx, sizeof(Like_t), 1, table->fp);
        table->cache_map[idx] = 1;
    }
    return table->likes+idx;

error:
    return NULL;
}