#include <stdio.h>
#include <math.h>

void leibnitz(int n);
void vypistabulky(int n);

int main (){
for (int i = 0; i < 10; i++){
	printf ("%d ", i);
}

int counter = 0;
do {
	printf ("%d", counter);
	++counter;
} while (counter < 10);

counter =0;
while (counter < 10){

	counter++;
	printf ("%d", counter);
}

leibnitz (1000 );
vypistabulky(100);


return 0;
}
void leibnitz (int n ){

double sum =0;
for (int i =0; i < n; i++){
        int c = (i%2==0)? 1 : -1;

        sum += (double) c/(2*i +1);
        printf ("%f\n", sum);
}
printf ("%lf",4*sum);
}
 

void vypistabulky(int n){
printf ("\n");
int counter =0;

for (int i =0; i < 10; i++){
	for (int j=0; j <10; j++){

	if (i==j || i == 9-j){
		printf ("%d",( i*10)+j);
	}
	if (j==9){
		printf ("\n");
	}
	else {
		printf ("  ");
}
}
}	
}
