

#include <string.h>

#include "cli.h"


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


#ifdef __ENABLE_HARDCODE_LOGIN__
static uint8_t _cli_hardcode_login(char *id, char *pass)
{
    return (!strcmp(id, LOGIN_ID) && !strcmp(pass, LOGIN_PASSWD));
}
#endif


/*
 * Show the help messages of every command at this level.
 * 
 * If the first character of the help message is 0x01, this command is hidden
 * and will not be shown in help output. Otherwise, the help messages were
 * displayed and if the command length gets longer, the output will be shifted
 * too.
 */
static void _cli_do_show_help(cmd_t *cmd_p)
{
    cmd_t   *p  = cmd_p;
    int     min = 6;
    int     len;

    while (p->cmd != NULL) {
        /* hidden command */
        if (p->help && p->help[0] == 0x01) {
            p++;
            continue;
        }

        len = strlen(p->cmd);
        if (len > min)
            min = len;

        cli_puts(p->cmd);

        /* if there is no help message, skip display */
        if (p->help) {
            len = min - len;
            do {
                cb->put(' ');
            } while (len-- > 0);

            cb->put('-');
            cb->put(' ');
            cli_puts(p->help);
        }

        cb->put('\n');
        p++;
    }
}


/*
 * Parse tokens
 *
 * Note: the spaces in the line are modified to '\0'.
 */
static int _cli_line_to_tokens(char *line)
{
    char    *ptr; // current pointer to the line
    int     toks = 0;

    memset(cb->tok, 0, sizeof(cb->tok));

    ptr = line;

    while (*ptr != 0) {
        cb->tok[toks++] = ptr;
        while (*ptr != 0 && *ptr != ' ')
            ptr++;
        while (*ptr == ' ')
            *ptr++ = '\0';
    }

    return toks;
}


/*
 * Find the fully matched command.
 *
 * @note    Duplicated commands were not considered because of space and CPU
 *          resource limit. Programmers MUST be careful when design command
 *          table.
 */
static cmd_t *_cli_find_one_match(cmd_t *cmd_p, char *str)
{
    cmd_t *match = NULL;

    while (cmd_p->cmd != NULL && !match) {
        if (!strcmp(cmd_p->cmd, str))
            match = cmd_p;
        cmd_p++;
    }

    return match;
}



static void _cli_do_cmd_no_sub(uint8_t len, uint8_t i, cmd_t *cmd_p)
{
    if (cmd_p->fptr) {
        cmd_p->fptr(len - i, cb->tok[i + 1]);
    } else {
        cli_puts(cb->tok[i]);
        cli_puts(" not handled\n");
    }
}


static void _cli_do_cmd_no_token(uint8_t len, uint8_t i, cmd_t *cmd_p)
{
    if (cmd_p->fptr) {
        cmd_p->fptr(len - i, cb->tok[i + 1]);
    } else if (cmd_p->sub) {
        cli_puts("incomplete command, more options:\n");
        _cli_do_show_help(cmd_p->sub);
    } else {
        cli_puts(cb->tok[i]);
        cli_puts(" not handled\n");
    }
}


/*
 * Process the input 'line' and return when ended.
 *
 * Note: Use interative instead of recursive to avoid stack overflow.
 */
static void _cli_do_cmd(char *line)
{
    int    toks;
    int    i;
    cmd_t  *cmd_p = cb->cmd;

    toks = _cli_line_to_tokens(line);

    /* traverse command tree */
    for (i = 0; i < toks; i++) {
        /* help */
        if (!strcmp(cb->tok[i], "?")) {
            _cli_do_show_help(cmd_p);
            break;
        }

        /* find match command */
        cmd_p = _cli_find_one_match(cmd_p, cb->tok[i]);
        if (!cmd_p) {
            cli_puts(cb->tok[i]);
            cli_puts(" unknown command\n");
            break;
        }

        /* out of tokens */
        if (i == toks - 1) {
            _cli_do_cmd_no_token(toks, i, cmd_p);
            break;
        }

        /* there are remaining tokens but no sub commands */
        if (!cmd_p->sub) {
             _cli_do_cmd_no_sub(toks, i, cmd_p);
             break;
        }

        /* descend to next level */
        cmd_p = cmd_p->sub;
    }
}


