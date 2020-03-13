

#include <string.h>

#include "cli.c"

/****************************************************************************
 *
 * Configurations.
 *
 ****************************************************************************/


#if __ENABLE_LOGIN__
/* enable hardcode login credentials by default */
#define __ENABLE_HARDCODE_LOGIN__
#endif


#ifdef __ENABLE_REAL_KNOCK__
#ifdef __ENABLE_HARDCODE_LOGIN__
/* disable hardcode login if a real authentication exists */
#undef __ENABLE_HARDCODE_LOGIN__
#endif
#endif


/****************************************************************************
 *
 * Constants.
 *
 ****************************************************************************/


#if __ENABLE_LOGIN__
#define MAX_ID          (16)
#endif


#ifdef __ENABLE_HARDCODE_LOGIN__
#define LOGIN_ID        "a"
#define LOGIN_PASSWD    "a"
#endif


/****************************************************************************
 *
 * Types.
 *
 ****************************************************************************/


/****************************************************************************
 *
 * Static variables.
 *
 ****************************************************************************/


/**
 * The only static variable in the mini-CLI.
 */
static cli_t        *cb;


/****************************************************************************
 *
 * Local functions.
 *
 ****************************************************************************/


/****************************************************************************
 *
 * Test Program
 *
 ****************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "io.h"
#if __ENABLE_LOGIN__
#include "knock.h"
#endif

typedef uint8_t (*test_fptr)(cli_t *);

uint8_t ls_example_a(uint8_t len, char *param)
{
    printf("ls -a called\n");
    return 0;
}

uint8_t ls_example(uint8_t len, char *param)
{
    printf("ls called\n");
    return 0;
}

/****************************************************************************/

/* case 1 */

static cmd_t   set_1[] =
{
    { "ls",           "list",     ls_example },
    { "lo",           "logout",   cli_logout },
    { "lon",          "longer",   cli_logout },
    { "ok",           "shorter",  cli_logout },
    { "long",         "longer",   cli_logout },
    { "ok",           "shorter",  cli_logout },
    { "longlongtime", "longer",   cli_logout },
    { "ok",           "shorter",  cli_logout },
    { NULL }
};

static cli_t   cnf_1 =
{
    .state = 0,
#if __ENABLE_LOGIN__
    .knock = knock,
#endif
    .get   = getch,
    .put   = putch,
    .cmd   = &set_1[0]
};

static uint8_t test_1(cli_t *cb)
{
    cli_init(cb);
    _cli_do_cmd("?");
    return 0;
}

/****************************************************************************/

/* case 2 */

static cmd_t   set_2[] =
{
    { "ls",           "list",     ls_example },
    { "lo",           "logout",   cli_logout },
    { NULL }
};


static cli_t   cnf_2 =
{
    .state = 1, // set to 1 to skip login
#if __ENABLE_LOGIN__
    .knock = knock,
#endif
    .get   = getch,
    .put   = putch,
    .cmd   = &set_2[0]
};


static uint8_t test_2(cli_t *cb)
{
    cli_init(cb);
    cli_task();
    return 0;
}


/****************************************************************************/

/* case 3 */

static cmd_t   set_3_1_1[] =
{
    { "-a",           "all",       ls_example_a },
    { NULL }
};

static cmd_t   set_3_1[] =
{
    { "-a",           "all",       ls_example_a },
    { "-l",           "long",      NULL },         // not handled
    { "-r",           "recursive", NULL,  set_3_1_1 },
    { NULL }
};

static cmd_t   set_3[] =
{
    { "ls",           "list",     ls_example, set_3_1 },
    { "lo",           "logout",   cli_logout },
    { NULL }
};

static cli_t   cnf_3 =
{
    .state = 1, // set to 1 to skip login
#if __ENABLE_LOGIN__
    .knock = knock,
#endif
    .get   = getch,
    .put   = putch,
    .cmd   = &set_3[0]
};


static uint8_t test_3(cli_t *cb)
{
    cli_init(cb);
    cli_task();
    return 0;
}


/****************************************************************************/



/****************************************************************************/

struct case_t {
    char        *name;
    cli_t       *conf;
    char        *desc;
    test_fptr   fptr;
} all[] = {
    { "name_len", &cnf_1, "description indentation test", test_1 },
    { "run",      &cnf_2, "logout command test",          test_2 },
    { "tokens",   &cnf_3, "token handling",               test_3 },
};

/****************************************************************************/

int main(int argc, char *argv[])
{
    int ret = 0;
    int i   = 1;
    int j;

    while (i < argc && !ret) {
        for (j = 0; j < sizeof(all) / sizeof(struct case_t); j++) {
            if (!strcmp(argv[i], all[j].name)) {
                printf("****\n");
                printf("Run case %d: %s - %s\n", j, all[j].name, all[j].desc);
                printf("****\n");
                ret = all[j].fptr(all[j].conf);
                break;
            }
        }

        if (j == sizeof(all) / sizeof(struct case_t)) {
            printf("requested case: %s not found\n", argv[i]);
            ret = 1;
        }

        i++;
    }

    return ret;
}

