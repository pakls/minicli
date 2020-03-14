
#ifndef __CLI_H__
#define __CLI_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
 *
 * Constants.
 *
 ****************************************************************************/


#ifndef __ENABLE_LOGIN__
/* allows external overwrite */
#define __ENABLE_LOGIN__        (1)
#endif


#define MAX_TOKENS              (20)


/****************************************************************************
 *
 * Types.
 *
 ****************************************************************************/


/**
 * The function pointer prototype to get a characeter from input source.
 *
 * Generally, standard getchar() can be used for this purpose. But other
 * implementation can also be used. For example, in semihosting mode, the
 * standard getchar() is replaced by debug adapter. But to make CLI work as
 * normal over telnet, a replacement getchar, which talks to telnet, can be
 * used.
 *
 * @note    The standard getch() returns int, which may return someothing
 *          other than a character from user. To reduce the CPU consumption
 *          of mini-CLI, the input source must be configured/written carefully
 *          to not return non-characters.
 */
typedef char    (*getch_fptr)(void);


/**
 * The function pointer prototype to put a characeter to an output target.
 *
 * Generally, standard putchar() can be used for this purpose. But other
 * implementation can also be used. For example, in semihosting mode, the
 * standard putchar() is replaced by debug adapter. But to make CLI work as
 * normal over telnet, a replacement putchar, which talks to telnet, can be
 * used.
 *
 * @note
 */
typedef void    (*putch_fptr)(char);


#if __ENABLE_LOGIN__
/**
 * If login is enabled and hardcode is not used. This is the callback function
 * that mini-CLI will call to authenicate the user.
 *
 * @retval  0   if validation of the combination of 'id' and 'pass' failed.
 *              other values if succeeded.
 */
typedef uint8_t (*knock_fptr)(char *id, char *pass);
#endif


/**
 * Function pointer type of CLI command handlers.
 *
 * All CLI handlers must follow this prototype. An example is cli_logout().
 *
 * @note    Currently, the return value of CLI handlers are ignore. However,
 *          to keep backward compatiblity, CLI handlers must return 0.
 *
 */
typedef uint8_t (*fp_t)(uint8_t len, char *param);


/**
 * Forward declare the type of cmd_t such that ancient compilers won't
 * complain.
 */
typedef struct cmd_s cmd_t;


struct cmd_s {
    char    *cmd;
    char    *help;
    fp_t    fptr;
    cmd_t   *sub;   ///< sub commands
};


typedef struct cli_s {
    uint8_t         state; ///< 0 if not logged in
    cmd_t           *cmd;
    getch_fptr      get;
    putch_fptr      put;
#if __ENABLE_LOGIN__
    knock_fptr      knock;
#endif
    char            *tok[MAX_TOKENS];
} cli_t;


/**************************************************************************** 
 *
 * Function prototypes.
 *
 ****************************************************************************/


/**
 * Initialization.
 */
void cli_init(cli_t *cli);


/**
 * The top-level function of the actual CLI.
 *
 * This function will never exit unless the user logged out.
 */
void cli_task(void);


/**
 * Print a text string.
 */
void cli_puts(char *s);


/**
 * The function that implements the logout function.
 *
 * @note    Implemented in mini-CLI to reduce the need for mini-CLI users
 *          (developers) because it is a must.
 */
uint8_t cli_logout(uint8_t len, char *param);


void cli_putc(char c);
void cli_putd(int dec);
void cli_putln(void);
void cli_putsp(void);
void cli_putX(uint32_t hex);
void cli_putx(uint32_t hex);
void cli_put0x(uint32_t hex);
void cli_put0X(uint32_t hex);


#ifdef __cplusplus
}
#endif

#endif /* __CLI_H__ */

