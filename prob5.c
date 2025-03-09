/*
Pentru un șir cu n elemente care conține valori din mulțimea {1, 2, ..., n - 1} astfel încât o singură valoare se repetă de două ori, să se identifice acea valoare carese repetă. De ex. în șirul [1,2,3,4,2] valoarea 2 apare de două ori.
*/

/*
	 My plan is to use a hashtable, maybe an intrusive one to get more practice with
	 them, keep a number of occurences for each. Now that i think about it I may 
	 not even need a hashtable, but i'll use for practice. 
 	 Also for efficiency reasons, read everything into a buffer at once, and then
	 parse it and keep a hashtable.
	 I will also need to check if there is a way to dinamically remember only the on		 es with 2 apparitions, and not to go through the whole hashtable again
  
	 nvm fuck that shit
	 vector de frecventa
*/

#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define CHUNK_SIZE 64

char *resize(char *buffer, size_t capacity) {
	char *temp = realloc(buffer, capacity);
	if (!temp) {
		perror("calloc");
		return NULL;
	}

	return temp;
}

char *fill_buffer(char *filename) {
	int fd = open(filename, O_RDONLY);
	char *chunk = (char*) malloc(sizeof(char) * CHUNK_SIZE);
	if (!chunk) {
		perror("malloc chunk");	
		return NULL;
	}

	size_t capacity = CHUNK_SIZE;
	size_t length = 0;
	char *buffer = (char*) malloc(sizeof(char) * capacity);
	int rv;


	// not the greatest error handling 
	// like tf happens when it reads 0 bytes
	// for some unknown reason

	while ((rv = read(fd, chunk, CHUNK_SIZE)) > 0) {
		if (length + rv >= capacity) {
			capacity *= 2;
			buffer = resize(buffer, capacity);
			if (!buffer) 
				return NULL;
		}

		memcpy(&buffer[length], chunk, rv);
		length += rv;
	}

	//printf("%s\n", buffer);	
	return buffer;
}

int32_t find_one(int *frecventa, size_t maxim) {
	for (int i = 0; i < maxim; i++) {
		// printf("frecventa i: %d\n", frecventa[i]);
		if (frecventa[i] == 2)
			return i;
	}
	return -1;
}

int parse_buffer(char *buffer, int *frecventa, size_t capacity) {
	size_t offset = 0;
	size_t max = 0;

	while (offset < strlen(buffer)) {
		
		while (!isalnum(buffer[offset]))
			++offset;
		
		if (offset >= strlen(buffer))
			break;

		size_t start_point = offset;

		while (isalnum(buffer[start_point]))
			++start_point;

		char *char_nr = (char*) malloc(sizeof(char) * (start_point - offset - 1));
		memcpy(char_nr, &buffer[offset], (start_point - offset));
		// printf("%s ", char_nr);
		int32_t number = strtol(char_nr, NULL, 0);
		// printf("%d\n", number);

		offset += (start_point - offset);
		++offset;

		if (number > capacity) {
			capacity = number + 1;
			int *temp = (int*) realloc(frecventa, sizeof(int) *capacity);
			if (!temp) {
				perror("realloc frecventa");
				free(frecventa);
				return -1;
			}
			frecventa = temp;
		}

		if (number > max)
			max = number;
		++frecventa[number];

	}

	return find_one(frecventa, max);

}


int main(int argc, char *argv[]) {
	
	if (argc != 2) {
		perror("specify input file");
		return 1;
	}

	char *buffer = fill_buffer(argv[1]);
	// printf("%s\n", buffer);

	int *frecventa = (int*) malloc(sizeof(int) * CHUNK_SIZE);

	if (!frecventa) {
		perror("malloc");
		return 1;
	}

	size_t capacity = CHUNK_SIZE;
	printf("%d\n", parse_buffer(buffer, frecventa, capacity));

	return 0;
}
