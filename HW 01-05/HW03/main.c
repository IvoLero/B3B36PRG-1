#include <stdio.h>
#include <stdlib.h>


void print (char* array, int size){
	for (int i=0; i < size; i++){
		printf("%c", *(array +i));
	}
	printf("\n");
}
void printI (int* array, int m, int n){
	for (int i=0; i < m ; i++){
		for (int j=0; j < n;j++){
			printf("%d ", *(array +i*n +j));
		}
		printf("\n");
	}
}
int controlchar(char c){
	return ((c-'a' >= 0 && 'z'-c >= 0)||(c-'A' >= 0 && 'Z'-c >= 0))? 1:0;   
}
int readinput(char**mem, int* counter){
	char c = getchar();
	*mem = (char*) malloc (1);
	*counter = 0;
	while (c != '\n'){
		if (!controlchar(c))
			return 100;*(*mem +*counter)= c;

		(*counter)++;
		c = getchar();
		*mem = (char*) realloc(*mem,*counter+1);
	}
	return 0;
	
}
void shift (char* origin, char* shifted, int offset, int size){
	
		int o, new;
		for (int i=0; i<size; i++){
			o = *(origin +i);
			new = o + offset;	//get the new letter
		
			
			if (new > 'z' && o >= 'a'){ 		//low letter jump from the range
				new =  'A' + (new - 'z'-1); 	//new - 'z' is delta
				if (new > 'Z'){			//jump to same range
					new+=6;
				}
			}
			else if (new > 'Z' && o <= 'Z'){	//upper letter jump from the range
				new = 'a' + (new - 'Z'-1);	//new - 'Z' +6 is delta
				if (new > 'z'){
					new = 'A' + (new - 'z'-1);//jump to the same range
				}	
				
			}
			*(shifted +i) = new;
			
	}
}
int compare (char* piece, char* shifted, int size){
	int matches = 0;
	for (int i=0; i < size; i++){
		if (*(shifted +i) == *(piece +i)){
			matches++;
		}
	}
	return matches;
}
int WagnerFischer (char*origin, char*piece, int size1, int size2){
	int m = size1+1;
	int n = size2 +1;
	int* metrice = malloc(sizeof(int) *m*n);

	for (int i= 0; i < m; i++){
		*(metrice + i*n) = i;
	}
	for (int i=0; i< n; i++){
		*(metrice + i) = i;
	}
	for (int i=1; i < m; i++){
		for (int j=1; j< n;j++){
			if (*(origin +i-1) == *(piece+j-1)){
				*(metrice + i*n + j) = *(metrice + (i-1)*n +j-1);
			} 
			else{
				int a = *(metrice + (i-1)*n +j-1)+1;
				int b = *(metrice + (i-1)*n +j)+1;
				int c = *(metrice + i*n +j-1)+1;
				*(metrice + i*n +j)= (a < b)? ((a<c)? a: c) : ((b<c)? b :c); 
			}
		}
	}
//	printI (metrice, m,n);
	int returnvalue = *(metrice + m*n-1);
	free(metrice);
	return returnvalue;
}
void findmatch(char* origin, char* piece, int size1, int size2){
	int maximummatches = 0;
	int minimumwagnerfischer = (size1>size2)? size1:size2;
	int matches;
	int bestoffset = 0;
	char* shifted = malloc(size1);

	for (int i =0; i< 26*2; i++){
		shift(origin, shifted, i, size1);
		//print(shifted, size);
		if (size1 == size2){
			matches = compare(piece,shifted, size2);
			if (matches > maximummatches){
				maximummatches=matches;
				bestoffset = i;
			}
		}
		else{
			matches = WagnerFischer(shifted,piece,size1,size2);
			if (matches < minimumwagnerfischer){
				minimumwagnerfischer=matches;
				bestoffset = i;
			}

		}

	}
	shift (origin, shifted, bestoffset, size1);
	print (shifted, size1);
	free(shifted);
	
}

	



int main (int argc, char* argv[]){
	char* firstline, *secondline;
	int size1, size2;
	if (readinput(&firstline,&size1)!=0){
		fprintf(stderr,"Error: Chybny vstup!\n");
		free(firstline);
		return 100;
	}
	else{

	//	print(firstline, size1);
		if (readinput(&secondline,&size2)!=0){
			fprintf(stderr,"Error: Chybny vstup!\n");
			free(firstline);
			free(secondline);
			return 100;
		}
		else{
		//	print(secondline, size2); 
			if (argc==1){
				if (size1 != size2){
					fprintf(stderr,"Error: Chybna delka vstupu!\n");
					free(firstline);
					free(secondline);
					return 101;
				}
				else {
					findmatch(firstline,secondline, size1,size2);
				}
			}
			else{
				findmatch(firstline,secondline,size1,size2);
				//printf("%d", WagnerFischer(firstline,secondline,size1,size2));	
			}
			free(firstline);
			free(secondline);
		}
	}
	return 0;
}

