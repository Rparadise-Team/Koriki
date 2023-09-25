#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <SDL/SDL_events.h>

FILE * fp;
int loggingEnabled;

void enableLogging() {
	loggingEnabled = 1;
}

void openLogFile() {
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	char pathToLogFilePlusFileName[300];
	snprintf(pathToLogFilePlusFileName,sizeof(pathToLogFilePlusFileName),"%s/.simplemenu/%d-%02d-%02d.log",getenv("HOME"),tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
	fp = fopen(pathToLogFilePlusFileName, "a");
}

void logMessage(const char* tag, const char* function, const char* message) {
	if (loggingEnabled) {
		time_t now;
		time(&now);
		char *time = ctime(&now);
		time[strlen(time)-1]='\0';
		if (fp==NULL) {
			openLogFile();
		}
		printf("%s | %-5s | %-40s | %s\n", time, tag, function, message);
		fprintf(fp,"%s | %-5s | %-40s | %s\n", time, tag, function, message);

	}
}

void closeLogFile() {
	if (fp!=NULL) {
		fclose(fp);
	}
}

int isLoggingEnabled() {
	return loggingEnabled;
}

void pushEvent() {
//    SDL_Event sdlevent;
//    sdlevent.type = SDL_MOUSEMOTION;
//    sdlevent.motion.x = 1;
//    SDL_PushEvent(&sdlevent);
}
