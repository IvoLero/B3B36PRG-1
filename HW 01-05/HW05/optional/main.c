#include <stdio.h>
#include <stdlib.h>

typedef struct {
	int rows;
	int columns;
	int**data;
} matrix;

void clear (matrix* M){
	for(int i =0; i< M->rows; i++){
		free(*(M->data +i));
	}
	free(M->data);
}

void copy(matrix*M, matrix*copy){
	copy->rows = M->rows;
	copy->columns=M->columns;
	int** rows = (int**)malloc(sizeof(int*) * M->rows);
	for(int i= 0; i< M->rows;i++){
		int* row = (int*)malloc(sizeof(int)* M->columns);
		for (int j=0;j<M->columns; j++){
			row[j]=M->data[i][j];
		}
		rows[i]=row;
	}
	copy->data = rows; 
}

void add (matrix* A, matrix*B){
	for (int i=0; i< A->rows; i++){
		for (int j=0; j<A->columns;j++){
			//*(*(A->data+i)+j)+= *(*(B->data +i)+j);
			A->data[i][j]+= B->data [i][j];
		}
	
	}
	clear(B);
	
}
void sub (matrix* A, matrix*B){
	for (int i=0; i< A->rows; i++){
		for (int j=0; j<A->columns;j++){
			*(*(A->data+i)+j)-= *(*(B->data +i)+j);
		}
	}
	clear(B);
}

int mult (matrix* A, matrix*B, matrix* C){

		
	if (A->columns == B-> rows){
		C->rows = A->rows;
		C->columns = B->columns;
		int **rows = (int**) malloc(sizeof(int*)* C->rows);
		for(int i =0; i< C->rows; i++){
			rows[i]= (int*) malloc(sizeof(int)* C->columns);
		}
		C->data = rows;
		for (int i=0; i < A->rows; i++){
			for (int k =0 ; k < B->columns; k++){
				 int sum = 0;
				 for(int j=0; j< A->columns; j++){
					sum+= A->data[i][j] * B->data[j][k];
				}
				//printf("%d", sum);
				C->data[i][k]=sum;
			}
		}
		clear(A);
		clear(B);
		return 1;
	}
	else{
		clear (A);
		clear(B);
		return 100;
	}
}


void printmatrix2(matrix*M){
	printf("[");
	
	for(int i= 0; i< M->rows;i++){
		if (i!=0) printf(" ");
		for (int j=0;j<M->columns; j++){
			
			printf("%d", M->data[i][j]);
			if (j!= M->columns-1)
				printf(" ");
		}
		if (i!=M->rows-1) printf(";");
		
	}
	printf("]\n");
	
}

void readinput(matrix* matrixarray){
	char descriptor=getchar();
	do {
		int rowcount =0;
		int columncount =0;
		
			//skips =[ 
		getchar();		
		int** rows = (int**)malloc(sizeof(int*)) ;	
			do {//row read
				getchar();	
				int counter=0;
				int num;
			
				int* row = (int*) malloc(sizeof(int));		
				while (scanf("%d", &num)==1){
					row[counter] = num;
					counter++;
					row = (int*) realloc(row, sizeof(int)* (counter+1));
				}
				columncount = counter;
				rows[rowcount] = row;
				rowcount++;
				rows = (int**)realloc(rows,sizeof(int*) * (rowcount+1));			
			
			}while (getchar() != ']'); 

			
		matrixarray[descriptor-'A'].rows = rowcount;  
		matrixarray[descriptor-'A'].columns = columncount;
		matrixarray[descriptor-'A'].data=rows;
		//printmatrix(&matrixarray[descriptor -'A']);
	//skip new line char
		getchar();
		descriptor=getchar();
	} while(descriptor != '\n');
	
}

int main(int argc, char* argv[]){ 
	
	matrix M = {0,0,NULL}, A, B, C, matrixproduct;	
	char previousoperator,operator;
	char descriptor;

	
	matrix matrixarray[26];
	for (int i=0;i<26;i++){
		matrixarray[i] = M;
	}
	readinput(matrixarray); 
	//TEST ARRAY ACCESS
	//printmatrix(&matrixarray[1]);
	
	
	descriptor = getchar();
	copy(&matrixarray[descriptor -'A'],&A);
	
	//TEST COPY
	//printmatrix2(&A);
	previousoperator = getchar();
	
	while (1==1){
		
		descriptor =getchar();
		copy (&matrixarray[descriptor-'A'], &B);
		operator = getchar();
		//control end of input
		if (operator ==EOF || operator == '\n')goto exit;		
		if (operator == '*' && previousoperator=='*'){
				//multiply
				if (mult(&A,&B,&matrixproduct)!=1){fprintf(stderr,"Error: Chybny vstup!\n");clear(&A);return 100;}
				A = matrixproduct;
			//	printmatrix(&A);

		}
		else if (operator== '*' && (previousoperator== '+' || previousoperator=='-')){
				
				do {
					descriptor = getchar();
					copy (&matrixarray[descriptor-'A'], &C);
					//multiply
					if (mult(&B,&C,&matrixproduct)!=1){fprintf(stderr,"Error: Chybny vstup!\n");return 100;}
					B=matrixproduct;
				//	printmatrix2(&B);
					
					//control end of input
					operator = getchar();
					if (operator==EOF || operator=='\n') goto exit;
					
					
				} while (operator=='*');
				
				if (previousoperator == '+')
					add(&A,&B);
				else if (previousoperator =='-')
					sub (&A,&B);

				//printmatrix(&A);
				previousoperator = operator;
				
		}
		

 

		else if (operator=='+' && previousoperator=='+'){
				add(&A,&B);
				//printmatrix(&A);
		}
		else if (operator== '+' && previousoperator=='-'){
				sub(&A,&B);
				previousoperator = operator;
		}

		else if (operator=='+' && previousoperator=='*'){
				//multiply
				if (mult(&A,&B,&matrixproduct)!=1){fprintf(stderr,"Error: Chybny vstup!\n");return 100;}
				A = matrixproduct;
				//printmatrix(&A);
				previousoperator = operator;
		}
		else if (operator== '-' && previousoperator=='+'){
				add(&A,&B);
			//	printmatrix(&A);
				previousoperator=operator;
		}
		else if (operator== '-' && previousoperator=='-'){
				sub(&A,&B);	
				//printmatrix(&A);
		}
		else if (operator== '-' && previousoperator=='*'){
				if (mult(&A,&B,&matrixproduct)!=1){fprintf(stderr,"Error: Chybny vstup!\n");return 100;}
				A=matrixproduct;
				//printmatrix(&A);
				previousoperator=operator;
		}

	}

	exit:
		if (previousoperator=='+'){
			add(&A,&B);
			printmatrix2(&A);
		}
		else if (previousoperator=='*'){
			//multiply
			if (mult(&A,&B,&matrixproduct)!=1){fprintf(stderr,"Error: Chybny vstup!\n");return 100;}
			A= matrixproduct;
			printmatrix2(&A);
		}
		else if (previousoperator== '-'){
			sub(&A,&B);
			printmatrix2(&A);
		}
		//clear the mess
		clear(&A);
		for (int i=0; i< 26; i++){
			clear (&matrixarray[i]);
		}
	return 0;
}
