#include <string.h>
#include <stdlib.h>
#include "User.h"
#include "Command.h"

///
/// Allocate new space for User_t
/// The caller should free the allocated space
///
User_t* new_User() {
    User_t *new_user = (User_t*)malloc(sizeof(User_t));
    new_user->id = 0;
    memset(new_user->name, 0, MAX_USER_NAME+1);
    memset(new_user->email, 0, MAX_USER_EMAIL+1);
    new_user->age = 0;
    return new_user;
}

Like_t* new_Like() {
    Like_t *new_like = (Like_t*)malloc(sizeof(Like_t));
    new_like->id1 = 0;
    new_like->id2 = 0;
    return new_like;
}

///
/// Transform from the input cmd to the User_t
///
User_t* command_to_User(Command_t *cmd) {
    User_t *user = new_User();
    if (!user || !cmd) {
        return NULL;
    }
    if (cmd->args_len != 7) {
        return NULL;
    }
    user->id = atoi(cmd->args[3]);
    strncpy(user->name, cmd->args[4], MAX_USER_NAME);
    strncpy(user->email, cmd->args[5], MAX_USER_EMAIL);
    user->age = atoi(cmd->args[6]);
    return user;
}

Like_t* command_to_Like(Command_t *cmd) {
    Like_t *like = new_Like();
    if (!like || !cmd) {
        return NULL;
    }
    if (cmd->args_len != 5) {
        return NULL;
    }
    like->id1 = atoi(cmd->args[3]);
    like->id2 = atoi(cmd->args[4]);
    return like;
}

