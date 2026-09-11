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
}TripleT_Window_Properties;

typedef struct TripleT_Configure_t{
    int new_x, new_y;
    int new_width, new_height;
}TripleT_Configure;

extern TripleT_Window *t3_create_window(const TripleT_Window_Properties t3_properties);
extern int t3_get_main_screen_width(void);
extern int t3_get_main_screen_height(void);
extern void t3_print_window_properties(const TripleT_Window *t3_window);
extern void t3_resize_window(TripleT_Window *t3_window, const TripleT_Configure configure);
extern void t3_destroy_window(TripleT_Window *t3_window);

#endif
