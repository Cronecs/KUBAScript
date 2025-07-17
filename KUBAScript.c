#include <stdio.h>


static char input[2048];

int main(int argc, char** argv) {
	puts("\n-------------------------------");
	puts("Welcome to KUBAScript");

	while (1)
	{
		fputs("input> ", stdout);

		fgets(input, 2048, stdin);

		printf("%s", input);
	}
	

	return 0;
}