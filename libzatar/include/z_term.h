#ifndef Z_TERM_H
#define Z_TERM_H

void z_term_clear_screen(void);
void z_term_clear_line(void);

void z_term_enable_line_wrap(void);
void z_term_disable_line_wrap(void);

void z_term_enter_alternative_screen(void);
void z_term_exit_alternative_screen(void);

void z_term_hide_cursor(void);
void z_term_show_cursor(void);

void z_term_set_cursor_position(int x, int y);
void z_term_set_cursor_horizontal_position(int x);

void z_term_move_cursor_up(int n);
void z_term_move_cursor_down(int n);
void z_term_move_cursor_right(int n);
void z_term_move_cursor_left(int n);

#endif
