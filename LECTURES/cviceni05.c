#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>


int clean_stdin(){
	while(getchar()!='\n');
	return 1;
}

double getRange(double* array, int n){
	double sum =0;

	for (int i=0;i<n;i++){
		sum+=array[i];
	}

	double average = sum/n;
	
	sum =0;
	for (int i=0; i< n;i++){
		sum += ((array[i]-average)*(array[i]-average));	
	}
//	printf("%lf", sum);
//	printf("%d",n);

	return sqrt(sum/n);
}
void fillArray(double* array, int n, int range){
	srand(time(NULL));
	for (int i=0; i < n; i++){
		array[i]= (double)rand()/(double)RAND_MAX * range;
	}
	
	
}
void printfloatArray(double* array, int n){
	for (int i=0;i <n; i++){
		printf("%.3lf ",array[i]);
	}
	printf("\n");
}

int main(){
		
	double* array = (double*) malloc(sizeof(double));
	double* sets [5];
	
	int length = 0;
	double num;
	int retval= 0;	  

	do{
		retval = scanf("%lf",&num);
		if (retval ==1){
			array[length] = num;
			length++;
			array= (double*) realloc(array,sizeof(double)*(length+1));
			
		}
		printf ("%d",retval);
		clean_stdin();
	}while (retval !=-1);
	
	printfloatArray(array,length);
	printf ("%lf\n", getRange(array,length));
	fillArray(array,length,100);
	printfloatArray(array,length);
	return 0;
}


