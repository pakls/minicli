#define TUPLE(_a,_b,_c,_d) (((_a)<<24) | ((_b)<<16) | ((_c)<<8) | ((_d)))

enum key_seq {
    KEY_ESC         = 27,
    KEY_DEL         = 127,
    KEY_UP          = TUPLE( 0 ,  0 , '[', 'A'),
    KEY_DN          = TUPLE( 0 ,  0 , '[', 'B'),
    KEY_RIGHT       = TUPLE( 0 ,  0 , '[', 'C'),
    KEY_LEFT        = TUPLE( 0 ,  0 , '[', 'D'),
    KEY_HOME        = TUPLE( 0 , '[', '1', '~'),
    KEY_INS         = TUPLE( 0 , '[', '2', '~'),
    KEY_DEL_2       = TUPLE( 0 , '[', '3', '~'),
    KEY_END         = TUPLE( 0 , '[', '4', '~'),
    KEY_PAGE_UP     = TUPLE( 0 , '[', '5', '~'),
    KEY_PAGE_DN     = TUPLE( 0 , '[', '6', '~'),
    KEY_F1_2        = TUPLE( 0 , '[', '1', 'P'),
    KEY_F2_2        = TUPLE( 0 , '[', '2', 'Q'),
    KEY_F3_2        = TUPLE( 0 , '[', '3', 'R'),
    KEY_F4_2        = TUPLE( 0 , '[', '4', 'S'),
    KEY_F1          = TUPLE('[', '1', '1', '~'),
    KEY_F2          = TUPLE('[', '1', '2', '~'),
    KEY_F3          = TUPLE('[', '1', '3', '~'),
    KEY_F4          = TUPLE('[', '1', '4', '~'),
    KEY_F5          = TUPLE('[', '1', '5', '~'),
    KEY_F6          = TUPLE('[', '1', '6', '~'),
    KEY_F7          = TUPLE('[', '1', '7', '~'),
    KEY_F8          = TUPLE('[', '1', '8', '~'),
    KEY_F9          = TUPLE('[', '2', '0', '~'),
    KEY_F10         = TUPLE('[', '2', '1', '~'),
    KEY_F11         = TUPLE('[', '2', '3', '~'),
    KEY_F12         = TUPLE('[', '2', '4', '~')
};

#define CURSOR_LEFT     TUPLE(KEY_ESC, '[', '1', 'D')
#define CURSOR_RIGHT    TUPLE(KEY_ESC, '[', '1', 'C')
#define SCREEN_CLEAR    TUPLE(KEY_ESC, '[', '2', 'J')
#define CURSOR_1_1      TUPLE(KEY_ESC, '[', ';', 'H')

#define cursor_move_left() cursor_move(CURSOR_LEFT)
#define cursor_move_right() cursor_move(CURSOR_RIGHT)

void cursor_move(uint32_t cursor_seq);

void term_clear(void);

