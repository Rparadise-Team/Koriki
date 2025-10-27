#include <asm-generic/ioctls.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <unistd.h>

void writeCenteredMessage(char *string, int rows, int cols) {
	int vertl = rows/2;
	int stringLength = strlen(string) / 2;
	for (int x = 0; x <= rows; x++) {
		printf("\n");
		if (x == vertl) {
			printf("\n%*s\n", cols / 2 + stringLength, string);
		}
	}
}

char *getFilePath(char *fileName) {
	char *retstr;
	char *lastslash;
	retstr = malloc(strlen (fileName) + 1);
	strcpy (retstr, fileName);
	lastslash = strrchr (retstr, '/');
	if (lastslash != NULL) {
		*lastslash = '\0';
	}
	return retstr;
}

size_t cmd_param_split(char *buffer, char *argv[], size_t argv_max_size)
{
	char *p, *start_of_word;
	int c;
	enum states { DULL=0, IN_WORD, IN_STRING} state = DULL;
	size_t argc = 0;
	int quote = 0;
	for (p = buffer; argc < argv_max_size && *p != '\0'; p++) {
		c = (unsigned char) *p;
		switch (state) {
		case DULL:
			if (c == ' ') {
				continue;
			}

			if (c == '"' ||c == '\'') {
				quote = c;
				state = IN_STRING;
				start_of_word = p+1;
				continue;
			}
			state = IN_WORD;
			start_of_word = p;
			continue;

		case IN_STRING:
			if (c == '"' || c == '\'') {
				if (c!=quote)
					continue;
				else
					quote = 0;
				strncpy(argv[argc],start_of_word, p - start_of_word);
				argc++;
				state = DULL;
			}
			continue;

		case IN_WORD:
			if(quote==0 && (c == '\"' ||c == '\''))
				quote = c;
			else if (quote == c)
				quote = 0;

			if (c==' ' && quote==0) {
				strncpy(argv[argc],start_of_word, p - start_of_word);
				argc++;
				state = DULL;
			}
			continue;
		}
	}

	if (state != DULL && argc < argv_max_size){
		strncpy(argv[argc],start_of_word, p - start_of_word);
		argc++;
	}
	return argc;
}

char * replace(
    char const * const original,
    char const * const pattern,
    char const * const replacement
) {
  size_t const replen = strlen(replacement);
  size_t const patlen = strlen(pattern);
  size_t const orilen = strlen(original);

  size_t patcnt = 0;
  const char * oriptr;
  const char * patloc;

  // find how many times the pattern occurs in the original string
  for (oriptr = original; patloc = strstr(oriptr, pattern); oriptr = patloc + patlen)
  {
    patcnt++;
  }

  {
    // allocate memory for the new string
    size_t const retlen = orilen + patcnt * (replen - patlen);
    char * const returned = (char *) malloc( sizeof(char) * (retlen + 1) );

    if (returned != NULL)
    {
      // copy the original string,
      // replacing all the instances of the pattern
      char * retptr = returned;
      for (oriptr = original; patloc = strstr(oriptr, pattern); oriptr = patloc + patlen)
      {
        size_t const skplen = patloc - oriptr;
        // copy the section until the occurence of the pattern
        strncpy(retptr, oriptr, skplen);
        retptr += skplen;
        // copy the replacement
        strncpy(retptr, replacement, replen);
        retptr += replen;
      }
      // copy the rest of the string.
      strcpy(retptr, oriptr);
    }
    return returned;
  }
}

int main(int argc, char *argv[]) {
	if (argc == 0) {
		printf("Oops!\n");
		return -1;
	}
    int i=0;
	char menuDirectory[100] = "";
	char *directory=argv[1];
	char *executable=argv[2];
	char *fileToBeExecutedWithFullPath=argv[3];
	int ret=0;
	getcwd(menuDirectory, sizeof(menuDirectory));
	ret = chdir(directory);
	pid_t pid;
	pid = fork();
	if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
		//it's a section meant for native apps
		if (executable[0]=='#') {
			ret = chdir(menuDirectory);
			if (ret!=-1) {
				//Native opk with a desktop file as parameter
				printf("path1\n");
				char *params[20];
				printf("%s\n", fileToBeExecutedWithFullPath);
				if (strstr(fileToBeExecutedWithFullPath,"|")!=NULL) {
					printf("path1.1\n");
					params[0]="opkrun";
					char *ptr = strtok(fileToBeExecutedWithFullPath, "|");
					int i=1;
					while(ptr != NULL) {
						params[i]=malloc(strlen(ptr));
						strcpy(params[i],ptr);
						ptr = strtok(NULL, "|");
						printf("%s\n", params[i]);
						i++;
					}
					params[i]=NULL;
					if (strstr(params[2],"default.")!=NULL) {
						params[2][strlen(params[2])-1]='\0';
					}
					printf("%s\n", params[2]);
					ret = execvp("opkrun",params);
				} else {
					printf("path2\n");
					if(strstr(fileToBeExecutedWithFullPath,".opk")) {
						ret = execlp("opkrun","invoker",fileToBeExecutedWithFullPath,NULL);
					} else {
						//it's an executable
						char* dirToSwitch = getFilePath(fileToBeExecutedWithFullPath);
						chdir(dirToSwitch);
						//non opk emulator with params
						char localExec[100];
						char* args[64];
						int argsCount;
						for(i=0;i<64;i++){
							args[i] = malloc(256);
							memset(args[i],0x0,256);
						}
						argsCount = cmd_param_split(fileToBeExecutedWithFullPath,args,64);
						if(args[0][0]!='/'&&strcmp(args[0],"sh")!=0) {
							strcpy(localExec,"./");
						} else {
							strcpy(localExec,"");
						}
						strcat(localExec,args[0]);
						args[argsCount]=NULL;
						if (strcmp(args[0],"sh")==0) {
							ret = execlp("sh","sh","-c",fileToBeExecutedWithFullPath+6,NULL);
						} else {
							ret = execvp(localExec,args);
						}

					}
				}
			}
		} else {
			//it's an emulator
			if (strstr(executable,".opk")) {
				//it's an opk emulator
				if (strcmp(fileToBeExecutedWithFullPath,"*")==0) {
					ret=execlp("opkrun","invoker",executable,NULL);
				} else {
					ret=execlp("opkrun","invoker",executable,fileToBeExecutedWithFullPath,NULL);
				}
			} else {
				//non opk emulator with params
				char localExec[100];
				char* args[64];
				int argsCount;
				for(i=0;i<64;i++){
					args[i] = malloc(256);
					memset(args[i],0x0,256);
				}
				if(args[0][0]!='/') {
					strcpy(localExec,"./");
				} else {
					strcpy(localExec,"");
				}
				argsCount = cmd_param_split(executable,args,64);
				strcat(localExec,args[0]);
				args[argsCount]=fileToBeExecutedWithFullPath;
				args[argsCount+1]=NULL;
				ret = execvp(localExec,args);
			}
		}
	} else {
		    // Código padre: esperar y volver a simplemenu
    int status;
    waitpid(pid, &status, 0);

    // Volver al directorio donde está simplemenu
    if (chdir(menuDirectory) == -1) {
        perror("chdir");
        exit(EXIT_FAILURE);
    }

    // Reemplazar invoker por simplemenu
    execlp("./simplemenu", "simplemenu", "640", "480", NULL);

    // Si execlp falla:
    perror("execlp");
    exit(EXIT_FAILURE);
	}
}