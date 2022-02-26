#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <libc/time.h>
#include <libc/string.h>

#define ACTUAL_WORD_LIST "/usr/assets/act.csv"
#define ACCEPTABLE_ANSWER_LIST "/usr/assets/acc.csv"

int cmp(const void* a, const void* b)
{
	return strcmp((char*)a, (char*)b);
}

int get_length(FILE* fp);

FILE *open_file(char *fp);

char *load_files(char *act_path, char *acc_path);

int acceptable_guess(char *haystack, int left_index, int right_index, char *guess);

void print_help();

int main(int argc, char **argv)
{
	if(argc ==2)
	{
		if(! strcmp(argv[1], "--help") || ! strcmp(argv[1], "-h"))
		{
			print_help();
			return 0;
		}
		else
		{
			printf("use %s -h or %s --help for help", argv[0], argv[0]);
			return 0;
		}
	
	}

	//load word lists into memory as char arrays
	char *buffer = load_files(ACTUAL_WORD_LIST, ACCEPTABLE_ANSWER_LIST);

	//get word of the day and store in WoTD variable.
	time_t seconds = time(NULL);
	char WoTD[6];
	for(int i = 0; i < 6; i++)
	{
		if(buffer[seconds/86400%2309*6+i] == '\n')
		{
				WoTD[i] = '\0';
		}
		else
		{
				WoTD[i] =  buffer[seconds/86400%2309*6+i];
		}
	}

	//convert all new lines to null terminators
	for(int i = 0; i < 77682; i++)
	{
		if(buffer[i] == '\n')
		{
			buffer[i] = '\0';
		}
	}

	//sort buffer to search entire word list
	qsort(buffer, 12947, 6*sizeof(char), cmp);

	//game loop starts here
	char guess[128];
	int guesses = 1;

	printf("Guess the 5 letter word of the day!\n");
	while(1)
	{
		printf("[%i] ", guesses);
		gets(guess);

		//check if input is more than 5 chars
		if(strlen(guess) != 6)
		{
			if(strlen(guess) > 6)
			{
				for(int i = 0; i < strlen(guess)-1; i++)
				{
					if( i < 5)
					{
						printf("%c", guess[i]);
					}
					else
					{
						printf("\x1b[41m%c", guess[i]);
					}
				}
				printf("\x1b[0m\n");
			}
			else
			{
				for(int i = 0; i < strlen(guess); i++)
				{
					printf("\x1b[41m%c", guess[i]);
				}
				printf("\x1b[0m\n");
			}
		}
		
		//check if input is not in the acceptable guess list
		else if(acceptable_guess(buffer, 0, 12947, guess) == -1)
		{
			for(int i = 0; i < strlen(guess)-1; i++)
			{
				printf("\x1b[41m%c", guess[i]);
			}
			printf("\x1b[0m\n");
		}
		else
		{
			guesses++;
			printf("Good word\n");
			return 0;
		}

	}	

		free(buffer);
		return 0;
}

int get_length(FILE* fp)
{
	int length = fseek(fp, 0L, SEEK_END);
	if (length < 0)
 	{
			perror("seek failed");
			fclose(fp);
			exit(1);
  	}
  	fseek(fp, 0, SEEK_SET);
  	return length + 1;
}

FILE *open_file(char *file)
{
	FILE *fp = fopen(file, "rb");
	if(!fp)
	{
		perror("fopen() failed");
		exit(1);
	}
	return fp;
}

char *load_files(char *act_path, char *acc_path)
{
	FILE *act = open_file(act_path);
	FILE *acc = open_file(acc_path);

	int act_length = get_length(act);
	int acc_length = get_length(acc);

		char *buffer = malloc(act_length + acc_length + 2);
		if (!buffer)
		{
		eprintf("Allocation failed.\n");
				fclose(act);
		fclose(acc);

		exit(1);
		}

		if(fread(buffer, act_length, 1, act) != 1)
		{
		perror("fread() failed.");
		free(buffer);
				fclose(act);
		fclose(acc);

				exit(1);
		}

	if(fread(buffer+act_length, acc_length, 1, acc) != 1)
	{
		perror("fread() failed.");
		free(buffer);
		fclose(act);
		fclose(acc);

		exit(1);
	}	

		fclose(act);
	fclose(acc);
		return buffer;
}

int acceptable_guess(char *haystack, int left_index, int right_index, char *guess)
{
	while(left_index <= right_index)
	{
		//get the index in the middle between left and right indexes
		int pivot = left_index + (right_index - left_index) / 2;

		//perform comparison
		int compare = strcmp(guess, &haystack[pivot*6]);
		printf("%s\n", &haystack[pivot*6]);

		//check if needle is present
		if(compare == 0)
			return pivot;

		//if x greater, ignore left half
		if(compare > 0)
			left_index = pivot + 1;

		//if x is smaller, ignore right half
		else
			right_index = pivot -1;
	}

	printf("element not present\n");
	return -1;
}

void print_help()
{
	//todo?
	printf("lol it's wordle\n");
}
