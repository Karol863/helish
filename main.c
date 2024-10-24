#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

#include <sys/wait.h>
#include <sys/stat.h>

#include "memory.h"

Arena a = {0};
Buffer b = {0};
char cwd[2048] = {0};

char *input(void) {
	char *line = arena_alloc(&a, CAPACITY);

	if (fgets(line, CAPACITY, stdin) == NULL) {
		fputs("Failed to read the input!\n", stderr);
	}

	return line;
}

char **tokenize(char *line) {
	usize count = 0;
	char *token = strtok(line, " \n");
	static char *tokens[COMMANDS] = {0};

	while ((token != NULL) && (count < COMMANDS)) {
		tokens[count++] = token;
		token = strtok(NULL, " \n");
	}

	for (usize i = 0; i < count; i++) {
		for (char *ch = tokens[i]; *ch != '\0'; ch++) {
			buffer_write(&b, *ch);
		}
		buffer_write(&b, ' ');
	}

	// thing of the path where'd i save the history.
	FILE *f = fopen("history.txt", "a");
	if (f != NULL) {
		for (usize i = b.tail; i != b.head; i = (i + 1) % COMMANDS) {
			fputc(b.buf[i], f);
		}
		fclose(f);
	}

	b.head = b.tail;

	tokens[count] = NULL;
	return tokens;
}

int exit_function(void) {
	exit(1);
}

// doesn't work when i attempt to check if the dir exists.
int cd_function(char **args) {
	if (args[1] == NULL) {
		fputs("cd <directory>\n", stderr);
	}

	struct stat st;

	if (stat(args[1], &st) == S_ISDIR(st.st_mode)) {
		chdir(args[1]);
	}

	getcwd(cwd, sizeof(cwd));
	return 1;
}

// figure out how to force removing.
int rm_function(char **args) {
	if (args[1] == NULL) {
		fputs("rm <file> or rm -r <directory>", stderr);
	}

	if (strcmp(args[0], "rm") == 0) {
		remove(args[1]);
	} else if (strcmp(args[0], "rmdir") == 0) {
		rmdir(args[1]);
	}

	return 1;
}

int mv_function(char **args) {
	if ((args[1] == NULL) || (args[2] =	NULL)) {
		fputs("mv <file> <file>\n", stderr);
	}

	rename(args[1], args[2]);
	return 1;
}

int mkdir_function(char **args) {
	if (args[1] == NULL) {
		fputs("mkdir <directory>\n", stderr);
	}

	struct stat st;

	if (stat(args[1], &st) == -1) {
		mkdir(args[1], 0777);
	}

	return 1;
}

int touch_function(char **args) {
	if (args[1] == NULL) {
		fputs("touch <file>\n", stderr);
	}

	FILE *f = fopen(args[1], "w");
	if (f != NULL) {
		fclose(f);
	}

	return 1;
}

int ls_function(char **args) {
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
			while ((dirent_r = readdir(dir_r)) != NULL) {
				printf("%s\n", dirent_r->d_name);
			}
			closedir(dir_r);
		}
	}

	return 1;
}

int clear_function(void) {
	fputs("\e[1;1H\e[2J", stdout);
	return 1;
}

int cp_function(char **args) {
	if ((args[1] == NULL) || (args[2] == NULL)) {
		fputs("cp <file> <file>\n", stderr);
	}

	char buf[4096];

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

// the flags doesn't seem correct.
int start(char **args) {
	int status = 0;
	pid_t pid = fork();

	if (pid == 0) {
		execvp(args[0], args);
	} else if (pid < 0) {
		fputs("Failed to create a process!\n", stderr);
	} else {
		do {
			waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && (!WIFSIGNALED(status)));
	}

	return 1;
}

int parse(char **args) {
	if (strcmp(args[0], "cd") == 0) {
		return cd_function(args);
	} else if (strcmp(args[0], "rm") == 0) {
		return rm_function(args);
	} else if (strcmp(args[0], "rmdir") == 0) {
		return rm_function(args);
	} else if (strcmp(args[0], "mv") == 0) {
		return mv_function(args);
	} else if (strcmp(args[0], "mkdir") == 0) {
		return mkdir_function(args);
	} else if (strcmp(args[0], "touch") == 0) {
		return touch_function(args);
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
	char buf[CAPACITY] = {0};
	char data[COMMANDS] = {0};

	arena_init(&a, buf, sizeof(buf));
	buffer_init(&b, data);

	int status = 0;
	char *line = NULL;
	char **args = NULL;

	getcwd(cwd, sizeof(cwd));

	do {
		printf("[%s]$ ", cwd);

		line = input();
		args = tokenize(line);
		status = parse(args);

		arena_free(&a);
	} while (status);

	return 0;
}

