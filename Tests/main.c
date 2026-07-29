#include <stdio.h>
#include <TripleT_UI/TripleT_Window.h>

int main(void){
    int number = 2;
    double_int(&number);
    
    printf("Hello world, Number = %d\n", number);

    return 0;
}
