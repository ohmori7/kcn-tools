
#ifndef EL__VIEWER_TEXT_MARKS_H
#define EL__VIEWER_TEXT_MARKS_H

struct view_state;

void goto_mark(unsigned char mark, struct view_state *vs);
void set_mark(unsigned char mark, struct view_state *vs);

void free_marks(void);

#endif