static void _cli_putx(uint32_t hex, uint8_t shift)
{
#define HEX_MAX         "FFFFFFFF" /* longest hex */
#define HEX_BUF_MAX     sizeof(HEX_MAX)

    char        buf[HEX_BUF_MAX + 1];
    uint8_t     i;
    uint8_t     h;

    buf[HEX_BUF_MAX] = '\0';

    for (i = HEX_BUF_MAX - 1; i > 0; i--) {
        h = hex & 0xF;
        if (h > 9)
            h += shift - 48 - 10;

        buf[i] = h + 48;
        hex    = hex >> 4;

        if (!hex)
            break;
    }

    cli_puts(&buf[i]);
}


static uint8_t _cli_getline(char *prompt, char echo, char *buf, uint8_t max)
{
    int  i = 0;
    char c;

    cli_puts(prompt);

    while (1) {
        buf[i] = 0;
        c = cb->get();
        if (c == 3)
            return 0;
        if (c == '\n') {
            cb->put('\n');
            return 1;
        }
        if (i < max) {
            buf[i++] = c;
            cb->put(echo ? echo : c);
        }
    }
}


#if  __ENABLE_LOGIN__
/**
 * The login function.
 *
 * The login function returns only if the login has succeed. Otherwise, the
 * user is stucked in login loop.
 */
static void _cli_login(void)
{
    char    id   [MAX_ID + 1];
    char    pass [MAX_ID + 1];

    /* already logged in */
    if (cb->state) return;

    while (1) {
        if (!_cli_getline("login: ", 0, id, MAX_ID))
            continue;

        if (!_cli_getline("password: ", '*', pass, MAX_ID))
            continue;

        /* validate */
        if (cb->knock(id, pass)) {
            cb->state = 1;
        }

        cli_puts("login failed\n");
    }
}
#endif


/****************************************************************************
 *
 * API functions.
 *
 ****************************************************************************/


void cli_init(cli_t *cli)
{
    cb          = cli;
#ifdef __ENABLE_HARDCODE_LOGIN__
    cb->knock   = _cli_hardcode_login;
#endif
}


void cli_task(void)
{
    char line[65];

#if  __ENABLE_LOGIN__
    _cli_login();
#endif

    do {
        if (_cli_getline("$ ", 0, line, 64))
            _cli_do_cmd(line);
    } while (cb->state);
}


void cli_puts(char *s)
{
    do {
        cb->put(*s);
    } while (*s++);
}


uint8_t cli_logout(uint8_t len, char *param)
{
    cb->state = 0;
    cli_puts("logout\n");
    return 0;
}


void cli_putd(int dec)
{
#define DEC_MAX         "-2147483648" /* longest integer */
#define DEC_BUF_MAX     sizeof(DEC_MAX)

    char        buf[DEC_BUF_MAX + 1];
    uint8_t     negative;
    uint8_t     i;

    buf[DEC_BUF_MAX] = '\0';
    negative         = (dec < 0);

    for (i = DEC_BUF_MAX - 1; i >= 0; i--) {
        buf[i] = (dec % 10) + 0x30;
        dec    = dec / 10;

        if (!dec) break;
    }

    if (negative) {
        i--;
        buf[i] = '-';
    }

    cli_puts(&buf[i]);
}


void cli_putln(void)
{
    cli_puts("\n");
}


void cli_putsp(void)
{
    cli_puts(" ");
}

void cli_putX(uint32_t hex)
{
    _cli_putx(hex, 65);
}


void cli_putx(uint32_t hex)
{
    _cli_putx(hex, 97);
}


void cli_put0x(uint32_t hex)
{
    cli_puts("0x");
    _cli_putx(hex, 97);
}


void cli_put0X(uint32_t hex)
{
    cli_puts("0x");
    _cli_putx(hex, 65);
}


#ifndef __DISABLE_UT__

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

#endif /*  __DISABLE_UT__ */

