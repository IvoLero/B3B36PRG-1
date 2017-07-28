#include <stdio.h>
#include <time.h>
#include <stdlib.h>

void printA(int*array,int size){
	for(int i=0; i< size; i++){
		printf ("%d %d\n",i, *(array +i));
	}
	printf("\n");
}
int* fillA (int* array, int size){


	for (int i=0; i< size; i++){
		array [i] = rand();
	}
	return array;
	
}

int strl(char* str){
	int i =0;
	while (*(str +i) != '\0'){
		i++;
	}
	return i;
}
int* splitsize (char* str){
	int i =0;	
	int* splits = (int*) malloc(sizeof(int));
	splits[0] = 0;
	int arrcounter =1;
	
	while (*(str+i)!= '\0'){
		if (*(str +i) == ' '){
			arrcounter++;
			splits = (int*) realloc(splits, arrcounter * sizeof(int));
			splits[arrcounter-1]=i;

		}	
		i++;
	}
	splits = (int*) realloc(splits, (arrcounter+1) * sizeof(int));
	splits[arrcounter]=i+1;

	return splits;
}
void split (char*str){

	int* splits = splitsize(str);
		
		for (int i= 0; i < sizeof(splits)/sizeof(int) +1; i++){
			int start = splits[i];
			int end = splits[i+1];
		//	printf ("%d\n",start);
			
			for (int j=start+1; j< end; j++){
				if (i==0 && j==1){
					printf("%c", str[0]);
				}	
				printf ("%c", *(str +j));
			}
			printf("\n");
		}
}



int main(int argc, char*argv[]){
	/*int size;
	scanf("%d",&size);
	int array [size];
	srand(time(NULL));
	int* pointer = fillA(array,size);
	printA(pointer, size);
	*/


	//string part
	/*char* string = "Everybody likes PRG";
	int* sp = split(string);
	for (int i=0; i< sizeof(sp)/sizeof(int);i++){
		printf("%s\n",string +sp[i]+1);
	}*/
	
	if (argc >1){
		char* str = argv[1];
		printf("%s\n",str);
		printf("%d\n",strl(str));
		split(str);
		}

	return 0;
}
