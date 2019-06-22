#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include "Util.h"
#include "Command.h"
#include "Table.h"
#include "SelectState.h"
#include "SetWhere.h"

///
/// Allocate State_t and initialize some attributes
/// Return: ptr of new State_t
///
State_t* new_State() {
    State_t *state = (State_t*)malloc(sizeof(State_t));
    state->saved_stdout = -1;
    return state;
}

///
/// Print shell prompt
///
void print_prompt(State_t *state) {
    if (state->saved_stdout == -1) {
        printf("db > ");
    }
}

///
/// Print the user in the specific format
///
void print_user(User_t *user, SelectArgs_t *sel_args) {
    int idx;
    printf("(");
    for (idx = 0; idx < sel_args->fields_len; idx++) {
        if (!strncmp(sel_args->fields[idx], "*", 1)) {
            printf("%d, %s, %s, %d", user->id, user->name, user->email, user->age);
        } else {
            if (idx > 0) printf(", ");

            if (!strncmp(sel_args->fields[idx], "id", 2)) {
                printf("%d", user->id);
            } else if (!strncmp(sel_args->fields[idx], "name", 4)) {
                printf("%s", user->name);
            } else if (!strncmp(sel_args->fields[idx], "email", 5)) {
                printf("%s", user->email);
            } else if (!strncmp(sel_args->fields[idx], "age", 3)) {
                printf("%d", user->age);
            }
        }
    }
    printf(")\n");
}

void print_like(Like_t *like, SelectArgs_t *sel_args) {
    int idx;
    printf("(");
    for (idx = 0; idx < sel_args->fields_len; idx++) {
        if (!strncmp(sel_args->fields[idx], "*", 1)) {
            printf("%d, %d", like->id1, like->id2);
        } else {
            if (idx > 0) printf(", ");

            if (!strncmp(sel_args->fields[idx], "id1", 3)) {
                printf("%d", like->id1);
            } else if (!strncmp(sel_args->fields[idx], "id2", 3)) {
                printf("%d", like->id2);
            }
        }
    }
    printf(")\n");
}

void print_aggre(SelectArgs_t *sel_args, AggreArgs_t *aggre_args) {
    int idx;
    printf("(");
    for (idx = 0; idx < sel_args->fields_len; idx++) {
        if (idx > 0) printf(", ");
        if (!strncmp(sel_args->fields[idx], "sum(id)", 7)) {
            printf("%d", aggre_args->idsum_result);
        } else if (!strncmp(sel_args->fields[idx], "avg(id)", 7)) {
            printf("%.3lf", aggre_args->idavg_result);
        } else if (!strncmp(sel_args->fields[idx], "sum(age)", 8)) {
            printf("%d", aggre_args->agesum_result);
        } else if (!strncmp(sel_args->fields[idx], "avg(age)", 8)) {
            printf("%.3lf", aggre_args->ageavg_result);
        } else if (!strncmp(sel_args->fields[idx], "count", 5)) {
            printf("%d", aggre_args->cnt_result);
        }
    }
    printf(")\n");
}

void print_like_aggre(SelectArgs_t *sel_args, AggreArgs_t *aggre_args) {
    int idx;
    printf("(");
    for (idx = 0; idx < sel_args->fields_len; idx++) {
        if (idx > 0) printf(", ");
        if (!strncmp(sel_args->fields[idx], "sum(id1)", 8)) {
            printf("%d", aggre_args->idsum_result);
        } else if (!strncmp(sel_args->fields[idx], "avg(id1)", 8)) {
            printf("%.3lf", aggre_args->idavg_result);
        } else if (!strncmp(sel_args->fields[idx], "sum(id2)", 8)) {
            printf("%d", aggre_args->agesum_result);
        } else if (!strncmp(sel_args->fields[idx], "avg(id2)", 8)) {
            printf("%.3lf", aggre_args->ageavg_result);
        } else if (!strncmp(sel_args->fields[idx], "count", 5)) {
            printf("%d", aggre_args->cnt_result);
        }
    }
    printf(")\n");
}

void update_user(User_t *user, SetArgs_t *set_args) {
    if (!strncmp(set_args->field, "id", 2)) {
        user->id = set_args->set_int;
    } else if (!strncmp(set_args->field, "name", 4)) {
        strcpy(user->name,set_args->set_str);
    } else if (!strncmp(set_args->field, "email", 5)) {
        strcpy(user->email,set_args->set_str);
    } else if (!strncmp(set_args->field, "age", 3)) {
        user->age = set_args->set_int;
    }
}

