#ifndef SELECT_STATE_H
#define SELECT_STATE_H

#include "Command.h"

void field_state_handler(Command_t *cmd, int arg_idx);
void table_state_handler(Command_t *cmd, int arg_idx);
void offset_state_handler(Command_t *cmd, int arg_idx);
void limit_state_handler(Command_t *cmd, int arg_idx);
void join_state_handler(Command_t *cmd, int arg_idx);


#endif
