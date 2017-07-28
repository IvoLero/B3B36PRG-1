#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]){
	
	
	if (argc < 3){
		fprintf(stderr, "Error: Chybejici argument(y).\n");
		return 100;
	}
	else if (*argv [1] == '\0'){
		fprintf(stderr, "Error: Prazdny PATTERN.\n");
		return 101;
	}
	else{

		FILE* f;
		f = fopen(argv[2], "r");
		if (f == NULL){
			fprintf(stderr,"Error: Soubor nelze otevrit.\n");
			return 102;
		}
		char rchr ;
		char* line;
		

		do{	
			line = (char*) malloc(1 * sizeof(char));
			if (line == NULL){
				fprintf(stderr,"Error: Alokace pameti selhala.\n");
				return 200;
			}

			int counter =0;
			rchr = (char)fgetc(f);
			int reading =0;
			int print =0;
			char* pattern = argv[1];
			int patterncounter= 0;
			int remember [100];
			int r = 0;

			while (rchr != '\n' && rchr !=EOF){
			//	printf ("%c %c\n",rchr, pattern[patterncounter]);
				if (reading ==0 && rchr==pattern[0] ){
					reading = 1;
					patterncounter++;
				}
				else if (reading==1){
					if (rchr !=pattern [patterncounter]){
						if (rchr ==pattern[0]){
							patterncounter=1;
						}
						else{
							reading=0;
							patterncounter=0;
						}
					}
					else if (pattern[patterncounter+1] =='\0'){
						reading =0;
						patterncounter=0;
						print=1;
					}
					else if (rchr == pattern[0]){
						remember[r] = counter;
						r++;
						patterncounter++;
						
					}
					else{
						patterncounter++;
					}
				}

				//printf ("%c\n",*(line+counter));
				*(line +counter) = rchr;
				counter++;
				rchr= (char)fgetc(f);
				line = (char*) realloc(line,(counter+1)* sizeof(char));
				if (line == NULL){
					fprintf(stderr,"Error: Alokace pameti selhala.\n");
					return 200;
				}

			}

			*(line + counter) = '\0';
			for(int i=0; i < r;i++){
				int c =0;
				while (remember [i] + c < counter && *(line +remember[i] +c) == pattern[c]){
					if (pattern [c+1] =='\0'){
						print=1;
					}
					c++;
				}
			}

			if (print==1){
				printf ("%s\n", line);
			}
			
			
			free(line);

		} while (rchr!= EOF);
		fclose(f);
		return 0;

		
	}

}
