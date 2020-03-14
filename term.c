
#include <stdint.h>

#include "cli.h"
#include "term.h"

void cursor_move(uint32_t cursor_seq)
{
    for (int i = 24; i >= 0; i -= 8)
        cli_putc((cursor_seq >> i) & 0xFF);
}

