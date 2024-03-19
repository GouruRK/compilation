#include <inttypes.h>
#include <stdio.h>
void my_putchar(int64_t);
char* my_getchar();
void my_putint(int64_t);

int main(){
    my_putchar('p');
    my_putchar('\n');
    printf("%s", my_getchar());
    printf("\n");
    my_putint(20);
    my_putchar('\n');
}