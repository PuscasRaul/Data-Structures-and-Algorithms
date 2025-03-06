#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// initial idea is to construct the sum for each word
// but use different weights for different positions in the word
// so that we can compare them correctly


// overall that's an ass idea
// shit don't work
// just strcmp raw dog it

#define CHUNK_SIZE 64

char *resize(size_t capacity, char *buffer) {
	char *new_buffer = realloc(buffer, capacity);	
	if (!new_buffer) {
		perror("realloc");
		return NULL;
	}
	return new_buffer;
}

char *fill_buffer(char *filename) {
	FILE *fp = fopen(filename, "r");
	size_t capacity = CHUNK_SIZE;
	size_t buf_length = 0;
	char *buffer = (char*) malloc(CHUNK_SIZE);
	if (!buffer) {
		perror("malloc");
		return NULL;
	}

	char c;
	while ((c = fgetc(fp)) > 0) {
		if (buf_length + 1 >= capacity) {
			capacity *= 2;
			char *temp= resize(capacity, buffer);	
			if (!temp) {
				free(buffer);
				return NULL;
			}
			buffer = temp;
		}
		buffer[buf_length++] = c;
	}
	return buffer;
}

size_t extract_word(char *line, size_t offset, char *word) {
	size_t word_len = 0;
	while (line[offset + word_len] != '\0' && line[offset + word_len] != ' ' &&
				 line[offset + word_len] != '\n') {
		++word_len;	
	}
	
	memcpy(word, &line[offset], word_len);
	word[word_len] = '\0';

	for (int i = strlen(word) - 1; i >= 0; i--) {
		if ((int) word[i] >= 97 && (int) word[i] <= 122)
			break;

		if ((int) word[i] >= 65 && (int) word[i] <= 90)
			break;

		word[i] = word[i + 1];
	}

	for(size_t i = 0 ; i < strlen(word); i++) {
		if ((int) word[i] >= 97 && (int) word[i] <= 122)
			break;

		if ((int) word[i] >= 65 && (int) word[i] <= 90)
			break;

		for (size_t j = i ; j < strlen(word); j++) {
			word[j] = word[j + 1];
		}
		--i;
	}

	return word_len;
}

void compare(char *word, char *max) {
	size_t i = 0;
	for (i = 0; i < strlen(word) && i < strlen(max); i++) {

		if (!isalpha(word[i]))
				continue;

		if (!isalpha(max[i]))
			continue;

		int cw = (int) tolower(word[i]);
		int cm = (int) tolower(max[i]);

		if (cw > cm){
			memcpy(max, word, strlen(word)); // found a match early, we just switch em
			max[strlen(word)] = '\0';
			return;
		}
	}

	// we have letters left in word, which means we finished max
	// and everything up to this was equal
	// and so word comes after in the dictionary

	if (i < strlen(word)) {
		memcpy(max, word, strlen(word));
		max[strlen(word)] = '\0';
	}
}

char *algoritm(char *buffer) {
	size_t len;
	char word[256];
	size_t offset = 0;
	char *max = (char*) malloc(sizeof(char) * 256); 

	if ((len = extract_word(buffer, offset, word))) {
		offset += len;

		while (!isalpha(buffer[offset]) && offset < strlen(buffer)) 
			++offset;
		memcpy(max, word, strlen(word));
		max[strlen(word)] = '\0';

		if (offset >= strlen(buffer))
			return max;	
	}

	while ((len = extract_word(buffer, offset, word))) {

		word[strlen(word)] = '\0';
		compare(word, max);
		offset += len;

		while (!isalpha(buffer[offset]) && offset < strlen(buffer)) 
			++offset;

		if (offset >= strlen(buffer))
			break;
	}

	printf("%s\n", max);
	return NULL;
}

int main(int argc, char *argv[]) {

	if (argc !=2) {
		perror("please introduce the file we read from");
		return 1;
	}

	char *buffer = fill_buffer(argv[1]);
	// printf("%s", buffer); 
	algoritm(buffer);
		
	return 0;
}
