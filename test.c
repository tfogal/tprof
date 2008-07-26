#include <stdio.h>

void function();
int
main()
{
    puts("yay main!");
    function();
    puts("back into main.");
    return 0;
}
void
function()
{
    puts("function!");
}
