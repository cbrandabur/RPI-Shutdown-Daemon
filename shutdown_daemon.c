/*
  * shutdown_daemon.c:
  * Simple daemon to monitor a gpio port and soft shutdown the Raspberry Pi
  * if the button is pressed.
  *
  * Copyright (c) 2014 Calin Brandabur.
  * This file is part of Casa-Automata:
  * http://casa-automata.com
  *
  ***********************************************************************
  *    shutdown_daemon is free software: you can redistribute it and/or modify
  *    it under the terms of the GNU Lesser General Public License as published by
  *    the Free Software Foundation, either version 3 of the License, or
  *    (at your option) any later version.
  *
  *    shutdown_daemon is distributed in the hope that it will be useful,
  *    but WITHOUT ANY WARRANTY; without even the implied warranty of
  *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  *    GNU Lesser General Public License for more details.
  *
  *    You should have received a copy of the GNU Lesser General Public License
  *    If not, see <http://www.gnu.org/licenses/>.
  ***********************************************************************
*/ 
/*
  ***********************************************************************
  * Usage:
  * requires WiringPI: https://projects.drogon.net/raspberry-pi/wiringpi/
  *
  * To compile it on Raspberry Pi B+:
  * gcc shutdown_daemon.c -l wiringPi -o shutdown_daemon
  *
  * To start, stop or see the daemons status:
  * ./shutdown_daemon start|stop|status
  ***********************************************************************
*/

#include <wiringPi.h>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#define GPIO_IN 21

const char *filepid="/run/shutdown_daemon.pid";

void signal_callback_handler(int signum) {
  pid_t cpid;
  FILE *cfpid;
  if( access( filepid, F_OK ) != -1 ) {
    cfpid=fopen(filepid, "r");
    fscanf(cfpid, "%d\n", &cpid);
    fclose(cfpid);
    if( cpid==getpid() ) {
      remove(filepid);
      abort();
    }
  }
}


static void daemonize() {
  pid_t dpid;
  FILE *dfpid;

  /* Fork off the parent process */
  dpid = fork();

  /* An error occurred */
  if (dpid < 0)
    exit(EXIT_FAILURE);

  /* Success: Let the parent terminate */
  if (dpid > 0)
    exit(EXIT_SUCCESS);

  /* On success: The child process becomes session leader */
  if (setsid() < 0)
    exit(EXIT_FAILURE);

  /* Fork off for the second time*/
  dpid = fork();

  /* An error occurred */
  if (dpid < 0)
    exit(EXIT_FAILURE);

  /* Success: Let the parent terminate */
  if (dpid > 0)
    exit(EXIT_SUCCESS);

  /* Set new file permissions */
  umask(0);

  /* Change the working directory to the root directory */
  /* or another appropriated directory */
  chdir("/");

  /* Close all open file descriptors */
  int x;
  for (x = sysconf(_SC_OPEN_MAX); x>0; x--) {
    close (x);
  }

  // Saving pid in the .pid file
  dfpid=fopen(filepid,"w");
  fprintf(dfpid, "%d\n", getpid());
  fclose(dfpid);

  signal(SIGINT, signal_callback_handler);
  signal(SIGHUP, signal_callback_handler);
  signal(SIGTERM, signal_callback_handler);
  signal(SIGQUIT, signal_callback_handler);
}


int main (int argc, char **argv) {
  FILE *fpid;
  pid_t pid=0;

  if ( argc != 2 ) {
    printf ("Usage: %s [start|stop|status]\n", argv[0]);
    return 1;
  }
  else {
    if( !strcmp(argv[1],"start") ) {
      if( access( filepid, F_OK ) != -1 ) {
        // pid file exist: daemon is running
        printf("Daemon is already running.\n");
      }
      else {
        // pid file doesn't exist: daemon is stopped
        printf("Casa-Automata shutdown daemon\n");
        printf("Monitoring GPIO PORT %i \n", GPIO_IN);
        printf("Daemon starting... \n");
        daemonize();

        wiringPiSetupGpio();
        pinMode (GPIO_IN, INPUT);
        pullUpDnControl (GPIO_IN, PUD_UP);

        for(;;){
          //body of program
          if(digitalRead(GPIO_IN) == 0) {
            printf("Shutting down...\n");
            system("shutdown -h -y 0");
          }
          delay(1000);
        }
      }
    }
    if ( !strcmp(argv[1],"stop") ) {
      if( access( filepid, F_OK ) != -1 ) {
        // pid file exist: daemon is running
        printf("Stopping daemon...");
        fpid=fopen(filepid,"r");
        fscanf(fpid, "%d\n", &pid);
        fclose(fpid);
        kill(pid, SIGINT);
        printf(" stopped!\n");
      }
      else {
        // pid file doesn't exist: daemon is stopped
        printf("Daemon is not running.\n");
      }

    }
    if ( !strcmp(argv[1],"status") ) {
      if( access( filepid, F_OK ) != -1 ) {
        // pid file exist: daemon is running
        fpid=fopen(filepid,"r");
        fscanf(fpid, "%d\n", &pid);
        fclose(fpid);
        printf("Daemon is running with pid: %d.\n",pid); 
      }
      else {
        // pid file doesn't exist: daemon is stopped
        printf("Daemon is stopped.\n");
      }
    }
  }
  return(EXIT_SUCCESS);  
}
