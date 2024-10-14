#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <curl/curl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "memory.h"

#define TOKENS_CAPACITY 4096
#define INPUT_CAPACITY 1024

Arena a = {0};

char *input(void) {
	char *line = arena_alloc(&a, INPUT_CAPACITY);
	if (fgets(line, INPUT_CAPACITY, stdin) == NULL) {
		fputs("Failed to read input!\n", stderr);
	}

	return line;
}

char **parse(char *line) {
	usize count = 0;
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
	"curl",
	"cp"
};

usize length(void) {
	return (sizeof(define_functions) / sizeof(char *));
}

i8 cd_function(char **args) {
	if (args[1] == NULL) {
		fputs("cd <directory>\n", stderr);
	}

	(void) chdir(args[1]);
	return 1;
}

i8 rm_function(char **args) {
	if (args[1] == NULL) {
		fputs("rm <file>\n", stderr);
	}

	remove(args[1]);
	return 1;
}

i8 mv_function(char **args) {
	if ((args[1] == NULL) || (args[2] == NULL)) {
		fputs("mv <file> <file>\n", stderr);
	}

	rename(args[1], args[2]);
	remove(args[1]);
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

// not done.
i8 cat_function(char **args) {
	if (args[1] == NULL) {
		fputs("cat <file>\n", stderr);
	}

	unsigned char buf[1 << 12];

	FILE *f = fopen(args[1], "r");
	if (f != NULL) {
		while (1) {
			usize read_buf = fread(buf, 1, sizeof(buf), f);
			if (read_buf == 0) {
				break;
			}
			printf("%s\n", buf);
		}
		fclose(f);
	}

	return 1;
}

// not done.
i8 echo_function(char **args) {
	if (args[1] == NULL) {
		fputs("echo <message>\n", stderr);
	}

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
			while ((dirent_r = readdir(dir_r)) != NULL) {
				printf("%s\n", dirent_r->d_name);
			}
			closedir(dir_r);
		}
	}

	return 1;
}

i8 clear_function(char **args) {
	args[1] = NULL;

	fputs("\e[1;1H\e[2J", stdout);

	return 1;
}

i8 grep_function(char **args) {
	if ((args[1] == NULL) || (args[2] == NULL)) {
		fputs("grep <message> <file>\n", stderr);
	}

	unsigned char buf[1 << 12];

	FILE *f = fopen(args[2], "r");
	if (f != NULL) {
		while (1) {
			usize read_buf = fread(buf, 1, sizeof(buf), f);
			if (read_buf == 0) {
				break;
			}

			char *result = strstr((char *)buf, args[1]);
			printf("%s\n", result);
		}
		fclose(f);
	}
 
	return 1;
}

i8 curl_function(char **args) {
	if ((args[1] == NULL) || (args[2] == NULL)) {
		fputs("curl <url> <output>\n", stderr);
	}

	CURL *curl;
	curl = curl_easy_init();
	CURLcode res = 0;

	if (curl) {
		FILE *f = fopen(args[2], "w");
		if (f != NULL) {
			curl_easy_setopt(curl, CURLOPT_URL, args[1]);
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);
			res = curl_easy_perform(curl);

			if (res == CURLE_OK) {
				curl_easy_cleanup(curl);
			}
			fclose(f);
		}
	}

	return 1;
}

i8 cp_function(char **args) {
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
		fclose(f1);
		fclose(f2);
	}

	return 1;
}

i8 (*exec_functions[]) (char **args) = {
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
	&curl_function,
	&cp_function
};

i8 start(char **args) {
	int status = 0;
	pid_t pid = fork();

	if (pid == 0) {
		if (execvp(args[0], args) == -1) {
			fputs("Command not found!\n", stderr);
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

i8 exec(char **args) {
	for (usize i = 0; i < length(); i++) {
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
	i8 status = 0;

	do {
		printf("> ");

		line = input();
		args = parse(line);
		status = exec(args);

		arena_free(&a);
	} while (status);

	return 1;
}
