#ifndef __BFTPD_MYSTRING_H
#define __BFTPD_MYSTRING_H

#define MAX_STRING_LENGTH 512

/* int pos(char *, char *); */
size_t pos(char *, char *);
void cutto(char *, int);
void mystrncpy(char *, char *, int);
int replace(char *, char *, char *, int);
char *readstr();
int int_from_list(char *list, int n);

#endif