// state = 1 if where not in, ex. delete
int set_idxlist(Table_t *table, int **idxList, int idxListLen, Command_t *cmd, int state) {
    int idxListCap = idxListLen;
    for (int i = 0; i < table->len; i ++) {
        User_t* user = get_User(table, i);

        if(where_test(cmd, user) ^ state) {        
            if (idxListCap == 0 || idxListCap == idxListLen) {
                int *new_buf = (int*) malloc( sizeof(int) * (idxListCap + 5) );
                memset(new_buf, 0, sizeof(int) * (idxListCap + 5));
                memcpy(new_buf, *idxList, sizeof(int) * (idxListCap));
                free(*idxList);
                *idxList = new_buf;
                idxListCap += 5;
            }
            (*idxList)[idxListLen] = i;
            idxListLen ++;
        }
    }
    return idxListLen;
}

///
/// Print the users for given offset and limit restriction
///
void print_users(Table_t *table, int *idxList, int idxListLen, Command_t *cmd) {
    int idx;
    int limit = cmd->cmd_args.sel_args.limit;
    int offset = cmd->cmd_args.sel_args.offset;

    if (offset == -1) {
        offset = 0;
    }

    idxListLen = set_idxlist(table, &idxList, idxListLen, cmd, 0);

    if (cmd->aggre_args.up) { // aggre
        cmd->aggre_args.idsum_result = 0;
        cmd->aggre_args.idavg_result = 0;
        cmd->aggre_args.agesum_result = 0;
        cmd->aggre_args.ageavg_result = 0;
        cmd->aggre_args.cnt_result = 0;
        if (cmd->where_args.up) {
            for (idx = 0; idx < idxListLen; idx++) {
                User_t *user = get_User(table, idxList[idx]);
                cmd->aggre_args.idsum_result += user->id;
                cmd->aggre_args.idavg_result += user->id;
                cmd->aggre_args.agesum_result += user->age;
                cmd->aggre_args.ageavg_result += user->age;
            }
            if (idxListLen) cmd->aggre_args.idavg_result /= idxListLen;
            if (idxListLen) cmd->aggre_args.ageavg_result /= idxListLen;
            cmd->aggre_args.cnt_result = idxListLen;
        } else {
            for (idx = 0; idx < table->len; idx++) {
                User_t *user = get_User(table, idx);
                cmd->aggre_args.idsum_result += user->id;
                cmd->aggre_args.idavg_result += user->id;
                cmd->aggre_args.agesum_result += user->age;
                cmd->aggre_args.ageavg_result += user->age;
            }
            if (table->len) cmd->aggre_args.idavg_result /= table->len;
            if (table->len) cmd->aggre_args.ageavg_result /= table->len;
            cmd->aggre_args.cnt_result = table->len;
        }
        if (offset == 0 && (limit > 0 || limit == -1))
            print_aggre(&(cmd->cmd_args.sel_args), &(cmd->aggre_args));
    } else { // print
        if (cmd->where_args.up) {
            for (idx = offset; idx < idxListLen; idx++) {
                if (limit != -1 && (idx - offset) >= limit) {
                    break;
                }
                print_user(get_User(table, idxList[idx]), &(cmd->cmd_args.sel_args));
            }
        } else {
            for (idx = offset; idx < table->len; idx++) {
                if (limit != -1 && (idx - offset) >= limit) {
                    break;
                }
                print_user(get_User(table, idx), &(cmd->cmd_args.sel_args));
            }
        }
    }
}

