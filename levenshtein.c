/*
 * Just a simple levenshetin try, it takes 2 sentences, uses 4 threads to read them
 * into 2 separate buffers, and then parses the words and computes the levenshtein distance
 * for each one of them.
*/
#include <fcntl.h>
#include <pthread.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_THREADS 4
#define BASE_SIZE 256

typedef struct{
	char *sentece;
	size_t capacity;
	size_t size;
}buffer;

// resources shared by all threads
buffer expected_output, actual_output;
int expected, actual;
pthread_mutex_t mtxA =  PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtxE =  PTHREAD_MUTEX_INITIALIZER;


char *resize_sentence(char *sentence, size_t capacity) {
	char *temp = (char*) realloc(sentence, capacity * sizeof(char));

	if (!temp) {
		perror("realloc failure");
		free(sentence);
	}

	return temp;
}

int32_t levenshtein_distance(char *first_word, char *second_word, 
														 ssize_t len_first, ssize_t len_second) {

	if (len_first <= 0)
		return len_second;

	if (len_second <= 0)
		return len_first;

	if (!first_word || !second_word)
		return -1;

	// move past first one
	if (first_word[0] == second_word[0]) 
		return levenshtein_distance(&first_word[1], &second_word[1], 
																 --len_first, --len_second);

	return 1 + fmin(fmin
	(levenshtein_distance(&first_word[1], second_word, len_first - 1, len_second),
	 levenshtein_distance(first_word, &second_word[1], len_first, len_second - 1)),
	 levenshtein_distance(&first_word[1], &second_word[1], len_first - 1, len_second - 1));
}

void *fill_expected(void *x) {

	pthread_mutex_lock(&mtxE);
	char local_buffer[BASE_SIZE];
	int rv;

	if ((rv = read(expected, local_buffer, BASE_SIZE)) < 0) {
		perror("read expected failure");
		return NULL;
	}

	if (!rv) {
		perror("EOF expected");
		return NULL;
	}

	if (expected_output.size + rv >= expected_output.capacity) {
		expected_output.capacity *= 2;
		expected_output.sentece = 
				resize_sentence(expected_output.sentece, expected_output.capacity);
	}

	memcpy(expected_output.sentece + expected_output.size , local_buffer, rv);
	expected_output.size += rv;
	pthread_mutex_unlock(&mtxE);

	return NULL;
}

void *fill_actual(void *x) {

	pthread_mutex_lock(&mtxA);
	char local_buffer[BASE_SIZE];
	int rv;
	rv = read(actual, local_buffer, BASE_SIZE);

	if (rv < 0) {
		perror("read actual failure");
		return NULL;
	}

	if (!rv) {
		perror("EOF actual");
		return NULL;
	}

	if (actual_output.size + rv >= actual_output.capacity) {
		actual_output.capacity *= 2;
		actual_output.sentece = 
					resize_sentence(actual_output.sentece, actual_output.capacity);
	}

	memcpy(actual_output.sentece + actual_output.size, local_buffer, rv);
	actual_output.size += rv;
	pthread_mutex_unlock(&mtxA);

	return NULL;
}

size_t parse_buffers(buffer expected, buffer actual) {
	char *expected_words, *actual_words;
	char *expected_save, *actual_save;
	expected_words = strtok_r(expected.sentece, " ;\"'\n", &expected_save);
	actual_words = strtok_r(actual.sentece, " ,;\"'\n", &actual_save);

	size_t total_distances = 0;

	while(expected_words && actual_words) {
		//printf("expected word: %s, found word: %s\n", expected_words, actual_words);
		int distance = levenshtein_distance(expected_words, actual_words, 
						strlen(expected_words), strlen(actual_words));
		// printf("levenshetin distance: %d\n", distance);
		total_distances += distance;
		expected_words = strtok_r(NULL, " ,;\"'\n", &expected_save);
		actual_words = strtok_r(NULL, " ,;\"'\n", &actual_save);
	} 

	while(expected_words) {
		total_distances += levenshtein_distance(expected_words, NULL, strlen(expected_words), 0);
		expected_words = strtok_r(NULL, " ,;\"'", &expected_save);
	}

	while(actual_words) {
		total_distances += levenshtein_distance(actual_words, NULL, strlen(actual_words), 0);
		actual_words = strtok_r(NULL, " ,;\"'", &actual_save);
	}

	return total_distances;
}

int main(int argc, char *argv[]) {

	if (argc != 3) {
		perror("specify the 2 files to read from. first: expected, 2nd: actual");
		return 1;
	}

	expected = open(argv[1], O_RDONLY);
	actual = open(argv[2], O_RDONLY);

	// initialize the 2 buffer
	expected_output.capacity = actual_output.capacity = BASE_SIZE;
	expected_output.size = actual_output.size = 0;

	expected_output.sentece = (char*) malloc(sizeof(char) * BASE_SIZE);
	if (!expected_output.sentece) {
		perror("expected, malloc failure");
		close(expected);
		close(actual);
		return 1;
	}

	actual_output.sentece = (char*) malloc(sizeof(char) * BASE_SIZE);
	if (!actual_output.sentece) {
		perror("actual, malloc failure");
		close(expected);
		close(actual);
		free(expected_output.sentece);
		return 1;
	}

	pthread_t eTid[MAX_THREADS / 2];
	pthread_t aTid[MAX_THREADS / 2];

	for (size_t i = 0; i < MAX_THREADS / 2; i++) {
		pthread_create(&aTid[i], NULL, fill_actual, NULL);
		pthread_create(&eTid[i], NULL, fill_expected, NULL);
	}

	for (size_t i = 0; i < MAX_THREADS / 2; i++) {
		pthread_join(eTid[i], NULL);
		pthread_join(aTid[i], NULL);
	} 

	expected_output.sentece[expected_output.size] = '\0';
	actual_output.sentece[actual_output.size] = '\0';
	printf("%s\n", expected_output.sentece);
	printf("%s\n", actual_output.sentece);

	int levenshtein_ditances = parse_buffers(expected_output, actual_output);
	printf("Levenshtein total: %d\n", levenshtein_ditances);

	return 0;
}
