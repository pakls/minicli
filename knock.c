
#include <string.h>

#include "knock.h"

/*
 * pretend there is a user '1' and passwod is '1'
 */
uint8_t knock(char *id, char *pass)
{
    if (!strcmp(id, "1") && !strcmp(pass, "1"))
        return 1;

    return 0;
}


