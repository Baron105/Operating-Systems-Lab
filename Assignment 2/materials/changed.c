#include<stdio.h>
#include<signal.h>

void abc();
int main()
{
    // signal(SIGINT,SIG_IGN); ignores ctrl c
    signal(SIGINT,abc); // changes default code with abcs code
    for(;;);
}
void abc()
{
    printf("You have pressed Ctrl-C\n");
}