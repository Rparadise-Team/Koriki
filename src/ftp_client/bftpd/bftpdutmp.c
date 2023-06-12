#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include "main.h"
#include "bftpdutmp.h"
#include "mypaths.h"
#include "logging.h"
#include "options.h"

FILE *bftpdutmp = NULL;
long bftpdutmp_offset = 0xFFFFFFFF;

void bftpdutmp_create_dir(char *path_to_file)
{
    char *dir_name, *index;

    if (! path_to_file) return;

    dir_name = strdup(path_to_file);
    if (! dir_name)
       return;

    index = strrchr(dir_name, '/');
    if (index)
    {
        index[0] = '\0';   /* wipe out everything after folder name */
        mkdir(dir_name, 0755);
    }
    free(dir_name);
}



void bftpdutmp_init()
{
	char *filename = strdup(config_getoption("PATH_BFTPDUTMP"));

        if (! filename)
            return;

	if ((!strcasecmp(filename, "none")) || (!filename[0]))
        {
                free(filename);
		return;
        }

    /* try to create the directory, if it does not exist yet */
    bftpdutmp_create_dir(filename);

    /*  we have to create the file if it doesn't exist */
    bftpdutmp = fopen(filename, "a");
    if (bftpdutmp)
        fclose(bftpdutmp);
    /* Then we can open it for reading and writing */
    if (!(bftpdutmp = fopen(filename, "r+"))) {
        bftpd_log("Could not open log file %s.\n", filename);
        control_printf(SL_FAILURE, "421-Could not open file %s\r\n"
                 "421 Server disabled for security reasons.", filename);
        exit(1);
    }
    rewind(bftpdutmp);
    // clean up memory
    free(filename);
}

void bftpdutmp_end()
{
    if (bftpdutmp) {
        if (bftpdutmp_offset != -1)
            bftpdutmp_log(0);
        fclose(bftpdutmp);
        bftpdutmp = NULL;
    }
}

void bftpdutmp_log(char type)
{
    struct bftpdutmp ut, tmp;
    long i;
    int max_length;

    if (!bftpdutmp)
        return;
    memset((void *) &ut, 0, sizeof(ut));
    ut.bu_pid = getpid();
    if (type) {
        ut.bu_type = 1;
        strncpy(ut.bu_name, user, sizeof(ut.bu_name));
        max_length = sizeof(ut.bu_host);
        strncpy(ut.bu_host, remotehostname, max_length);
        ut.bu_host[max_length - 1] = '\0';     /* avoid non-null terminated string */
       /* Determine offset of first user marked dead */
        rewind(bftpdutmp);
        i = 0;
        while (fread((void *) &tmp, sizeof(tmp), 1, bftpdutmp)) {
            if (!tmp.bu_type)
                break;
            i++;
        }
        bftpdutmp_offset = i * sizeof(tmp);
    }
 
    else
        ut.bu_type = 0;

    time(&(ut.bu_time));
    fseek(bftpdutmp, bftpdutmp_offset, SEEK_SET);
    fwrite((void *) &ut, sizeof(ut), 1, bftpdutmp);
    fflush(bftpdutmp);
}

char bftpdutmp_pidexists(pid_t pid)
{
    struct bftpdutmp tmp;
	if (!bftpdutmp)
		return 0;
    rewind(bftpdutmp);
    while (fread((void *) &tmp, sizeof(tmp), 1, bftpdutmp)) {
        if ((tmp.bu_pid == pid) && (tmp.bu_type))
            return 1;
    }
    return 0;
}

int bftpdutmp_usercount(char *username)
{
    struct bftpdutmp tmp;
	int count = 0;
	if (!bftpdutmp)
		return 0;
    rewind(bftpdutmp);
    while ( fread((void *) &tmp, sizeof(tmp), 1, bftpdutmp) ) {
		/*
                 Took this out. It seems to just be taking up log space. -- Jesse 
                 bftpd_log("bu_name=%s; username=%s, bu_type=%i\n", tmp.bu_name, username, tmp.bu_type);
                */ 
        if (tmp.bu_type && ( !strcmp(tmp.bu_name, username) || !strcmp(username, "*")))
			count++;
    }

    if (count < 0)
       count = 1;
    return count;
}

int bftpdutmp_dup_ip_count(char *ip_address)
{
   struct bftpdutmp tmp;
   int count = 0;

   if (! bftpdutmp)
      return 0;

   rewind(bftpdutmp);
   while ( fread( (void *) &tmp, sizeof(tmp), 1, bftpdutmp) )
   {
      if (tmp.bu_type && (! strcmp(tmp.bu_host, ip_address) ) )
         count++; 
   }

   return count;
}



/*
This function removes a log entry of
a dead client. This is called
when the bftpd parent catches a
signal indicating the child/client died.
This makes it look like the child
logged out properly.
-- Jesse
*/
void bftpdutmp_remove_pid(int pid)
{
    struct bftpdutmp current;
    int index = 0;

    if (! bftpdutmp)
       return;

    rewind(bftpdutmp);
    /* search for a matching pid */
    while ( fread( (void *) &current, sizeof(current), 1, bftpdutmp) )
    {
       /* over-write the pid */
       if ( current.bu_pid == pid )
       {
           fseek(bftpdutmp, index * sizeof(current), SEEK_SET);
           memset(&current, 0, sizeof(current));
           fwrite( (void *) &current, sizeof(current), 1, bftpdutmp);
       }
       index++;
    }
}


