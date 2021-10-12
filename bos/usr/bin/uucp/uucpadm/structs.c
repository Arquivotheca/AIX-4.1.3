static char sccsid[] = "@(#)80	1.4  src/bos/usr/bin/uucp/uucpadm/structs.c, cmduucp, bos411, 9428A410j 1/17/94 14:06:10";
/* 
 * COMPONENT_NAME: CMDUUCP structs.c
 * 
 * FUNCTIONS: finis 
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

#include <curses.h>
#include <sys/stat.h>
#include "defs.h"

/*
 *  Globals
 */
char File[FILES][MAXFILE];		/* buffer for config file */
char entries[MAXENT][MAXLINE];		/* buffer for menu entries */
int  Badfile = 0;
int  sysfile_present ;
int  cury[MAXENT+1], curx[MAXENT+1];		/* cursor positions */
WINDOW *helpwin;				/* help window */
WINDOW *active;					/* active window */
void exit ();
int  do_menu ();
int  do_devices ();
int  do_dialc ();
int  do_dialers ();
int  do_perms ();
int  do_poll ();
int  do_sysfiles (); 
int  do_systems (); 
int  derror ();
int  finis ();
int  spec_files ();
unsigned int sleep();

/*
 *  Control structures
 */

struct files Rules[FILES];

struct menu_shell devices =
{
    "devices",
    "Devices Maintenance",
	5,
    "Type: ",
    "Line1: ",
    "Line2: ",
    "Class: ",
    "Dialer: ",
};

struct menu_shell dialc =
{
    "dialc",
    "Uucp Dialcode Maintenance",
	2,
    "Abr: ",
    "Dialcode: ",
};

struct menu_shell dialers =
{
    "dialers",
    "Dialers Maintenance",
	3,
    "Dialer: ",
    "Substitute sequence: ",
    "Expect-send sequence: ",
};

struct menu_shell perms =
{
    "perms",
    "Uucp Permissions Maintenance",
	8,
    "L/M: ",
    "Request: ",
    "Sendfiles: ",
    "Read: ",
    "Write: ",
    "Callback: ",
    "Commands: ",
    "Validate: ",
};

struct menu_shell poll_ =
{
    "poll",
    "Uucp Poll Maintenance",
	2,
    "System: ",
    "Hours: ",
};

struct menu_shell systems =
{
    "systems",
    "Systems Maintenance",
	7,
    "Name: ",
    "Time: ",
    "Type: ",
    "Class: ",
    "Phone: ",
    "Login: ",
    "Password: ",
};

struct menu_shell sysfiles =
{
    "sysfiles",
    "Basic Networking Utilities Sysfiles Maintenance",
	4,
    "Service: ",
    "Systems: ",
    "Devices: ",
    "Dialers: ",
};

struct spec_shell devices_spec =
{
    "devices_spec",
    "Add/Change Devices File(s)",
    SPECMAXENT,
    "Specify Devices File: ",
    "",
    "The following file(s) was (were) listed as uucico Devices file(s): ",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "The following file(s) was (were) listed as cu Devices file(s): ",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
};

struct spec_shell dialers_spec =
{
    "dialers_spec",
    "Add/Change Dialers File(s)",
    SPECMAXENT,
    "Specify Dialers File: ",
    "",
    "The following file(s) was (were) listed as uucico Dialers file(s): ",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "The following file(s) was (were) listed as cu Dialers file(s): ",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
};

struct spec_shell systems_spec =
{
    "systems_spec",
    "Add/Change Systems File(s)",
    SPECMAXENT,
    "Specify Systems File: ",
    "",
    "The following file(s) was (were) listed as uucico Systems file(s): ",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "The following file(s) was (were) listed as cu Systems file(s): ",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
};

struct menu_ent option_spec_dialers =
{
    "Add/Change Dialers File(s)",
    spec_files,
    (int) &dialers_spec,
};

struct menu_ent option_spec_devices =
{
    "Add/Change Devices File(s)",
    spec_files,
    (int) &devices_spec,
};

struct menu_ent option_spec_systems =
{
    "Add/Change Systems File(s)",
    spec_files,
    (int) &systems_spec,
};

struct menu_ent option_devices =
{
    "Add/Change Devices File(s)",
    do_devices,
    (int) &devices,
};

struct menu_ent option_poll =
{
    "Add/Change Uucp Poll File",
    do_poll,
    (int) &poll_,
};

struct menu_ent option_perms =
{
    "Add/Change Uucp Permissions File",
    do_perms,
    (int) &perms,
};

struct menu_ent option_dialcode =
{
    "Add/Change Phone Dialcodes",
    do_dialc,
    (int) &dialc,
};

struct menu_ent option_dialers =
{
    "Add/Change Dialers File(s)",
    do_dialers,
    (int) &dialers,
};


