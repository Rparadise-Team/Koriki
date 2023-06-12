#include <config.h>
#include <stdio.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_DIRENT_H
#  include <dirent.h>
#else
#    define dirent direct
#    define NAMLEN(dirent) (dirent)->d_namlen
#  ifdef HAVE_SYS_NDIR_H
#    include <sys/ndir.h>
#  endif
#  ifdef HAVE_SYS_DIR_H
#    include <sys/dir.h>
#  endif
#  ifdef HAVE_NDIR_H
#    include <ndir.h>
#  endif
#endif

#include <unistd.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <mystring.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#include <pwd.h>
#include <grp.h>
#include <errno.h>
/* #include <glob.h> */
#include <fnmatch.h>

#include "cwd.h"
#include "options.h"
#include "main.h"
#include "login.h"
#include "dirlist.h"


struct hidegroup *hidegroups = NULL;

void add_to_hidegroups(int gid)
{
    static struct hidegroup *tmp = NULL;
    if (tmp)
        tmp = tmp->next = malloc(sizeof(struct hidegroup));
    else
        hidegroups = tmp = malloc(sizeof(struct hidegroup));

    if (tmp)
    {
      tmp->next = NULL;
      tmp->data = gid;
    }
}

void hidegroups_init()
{
    char *foo = strdup(config_getoption("HIDE_GROUP"));
    char *foo_save = foo;
    char *bar;
    struct group *tmpgrp;

    if (! foo) return;     /* avoid segfault */
    while ((bar = strtok(foo, ","))) {
        foo = NULL; /* strtok requirement */
        if ((strcmp(bar, "0")) && (!strtoul(bar, NULL, 10))) {
            /* bar is not numeric */
            if ((tmpgrp = getgrnam(bar)))
                add_to_hidegroups(tmpgrp->gr_gid);
        } else
            if (strtoul(bar, NULL, 10))
                add_to_hidegroups(strtoul(bar, NULL, 10));
    }
    free(foo_save);
}

void hidegroups_end()
{
    struct hidegroup *tmp = hidegroups;
    if (hidegroups)
        while (hidegroups->next) {
            tmp = hidegroups->next;
            free(hidegroups);
            hidegroups = tmp;
        }
}

void bftpd_stat(char *name, FILE * client)
{
    struct stat statbuf;
	char temp[MAXCMD + 6], linktarget[MAXCMD + 128], perm[12], timestr[17],
		uid[USERLEN + 1], gid[USERLEN + 1];
    struct tm filetime;
    struct tm *tea_time, *local_time;
    time_t t;
    int status;

    if (lstat(name, (struct stat *) &statbuf) == -1) { // used for command_stat
        fprintf(client, "213-Error: %s.\n", strerror(errno));
        return;
    }
#ifdef S_ISLNK
	if (S_ISLNK(statbuf.st_mode)) {
		strcpy(perm, "lrwxrwxrwx");
                memset(temp, '\0', sizeof(temp));
                status = readlink(name, temp, sizeof(temp) - 2);
                if (status < 0)
                {
                     fprintf(client, "213-Error: %s.\n", strerror(errno) );
                     return;
                }
                temp[MAXCMD] = '\0';
		snprintf(linktarget, MAXCMD+120, " -> %s", temp);
	} else {
#endif
		strcpy(perm, "----------");
		if (S_ISDIR(statbuf.st_mode))
			perm[0] = 'd';
		if (statbuf.st_mode & S_IRUSR)
			perm[1] = 'r';
		if (statbuf.st_mode & S_IWUSR)
			perm[2] = 'w';
		if (statbuf.st_mode & S_IXUSR)
			perm[3] = 'x';
		if (statbuf.st_mode & S_IRGRP)
			perm[4] = 'r';
		if (statbuf.st_mode & S_IWGRP)
			perm[5] = 'w';
		if (statbuf.st_mode & S_IXGRP)
			perm[6] = 'x';
		if (statbuf.st_mode & S_IROTH)
			perm[7] = 'r';
		if (statbuf.st_mode & S_IWOTH)
			perm[8] = 'w';
		if (statbuf.st_mode & S_IXOTH)
			perm[9] = 'x';
		linktarget[0] = '\0';
#ifdef S_ISLNK
	}
#endif
    /* memcpy(&filetime, localtime(&(statbuf.st_mtime)), sizeof(struct tm)); */
    local_time = localtime(&(statbuf.st_mtime));
    if (! local_time) return;
    memcpy(&filetime, local_time, sizeof(struct tm));
    time(&t);
    tea_time = localtime(&t);
    if (! tea_time) return;
    /* if (filetime.tm_year == localtime(&t)->tm_year) */
    if (filetime.tm_year == tea_time->tm_year)
    	mystrncpy(timestr, ctime(&(statbuf.st_mtime)) + 4, 12);
    else
        strftime(timestr, sizeof(timestr), "%b %d  %G", &filetime);
    mygetpwuid(statbuf.st_uid, passwdfile, uid)[8] = 0;
    mygetpwuid(statbuf.st_gid, groupfile, gid)[8] = 0;
	fprintf(client, "%s %3i %-8s %-8s %12llu %s %s%s\r\n", perm,
			(int) statbuf.st_nlink, uid, gid,
			(unsigned long long) statbuf.st_size,
			timestr, name, linktarget);
}

