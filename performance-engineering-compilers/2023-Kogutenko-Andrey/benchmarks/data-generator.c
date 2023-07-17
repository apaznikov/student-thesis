#include <stdio.h>
#include <stdlib.h>

int main(void) {
	fputs("Enter a size of sequence -- ", stdout);
	
	size_t size;
	scanf("%zu", &size);
	if (size == 0) return EXIT_SUCCESS;

	FILE *file = fopen("data.txt", "w");
	if (!file) {
		puts("ERROR! Can't open file!");
		return EXIT_FAILURE;
	}

	fprintf(file, "%zu\n", size);
	for (size_t i = 0, end = size - 1; i < end; ++i)
		fprintf(file, "%d ", rand());
	fprintf(file, "%d", rand());

	return EXIT_SUCCESS;
}

