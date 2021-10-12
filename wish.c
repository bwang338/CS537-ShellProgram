#include<unistd.h>
#include<sys/wait.h>
#include<string.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>
#include<ctype.h>

char *path[100];
int pathNumber = 1;
char error_message[30] = "An error has occurred\n";
int redirection = 0;
char *newFile;

char *trimwhitespace(char *str){
	char *end;
	while(isspace((unsigned char)*str)) str++;
	if(*str == 0) return str;
	end = str + strlen(str) - 1;
	while(end > str && isspace((unsigned char)*end)) end--;
	end[1] = '\0';
	return str;
}

void loop(char *argv[], int size){
	if (size < 3){
		write(STDERR_FILENO, error_message, strlen(error_message));
		return;
	}
	int numLoops = atoi(argv[1]);
	if (numLoops < 0){
		write(STDERR_FILENO, error_message, strlen(error_message));
		return;
	}
	for (int i = 0; i < strlen(argv[1]); i++){
		if (argv[1][i] > '9' || argv[1][i] < '0'){
			write(STDERR_FILENO, error_message, strlen(error_message));
                	return;
		}
	}
	char *arr[10];
	for (int i = 0; i < size - 2; i++){
		arr[i] = strdup(argv[i+2]);
	}
	int newSize = size - 2;
	int countLoops = 0;
	int loopVariable = 0;
	int indexOfLoop;
	for (int i = 0; i < newSize; i++){
		if (strcmp(arr[i],"$loop") == 0){
			countLoops = 1;
			indexOfLoop = i;
			break;
		}
	}
	arr[newSize] = NULL;
	for (int i = 0; i < numLoops; i++){
		int rc = fork();
		if (rc == 0){
			int j;
			for (j = 0; j < pathNumber; j++){
				if (access(path[j], X_OK) == 0){
					if (countLoops == 1){
                                                loopVariable = i + 1;
                                                char text[20];
                                                sprintf(text, "%d", loopVariable);
                                                arr[indexOfLoop] = text;
                                        }
					strcat(path[j], "/");
					strcat(path[j], arr[0]);
					execv(path[j], arr);
				}
			}
			if (j == pathNumber){
				write(STDERR_FILENO, error_message, strlen(error_message));
				exit(0);
			}
		}
		//parent
		else{
			(void) wait(NULL);
		}
	}
	return;

}

