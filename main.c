#include "memory.h"

#include <unistd.h>
#include <stdlib.h>

#include <sys/wait.h>
#include <sys/types.h>

#define INPUT_CAPACITY 4096
#define TOKENS_CAPACITY 1024

Arena a = {0};

char *input(void) {
	char *line = arena_alloc(&a, INPUT_CAPACITY);

	if (!(fgets(line, INPUT_CAPACITY, stdin))) {
		fputs("Failed to read the input!\n", stderr);
		exit(EXIT_FAILURE);
	}

	return line;
}

char **parse(char *line) {
	int count = 0;
	char *token = strtok(line, " \n");
	char **tokens = arena_alloc(&a, TOKENS_CAPACITY);

	while (token != NULL) {
		tokens[count++] = token;
		token = strtok(NULL, " \n");
	}

	tokens[count] = NULL;
	return tokens;
}

char *define_functions[] = {
	"cd"
};

int length(void) {
	return (sizeof(define_functions) / sizeof(char *));
}

int cd_function(char **args) {
	if (args[1] == NULL) {
		fputs("cd (directory)\n", stderr);
	} else if (chdir(args[1]) != 0) {
		fputs("cd\n", stderr);
	}

	return EXIT_FAILURE;
} 

int (*exec_functions[]) (char **args) = {
	&cd_function
};

int start(char **args) {
	int status = 0;
	pid_t pid = fork();

	if (pid == 0) {
		if (execvp(args[0], args) == -1) {
			fputs("Directory not found!\n", stderr);
		}
	} else if (pid < 0) {
		fputs("Failed to create a process!\n", stderr);
		exit(EXIT_FAILURE);
	} else {
		do {
			waitpid(pid, &status, WUNTRACED);
		} while (!(WIFEXITED(status)) && (!(WIFSIGNALED(status))));
	}

	return EXIT_FAILURE;
}

int exec(char **args) {
	if (args[0] == NULL) {
		fputs("Command not found!\n", stderr);
	}

	for (int i = 0; i < length(); i++) {
		if (strcmp(args[0], define_functions[i]) == 0) {
			return (*exec_functions[i]) (args);
		}
	}

	return start(args);
}

int main(void) {
	char buf[INPUT_CAPACITY + TOKENS_CAPACITY];
	arena_init(&a, buf, sizeof(buf));

	char *line = NULL;
	char **args = NULL;
	int status = 0;

	do {
		printf("> ");
		line = input();
		args = parse(line);
		status = exec(args);

		arena_free(&a);
	} while (status);

	return EXIT_SUCCESS;
}
