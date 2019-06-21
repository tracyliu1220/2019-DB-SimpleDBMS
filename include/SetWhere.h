#ifndef SETWHERE_H
#define SETWHERE_H

#include "Command.h"
#include "User.h"
#include "stdio.h"

void where_state_handler(Command_t *cmd, size_t arg_idx);

void set_state_handler(Command_t *cmd, size_t arg_idx);

// true or false
int where_test(Command_t *cmd, User_t *user);

#endif
