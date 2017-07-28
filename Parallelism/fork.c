#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
int main ()
{
  pid_t pid;
  int retcode;
  pid = fork();
  if (pid == 0) {
    printf("Potomek...\n");
    sleep(1);
    printf("Potomek skoncil\n");
    exit(99);
  } else if (pid < 0) {
    printf("Fork selhal\n");
    exit(2);
  } else {
    printf("Rodic zna PID potomka: %d...a ceka na jeho konec\n",pid);
    wait(&retcode);
    printf("Potomek skoncil s kodem: %d\n",WEXITSTATUS(retcode));
    printf("Rodic konci\n");
    exit(3);
  }
  exit(1);
}