void dirlist_one_file(char *name, FILE *client, char verbose)
{
    struct stat statbuf;
    struct hidegroup *tmp = hidegroups;
    char *filename_index;      /* place where filename starts in path */

    if (!stat(name, (struct stat *) &statbuf)) {
        if (tmp)
            do {
                if (statbuf.st_gid == tmp->data)
                    return;
            } while ((tmp = tmp->next));
    }

    /* find start of filename after path */
    filename_index = strrchr(name, '/');
    if (filename_index)
       filename_index++;   /* goto first character after '/' */
    else
       filename_index = name;    

    if (verbose)
        bftpd_stat(name, client);
    else
        fprintf(client, "%s\r\n", filename_index);
}

void dirlist(char *name, FILE * client, char verbose, int show_hidden)
{
    DIR *directory = NULL;
    FILE *can_see_file;
    int show_nonreadable_files = FALSE;
    int show_hidden_files = FALSE;
    int skip_file, file_is_hidden;
    char *local_cwd = NULL;
    char *pattern = NULL, *short_pattern;
    /* int i; */
    struct dirent *dir_entry;
    /* glob_t globbuf; */

    if ( (show_hidden) && 
         (! strcasecmp( config_getoption("SHOW_HIDDEN_FILES"), "yes") ) )
       show_hidden_files = TRUE;

    /* check for always show hidden */
    if (! strcasecmp( config_getoption("SHOW_HIDDEN_FILES"), "always") )
       show_hidden_files = TRUE;

    if (! strcasecmp( config_getoption("SHOW_NONREADABLE_FILES"), "yes") )
       show_nonreadable_files = TRUE;

    if ((strstr(name, "/.")) && strchr(name, '*'))
        return; /* DoS protection */

     if ((directory = opendir(name))) 
     {
        closedir(directory);
        local_cwd = bftpd_cwd_getcwd();
        if ( chdir(name) == -1)
            fprintf(client, "Chdir failed: %s\n", strerror(errno));
     }
     else
       pattern = name;

             /*
             glob("*", 0, NULL, &globbuf);
             if (show_hidden_files)
                 glob(".*", GLOB_APPEND, NULL, &globbuf);
	} 
        else
        {
    	     if ( (name[0] == '*') && (show_hidden_files) )
             {
                glob(name, 0, NULL, &globbuf);
                glob(".*", GLOB_APPEND, NULL, &globbuf);
             }
             else
                glob(name, 0, NULL, &globbuf);
        }
             */

        /*
	for (i = 0; i < globbuf.gl_pathc; i++)
        {
            if (! show_nonreadable_files) 
            {
               if ( (can_see_file = fopen(globbuf.gl_pathv[i], "r") ) == NULL)
                   continue;
               else
                   fclose(can_see_file);
            }

            dirlist_one_file(globbuf.gl_pathv[i], client, verbose);
        }
     
	globfree(&globbuf);
        */
        directory = opendir(".");
        if (directory)
        {
          dir_entry = readdir(directory);
          while (dir_entry)
          {
              /* This check makes sure we skipped named pipes,
                 which cannot be opened like normal files
                 without hanging the server. -- Jesse
              */
#ifndef __minix
              if (dir_entry->d_type != DT_FIFO)
              {
#endif
              can_see_file = fopen(dir_entry->d_name, "r");
              if (can_see_file) 
                  fclose(can_see_file);
              file_is_hidden = (dir_entry->d_name[0] == '.') ? TRUE : FALSE;
              skip_file = TRUE;

              if ( (! file_is_hidden) || (show_hidden_files) )
              {
                  if ( (can_see_file) || (show_nonreadable_files) )
                  {
                      if (pattern)
                      {
                        /* strip leading path */
                        short_pattern = strrchr(pattern, '/');
                        if (short_pattern)
                           short_pattern++;
                        else
                           short_pattern = pattern;
                        skip_file = fnmatch(short_pattern, dir_entry->d_name,
                                            FNM_NOESCAPE);
                      }
                      else
                        skip_file = FALSE;
                  } 
              }
              if (! skip_file)
                  dirlist_one_file(dir_entry->d_name, client, verbose); 
#ifndef __minix
              }
#endif
              dir_entry = readdir(directory);
          }
          closedir(directory);
        }       /* unable to open directory */

	if (local_cwd) {
		if (chdir(local_cwd) == -1)
                    fprintf(client, "Chdir failed: %s\n", strerror(errno));
		free(local_cwd);
	}
}

