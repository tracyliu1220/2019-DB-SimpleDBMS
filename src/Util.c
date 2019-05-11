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
    size_t idx;
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

void print_aggre(SelectArgs_t *sel_args, AggreArgs_t *aggre_args) {
    size_t idx;
    printf("(");
    for (idx = 0; idx < sel_args->fields_len; idx++) {
        if (idx > 0) printf(", ");
        if (!strncmp(sel_args->fields[idx], "sum", 3)) {
            printf("%d", aggre_args->sum_result);
        } else if (!strncmp(sel_args->fields[idx], "avg", 3)) {
            printf("%.3lf", aggre_args->avg_result);
        } else if (!strncmp(sel_args->fields[idx], "count", 3)) {
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
size_t set_idxlist(Table_t *table, int **idxList, size_t idxListLen, Command_t *cmd, int state) {
    size_t idxListCap = idxListLen;
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
void print_users(Table_t *table, int *idxList, size_t idxListLen, Command_t *cmd) {
    size_t idx;
    int limit = cmd->cmd_args.sel_args.limit;
    int offset = cmd->cmd_args.sel_args.offset;

    if (offset == -1) {
        offset = 0;
    }

    idxListLen = set_idxlist(table, &idxList, idxListLen, cmd, 0);

    if (cmd->aggre_args.up) { // aggre
        cmd->aggre_args.sum_result = 0;
        cmd->aggre_args.avg_result = 0;
        cmd->aggre_args.cnt_result = 0;
        if (cmd->where_args.up) {
            for (idx = 0; idx < idxListLen; idx++) {
                User_t *user = get_User(table, idxList[idx]);
                // sum
                if (cmd->aggre_args.sum_up
                    && !strncmp(cmd->aggre_args.sum_field, "id", 2)) {
                    cmd->aggre_args.sum_result += user->id;
                } else if (cmd->aggre_args.sum_up
                    && !strncmp(cmd->aggre_args.sum_field, "age", 3)) {
                    cmd->aggre_args.sum_result += user->age;
                }
                // avg
                if (cmd->aggre_args.avg_up
                    && !strncmp(cmd->aggre_args.avg_field, "id", 2)) {
                    cmd->aggre_args.avg_result += user->id;
                } else if (cmd->aggre_args.avg_up && 
                    !strncmp(cmd->aggre_args.avg_field, "age", 3)) {
                    cmd->aggre_args.avg_result += user->age;
                }
            }
            cmd->aggre_args.avg_result /= idxListLen;
            cmd->aggre_args.cnt_result = idxListLen;
        } else {
            for (idx = 0; idx < table->len; idx++) {
                User_t *user = get_User(table, idx);
                // sum
                if (cmd->aggre_args.sum_up
                    && !strncmp(cmd->aggre_args.sum_field, "id", 2)) {
                    cmd->aggre_args.sum_result += user->id;
                } else if (cmd->aggre_args.sum_up
                    && !strncmp(cmd->aggre_args.sum_field, "age", 3)) {
                    cmd->aggre_args.sum_result += user->age;
                }
                // avg
                if (cmd->aggre_args.avg_up
                    && !strncmp(cmd->aggre_args.avg_field, "id", 2)) {
                    cmd->aggre_args.avg_result += user->id;
                } else if (cmd->aggre_args.avg_up && 
                    !strncmp(cmd->aggre_args.avg_field, "age", 3)) {
                    cmd->aggre_args.avg_result += user->age;
                }
            }
            cmd->aggre_args.avg_result /= table->len;
            cmd->aggre_args.cnt_result = table->len;
        }
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

int update_users(Table_t *table, int *idxList, size_t idxListLen, Command_t *cmd) {
    int ret = 1;
    int legal = 1;
    size_t idx;

    idxListLen = set_idxlist(table, &idxList, idxListLen, cmd, 0);
    if (!strncmp(cmd->set_args.field, "id", 2)) {
        if (idxListLen > 1) legal = 0;
        for (idx = 0; idx < idxListLen; idx ++) {
            if (get_User(table, idxList[idx])->id == cmd->set_args.set_int)
                legal = 0;
        }
    }

    if (cmd->where_args.up && legal) {
        for (idx = 0; idx < idxListLen; idx++)
            update_user(get_User(table, idxList[idx]), &(cmd->set_args));
    } else if (!cmd->where_args.up && (strncmp(cmd->set_args.field, "id", 2) || table->len <= 1)) {
        for (idx = 0; idx < table->len; idx++) {
            update_user(get_User(table, idx), &(cmd->set_args));
        }
    } else {
        ret = 0;
    }
    return ret;
}

void delete_users(Table_t *table, int *idxList, size_t idxListLen, Command_t *cmd) {
    size_t idx;
    int len = 0;
    idxListLen = set_idxlist(table, &idxList, idxListLen, cmd, 1);

    if (cmd->where_args.up) {
        for (idx = 0; idx < idxListLen; idx++) {
            table->users[len] = table->users[idxList[idx]];
            len ++;
        }
    } else {
        len = 0;
    }
    table->len = len;
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
        archive_table(table);
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
    } else if (!strncmp(cmd->args[0], ".load", 5)) {
        if (cmd->args_len == 2) {
            load_table(table, cmd->args[1]);
        }
    } else if (!strncmp(cmd->args[0], ".help", 5)) {
        print_help_msg();
    }
}

///
/// Handle query type commands
/// Return: command type
///
int handle_query_cmd(Table_t *table, Command_t *cmd) {
    if (!strncmp(cmd->args[0], "insert", 6)) {
        handle_insert_cmd(table, cmd);
        return INSERT_CMD;
    } else if (!strncmp(cmd->args[0], "select", 6)) {
        handle_select_cmd(table, cmd);
        return SELECT_CMD;
    } else if (!strncmp(cmd->args[0], "update", 6)) {
        handle_update_cmd(table, cmd);
        return SELECT_CMD;
    } else if (!strncmp(cmd->args[0], "delete", 6)) {
        handle_delete_cmd(table, cmd);
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
int handle_insert_cmd(Table_t *table, Command_t *cmd) {
    int ret = 0;
    User_t *user = command_to_User(cmd);
    if (user) {
        ret = add_User(table, user);
        if (ret > 0) {
            cmd->type = INSERT_CMD;
        }
    }
    return ret;
}

///
/// The return value is the number of rows select from table
/// If the select operation success, then change the input arg
/// `cmd->type` to SELECT_CMD
///
int handle_select_cmd(Table_t *table, Command_t *cmd) {
    cmd->type = SELECT_CMD;
    field_state_handler(cmd, 1);

    print_users(table, NULL, 0, cmd);
    return table->len;
}

int handle_update_cmd(Table_t *table, Command_t *cmd) {
    int ret = 0;
    set_state_handler(cmd, 3);
    update_users(table, NULL, 0, cmd);
    return ret;
}

int handle_delete_cmd(Table_t *table, Command_t *cmd) {
    int ret = 0;
    if (cmd->args_len > 3 && !strncmp(cmd->args[3], "where", 5))
        where_state_handler(cmd, 4);

    delete_users(table, NULL, 0, cmd);
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

