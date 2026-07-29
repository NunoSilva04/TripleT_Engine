#ifndef __TRIPLE_T_WINDOW_H__
#define __TRIPLE_T_WINDOW_H__

#define MAX_TRIPLE_T_WINDOW_NAME 64

typedef struct TripleT_Window_t TripleT_Window;

typedef struct{
    char window_name[MAX_TRIPLE_T_WINDOW_NAME];
    int x, y;
    unsigned int width, height;
    unsigned int border_color, border_width;
    unsigned int background_color;
}Triple_T_Window_Properties;

TripleT_Window *create_window(const Triple_T_Window_Properties t3_properties);
void destroy_window(TripleT_Window *t3_window);

#endif
