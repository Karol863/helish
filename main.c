#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/wait.h>
#include <sys/types.h>

#define INPUT_CAPACITY 1024
#define TOKENS_CAPACITY 512

char *input(void) {
	char *line = NULL;
	size_t length = 0;

	if (!(getline(&line, &length, stdin))) {
		fputs("Failed to read the input!\n", stderr);
		exit(EXIT_FAILURE);
	}

	if (feof(stdin)) {
		fputs("End of line!\n", stderr);
		exit(EXIT_FAILURE);
	}

	return line;
}

char **parse(char *buf) {
	int count = 0;
	char *token = strtok(buf, " \n");
	char **tokens = calloc(TOKENS_CAPACITY, sizeof(char));

	if (!(tokens)) {
		fputs("Failed to allocate memory!\n", stderr);
		exit(EXIT_FAILURE);
	}

	while (token != NULL) {
		tokens[count++] = token;

		if (tokens >= (char **)TOKENS_CAPACITY) {
			tokens = realloc(tokens, 2 * INPUT_CAPACITY);
			if (!(tokens)) {
				fputs("Failed to reallocate memory!\n", stderr);
				exit(EXIT_FAILURE);
			}
		}
		token = strtok(NULL, " \n");
	}
	tokens[count] = NULL;
	return tokens;
} 

int cd_function(char **args) {
	if (args[1] == NULL) {
		fputs("cd (directory)\n", stderr);
	} else if (chdir(args[1]) != 0) {
		fputs("cd\n", stderr);
	} 

	return 1;
}

char *define_functions[] = {
	"cd"
};

int (*exec_functions[]) (char **args) = {
	&cd_function
};

int length(void) {
	return (sizeof(define_functions) / (sizeof(char*)));
}

int start(char **args) {
	int status = 0;
	pid_t pid = fork();

	if (pid == 0) {
		if (execvp(args[0], args) == -1) {
			fputs("Helish!\n", stderr);
		}
	} else if (pid < 0) {
		fputs("Failed to create a process!\n", stderr);
		exit(EXIT_FAILURE);
	} else {
		do {
			waitpid(pid, &status, WUNTRACED);
		} while (!(WIFEXITED(status)) && (!(WIFSIGNALED(status))));
	}

	return 1;
}

int exec(char **args) {
	if (args[0] == NULL) {
		fputs("Command not found!\n", stderr);
	}

	for (int i = 0; i < length(); i++) {
		printf("length: %d\n", length());
		if (strcmp(args[0], define_functions[i]) == 0) {
			return (*exec_functions[i])(args);
		}
	}
	return start(args);
}

int main(void) {
	char *line = NULL;
	char **args = NULL;
	int status = 0;

	do {
		printf("> ");
		line = input();
		args = parse(line);
		status = exec(args);

		free(line);
		free(args);
	} while (status);

	return 0;
}
