static char sccsid[] = "@(#)82	1.9  src/bos/usr/bin/uucp/uucpadm/uucpadm.c, cmduucp, bos411, 9428A410j 8/3/93 16:15:52";
/* 
 * COMPONENT_NAME: CMDUUCP uucpadm.c
 * 
 * FUNCTIONS: Muucpadm, finish 
 *
 * ORIGINS: 10  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *  uucpadm
 *
 *  Copyright (c) 1988 IBM Corporation.  All rights reserved.
 *
 *
 */

/*
 *  uucpadm - edit uucp configuration files.
 */

#include <curses.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include "defs.h"

#define USAGE	"usage: uucpadm <filename>\n"

struct termios sg;
extern struct menu master;

void exit ();
/*void perror ();*/
int derror ();
char *termdef ();
void finish(int);
extern int errno, sysfile_present;

extern struct files Rules[FILES];
extern WINDOW *active;
extern WINDOW *helpwin;

main (argc, argv)
int  argc;
char *argv[];
{
	int    err;
	
	if ((signal(SIGINT,(void(*)(int)) finish)) == (void(*)(int))EOF)
	{
	fprintf (stderr, "uucpadm: signal failure");
	perror (" ");
	exit (EX_SOFTWARE);
    	}
	if (tcgetattr (fileno(stdin), &sg) == -1)
		exit (EX_SOFTWARE);
 	/* set up standard screen and clear terminal */
	initscr();
	cbreak();
	nonl();
	noecho();
	refresh();
/* check user authority for this program. */
	if (geteuid() || getegid())
		derror(EX_USAGE,"Must be run by root. Execute permission denied.");
/* Check terminal size. Uucpadm designed to be full screen utility. */
	if (COLS < REQ || LINES < LREQ)
		derror(EX_USAGE,"Sorry, full screen required for menus.");
/* set up windows    */
	if ((helpwin = newwin(0,0,0,0)) == NULL)
		derror(EX_SC,"Unable to allocate help screen!");
	if ((active = newwin(0,0,0,0)) == NULL)
		derror(EX_SC,"Unable to allocate active screen!");

/*
*  Set the configuration files up
*/
	Rules[Devices].spot = Devices;
	Rules[Systems].spot = Systems;
	Rules[Permissions].spot = Permissions;
	Rules[Poll].spot = Poll;
	Rules[Dialers].spot = Dialers;
	Rules[Dialcode].spot = Dialcode;
	Rules[Sysfiles].spot = Sysfiles;
	Rules[Devices].name = DEVFILE;
	Rules[Systems].name = SYSFILE;
	Rules[Permissions].name = PERMISSIONS;
	Rules[Poll].name = POLL;
	Rules[Dialers].name = DIALERFILE;
	Rules[Dialcode].name = DIALCODES;
	Rules[Sysfiles].name = SYSFILES;
	Rules[Devices].delimit = " ";
	Rules[Systems].delimit = " ";
	Rules[Permissions].delimit = " \t";
	Rules[Poll].delimit = " \t";
	Rules[Dialers].delimit = "\t ";
	Rules[Dialcode].delimit = " \t";
	Rules[Sysfiles].delimit = " \t";
	Rules[Devices].comment = "#";
	Rules[Systems].comment = "#";
	Rules[Permissions].comment = "#";
	Rules[Poll].comment = "#";
	Rules[Dialers].comment = "#";
	Rules[Dialcode].comment = "#";
	Rules[Sysfiles].comment = "#";
	Rules[Permissions].cont = "\\";
	Rules[Sysfiles].cont = "\\";
/*
*  Open the configuration files & load.
*  The philosophy is to open the
*  files immediately since 99% of the time we will touch them.
*  With this approach, we exit quickly if there are file
*  problems.
*/

/* The above philosophy no longer applies to all config files.	*/
/* The Devices, Systems and Dialers entries are chosen in the	*/
/* spec_* routines when the user decides which files he/she	*/
/* wants to modify for Devices, Systems and Dialers files.      */
/* In addition, the Sysfiles file can be created 'on the run.'	*/
/*	Rules[Devices].fd = open (Rules[Devices].name, O_RDWR); */
/*	if (Rules[Devices].fd < 0)				*/
/*		derror (EX_ACCESS, "Can't open Devices file");	*/
/*	Rules[Systems].fd = open (Rules[Systems].name, O_RDWR); */
/*	if (Rules[Systems].fd < 0)				*/
/*		derror (EX_ACCESS, "Can't open Systems file");	*/
/*	Rules[Dialers].fd = open (Rules[Dialers].name, O_RDONLY);*/
/*	if (Rules[Dialers].fd < 0)				*/
/*		derror (EX_ACCESS, "Can't open Dialers file"); 	*/

	Rules[Permissions].fd = open (Rules[Permissions].name, O_RDWR);
	if (Rules[Permissions].fd < 0)
		derror (EX_ACCESS, "Can't open Permissions file");
	if ((err = bload(&Rules[Permissions])) != EX_OK)
 		derror (err,"Can't load Permissions file");

	Rules[Poll].fd = open (Rules[Poll].name, O_RDWR);
	if (Rules[Poll].fd < 0)
		derror (EX_ACCESS, "Can't open Poll file");
	if ((err = bload(&Rules[Poll])) != EX_OK)
 		derror (err,"Can't load Poll file");

	Rules[Dialcode].fd = open (Rules[Dialcode].name, O_RDWR);
	if (Rules[Dialcode].fd < 0)
		derror (EX_ACCESS, "Can't open Dialcode file");
	if ((err = bload(&Rules[Dialcode])) != EX_OK)
 		derror (err,"Can't load Dialcode file");

	Rules[Sysfiles].fd = open (Rules[Sysfiles].name, O_RDWR);
	if (Rules[Sysfiles].fd < 0) {
		if (errno == ENOENT)	/* If it doesn't exist,    */
					/* Flag for later creation */
			sysfile_present = 0;
		else
			derror (EX_ACCESS, "Can't open Sysfiles file");
	}
	else {
		sysfile_present = 1;
		if ((err = bload(&Rules[Sysfiles])) != EX_OK)
 			derror (err,"Can't load Sysfiles file");
	}

    /*
     *  Begin edit session with top menu.
     *  A menu selection can write the file back or exit the program.
     */
    while (1)
    {
        CALL (do_menu (&master));
    }

/*NOTREACHED*/
}
void finish (int s)
	{
	int i;
	for (i=0;i < FILES;i++)
		(void) close(Rules[i].fd);
	clear();
	refresh();
	endwin();
	exit(EX_OK);
	}
