#ifndef UTILS_DEFINED
#define UTILS_DEFINED
void openLogFile();
void logMessage(const char* tag, const char* function, const char* message);
void closeLogFile();
void enableLogging();
void pushEvent();
#endif
