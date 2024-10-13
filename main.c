#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "memory.h"

#define INPUT_CAPACITY 1024
#define TOKENS_CAPACITY 4096

Arena a = {0};

char *input(void) {
	char *line = arena_alloc(&a, INPUT_CAPACITY);
	if (fgets(line, INPUT_CAPACITY, stdin) == NULL) {
		fputs("Failed to read input!\n", stderr);
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
	"cd",
	"rm",
	"mv",
	"mkdir",
	"touch",
	"cat",
	"echo",
	"ls",
	"clear",
	"grep",
	"cp"
};

int length(void) {
	return (sizeof(define_functions) / sizeof(char *));
}

int cd_function(char **args) {
	if (args[1] == NULL) {
		fputs("cd <directory>\n", stderr);
	} else if (chdir(args[1]) != 0) {
		fputs("", stderr);
	}

	return 1;
}

int rm_function(char **args) {
	if (args[1] == NULL) {
		fputs("rm <file>\n", stderr);
	} else if (remove(args[1]) != 0) {
		fputs("", stderr);
	}

	return 1;
}

int mv_function(char **args) {
	if ((args[1] == NULL) || (args[2] == NULL)) {
		fputs("mv <file> <file>\n", stderr);
	} else if (rename(args[1], args[2]) != 0) {
		fputs("", stderr);
	}

	remove(args[1]);
	return 1;
}

int mkdir_function(char **args) {
	if (args[1] == NULL) {
		fputs("mkdir <directory>\n", stderr);
	} else if (mkdir(args[1], S_IRWXU | S_IRWXG | S_IRWXO) != 0) {
		fputs("", stderr);
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

int cat_function(char **args) {
	if (args[1] == NULL) {
		fputs("cat <file>\n", stderr);
	}

	unsigned char buf[1 << 12];

	FILE *f = fopen(args[1], "r");
	if (f != NULL) {
		while (1) {
			size_t read_buf = fread(buf, 1, sizeof(buf), f);
			if (read_buf == 0) {
				break;
			}
		}
		printf("%s\n", buf);
	}

	if (f != NULL) {
		fclose(f);
	}

	return 1;
}

int echo_function(char **args) {
	if (args[1] == NULL) {
		fputs("echo <message>\n", stderr);
	}

	printf("%s\n", args[1]);

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
		}
	} else if (args[1] != NULL) {
		dir_r = opendir(args[1]);
		if (dir_r != NULL) {
			while ((dirent_r = readdir(dir_r)) != NULL) {
				printf("%s\n", dirent_r->d_name);
			}
		}
	}

	if (dir_r != NULL) {
		closedir(dir_r);
	}

	return 1;
}

int clear_function(char **args) {
	args[1] = NULL;

	fputs("\e[1;1H\e[2J", stdout);

	return 1;
}

int grep_function(char **args) {
	if ((args[1] == NULL) || (args[2] == NULL)) {
		fputs("grep <message> <file>\n", stderr);
	}

	unsigned char buf[1 << 12];

	FILE *f = fopen(args[2], "r");
	if (f != NULL) {
		while (1) {
			size_t read_buf = fread(buf, 1, sizeof(buf), f);
			if (read_buf == 0) {
				break;
			}

			char *result = strstr((char *)buf, args[1]);
			printf("%s\n", result);
		}
	}

	if (f != NULL) {
		fclose(f);
	}
 
	return 1;
}

int cp_function(char **args) {
	if ((args[1] == NULL) || (args[2] == NULL)) {
		fputs("cp <file> <file>\n", stderr);
	}

	unsigned char buf[1 << 12];

	FILE *f1 = fopen(args[1], "r");
	FILE *f2 = fopen(args[2], "w");
	if ((f1 != NULL) && (f2 != NULL)) {
		while (1) {
			size_t read_buf = fread(buf, 1, sizeof(buf), f1);
			if (read_buf == 0) {
				break;
			}

			fwrite(buf, 1, read_buf, f2);
		}
	}

	if ((f1 != NULL) && (f2 != NULL)) {
		fclose(f1);
		fclose(f2);
	}

	return 1;
}

int (*exec_functions[]) (char **args) = {
	&cd_function,
	&rm_function,
	&mv_function,
	&mkdir_function,
	&touch_function,
	&cat_function,
	&echo_function,
	&ls_function,
	&clear_function,
	&grep_function,
	&cp_function
};

int start(char **args) {
	int status = 0;
	pid_t pid = fork();

	if (pid == 0) {
		if (execvp(args[0], args) == -1) {
			fputs("Failed to replace a process imagine!\n", stderr);
		}
	} else if (pid < 0) {
		fputs("Failed to create a process!\n", stderr);
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

	return 1;
}
