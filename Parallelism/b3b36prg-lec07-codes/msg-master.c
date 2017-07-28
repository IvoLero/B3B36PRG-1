#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdio.h>

#define KEY 1000
#define SIZE 256
#define EXIT_MSG 10

#include <errno.h>
struct msgbuf { 
   long mtype;
   char mtext[SIZE];
};

int main(int argc, char *argv[]) 
{
   struct msgbuf msg;
   int id = msgget(KEY, IPC_CREAT | 0666);
   int r;
   if (id != -1) {
      msg.mtype = 3; //type must be > 0
      printf("Wait for other process\n");
      r = msgrcv(id, &msg, sizeof(msg.mtext), 3, 0);
      printf("Slave msg received, press ENTER to send work msg\n");
      getchar();
      strcpy(msg.mtext, "Do work");
      msg.mtype = 4; //work msg is type 4
      r = msgsnd(id, &msg, sizeof(msg.mtext), 0);
      fprintf(stderr, "msgsnd r:%d\n",r);
      printf("Wait for receive work results\n");
      msg.mtype = 5;
      r = msgrcv(id, &msg, sizeof(msg.mtext), 5, 0);
      printf("Received message:%s\n", msg.mtext);
      printf("Press ENTER to send exit msg\n");
      getchar();
      msg.mtype = EXIT_MSG; //I choose type 10 as exit msg
      r = msgsnd(id, &msg, 0, 0);
   }
   return 0;
}
