#include<unistd.h>
#include<sys/wait.h>
#include<string.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>

char *path[100];
int pathNumber = 1;
char error_message[30] = "An error has occurred\n";

int main(int argc, char *argv[]){
	path[0] = strdup("/bin");
	//continually runs wish bash until exit
	if (argc == 1){
		char *input;
		size_t size = 10;
		size_t characters;
		input = (char*) malloc(size * sizeof(char));
		//loop input until exit is called
restart:	
		printf("wish> ");
		while ((characters = getline(&input, &size, stdin))){
			input[strlen(input)-1] = '\0';
			//check if exit is called
			if (strcmp(input, "exit") == 0){
				exit(0);
			}
			//check if cd or path is called
			char *inputCopy = strdup(input);
			char *builtIn;
			if ((builtIn = strsep(&inputCopy, " ")) != NULL){
				//if it's cd
				if (strcmp(builtIn, "cd") == 0){
					char* newPlace;
					char* test;
					if ((newPlace = strsep(&inputCopy, " ")) == NULL){
					}
					else if ((test = strsep(&inputCopy, " ")) != NULL){
					}
					else if (chdir(newPlace) != 0){
					}
					else{
						goto restart;
					}
				}
				//if it's path
				else if (strcmp(builtIn, "path") == 0){
					if ((builtIn = strsep(&inputCopy, " ")) == NULL){
						for (int z = 0; z < pathNumber; z++){
							path[z] = '\0';
						}
					}
					else{
						path[pathNumber] = builtIn;
						pathNumber++;
						while ((builtIn = strsep(&inputCopy, " ")) != NULL){
							path[pathNumber] = builtIn;
							pathNumber++;
						}
					}
					goto restart;
				}

			}
			//this is where we create the child process
			int rc = fork();
			if (rc == 0){
				char *found;
				char *arr[10];
				//loop through the different arguments within a given line to put in execv
				int i = 0;
				while ((found = strsep(&input, " ")) != NULL){
					if (strcmp(found, ">") == 0){
						char* file;
						char* test;
						if ((file = strsep(&input, " ")) == NULL){
							write(STDERR_FILENO, error_message, strlen(error_message));
							exit(0);
						}	
						else if ((test = strsep(&input, " ")) != NULL){
							write(STDERR_FILENO, error_message, strlen(error_message));
							exit(0);
						}
						else{
							(void) close(STDOUT_FILENO);
							open(file, O_WRONLY | O_CREAT | O_TRUNC);
							goto exec;
						}

					}
					arr[i]=strdup(found);
					i++;
				}
				arr[i] = NULL;
				//find the right execv directory and run it in there or else error
exec:
				for (int j = 0; j < pathNumber; j++){
					if (access(path[j], X_OK) == 0){
						strcat(path[j], "/");
						strcat(path[j], arr[0]);
						execv(path[j], arr);
					}
				}
				write(STDERR_FILENO, error_message, strlen(error_message));
				exit(0);
			}
			if (rc > 0){
				(void)wait(NULL);
				printf("wish> ");
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
		size_t size = 10;
		ssize_t characters;
		input = (char*) malloc(size* sizeof(char));
restartFile:
		while ((characters = getline(&input, &size, fp)) != -1){
			input[strlen(input-1)] = '\0';
			//check if line has exit
			if (strcmp(input, "exit") == 0){
				fclose(fp);
				fp = NULL;
				exit(0);
			}
			//check if cd or path is called
                        char *inputCopy = strdup(input);
                        char *builtIn;
                        if ((builtIn = strsep(&inputCopy, " ")) != NULL){
                                //if it's cd
                                if (strcmp(builtIn, "cd") == 0){
                                        char* newPlace;
                                        char* test;
                                        if ((newPlace = strsep(&inputCopy, " ")) == NULL){
                                                write(STDERR_FILENO, error_message, strlen(error_message));
                                        }
                                        else if ((test = strsep(&inputCopy, " ")) != NULL){
                                                write(STDERR_FILENO, error_message, strlen(error_message));
                                        }
                                        else if (chdir(newPlace) != 0){
                                                write(STDERR_FILENO, error_message, strlen(error_message));
                                        }
                                        else{
                                                goto restartFile;
                                        }
                                }
                                //if it's path
                                else if (strcmp(builtIn, "path") == 0){
                                        if ((builtIn = strsep(&inputCopy, " ")) == NULL){
                                                for (int z = 0; z < pathNumber; z++){
                                                        path[z] = '\0';
                                                }
                                        }
                                        else{
                                                path[pathNumber] = builtIn;
                                                pathNumber++;
                                                while ((builtIn = strsep(&inputCopy, " ")) != NULL){
                                                        path[pathNumber] = builtIn;
                                                        pathNumber++;
                                                }
                                        }
                                        goto restartFile;
                                }

                        }
			//create child process
			int rc = fork();
			if (rc == 0){
				char *found;
				char *arr[10];
				//loop through any arguments per line and put them in an array for execv
				int i = 0;
				while ((found = strsep(&input, " ")) != NULL){
					if (strcmp(found, ">") == 0){
						char *file;
						char *test;
						if ((file = strsep(&input, " ")) == NULL){
							write(STDERR_FILENO, error_message, strlen(error_message));
                                                        exit(0);
						}
						else if ((test = strsep(&input, " ")) != NULL){
                                                        write(STDERR_FILENO, error_message, strlen(error_message));
                                                        exit(0);
                                                }
						else{
                                                        (void) close(STDOUT_FILENO);
                                                        open(file, O_WRONLY | O_CREAT | O_TRUNC);
                                                        goto execFile;
                                                }
					}
					arr[i]=strdup(found);
					i++;
				}
				arr[i] = NULL;
				//find right execv directory to run
execFile:
				for (int j = 0; j < pathNumber; j++){
					if (access(path[j], X_OK) == 0){
						strcat(path[j], "/");
						strcat(path[j], arr[0]);
						execv(path[j], arr);
					}
				}
				write(STDERR_FILENO, error_message, strlen(error_message));
                                exit(0);
			}
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
