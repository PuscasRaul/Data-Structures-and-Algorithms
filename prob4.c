/*
Să se determine cuvintele unui text care apar exact o singură dată în acel text. De ex. cuvintele care apar o singură dată în ”ana are ana are mere rosii ana" sunt: 'mere' și 'rosii'
*/

#include <stddef.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

// my only solution left to speed this up
// is threading it
// i guess? can't really think of anything else
// atleast to thread the reading part

// in this algorithm
// i will count as a correct word, any word that begins with a letter and end's with one

/*
this would work
but there are too many forks being called
making the program painfully slow

void cut_word(char *file, char *word) {
	// the file is already open
	int pid = fork();

	if (pid < 0) {
		perror("fork sed");
		exit(EXIT_FAILURE);
	}

	if (!pid) {
		// sed for deletion
		char *command = "sed";
		char sed_command[256];
    snprintf(sed_command, sizeof(sed_command), "s/\\<%s\\>//gI", word);

		execlp(command, command, "-i", sed_command, file, NULL);

		perror("execlp sed");
		exit(EXIT_FAILURE);
	}

	if (pid > 0) {
		// don't really have to do anything
		wait(NULL);
	}
}
*/

void search_word(char *word, char *filename) {
	if (!strlen(word))
		return;

	// fprintf(stderr, "Searching for word: %s\n", word);
	// fprintf(stderr, "Word length: %ld\n", strlen(word));
	// we open a pipe
	
	int pfd[2];
	if (pipe(pfd) < 0) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}

	int pid = fork();

	if (pid < 0) {
		perror("fork() failure");
		exit(EXIT_FAILURE);
	}

	// inside child process
	if (!pid) {

		// call to exec for this grep
		close(pfd[0]); // close the read side
		char *command = "grep";	

		// redirect the output of the grep command
		// so that we can count the number of apparitions 
		if (dup2(pfd[1], STDOUT_FILENO) < 0) {
			perror("dup2");
			exit(EXIT_FAILURE);
		}
		// fprintf(stderr, "just after dup2\n");

		execlp(command, command, "-o", "-i", word, filename, NULL);

		perror("execlp");
		exit(1);
	}

	// inside parent process
	if (pid > 0) {

		wait(NULL); // close the write end and wait for grep command
		close(pfd[1]);
		// FILE *write_file = fopen("output.txt", "a");

		char temporary[256];
		// fprintf(stderr, "Just before reading\n");
		if (read(pfd[0], temporary, strlen(word) + 1)) {
			// fprintf(stderr, "Atleast 1 apparition\n");
			if (!read(pfd[0], temporary, strlen(word)))
				printf("%s, ", word);
		} 
		// fprintf(stderr, "\n\n");
		// fclose(write_file);
		close(pfd[0]);
	}
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

	for (size_t i = 0 ; i < strlen(word); i++) {
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

void algoritm(char *filename) {
	FILE *file = fopen(filename, "r");
	
	// read line by line
	// parse the words
	// send them to search word 
	
	char *line = malloc(sizeof(char) * 10000); // i will say a line has
																						// at max 1000 characters
																						// (not checked)

	while (fgets(line, 10000, file) != NULL) {
		// perror("successfully read line");
		size_t offset = 0;	
		// extract every word
		while (line[offset] != '\0') {
			char *word = malloc(sizeof(char) * 100);
			size_t len = extract_word(line, offset, word); 
			if (len == 0)
				break;
			// printf("%s\n", word);
			search_word(word, filename);
			free(word);
			offset += len;
			while(line[offset] == ' ' || line[offset] == '\n') // go to the next word
				++offset;
		}
		// fflush(stdout);
	}
}

int main(int argc, char *argv[]) {
	
	if (argc != 2) {
		fprintf(stderr, "Provide the name of the file\n");
		exit(EXIT_FAILURE);
	}

	clock_t start, end;
	start = clock();
	
	algoritm(argv[1]);	
	end = clock();

	double cpu_time = ((double) (end - start)) / CLOCKS_PER_SEC; 
	printf("\nRuntime: %6.3f", cpu_time);
	return 0;
}
