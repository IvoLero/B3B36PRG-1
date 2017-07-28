#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define KEY 1000
#define SIZE 256
#define EXIT_MSG 10

struct msgbuf {
   long mtype;
   char mtext[SIZE];
};

int main(int argc, char *argv[])
{
   struct msgbuf msg;
   int id = msgget(KEY, 0);
   int r;
   int i;
   if (id != -1) {
      msg.mtype = 3;
      printf("Inform main process\n");
      strcpy(msg.mtext, "I'm here, ready to work");
      r = msgsnd(id, &msg, sizeof(msg.mtext), 0);
      printf("Wait for work\n");
      r = msgrcv(id, &msg, sizeof(msg.mtext), 4, 0); //work msg is type 4
      printf("Received message:%s\n", msg.mtext);
      for (i = 0; i < 4; i++) {
	 sleep(1);
	 printf("."); 
	 fflush(stdout);
      }
      printf("done\n");
      printf("Work done, send wait for exit\n");
      strcpy(msg.mtext, "I'm going to wait for exit msg");
      msg.mtype = 5;
      r = msgsnd(id, &msg, sizeof(msg.mtext), 0);
      msg.mtype = 10;
      printf("Wait for exit msg\n");
      r = msgrcv(id, &msg, sizeof(msg.mtext), EXIT_MSG, 0);
      printf("Exit message has been received\n");
   }
   return 0;
}