void print_likes(Table_t *table, Command_t *cmd) {
    int idx;
    int limit = cmd->cmd_args.sel_args.limit;
    int offset = cmd->cmd_args.sel_args.offset;

    if (offset == -1) {
        offset = 0;
    }

    if (cmd->aggre_args.up) { // aggre
        cmd->aggre_args.idsum_result = 0;
        cmd->aggre_args.idavg_result = 0;
        cmd->aggre_args.agesum_result = 0;
        cmd->aggre_args.ageavg_result = 0;
        cmd->aggre_args.cnt_result = 0;
        
        for (idx = 0; idx < table->len; idx++) {
            Like_t *like = get_Like(table, idx);
            cmd->aggre_args.idsum_result += like->id1;
            cmd->aggre_args.idavg_result += like->id1;
            cmd->aggre_args.agesum_result += like->id2;
            cmd->aggre_args.ageavg_result += like->id2;
        }
        if (table->len) cmd->aggre_args.idavg_result /= table->len;
        if (table->len) cmd->aggre_args.ageavg_result /= table->len;
        cmd->aggre_args.cnt_result = table->len;
        
        if (offset == 0 && (limit > 0 || limit == -1))
            print_like_aggre(&(cmd->cmd_args.sel_args), &(cmd->aggre_args));
    } else {
        for (idx = offset; idx < table->len; idx++) {
            if (limit != -1 && (idx - offset) >= limit) {
                break;
            }
            print_like(get_Like(table, idx), &(cmd->cmd_args.sel_args));
        }
    }
}

int update_users(Table_t *table, int *idxList, int idxListLen, Command_t *cmd) {
    int ret = 1;
    int legal = 1;
    int idx;

    idxListLen = set_idxlist(table, &idxList, idxListLen, cmd, 0);
    if (!strncmp(cmd->set_args.field, "id", 2)) {
        if (idxListLen != 1) legal = 0;
        if (table->idx.count(cmd->set_args.set_int))
            legal = 0;
    }

    if (cmd->where_args.up && legal) {
        for (idx = 0; idx < idxListLen; idx++)
            update_user(get_User(table, idxList[idx]), &(cmd->set_args));
        if (!strncmp(cmd->set_args.field, "id", 2)) {
            User_t *user = get_User(table, idxList[idx]);
            table->idx.erase(user->id);
            table->idx.insert(cmd->set_args.set_int);
        }
    } else if (!cmd->where_args.up && (strncmp(cmd->set_args.field, "id", 2) || table->len <= 1)) {
        for (idx = 0; idx < table->len; idx++) {
            update_user(get_User(table, idx), &(cmd->set_args));
        }
    } else {
        ret = 0;
    }
    return ret;
}

void delete_users(Table_t *table, int *idxList, int idxListLen, Command_t *cmd) {
    int idx;
    int len = 0;
    idxListLen = set_idxlist(table, &idxList, idxListLen, cmd, 1);
    table->idx.clear();

    if (cmd->where_args.up) {
        for (idx = 0; idx < idxListLen; idx++) {
            table->users[len] = table->users[idxList[idx]];
            table->idx.insert(table->users[len].id);
            len ++;
        }
    } else {
        len = 0;
    }
    table->len = len;
}

void join_tables(Table_t *user_table, Table_t *like_table, int *idxList, int idxListLen, Command_t *cmd) {
    int idx;
    int limit = cmd->cmd_args.sel_args.limit;
    int offset = cmd->cmd_args.sel_args.offset;

    if (offset == -1) {
        offset = 0;
    }

    int count = 0;
    idxListLen = set_idxlist(user_table, &idxList, idxListLen, cmd, 0);

    if (cmd->where_args.up) {
        for (idx = offset; idx < idxListLen; idx++) {
            if (limit != -1 && (idx - offset) >= limit) {
                break;
            }
            User_t *user = get_User(user_table, idxList[idx]);
            if (cmd->join_args.rhs == 1 
                && like_table->idx1.count(user->id)) {
                count ++;
            } else if (cmd->join_args.rhs == 2 
                && like_table->idx2.count(user->id)) {
                count += like_table->idx2[user->id];
            }
        }
    } else {
        for (idx = offset; idx < user_table->len; idx++) {
            if (limit != -1 && (idx - offset) >= limit) {
                break;
            }
            User_t *user = get_User(user_table, idx);
            if (cmd->join_args.rhs == 1 
                && like_table->idx1.count(user->id)) {
                count ++;
            } else if (cmd->join_args.rhs == 2 
                && like_table->idx2.count(user->id)) {
                count += like_table->idx2[user->id];
            }
        }
    }
    if (offset == 0 && (limit > 0 || limit == -1))
        printf("(%d)\n", count);
}

