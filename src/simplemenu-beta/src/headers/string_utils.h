#ifndef STRING_UTILS
#define STRING_UTILS
char *replaceWord(const char *s, const char *oldW, const char *newW);
char *toLower (char* string);
char *toUpper (char* string);
int positionWhereGameNameStartsInFullPath (char* string);
char *getNameWithoutExtension(char *fileName);
char *getNameWithoutPath(char *fileName);
char *getRomPath (char *fileName);
char *getExtension (char *stringWithExtension);
void stripGameName(char *gameName);
void stripGameNameLeaveExtension(char *gameName);
char *getGameName(char *gameName);
int sortStringArray (const void *lhs, const void *rhs);
char *getAliasWithoutAlternateNameOrParenthesis(char *fileName);
#endif
