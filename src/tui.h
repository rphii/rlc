#ifndef TUI_H

#include "err.h"

#define ERR_tui_colorprint_init(...) "failed enabling color prints"
ErrDecl tui_colorprint_init(void);

void tui_buffer_exit(void);
void tui_buffer_enter(void);

void tui_cursor_hide(void);
void tui_cursor_show(void);

int tui_getch(void);
void tui_clear(void);

#define TUI_H
#endif