///
/// This function received an output argument
/// Return: category of the command
///
int parse_input(char *input, Command_t *cmd) {
    char *token;
    int idx;
    token = strtok(input, " ,\n");
    for (idx = 0; strlen(cmd_list[idx].name) != 0; idx++) {
        if (!strncmp(token, cmd_list[idx].name, cmd_list[idx].len)) {
            cmd->type = cmd_list[idx].type;
        }
    }
    // about spaces
    while (token != NULL) {
        char *op = strpbrk(token, "=<>!");
        if (op != NULL) {
            int len;
            len = (int)(op - token);
            if (len) {
                char *left_side = strndup(token, len);
                add_Arg(cmd, left_side);
            }
            len = strspn(op, "=<>!");
            char *operation = strndup(op, len);
            add_Arg(cmd, operation);
            char *right_side = op + len;
            len = strlen(right_side);
            if (len) {
                add_Arg(cmd, right_side);
            }
        } else {
            add_Arg(cmd, token);
        }
        token = strtok(NULL, " ,\n");
    }
    return cmd->type;
}

///
/// Handle built-in commands
/// Return: command type
///
void handle_builtin_cmd(Table_t *table, Command_t *cmd, State_t *state) {
    if (!strncmp(cmd->args[0], ".exit", 5)) {
        exit(0);
    } else if (!strncmp(cmd->args[0], ".output", 7)) {
        if (cmd->args_len == 2) {
            if (!strncmp(cmd->args[1], "stdout", 6)) {
                close(1);
                dup2(state->saved_stdout, 1);
                state->saved_stdout = -1;
            } else if (state->saved_stdout == -1) {
                int fd = creat(cmd->args[1], 0644);
                state->saved_stdout = dup(1);
                if (dup2(fd, 1) == -1) {
                    state->saved_stdout = -1;
                }
                __fpurge(stdout); //This is used to clear the stdout buffer
            }
        }
    } else if (!strncmp(cmd->args[0], ".help", 5)) {
        print_help_msg();
    }
}

///
/// Handle query type commands
/// Return: command type
///
int handle_query_cmd(Table_t *user_table, Table_t *like_table, Command_t *cmd) {
    if (!strncmp(cmd->args[0], "insert", 6)) {
        handle_insert_cmd(user_table, like_table, cmd);
        return INSERT_CMD;
    } else if (!strncmp(cmd->args[0], "select", 6)) {
        handle_select_cmd(user_table, like_table, cmd);
        user_table->triggered = 1;
        return SELECT_CMD;
    } else if (!strncmp(cmd->args[0], "update", 6)) {
        handle_update_cmd(user_table, cmd);
        user_table->triggered = 1;
        return SELECT_CMD;
    } else if (!strncmp(cmd->args[0], "delete", 6)) {
        handle_delete_cmd(user_table, cmd);
        user_table->triggered = 1;
        return SELECT_CMD;
    } else {
        return UNRECOG_CMD;
    }
}

///
/// The return value is the number of rows insert into table
/// If the insert operation success, then change the input arg
/// `cmd->type` to INSERT_CMD
///
int handle_insert_cmd(Table_t *user_table, Table_t *like_table, Command_t *cmd) {
    int ret = 0;
    if (!strncmp(cmd->args[2], "user", 4)) {
        User_t *user = command_to_User(cmd);
        if (user) {
            ret = add_User(user_table, user);
            if (ret > 0) {
                cmd->type = INSERT_CMD;
            }
        }
    } else if (!strncmp(cmd->args[2], "like", 4)) {
        Like_t *like = command_to_Like(cmd);
        if (like) {
            ret = add_Like(like_table, like);
            if (ret > 0) {
                cmd->type = INSERT_CMD;
            }
        }
    }
    return ret;
}

