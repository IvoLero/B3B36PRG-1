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





void printmatrix (matrix* M){
	//printf("Printing the matrix\n");
	printf("%d %d\n", M->rows, M->columns);
	for(int i= 0; i< M->rows;i++){
		for (int j=0;j<M->columns; j++){
			if (j!= M->columns -1)
				printf("%d ", M->data[i][j]);
			else
				printf("%d\n", M->data[i][j]);

		}
			}
}

int readinput(matrix* M){
	int m,n; 
	int sucess =1;
	scanf("%d",&n); scanf("%d",&m);
	M->rows = n; M->columns = m;
	int** rows = (int**)malloc(sizeof(int*) *n);
	int num;
	//printf("%d %d", M->rows, M->columns);
	for (int i=0; i < n; i++){
		rows[i] = (int*) malloc(sizeof(int)*m);
	
		for (int j=0; j < m; j++){	
			 if (scanf("%d", &num) == 1)
				*(*(rows+i)+j) = num;
			else
				sucess=0;
		}
	}
	M->data = rows;
	if (sucess)return 1;
	else {
		clear(M);
		return 100;
	}
}

int main(int argc, char* argv[]){ 
		
	matrix A,B,C, matrixproduct;
	char previousoperator,operator;

	//control input	
	if (readinput(&A)==100){fprintf(stderr,"Error: Chybny vstup!\n"); return 100;}
	//printmatrix(&A);

	//new line skop
	getchar();	
	scanf("%c",&previousoperator);
	getchar();	
	while (1==1){
	//	printf("Enter the matrix\n");	
		//control input
		if (readinput(&B)==100){fprintf(stderr,"Error: Chybny vstup!\n");clear(&A);return 100;}
	//	printmatrix (&B);
		
		getchar();
		//control end of input
		if (scanf("%c",&operator)!=1)goto exit;		
		getchar();

		if (operator == '*' && previousoperator=='*'){
				//multiply
				if (mult(&A,&B,&matrixproduct)!=1){fprintf(stderr,"Error: Chybny vstup!\n");clear(&A);return 100;}
				A = matrixproduct;
			//	printmatrix(&A);

		}
		else if (operator== '*' && (previousoperator== '+' || previousoperator=='-')){
				
				do {

					//control input
					if (readinput(&C)==100){fprintf(stderr,"Error: Chybny vstup!\n");return 100;}

					//multiply
					if (mult(&B,&C,&matrixproduct)!=1){fprintf(stderr,"Error: Chybny vstup!\n");return 100;}
					B=matrixproduct;
					//printmatrix(&B);

					
					getchar();
					//control end of input
					if (scanf("%c",&operator)!=1) goto exit;
					getchar();
					
					
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
			printmatrix(&A);
		}
		else if (previousoperator=='*'){
			//multiply
			if (mult(&A,&B,&matrixproduct)!=1){fprintf(stderr,"Error: Chybny vstup!\n");return 100;}
			A= matrixproduct;
			printmatrix(&A);
		}
		else if (previousoperator== '-'){
			sub(&A,&B);
			printmatrix(&A);
		}
		clear(&A);
	return 0;
}
