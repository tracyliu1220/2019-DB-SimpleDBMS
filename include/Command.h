#ifndef COMMAND_H
#define COMMAND_H

enum { 
    UNRECOG_CMD,
    BUILT_IN_CMD,
    QUERY_CMD,
};

enum {
    INSERT_CMD = 100,
    SELECT_CMD,
};

typedef struct {
    char name[256];
    int len;
    unsigned char type;
} CMD_t;

extern CMD_t cmd_list[];

typedef struct SelectArgs {
    char **fields;
    size_t fields_len;
    int offset;
    int limit;
} SelectArgs_t;

typedef union {
    SelectArgs_t sel_args;
} CmdArg_t;

typedef struct WhereArgs {
    int up;
    int type; // 0: 1, 1: and, 2: or
    int str_cnt, int_cnt;
    char **str_con;
    char **int_con_left;
    int int_con[2];
    int str_logic[2];
    int int_logic[2];
} WhereArgs_t;

typedef struct SetArgs {
    int type; // 0: int, 1: str
    char *field;
    int set_int;
    char *set_str;
} SetArgs_t;

typedef struct AggreArgs {
    int up;
    int idsum_up, idavg_up, agesum_up, ageavg_up;
    int idsum_result, agesum_result;
    double idavg_result, ageavg_result;
    int cnt_result;
} AggreArgs_t;

typedef struct Command {
    unsigned char type;
    char **args;
    size_t args_len;
    size_t args_cap;
    CmdArg_t cmd_args;
    WhereArgs_t where_args;
    SetArgs_t set_args;
    AggreArgs_t aggre_args;
} Command_t;

Command_t* new_Command();
int add_Arg(Command_t *cmd, const char *arg);
int add_select_field(Command_t *cmd, const char *argument);
void cleanup_Command(Command_t *cmd);

#endif
