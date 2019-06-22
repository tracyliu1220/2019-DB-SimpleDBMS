#ifndef DB_UTIL_H
#define DB_UTIL_H
#include "Command.h"
#include "Table.h"

typedef struct State {
    int saved_stdout;
} State_t;

State_t* new_State();
void print_prompt(State_t *state);
int set_idxlist(Table_t *table, int **idxList, int idxListLen, Command_t *cmd, int state);

void print_user(User_t *user, SelectArgs_t *sel_args);
void print_users(Table_t *table, int *idxList, int idxListLen, Command_t *cmd);

void print_like(Like_t *like, SelectArgs_t *sel_args);
void print_likes(Table_t *table, Command_t *cmd);

int parse_input(char *input, Command_t *cmd);

void handle_builtin_cmd(Table_t *user_table, Command_t *cmd, State_t *state);
int handle_query_cmd(Table_t *user_table, Table_t *like_table, Command_t *cmd);
int handle_insert_cmd(Table_t *user_table, Table_t *like_table, Command_t *cmd);
int handle_select_cmd(Table_t *user_table, Table_t *like_table, Command_t *cmd);

int handle_update_cmd(Table_t *user_table, Command_t *cmd);
int handle_delete_cmd(Table_t *user_table, Command_t *cmd);

void print_help_msg();

#endif
