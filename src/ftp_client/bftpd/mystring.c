#include <stdio.h>
#include <string.h>
#include <stdlib.h>

size_t pos(char *haystack, char *needle)
{
        char *needle_position;
        size_t found_length, haystack_length;
        needle_position = strstr(haystack, needle);
        if (needle_position)
        {
           found_length = strlen(needle_position);
           haystack_length = strlen(haystack);
           return haystack_length - found_length;
        }
        /*
	if (strstr(haystack, needle)) {
		return (int) strstr(haystack, needle) - (int) haystack;
	}
        */ 
        else {
		return -1;
	}
}

void cutto(char *str, int len)
{
	memmove(str, str + len, strlen(str) - len + 1);
}

void mystrncpy(char *dest, char *src, int len)
{
	strncpy(dest, src, len);
	*(dest + len) = 0;
}


/*
Search through str looking for what. Replace any what with
the contents of by. Do not let the string get longer than max_length.
*/
int replace(char *str, char *what, char *by, int max_length)
{
	char *foo, *bar = str;
        int i = 0;
        int str_length, what_length, by_length;

        /* do a sanity check */
        if (! str) return 0;
        if (! what) return 0;
        if (! by) return 0;

        what_length = strlen(what);
        by_length = strlen(by);
        str_length = strlen(str);

        foo = strstr(bar, what);
        /* keep replacing if there is something to replace and it
           will no over-flow
        */
	while ( (foo) && 
                ( (str_length + by_length - what_length) < (max_length - 1) ) ) 
        {
            bar = foo + strlen(by);
            memmove(bar, foo + strlen(what), strlen(foo + strlen(what)) + 1);
	    memcpy(foo, by, strlen(by));
            i++;
            foo = strstr(bar, what);
            str_length = strlen(str);
	}
    return i;
}

/* int_from_list(char *list, int n) 
 * returns the n'th integer element from a string like '2,5,12-15,20-23'
 * if n is out of range or *list is out of range, -1 is returned.
 */

int int_from_list(char *list, int n)
{
    char *str, *tok;
    int count = -1, firstrun = 1;
    int list_length;

    if (! list) return -1;
    list_length = strlen(list);
    str = (char *) malloc(sizeof(char) * (list_length + 2));
    if (!str)
        return -1;
    
    memset(str, 0, list_length + 2);
    strncpy(str, list, list_length + 1);
    
    /* append ',' to the string so we can always use strtok() */
	str[list_length] = ',';
    
    for (;;) {
        if (firstrun)
            tok = strtok(str, ",");
        else
            tok = strtok(NULL, ",");
        
        if (!tok || *tok == '\0') {
			free(str);
            return -1;
		}
        
        if (strchr(tok, '-')) {
            char *start, *end;
            int s, e;
            start = tok;
            end = strchr(tok, '-');
            *end = '\0';
            end++;
            if (!*start || !*end) {
				free(str);
                return -1;
			}
            s = atoi(start);
            e = atoi(end);
            if (s < 1 || e < 1 || e < s) {
				free(str);
                return -1;
			}
            count += e - s + 1;
            if (count >= n) {
				free(str);
                return (e - (count - n));
			}
        } else {
            count++;
            if (count == n) {
				int val = atoi(tok);
				free(str);
                return val;
			}
        }
        firstrun = 0;
    }
}
