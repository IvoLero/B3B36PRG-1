#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int const range = 1000000;
int * sieveofErastoten();
void manage_output(int *sieve,long num);
void manage_output2 (int *sieve, int size);
int  divide (int divider, int end);
void printA(int*arr, int length);

int length =0;
int * rest;
int * num ;

int main(int argc, char* argv []){
	long nm;
	int* sieve =sieveofErastoten();
	num = malloc(sizeof(int) * 100);
	rest = malloc (sizeof (int) *100);	

	while (1){
	   if (argc ==1){
		if (scanf("%ld",&nm) !=1 || nm < 0){
		fprintf (stderr,"Error: Chybny vstup!\n");
		
		free(sieve);
		free(rest);
		free(num);
		return 100;
		}
		else if (nm==0){
		free(sieve);
		free(rest);
		free(num);
		return 0;
		}
		else{
		manage_output(sieve,nm);
		}
	}
	 else if (argc ==2 && !strcmp(argv[1],"-prg-optional")){
		int pointer = 0;
		int chr;
			for (int i =0; i < 100; i++){
			num[i] = rest[i] = 0;
		}

		while ((chr = getchar())!= '\n' && chr !=EOF){
			*(num +pointer)=chr-'0';
			pointer++;
		}
		if (*num !=0){
				length = pointer;
				manage_output2(sieve, pointer);
				}
		else{
			free(sieve);
			free(rest);
			free(num);
			return 0;
		}
	}
	else{
		return 0;
	}
    }		

}

void manage_output2 (int *sieve, int size){
	int u;
	printf("Prvociselny rozklad cisla ");
	printA(num,length);
	printf(" je:\n");
	for (int i =2; i < range; i++){
		if (sieve[i] ==0 &&(u=divide(i,length))!= 0){

                        int times =0; 
                        while (u!=0){
				times++;
				for (int j =0; j < length; j++){
					num [j] = rest[j];
				}	 
				u = divide(i,length);
                        } 
			printf ("%d",i);
			if(times !=1){  
                              	 
                                printf ("^%d",times); 
                        } 
                        if (length ==1 && num [0] ==1){ 
                                printf ("\n"); 
                                break;   
                        }
                        else{
                                printf (" x ");
                        }
 
                }
		}
}

void printA (int *arr, int length){
	for (int i=0; i < length; i++){
		printf ("%d", arr[i]);
	}
}

int divide ( int divider, int end){
//	printf ("deleni cislem %d cisla ",divider);
//	printA(num,length);
	int n=0;
	int pointer =0;
	int pointer2 =0;
	while (divider > n && pointer <end){
		n*=10;
	//	printA(num,length);
		n += num[pointer];
		pointer++;
	}
	rest [pointer2] = n/divider;
	n %=divider;
	pointer2++;
	while (pointer< end){
		n*=10;
		n+=num[pointer];
		rest [pointer2]= n/divider;
		n%=divider;
		pointer++;
		pointer2++;
	}
//	printf ("zbytek: %d\n",n);
	if (n==0){
		length = pointer2;
		return 1;
		
	}
	else{
		return 0 ; 
	}
	printf ("\n");

}

void manage_output(int * sieve, long number){
	long factor = number;
	
	printf("Prvociselny rozklad cisla %ld je:\n", number);
	if (number==1){
	 	printf ("1\n");
	}
	else{
	

	int pointer = 2;
	while (factor !=1){
		if (sieve [pointer]==0 && factor%pointer ==0){
			int times =0;
			while (factor%pointer==0){
				factor /=pointer;
				times++;
			}
			if(times ==1){ 
				printf ("%d",pointer);
			}
			else{
				printf ("%d^%d",pointer,times);
			}
			if (factor==1){
				printf ("\n");
				break;	
			}
			else{
				printf (" x ");
			}
						
		}
		pointer++;
		if (pointer == range){
			printf ("%ld\n", factor);
			break;
		}
	
		
	}
	}
}

int *sieveofErastoten(){
	int* sieve=malloc(sizeof(int)*range);
	for (int i =0; i < range; i++){
		*(sieve +i)=0;
	}
	sieve [0]= 1;
	sieve[1]=1;
	int const sqrt = 1000;
	for (int i =2; i <= sqrt; i++){
		if (sieve [i] ==0){
		for (int j = 2*i; j <range; j+=i){
			sieve [j] = 1;
		}
		}	
	}
	return sieve;
}
