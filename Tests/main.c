#include <TripleT_UI/TripleT_Window.h>
#include <string.h>
#include <stdio.h>

int main(void){
    Triple_T_Window_Properties properties = {0};
    strcpy(properties.window_name, "First Window");
    properties.x = 0;
    properties.y = 0;
    properties.width = 800;
    properties.height = 600;
    properties.border_width = 30;
    properties.border_color = 0;
    properties.background_color = 0xFFFFFFFF;

    TripleT_Window *t3_window = create_window(properties);
    int number = 0;
    for(int i = 0; i < 1000000000; i++)
	number++;
    printf("Done\n");

    while(1){
    }

    destroy_window(t3_window);
    return 0;
}
