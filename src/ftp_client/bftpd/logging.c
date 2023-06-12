#include <config.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#include <main.h>
#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "logging.h"
#include "options.h"
#include "mystring.h"

FILE *logfile = NULL;
FILE *send_receive_file = NULL;
FILE *statuslog = NULL;
FILE *statuslogforreading = NULL;
int time_zone_difference = 0;
char log_syslog = 0;

void log_init()
{
    char *foo = config_getoption("LOGFILE");
#ifdef HAVE_SYSLOG_H
	if (!strcasecmp(foo, "syslog")) {
             log_syslog = 1;
	     openlog(global_argv[0], LOG_PID, LOG_DAEMON);
	} else
#endif
    if (foo[0])
        if (!(logfile = fopen(foo, "a"))) {
    		control_printf(SL_FAILURE, "421-Could not open log file.\r\n"
    		         "421 Server disabled for security reasons.");
    		exit(1);
    	}
    statuslog = fopen(PATH_STATUSLOG, "a");
    if (! statuslog) {
        control_printf(SL_FAILURE, "421-Could not open statuslog file.\r\n%s.",
                         strerror(errno));
        exit(1);
    }
        
    /* This one is for the admin code. */
    statuslogforreading = fopen(PATH_STATUSLOG, "r");
    if (! statuslogforreading) {
        control_printf(SL_FAILURE, "421-Could not open statuslog file for reading.\r\n%s.",
                         strerror(errno));
        exit(1);
    }
}

/* Status file format:
 * <ID> <Type> <Type2> <String>
 * ID: PID of server process
 * Type: SL_INFO for information, SL_COMMAND for command, SL_REPLY for reply.
 * Type2: If Type = SL_INFO or SL_COMMAND, Type2 is SL_UNDEF. Else,
 * it's SL_SUCCESS for success and SL_FAILURE for error.
 */
void bftpd_statuslog(char type, char type2, char *format, ...)
{
    if (statuslog) {
        va_list val;
        char buffer[1024];
        va_start(val, format);
        vsnprintf(buffer, sizeof(buffer), format, val);
        va_end(val);
        fseek(statuslog, 0, SEEK_END);
        fprintf(statuslog, "%i %i %i %s\n", (int) getpid(), type, type2,
                buffer);
        fflush(statuslog);
    }
}

void bftpd_log(char *format, ...)
{
	va_list val;
	char buffer[1024], timestr[40];
	time_t t;
	va_start(val, format);
	vsnprintf(buffer, sizeof(buffer), format, val);
	va_end(val);
	if (logfile) {
		fseek(logfile, 0, SEEK_END);
		time(&t);
                t = Adjust_Clock(t);
		strcpy(timestr, (char *) ctime(&t) ); 
                /* This isn't for NULL termination so much as getting
                   rid of the trailing newline. */
                timestr[strlen(timestr) - 1] = '\0';
		fprintf(logfile, "%s %s[%i]: %s", timestr, global_argv[0],
				(int) getpid(), buffer);
		fflush(logfile);
	}
#ifdef HAVE_SYSLOG_H
    else if (log_syslog)
        syslog(LOG_DAEMON | LOG_INFO, "%s", buffer);
#endif
}

void log_end()
{
	if (logfile) {
		fclose(logfile);
		logfile = NULL;
	}
#ifdef HAVE_SYSLOG_H
    else if (log_syslog)
		closelog();
#endif
    if (statuslog) {
        fclose(statuslog);
        statuslog = NULL;
    }
}



/* This function returns a static string which includes
 * the current date.
*/
char *Current_Date()
{
   static char output[32];
   time_t my_time;
   struct tm *my_local;

   memset(output, '\0', 32);
   my_time = time(NULL);
   my_local = localtime(& my_time);
   if (my_local)
       sprintf(output, "%d-%d-%d", my_local->tm_year + 1900, my_local->tm_mon + 1, my_local->tm_mday);
   return output;
}




/*
 * This function opens the log file which keeps track of the amount
 * of data sent or received.
*/
int Open_Send_Receive_Log()
{
    char *foldername, *filename, *my_date;

    foldername = config_getoption("BANDWIDTH");
    if (! foldername[0])
       return 1;

    my_date = Current_Date();
    filename = (char *) calloc( strlen(foldername) + strlen(my_date) + 16, sizeof(char) );
    if (! filename)
       return 0;

    sprintf(filename, "%s/%s.txt", foldername, my_date);
    send_receive_file = fopen(filename, "a");
    free(filename);
    if (! send_receive_file)
       return 0;
    else
       return 1;
}



/*
 * This function writes to a log file the total number of
 * bytes we have sent or received in this session. The log file
 * has the format:
 * username bytes_sent bytes_received
 * Each entry is appended to the end of the file.
 *
*/
int Update_Send_Recv(char *username, double sent, double received)
{
    if (! send_receive_file)
       return 1;

    fseek(send_receive_file, 0, SEEK_END);
    /* file is open, write data */
    fprintf(send_receive_file, "%s %.0f %.0f\n", username, sent, received);
    fflush(send_receive_file);
    return 1;
}



/* Find the difference between the local time and GMT.
The result is stored in time_zone_difference.
*/

int Get_Time_Zone_Difference()
{
   time_t my_time;
   struct tm *local_time, *gmt_time;
   struct tm local, gmt;

   /* grab current time */
   time( &my_time );
   local_time = localtime( &my_time );
   memcpy(&local, local_time, sizeof(struct tm) );
   gmt_time = gmtime( &my_time );
   memcpy(&gmt, gmt_time, sizeof(struct tm) );

   /* same day, find straight difference */
   if (local.tm_mday == gmt.tm_mday)
      time_zone_difference = local.tm_hour - gmt.tm_hour;
   /* local time is a day ahead */
   else if (local.tm_mday > gmt.tm_mday)
      time_zone_difference = (local.tm_hour + 24) - gmt.tm_hour;
   /* local zone is behind */
   else if (local.tm_mday < gmt.tm_mday)
      time_zone_difference = local.tm_hour - (gmt.tm_hour + 24);

   return time_zone_difference;
}


time_t Adjust_Clock(time_t old_time)
{
   time_t new_time;

   new_time = old_time + (time_zone_difference * 3600);
   return new_time;
}
