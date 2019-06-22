#include <string.h>
#include <stdlib.h>
#include "Command.h"
#include "SelectState.h"
#include "User.h"
#include "stdio.h"

void where_state_handler(Command_t *cmd, int arg_idx) {
	cmd->where_args.str_con = (char **) malloc(sizeof(char*) * 5);
	cmd->where_args.int_con_left = (char **) malloc(sizeof(char*) * 5);
	cmd->where_args.type = 0;
	cmd->where_args.str_cnt = 0;
	cmd->where_args.int_cnt = 0;
    cmd->where_args.up = 1;
    for (int cases = 0; cases < 2; cases ++) {
    	    char *src1 = cmd->args[arg_idx];
    	    char *src2 = cmd->args[arg_idx + 2];
    	    char *logic = cmd->args[arg_idx + 1];
    	    int logic_mark = 0;
    	    int str_cnt = cmd->where_args.str_cnt;
    	    int int_cnt = cmd->where_args.int_cnt;
    	    int if_str = 0;

    	    // set type;
    	    if (!strncmp(src1, "name", 4)
    	    	|| !strncmp(src1, "email", 5)
    	    	|| !strncmp(src2, "name", 4)
    	    	|| !strncmp(src2, "email", 5) )
    	    	if_str = 1;

    	    if (if_str) {
    	    	cmd->where_args.str_con[str_cnt * 2] = src1;
    	    	cmd->where_args.str_con[str_cnt * 2 + 1] = src2;
                
                // set logic
                if (!strncmp(logic, "=", 1)) logic_mark = 0;
                else if (!strncmp(logic, "!=", 2)) logic_mark = 1;

                cmd->where_args.str_logic[str_cnt] = logic_mark;
    	    	cmd->where_args.str_cnt ++;
    	    } else {
    	    	cmd->where_args.int_con_left[int_cnt] = src1;
    	    	cmd->where_args.int_con[int_cnt] = atoi(src2);
                
                // set logic
                if (!strncmp(logic, "=", 1)) logic_mark = 0;
                else if (!strncmp(logic, "!=", 2)) logic_mark = 1;
                else if (!strncmp(logic, ">=", 2)) logic_mark = 4;
                else if (!strncmp(logic, "<=", 2)) logic_mark = 5;
                else if (!strncmp(logic, ">", 1))  logic_mark = 2;
                else if (!strncmp(logic, "<", 1))  logic_mark = 3;

                cmd->where_args.int_logic[int_cnt] = logic_mark;
    	    	cmd->where_args.int_cnt ++;
    	    }
        if (arg_idx + 3 >= cmd->args_len) {
            break;
        } else if (!strncmp(cmd->args[arg_idx + 3], "and", 3)) {
        	cmd->where_args.type = 1;
        } else if (!strncmp(cmd->args[arg_idx + 3], "or", 2)) {
        	cmd->where_args.type = 2;
        } else {
            arg_idx += 3;
        	break;
        }
        arg_idx += 4;
    }
    /*
    for (int i = 0; i < cmd->where_args.str_cnt; i ++) {
    	printf("%s %s", cmd->where_args.str_con[i * 2], cmd->where_args.str_con[i * 2 + 1]);
        printf(" %d\n", cmd->where_args.str_logic[i]);
    }
    for (int i = 0; i < cmd->where_args.int_cnt; i ++) {
    	printf("%s %d", cmd->where_args.int_con_left[i], cmd->where_args.int_con[i]);
        printf(" %d\n", cmd->where_args.int_logic[i]);
    }
    */
    if (arg_idx == cmd->args_len) {
        return;
    } else if (!strncmp(cmd->args[arg_idx], "offset", 6)) {
        offset_state_handler(cmd, arg_idx+1);
        return;
    } else if (!strncmp(cmd->args[arg_idx], "limit", 5)) {
        limit_state_handler(cmd, arg_idx+1);
        return;
    }
}

void set_state_handler(Command_t *cmd, int arg_idx) {
	int set_type = 0;
	if (!strncmp(cmd->args[arg_idx], "name", 4)
		|| !strncmp(cmd->args[arg_idx], "email", 5))
		set_type = cmd->set_args.type = 1;
	cmd->set_args.field = cmd->args[arg_idx];
	if (set_type) { // str
		cmd->set_args.set_str = cmd->args[arg_idx + 2];
	} else { // int
		cmd->set_args.set_int = atoi(cmd->args[arg_idx + 2]);
	}
	arg_idx += 3;
	if (arg_idx == cmd->args_len) {
        return;
    } else if (!strncmp(cmd->args[arg_idx], "where", 5)) {
        where_state_handler(cmd, arg_idx+1);
        return;
    }
}

// true or false
int where_test(Command_t *cmd, User_t *user) {
	int ret, if_and;
	if (cmd->where_args.type == 1) {
		if_and = 1;
		ret = 1;
	} else {
		if_and = 0;
		ret = 0;
	}
    int ans;
	for (int i = 0; i < cmd->where_args.str_cnt; i ++) {
        char *src1;
        if (!strncmp(cmd->where_args.str_con[i * 2], "name", 4))  src1 = user->name;
        else src1 = user->email;
        char *src2 = cmd->where_args.str_con[i * 2 + 1];
		ans = 0;
        int logic = cmd->where_args.str_logic[i];
		if (logic == 0 && (!strcmp(src1, src2)) )
			ans = 1;
		else if (logic == 1 && (strcmp(src1, src2)) )
			ans = 1;
		if (if_and) ret = ret && ans;
		else ret = ret || ans;
	}
	for (int i = 0; i < cmd->where_args.int_cnt; i ++) {
        int src1;
        if (!strncmp(cmd->where_args.int_con_left[i], "id", 2))  src1 = user->id;
        else src1 = user->age;
        int src2 = cmd->where_args.int_con[i];
        int logic = cmd->where_args.int_logic[i];
		ans = 0;

        if (logic == 0 && src1 == src2) ans = 1;
        else if (logic == 1 && src1 != src2) ans = 1;
        else if (logic == 2 && src1 > src2) ans = 1;
        else if (logic == 3 && src1 < src2) ans = 1;
        else if (logic == 4 && src1 >= src2) ans = 1;
        else if (logic == 5 && src1 <= src2) ans = 1;

		if (if_and) ret = ret && ans;
		else ret = ret || ans;
	}
    return ret;
}
