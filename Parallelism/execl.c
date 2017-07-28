#include <stdio.h>
#include <unistd.h>
int main ()
{
  int ret;
  printf("Start... execl /bin/ls\n");
  ret = execl ("/bin/echo", "echo", "Hello World", (char *)NULL);
  ret = execl ("/bin/ls", "ls", "-ll", (char *)0);
  printf("Chyba: navrat uz nebude\n");
  return 1;
}