struct menu_ent option_sysfiles =
{
    "Add/Change Sysfiles File",
    do_sysfiles,
    (int) &sysfiles,
};

struct menu_ent option_systems =
{
    "Add/Change Systems File(s)",
    do_systems,
    (int) &systems,
};

struct menu_ent *handoff[] =
{
	&option_devices,
	&option_systems,
	NULL,
	NULL,
	&option_dialers,
	NULL,
	NULL
};


struct menu_ent master_finis =
{
    "Exit ",
    finis,
    0,
};

struct menu master =
{
    "master",
    "Uucpadm Options:",
    8,
    &option_sysfiles,
    &option_spec_devices,
    &option_perms,
    &option_spec_systems,
    &option_poll,
    &option_dialcode,
    &option_spec_dialers,
    &master_finis,
};


/*
 *  finis - stop the program
 */
int finis (i)
int  i;
{
	for (i=0;i < FILES;i++)
	(void) close (Rules[i].fd);
	clear();
	refresh();
	endwin();
        exit (EX_OK);

/*NOTREACHED*/
}

char *Filename[] =
{
	"Devices",
	"Systems",
	"",
	"",
	"Dialers",
	"",
	"",
};

char *sysfiletag[] =
{
	"devices=",
	"systems=",
	"",
	"",
	"dialers=",
	"",
	"",
};

char *help_token[] =
{
	"devices_spec",
	"systems_spec",
	"",
	"",
	"dialers_spec",
	"",
	"",
};

char *sysfiles_image[] =
{
"#\n",
"# COMPONENT_NAME: CMDUUCP Sysfiles\n",
"#\n",
"# FUNCTIONS:\n",
"#\n",
"# ORIGINS: 10  27  3\n",
"#\n",
"# (C) COPYRIGHT International Business Machines Corp. 1993\n",
"# All Rights Reserved\n",
"# Licensed Materials - Property of IBM\n",
"#\n",
"# US Government Users Restricted Rights - Use, duplication or\n",
"# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.\n",
"#\n",
"#\n",
"#\n",
"#ident  \"@(#)uucp:Sysfiles      1.2\"\n",
"#\n",
"#       Sysfiles provides a means of specifying different Systems,\n",
"#       Devices and Dialers files for the uucico and cu services.  This is\n",
"#       useful for certain networks where, for example, requests for\n",
"#       interactive login service might be accepted on a different basis\n",
"#       than requests for uucico service (hence different Systems files),\n",
"#       or where there are different chat scripts to request each service\n",
"#       (hence different Dialers files).\n",
"#\n",
"#       Another use of Sysfiles is to split large Systems files into\n",
"#       smaller, more manageable files (e.g., local and global Systems\n",
"#       files).\n",
"#\n",
"#       FORMAT:\n",
"#\n",
"#       service=<service name>  systems=<systems file list> \\\n",
"#                               devices=<devices file list> \\\n",
"#                               dialers=<dialers file list>\n",
"#\n",
"#       Where service name is \"uucico\" or \"cu\".  Each file list\n",
"#       is a list of colon-separated file names.  File names are relative to\n",
"#       /etc/uucp unless a full path name is given.  Files are searched\n",
"#       in the order that they appear in the file list.\n",
"#\n",
"#       The defaults are the usual uucp files: /etc/uucp/Systems\n",
"#       /etc/uucp/Devices and /etc/uucp/Dialers.\n",
"#\n",
"#       EXAMPLE 1:\n",
"#       This example uses different systems and dialers files to separate\n",
"#       the uucico- and cu-specific info, with information that they\n",
"#       use in common still in the \"usual\" Systems and Dialers files.\n",
"#\n",
"#       service=uucico  systems=Systems.cico:Systems \\\n",
"#                       dialers=Dialers.cico:Dialers\n",
"#       service=cu      systems=Systems.cu:Systems \\\n",
"#                       dialers=Dialers.cu:Dialers\n",
"#\n",
"#       EXAMPLE 2:\n",
"#       This example uses the default Devices and Dialers files for uucico \n",
"#       and cu, but has split the Systems file into local, company-wide, \n",
"#       and global files.\n",
"#\n",
"#       service=uucico  systems=Systems.local:Systems.company:Systems\n",
"#       service=cu      systems=Systems.local:Systems.company:Systems\n",
"#\n",
"# The only valid service types at this time are \"cu\" and \"uucico.\"  Uucico-\n",
"# related commands (uucp, uux, etc.) use the \"uucico\" service entries.  All\n",
"# other commands (cu, ct, slattach, etc.) utilize the \"cu\" service entries.\n",
"service=cu\tsystems=Systems \\\n\t\tdevices=Devices \\\n\t\tdialers=Dialers\n",
"service=uucico\tsystems=Systems \\\n\t\tdevices=Devices \\\n\t\tdialers=Dialers\n",
"",
};
