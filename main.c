#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

#include <sys/wait.h>
#include <sys/stat.h>

#include "memory.h"

#define INPUT_CAPACITY 4096
#define TOKEN_CAPACITY 1024

Arena a_input = {0};
Arena a_tokens = {0};

char *input(void) {
	char *line = arena_alloc(&a_input, INPUT_CAPACITY);
	if (fgets(line, INPUT_CAPACITY, stdin) == NULL) {
		fputs("Failed to read input!\n", stderr);
	}

	return line;
}

char **tokenize(char *line) {
	usize count = 0;
	char *token = strtok(line, " \t\r\n\a");
	char **tokens = arena_alloc(&a_tokens, TOKEN_CAPACITY);

	while (token != NULL) {
		tokens[count++] = token;
		token = strtok(NULL, " \t\r\n\a");
	}

	tokens[count] = NULL;
	return tokens;
}

i8 exit_function(void) {
	exit(1);
}

i8 cd_function(char **args) {
	if (args[1] == NULL) {
		fputs("cd <directory>\n", stderr);
	}

	chdir(args[1]);
	return 1;
}

i8 rm_function(char **args) {
	if (args[1] == NULL) {
		fputs("rm <file>\n", stderr);
	}

	remove(args[1]);
	return 1;
}

i8 move_function(char **args) {
	if ((args[1] == NULL) || (args[1] == NULL)) {
		fputs("move <file> <file>\n", stderr);
	}

	rename(args[1], args[2]);
	return 1;
}

i8 mkdir_function(char **args) {
	if (args[1] == NULL) {
		fputs("mkdir <directory>\n", stderr);
	}

	mkdir(args[1], S_IRWXU | S_IRWXG | S_IRWXO);
	return 1;
}


i8 touch_function(char **args) {
	if (args[1] == NULL) {
		fputs("touch <file>\n", stderr);
	}

	FILE *f = fopen(args[1], "w");
	if (f != NULL) {
		fclose(f);
	}

	return 1;
}

i8 echo_function(char **args) {
	printf("%s\n", args[1]);
	return 1;
}

i8 ls_function(char **args) {
	DIR *dir_r;
	struct dirent *dirent_r;

	if (args[1] == NULL) {
		dir_r = opendir(".");
		if (dir_r != NULL) {
			while ((dirent_r = readdir(dir_r)) != NULL) {
				printf("%s\n", dirent_r->d_name);
			}
			closedir(dir_r);
		}
	} else if (args[1] != NULL) {
		dir_r = opendir(args[1]);
		if (dir_r != NULL) {
			while((dirent_r = readdir(dir_r)) != NULL) {
				printf("%s\n", dirent_r->d_name);
			}
		}
		closedir(dir_r);
	}
	return 1;
}

i8 clear_function(void) {
	fputs("\e[1;1H\e[2J", stdout);
	return 1;
}

i8 cp_function(char **args) {
	if ((args[1] == NULL) || (args[2] == NULL)) {
		fputs("cp <file> <file>\n", stderr);
	}

	char buf[4096] = {0};

	FILE *f1 = fopen(args[1], "r");
	FILE *f2 = fopen(args[2], "w");
	if ((f1 != NULL) && (f2 != NULL)) {
		while (1) {
			usize read_buf = fread(buf, 1, sizeof(buf), f1);
			if (read_buf == 0) {
				break;
			}

			fwrite(buf, 1, sizeof(buf), f2);
		}
		fclose(f1);
		fclose(f2);
	}
	return 1;
}

i8 start(char **args) {
	int status = 0;
	pid_t pid = fork();

	if (pid == 0) {
		execvp(args[0], args);
	} else if (pid < 0) {
		fputs("Failed to create a process!\n", stderr);
	} else {
		do {
			waitpid(pid, &status, WUNTRACED);
		} while (!(WIFEXITED(status)) && (!(WIFSIGNALED(status))));
	} 
	return 1;
}

i8 parse(char **args) {
	if (strcmp(args[0], "cd") == 0) {
		return cd_function(args);
	} else if (strcmp(args[0], "rm") == 0) {
		return rm_function(args);
	} else if (strcmp(args[0], "move") == 0) {
		return move_function(args);
	} else if (strcmp(args[0], "mkdir") == 0) {
		return mkdir_function(args);
	} else if (strcmp(args[0], "touch") == 0) {
		return touch_function(args);
	} else if (strcmp(args[0], "echo") == 0) {
		return echo_function(args);
	} else if (strcmp(args[0], "ls") == 0) {
		return ls_function(args);
	} else if (strcmp(args[0], "clear") == 0) {
		return clear_function();
	} else if (strcmp(args[0], "cp") == 0) {
		return cp_function(args);
	} else if (strcmp(args[0], "exit") == 0) {
		return exit_function();
	} else {
		fputs("Command not found!\n", stderr);
	}

	return start(args);
}

int main(void) {
	char buf_input[INPUT_CAPACITY] = {0};
	char buf_token[TOKEN_CAPACITY] = {0};

	arena_init(&a_input, buf_input, sizeof(buf_input));
	arena_init(&a_tokens, buf_token, sizeof(buf_token));

	i8 status = 0;
	char *line = NULL;
	char **args = NULL;

	do {
		printf("> ");

		line = input();
		args = tokenize(line);
		status = parse(args);

		arena_free(&a_input);
		arena_free(&a_tokens);
	} while (status);

	return 0;
}
