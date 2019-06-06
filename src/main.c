#include <stdio.h>


int main()
{
	unsigned char answer = 'y';
	unsigned char i = 0;
	char* word;
	unsigned int number;
	printf("Hello ESPL \n");
	printf("I am Khanh \n");	

	while((answer == 'y') || (answer == 'Y'))
	{
		
		printf("Please enter a number between 0 and 2^17-1 (unsigned int number)! : \n");
		scanf("%d", &number);	
		word = num_to_words(number);
	
		printf("Your number: %d \n", number);
		while(word[i] != '\0')
		{	
			printf("%c", word[i]);
			i++;		
		}

		printf("\nDo you want to continue ? [Y/y or N/n]");
		scanf("%s", &answer);
		while (answer !=  'y' && answer !=  'Y' && answer !=  'n' && answer !=  'N')
		{
			printf("\nPlease press these 4 characters [Y/y or N/n] ");
			scanf("%s", &answer);
		}
		
	}
		return 0;
}
