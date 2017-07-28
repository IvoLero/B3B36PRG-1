#include <stdio.h>

int main(){
	static char *day_of_week[] = {"Mo", "Tu", "We", "Th", "Fr", "Sa", "Su" };
	static char *name_of_month[] = {
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"
};
	static int days_in_month[] = { 31, 29, 31, 30, 31, 30,
                               31, 31, 30, 31, 30, 31 };
	static int first_day_in_september = 3; // 1. 9. 2016 is Thursday
	static int first_day_in_year = 4; // 1. 1. 2016 is Friday	
	calc (4);
	return 0;
}
void calc (int month){
	//pole pointeru na char
	for (int i = 0; i < 7; i++){
		printf ("%s ", day_of_week[i]);
	}
	printf ("\n");

	int max = days_inmonth[month];
	int first_day = 3;
	int counter = 1;
	for (int i = 0; i < 5; i++){
		for (int j = 0; j < 7; j++){
			if (i==0 && j < first_day){
				printf("   ");
			}
			else if (i==4 && counter>max){
				printf ("   ");
			}
			else {
				if (counter < 10){
					printf (" %d", counter);
				}
				else{
					printf ("%d",counter);	
				}
			}
			
	}
	printf ("\n");
}
}