int main(int argc, char *argv[]){
	path[0] = strdup("/bin");
	//continually runs wish bash until exit
	if (argc == 1){
		char *input;
		size_t size = 100;
		ssize_t characters;
		input = (char*) malloc(size * sizeof(char));
restart:	
		//loop input until exit is called
		printf("wish> ");
		while ((characters = getline(&input, &size, stdin))){
			if (strlen(input) == 1) {
				goto restart;
			}
			input[strlen(input)-1] = '\0';
			//check for redirection
			int count = 0;
			for (int i = 0; i < strlen(input); i++){
				if (input[i] == '>'){
					count++;
				}
				
			}
			if (count > 1){
				write(STDERR_FILENO, error_message, strlen(error_message));
                                goto restart;
			}
			char *inputDuplicate = strdup(input);
			char *commands;
			redirection = 0;
			newFile = NULL;
			if (count == 1){
				commands = strsep(&inputDuplicate, ">");
				if (strlen(commands) == 0){
                                        write(STDERR_FILENO, error_message, strlen(error_message));
                                        goto restartFile;
                                }
				input = strdup(commands);
				newFile = strsep(&inputDuplicate, ">");
				newFile = trimwhitespace(newFile);
				char *newFileDuplicate = strdup(newFile);
				if (strchr(newFileDuplicate, ' ') != NULL){
					write(STDERR_FILENO, error_message, strlen(error_message));
					goto restart;
				}
				else redirection = 1;
			}
			char *found;
			char *arr[10];
			//loop through the different arguments within a given line to put in execv
			int i = 0;
			while ((found = strsep(&input, " \t\n\r")) != NULL){
				if (strcmp(found, "") == 0) continue;
				arr[i]=strdup(found);
				i++;
			}
			if (i == 0){
				goto restart;
			}
			arr[i] = NULL;

			//check if first argument is loop
			if (strcmp(arr[0], "loop") == 0){
				loop(arr, i);
				goto restart;
			}
			//check if first argument is exit
                        if (strcmp(arr[0], "exit") == 0){
                                if (i != 1){
					write(STDERR_FILENO, error_message, strlen(error_message));
                                        goto restart;
				}
				free(input);
				exit(0);
                        }
                        //check if first argument is cd
                        if (strcmp(arr[0], "cd") == 0){
                                if (i != 2){
                                        write(STDERR_FILENO, error_message, strlen(error_message));
                                	goto restart;
				}
				if (chdir(arr[1]) != 0){
					write(STDERR_FILENO, error_message, strlen(error_message)); 
					goto restart;
				}
				goto restart;
                        }
                        //check if first argument is path
                        if (strcmp(arr[0], "path") == 0){
                                if (i == 1){
                                        for (int pathChar = 0; pathChar < pathNumber; pathChar++){
                                                path[pathChar] = '\0';
                                        }
                                }
                                else{
                                        pathNumber = 0;
                                        for (int pathElement = 1; pathElement < i; pathElement++){
                                                path[pathNumber] = arr[pathElement];
                                                pathNumber++;
                                        }
                                }
                                goto restart;
                        }
			//find the right execv directory and run it in there or else error
			int rc = fork();
			if (rc == 0){
				if (redirection == 1){
					(void) close(STDOUT_FILENO);
					if(open(newFile, O_WRONLY | O_CREAT | O_TRUNC, 0666) == -1){
						write(STDERR_FILENO, error_message, strlen(error_message));
						exit(0);
					}
				}
				int j;
				for (j = 0; j < pathNumber; j++){
					if (access(path[j], X_OK) == 0){
						strcat(path[j], "/");
						strcat(path[j], arr[0]);
						execv(path[j], arr);
					}
				}
				if (j == pathNumber){
					write(STDERR_FILENO, error_message, strlen(error_message));
					exit(0);
				}
			}
			if (rc > 0){
				(void)wait(NULL);
				goto restart;
			}
		}
	}
	//reads input from file
	else if (argc == 2){
		FILE *fp;
		fp = fopen(argv[1], "r");
		if (fp == NULL){
			write(STDERR_FILENO, error_message, strlen(error_message)); 
			exit(1);
		}
		char *input;
		size_t size = 100;
		ssize_t characters;
		input = (char*) malloc(size* sizeof(char));
restartFile:
		while ((characters = getline(&input, &size, fp)) != -1){
			if (strlen(input) <= 1){
				goto restartFile;
			}
			input[strlen(input)-1] = '\0';
			//check for redirection
			int count = 0;
                        for (int i = 0; i < strlen(input); i++){
                                if (input[i] == '>'){
                                        count++;
                                }
                        }
                        if (count > 1){
                                write(STDERR_FILENO, error_message, strlen(error_message));
                                goto restartFile;
                        }
                        char *inputDuplicate = strdup(input);
                        char *commands;
			redirection = 0;
                        newFile = NULL;
                        if (count == 1){
                                commands = strsep(&inputDuplicate, ">");
                                if (strlen(commands) == 0){
					write(STDERR_FILENO, error_message, strlen(error_message));
                                        goto restartFile;
				}
				input = strdup(commands);
                                newFile = strsep(&inputDuplicate, ">");
                                newFile = trimwhitespace(newFile);
                                char *newFileDuplicate = strdup(newFile);
                                if (strchr(newFileDuplicate, ' ') != NULL){
                                        write(STDERR_FILENO, error_message, strlen(error_message));
                                        goto restartFile;
                                }
				else redirection = 1;
                        }
			char *found;
			char *arr[100];
			//loop through any arguments per line and put them in an array
			int i = 0;
			while ((found = strsep(&input, " \t\n\r")) != NULL){
				if (strcmp(found, "") == 0) continue;
				arr[i]=strdup(found);
				i++;
			}
			if (i == 0) goto restartFile;
			arr[i] = NULL;
			
			//check if first argument is loop
                        if (strcmp(arr[0], "loop") == 0){
                                loop(arr, i);
                                goto restartFile;
                        }

			//check if first argument is exit
			if (strcmp(arr[0], "exit") == 0){
				if (i != 1){
                                        write(STDERR_FILENO, error_message, strlen(error_message));
                                        goto restartFile;
                                }
				fclose(fp);
				free(input);
				exit(0);
			}
			//check if first argument is cd
			if (strcmp(arr[0], "cd") == 0){
				if (i != 2){
					write(STDERR_FILENO, error_message, strlen(error_message));
					goto restartFile;
				}
				if (chdir(arr[1]) != 0){
					write(STDERR_FILENO, error_message, strlen(error_message));
					goto restartFile;
				}
				goto restartFile;
			}
			//check if first argument is path
			if (strcmp(arr[0], "path") == 0){
				if (i == 1){
					for (int pathChar = 0; pathChar < pathNumber; pathChar++){
						path[pathChar] = '\0';
					}
				}
				else{
					pathNumber = 0;
					for (int pathElement = 1; pathElement < i; pathElement++){
						path[pathNumber] = arr[pathElement];
						pathNumber++;	
					}
				}
				goto restartFile;
			}
			//find right execv directory to run
			int rc = fork();
			if (rc == 0){
				if (redirection == 1){
					
                                        (void) close(STDOUT_FILENO);
                                        if (open(newFile, O_WRONLY | O_CREAT | O_TRUNC, 0666) == -1){
						write(STDERR_FILENO, error_message, strlen(error_message));
						exit(0);
					}
                                }
				int j;
				for (j = 0; j < pathNumber; j++){
					if (access(path[j], X_OK) == 0){
						strcat(path[j], "/");
						strcat(path[j], arr[0]);
						execv(path[j], arr);
					}
				}
				if (j == pathNumber){
					write(STDERR_FILENO, error_message, strlen(error_message));
					exit(0);
				}
			}
			//parent process
			if (rc > 0){
				(void) wait(NULL);
			}

		}
	}
	//wrong number of arguments
	else{
		write(STDERR_FILENO, error_message, strlen(error_message));
		exit(1);
	}
}
