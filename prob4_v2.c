// first create a client connection to the redis server
// and the protocol required
// then implement the parsing 
// and boyler moore

#include <time.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

const uint32_t port = 34900;

// enum used for the message protocol received from the server
enum {
	TAG_NIL = 0,
	TAG_ERR = 1,
	TAG_STR = 2,
	TAG_INT = 3, 
	TAG_DBL = 4,
	TAG_ARR = 5
};

int init_connection(uint32_t port) {
	int socket_fd;
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket()");
		return -1;	
	}

	struct sockaddr_in server;
	memset(&server, 0, sizeof(struct sockaddr_in));
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_family = AF_INET;

	if (connect(socket_fd, (struct sockaddr*) &server, sizeof(server)) < 0) {
		perror("connect");
		return -1;
	}

	return socket_fd;
}

ssize_t write_all(int socket_fd, void *buf, size_t buf_len) {
	ssize_t bytes_written = 0;
	size_t bytes_left = buf_len;
	char *buffer = (char*) buf;
	size_t offset = 0;

	while (bytes_left) {
		bytes_written = write(socket_fd, buffer + offset, bytes_left);

		if (bytes_written > 0) {
			bytes_left -= bytes_written;
			offset += bytes_written;
			continue;
		}

		if (!bytes_written && (errno == EINTR || errno == EAGAIN)) {
			continue;	
		}

		break; // other error or eof
	}

	return (ssize_t) (buf_len - offset);
}

ssize_t read_all(int socket_fd, void *buf, size_t buf_len) {
	ssize_t bytes_read= 0;
	size_t bytes_left = buf_len;
	char *buffer = (char*) buf;
	size_t offset = 0;

	while (bytes_left) {
		bytes_read= read(socket_fd, buffer + offset, bytes_left);

		if (bytes_read > 0) {
			bytes_left -= bytes_read;
			offset += bytes_read;
			continue;
		}

		if (!bytes_read && (errno == EINTR || errno == EAGAIN)) {
			continue;	
		}

		break; // other error or eof
	}

	return (ssize_t) (buf_len - offset);
}

ssize_t send_req(int socket_fd, char *cmdv[], size_t cmdc) {
	uint32_t net_len, len = 4; // reserve 4 bytes for message length 
	for (size_t i = 0; i < cmdc; i++) {
		len = len + strlen(cmdv[i]) + 4;
	}

	net_len = htonl(len);
	char buffer[256]; // should be enough, we send simple commands
	memcpy(&buffer[0], &net_len, sizeof(uint32_t));
	net_len = htonl(cmdc);
	memcpy(&buffer[4], &net_len, sizeof(uint32_t));
	size_t offset = 8;

	for (size_t i = 0; i < cmdc; i++) {
		net_len = htonl(strlen(cmdv[i]));
		memcpy(&buffer[offset], &net_len, sizeof(uint32_t));	
		offset += 4;

		memcpy(&buffer[offset], cmdv[i], strlen(cmdv[i]));
		offset += strlen(cmdv[i]);
	}

	return write_all(socket_fd, buffer, 4 + len);
}

void parse_request(char *buffer, int *return_val) {
	// realistically
	// we are interested in 2 cases
	// str and int, probably string only i do not think i send any ints right now

	if (buffer[0] == TAG_NIL) {
		*return_val = 0;
	}
	else
		*return_val = 1; // keep it simple
										 // we are only interested if anything exists in the hashmap
										 // for said word
										 // no need to check for value or anything else
}

ssize_t read_request(int socket_fd, int *return_val) {
	char buffer[256];
	uint32_t net_len = 0, len = 0;

	if (read_all(socket_fd, &net_len, sizeof(uint32_t))) {
		perror("read length");
		return 1;
	}

	len = ntohl(net_len);
	if (read_all(socket_fd, buffer, len)) {
		perror("read request");
		return 1;
	}

	parse_request(buffer, return_val);
	return 0;
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

		execlp(command, command, "-o", "-i", word, filename, NULL);

		perror("execlp");
		exit(1);
	}

	// inside parent process
	if (pid > 0) {

		wait(NULL); // close the write end and wait for grep command
		close(pfd[1]);

		char temporary[256];
		if (read(pfd[0], temporary, strlen(word) + 1)) {
			if (!read(pfd[0], temporary, strlen(word)))
				printf("%s\n", word);
			else {
				char *cmds[3];
				cmds[0] = "set";
				cmds[1] = word;
				cmds[2] = "1";

				int socket_fd;
				if ((socket_fd = init_connection(port)) < 0) 
					return;

				if (send_req(socket_fd, cmds, 3)) {
					perror("send_req set");
					return;
				}

				int return_val;
				if (read_request(socket_fd, &return_val)) {
					perror("read_request set");
					return;
				}

				close(socket_fd);
			}
		} 
		// fprintf(stderr, "\n\n");
		// fclose(write_file);
		close(pfd[0]);
	}
}

void algoritm(char *filename) {
	FILE *fp = fopen(filename, "r");
	char *word = (char*) malloc(sizeof(char) * 100);

	while (fscanf(fp, "%s ", word) > 0) {
		// printf("word: %s\n", word);
		// cut unnecessary characters 
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

	if (!strlen(word))
		continue;

	char *cmds[2];
	cmds[0] = "get";
	cmds[1] = word;

	int socket_fd;
	if ((socket_fd = init_connection(port)) < 0) 
		return;	

	if (send_req(socket_fd, cmds, 2)) {
		perror("send_req_get");
		return;
	}

	int return_value = 1;
	if (read_request(socket_fd, &return_value)) {
		perror("read_req_get");
		return;
	}
	close(socket_fd); // no hanging connection

	if (return_value)
		continue;

	search_word(word, filename);
	}

	free(word);
}


int main(int argc, char *argv[]) {
	clock_t start, end;

	/*
		if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			perror("socket");
			return 1;
		}

		memset(&server, 0, sizeof(struct sockaddr_in));
		server.sin_port = htons(port);
		server.sin_family = AF_INET;
		server.sin_addr.s_addr = htonl(INADDR_ANY);

		if (connect(socket_fd, (struct sockaddr*) &server, sizeof(struct sockaddr_in))) {
			perror("connect");
			return 1;
		}
	*/

	/*
		int socket_fd = init_connection(34900);	
		char *cmds[2]; 
		cmds[0] = "get";
		char *word = "''andrei/\\";
		cmds[1] = word;

		for (size_t i = 0; i < 2; i++) {
			printf("%s ", cmds[i]);
		}

		printf("\n");
		if (send_req(socket_fd, cmds, 2)) {
			perror("send_req");
			return 1;
		}

		int return_val; 
		if (read_request(socket_fd, &return_val)) {
			perror("read_req");
			return 1;
		}

		printf("val returned: %d\n", return_val);
		close(socket_fd);
	*/	

	start = clock(); 
	algoritm(argv[1]);
	end = clock(); 
	printf("\nclocked at: %f", (double)(end - start) / CLOCKS_PER_SEC);

	return 0;
}