///
/// The return value is the number of rows select from table
/// If the select operation success, then change the input arg
/// `cmd->type` to SELECT_CMD
///
int handle_select_cmd(Table_t *user_table, Table_t *like_table, Command_t *cmd) {
    cmd->type = SELECT_CMD;
    field_state_handler(cmd, 1);
    int triggered = 0;

    // for query optimization
    if (!user_table->triggered) {
        // t1 -- select id, name from user offset <offset num> limit <limit num> 
        if (cmd->args_len == 9
            && !strncmp(cmd->args[0], "select", 6)
            && !strncmp(cmd->args[1], "id", 2)
            && !strncmp(cmd->args[2], "name", 4)
            && !strncmp(cmd->args[3], "from", 4)
            && !strncmp(cmd->args[4], "user", 4)
            && !strncmp(cmd->args[5], "offset", 6)
            && !strncmp(cmd->args[7], "limit", 5)) {
                int offset = atoi(cmd->args[6]);
                int limit = atoi(cmd->args[8]);
                query_opt_t1(user_table, offset, limit);
                triggered = 1;
        // t2 -- select name, age from user where age <= {upper} and age >= {lower}
        } else if (cmd->args_len == 13
            && !strncmp(cmd->args[0], "select", 6)
            && !strncmp(cmd->args[1], "name", 4)
            && !strncmp(cmd->args[2], "age", 3)
            && !strncmp(cmd->args[3], "from", 4)
            && !strncmp(cmd->args[4], "user", 4)
            && !strncmp(cmd->args[5], "where", 5)
            && !strncmp(cmd->args[6], "age", 3)
            && !strncmp(cmd->args[7], "<=", 2)
            && !strncmp(cmd->args[9], "and", 3)
            && !strncmp(cmd->args[10], "age", 3)
            && !strncmp(cmd->args[11], ">=", 2)) {
                int upper = atoi(cmd->args[8]);
                int lower = atoi(cmd->args[12]);
                query_opt_t2(user_table, lower, upper);
                triggered = 1;
        // t3 -- select count(*) from user where age <= {upper} and age >= {lower}
        } else if (cmd->args_len == 12
            && !strncmp(cmd->args[0], "select", 6)
            && !strncmp(cmd->args[1], "count(*)", 8)
            && !strncmp(cmd->args[2], "from", 4)
            && !strncmp(cmd->args[3], "user", 4)
            && !strncmp(cmd->args[4], "where", 5)
            && !strncmp(cmd->args[5], "age", 3)
            && !strncmp(cmd->args[6], "<=", 2)
            && !strncmp(cmd->args[8], "and", 3)
            && !strncmp(cmd->args[9], "age", 3)
            && !strncmp(cmd->args[10], ">=", 2)) {
                int upper = atoi(cmd->args[7]);
                int lower = atoi(cmd->args[11]);
                query_opt_t3(user_table, lower, upper);
                triggered = 1;
        // t4 -- select count(*) from user join like on id = id1 where name = "{target_user}"
        } else if (cmd->args_len == 14
            && !strncmp(cmd->args[0], "select", 6)
            && !strncmp(cmd->args[1], "count(*)", 8)
            && !strncmp(cmd->args[2], "from", 4)
            && !strncmp(cmd->args[3], "user", 4)
            && !strncmp(cmd->args[4], "join", 4)
            && !strncmp(cmd->args[5], "like", 4)
            && !strncmp(cmd->args[6], "on", 2)
            && !strncmp(cmd->args[7], "id", 2)
            && !strncmp(cmd->args[8], "=", 1)
            && !strncmp(cmd->args[9], "id1", 3)
            && !strncmp(cmd->args[10], "where", 5)
            && !strncmp(cmd->args[11], "name", 4)
            && !strncmp(cmd->args[12], "=", 1)) {
                query_opt_t4(user_table, like_table, cmd);
                triggered = 1;
        // t5 -- select count(*) from user join like on id = id2 where age < {target_age}
        } else if (cmd->args_len == 14
            && !strncmp(cmd->args[0], "select", 6)
            && !strncmp(cmd->args[1], "count(*)", 8)
            && !strncmp(cmd->args[2], "from", 4)
            && !strncmp(cmd->args[3], "user", 4)
            && !strncmp(cmd->args[4], "join", 4)
            && !strncmp(cmd->args[5], "like", 4)
            && !strncmp(cmd->args[6], "on", 2)
            && !strncmp(cmd->args[7], "id", 2)
            && !strncmp(cmd->args[8], "=", 1)
            && !strncmp(cmd->args[9], "id2", 3)
            && !strncmp(cmd->args[10], "where", 5)
            && !strncmp(cmd->args[11], "age", 3)
            && !strncmp(cmd->args[12], "<", 1)) {
                query_opt_t5(user_table, like_table, cmd);
                triggered = 1;
            }
    }

    if (triggered) return user_table->len;

    if (cmd->table1 == 0) { // user
        if (cmd->join_args.up) { // join
            join_tables(user_table, like_table, NULL, 0, cmd);
            return user_table->len;
        } else {
            print_users(user_table, NULL, 0, cmd);
            return user_table->len;
        }
    } else {
        print_likes(like_table, cmd);
        return like_table->len;
    }
}

