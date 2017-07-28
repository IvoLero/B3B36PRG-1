#include <stdio.h>
#include <unistd.h>

int main (int argc, char *argv[]){
	
	int i=2;
	int c;
	int a = 10;
	printf("%lu %lu\n", sizeof(a), sizeof(a + 1.0f));
	for (c = 1, i = 0; i < 3; i++, c += 2) {
printf("i: %d c: %d\n", i, c);
}
	printf ("%d\n",++i);
	printf ("%d\n",i--);
	i = sizeof(int);
	printf ("int size %d\n",i);
	i = sizeof (char);
	printf ("char size %d\n",i);
	i = sizeof (double);
	printf ("double size%d\n",i);
	fprintf(stdout, "FUCK\n");
	printf("%ld\n", sysconf(_SC_PAGESIZE));
	 a = 24;
	printf ("%d\n", a>>2);
	return argc;

}