int handle_update_cmd(Table_t *user_table, Command_t *cmd) {
    int ret = 0;
    set_state_handler(cmd, 3);
    update_users(user_table, NULL, 0, cmd);
    return ret;
}

int handle_delete_cmd(Table_t *user_table, Command_t *cmd) {
    int ret = 0;
    if (cmd->args_len > 3 && !strncmp(cmd->args[3], "where", 5))
        where_state_handler(cmd, 4);

    delete_users(user_table, NULL, 0, cmd);
    return ret;
}

///
/// Show the help messages
///
void print_help_msg() {
    const char msg[] = "# Supported Commands\n"
    "\n"
    "## Built-in Commands\n"
    "\n"
    "  * .exit\n"
    "\tThis cmd archives the table, if the db file is specified, then exit.\n"
    "\n"
    "  * .output\n"
    "\tThis cmd change the output strategy, default is stdout.\n"
    "\n"
    "\tUsage:\n"
    "\t    .output (<file>|stdout)\n\n"
    "\tThe results will be redirected to <file> if specified, otherwise they will display to stdout.\n"
    "\n"
    "  * .load\n"
    "\tThis command loads records stored in <DB file>.\n"
    "\n"
    "\t*** Warning: This command will overwrite the records already stored in current table. ***\n"
    "\n"
    "\tUsage:\n"
    "\t    .load <DB file>\n\n"
    "\n"
    "  * .help\n"
    "\tThis cmd displays the help messages.\n"
    "\n"
    "## Query Commands\n"
    "\n"
    "  * insert\n"
    "\tThis cmd inserts one user record into table.\n"
    "\n"
    "\tUsage:\n"
    "\t    insert <id> <name> <email> <age>\n"
    "\n"
    "\t** Notice: The <name> & <email> are string without any whitespace character, and maximum length of them is 255. **\n"
    "\n"
    "  * select\n"
    "\tThis cmd will display all user records in the table.\n"
    "\n";
    printf("%s", msg);
}

void query_opt_t1(Table_t *user_table, int offset, int limit) {
    // t1 -- select id, name from user offset <offset num> limit <limit num> 
    // printf("opt1\n");
    for (int i = offset; i < user_table->len; i++) {
        if (limit != -1 && (i - offset) >= limit) {
            break;
        }
        User_t *user = get_User(user_table, i);
        printf("(%d, %s)\n", user->id, user->name);
    }
}

void query_opt_t2(Table_t *user_table, int lower, int upper) {
    // t2 -- select name, age from user where age <= {upper} and age >= {lower}
    for (int i = 0; i < user_table->len; i ++) {
        User_t *user = get_User(user_table, i);
        if (user->age <= upper && user->age >= lower)
            printf("(%s, %d)\n", user->name, user->age);
    }
}

void query_opt_t3(Table_t *user_table, int lower, int upper) {
    // t3 -- select count(*) from user where age <= {upper} and age >= {lower}
    // printf("opt3\n");
    int count = 0;
    for (int i = lower; i <= upper; i ++) {
        count += user_table->ages[i];
    }
    printf("(%d)\n", count);
}

void query_opt_t4(Table_t *user_table, Table_t *like_table, Command_t *cmd) {
    // t4 -- select count(*) from user join like on id = id1 where name = "{target_user}"
    // printf("opt4\n");
    int count = 0;
    for (int i = 0; i < user_table->len; i ++) {
        User_t *user = get_User(user_table, i);
        if (!strcmp(user->name, cmd->args[13])
            && like_table->idx1.count(user->id))
            count ++;
    }
    printf("(%d)\n", count);
}

void query_opt_t5(Table_t *user_table, Table_t *like_table, Command_t *cmd) {
    // t5 -- select count(*) from user join like on id = id2 where age < {target_age}
    // printf("opt5\n");
    int count = 0;
    for (int i = 0; i < user_table->len; i ++) {
        User_t *user = get_User(user_table, i);
        if (user->age < atoi(cmd->args[13])
            && like_table->idx2.count(user->id))
            count += like_table->idx2[user->id];
    }
    printf("(%d)\n", count);
}