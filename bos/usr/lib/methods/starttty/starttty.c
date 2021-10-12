static char sccsid[] = "@(#)27	1.31  src/bos/usr/lib/methods/starttty/starttty.c, cfgtty, bos41J, 9513A_all 3/28/95 16:16:16";
/*
 * COMPONENT_NAME: (CFGMETHODS) starttty.c - TTY Start Method
 *
 * FUNCTIONS: main(), err_exit()
 *
 * ORIGINS: 27, 28
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* For performance purpose, use of ILS macros for isspace */
#define _ILS_MACROS
#include <stdio.h>
#include <string.h>
#include <cf.h>

#include <locale.h>
#include <sys/types.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <ctype.h>           /* for ILS macros */

#include "pparms.h"
#include "cfgdebug.h"
#define DIAG 3
#define PDCN_SSTRING "connwhere = s1 and connkey = rs232"

void err_exit(int exitcode);
int save_name(int defcons, char *devname);
int defineit(char *class, char *subclass, char *type, char *parent, char *connect);
int get_tty_entry(int *bus_eu, int *port, int *slot, 
		char *unit, char *subclass, char *chgparms);
int parse_entry(char *entry, char **keyword, char **value);
int get_parent_attribute(int bus_eu, int port, int slot, char *unit, 
						char *parent, char *connect);

/*
 * NAME: main
 * 
 * FUNCTION: This program creates temporary ttys, if needed, 
 *	     for use as console.
 * 
 * EXECUTION ENVIRONMENT:
 *
 *	This program is invoked by the IPL process.
 *           
 * NOTES: The temporary consoles are created according to the consdef file.
 *
 * RETURNS: Exits with 0 on success, >0 Error code.
 */
char	*class, *type;		/* parameters for define */
char	subclass[16];		/* subclass of console device */
char	connect[16];		/* connection of console device */
char	parent[16];		/* parent of console device */
char	unit[3];		/* location code for unit */
int	slot;			/* slot of adapter console is on */
int	bus_eu;			/* bus or eu adapter is in */
int	port;			/* port of console */
struct	CuDv	CuDv;		/* customized devices object */
char	sstring[256];		/* search string */
char	params[256];		/* parameters for define method */
char	chgparams[256];		/* parameters for change method */
int	ipl_phase;
struct	Class	*cusatt;	/* customized devices class handle */
struct	CuAt	CuAt;		/* customized attributes object */
struct	Class	*predev;	/* predefined devices class handle */
struct	PdDv	PdDv;		/* predefined devices object */
int	tty_cnt;		/* No. of ttys defined so far */
int	tty_limit;		/* Max number that can be defined */
				/* For 8 Mb machines limit is 3 */ 
				/* otherwise 128                */

main(argc,argv,envp)
int	argc;
char	*argv[];
char	*envp[];
{
	char	*consname;		/* name of console */
	struct PdCn connobj;
	int	c,rc;			/* parms save, return codes */
	int	which_obj;

	setlocale(LC_ALL, "");
	ipl_phase = PHASE2;

	/***********************************************
	  Retrieve logical name from argument list and
	  validate the logical name
	 ***********************************************/

	while((c=getopt(argc,argv,":123?"))!=EOF)
	{
		switch(c)
		{
			case '1':
				ipl_phase = PHASE1;
				break;
			case '2':
				ipl_phase = PHASE2;
				break;
			case '3':
				ipl_phase = DIAG;
				break;
			default:
				break;
		}
	}


	/***************
	  Start up odm
	 ***************/
	if (odm_initialize() < 0)
		exit (E_ODMINIT);

	/***********************************
	  Open Customized Attributes Class
	 ***********************************/
	if ((int)(cusatt=odm_open_class(CuAt_CLASS)) == -1)
		err_exit (E_ODMOPEN);

	/********************************
	  Open Predefined Devices Class
	 ********************************/
	if ((int)(predev=odm_open_class(PdDv_CLASS)) == -1)
		err_exit (E_ODMOPEN);

	/******************************************
	  See if there is a system console already
	 ******************************************/
	strcpy (sstring,"name = 'sys0' AND attribute = 'syscons'");
	if ((rc = (int)odm_get_first(cusatt,sstring,&CuAt)) == -1) {
		DEBUG_1 ("starttty: get_obj failed, crit=%s\n",sstring)
		err_exit (E_ODMGET);
	} else if (rc == 0 || CuAt.value[0] == '\0') 
			;
	else {
		/*****************************************
		  Check status of current system console
		 *****************************************/
		if ((consname = strrchr (CuAt.value,'/')) != NULL) {
			consname++;
			if (!strcmp(consname,"hft"))
				sprintf(sstring,"name = '%s0'",
							consname);
			else
				sprintf(sstring,"name = '%s'",
							consname);
		} else
			sprintf(sstring,"name = '%s'",CuAt.value);
		if ((rc=(int)odm_get_first(CuDv_CLASS,sstring,&CuDv))
								==-1) {
			DEBUG_1 ("starttty: get_obj failed, crit=%s\n",
							sstring)
			err_exit (E_ODMGET);
		} else if (CuDv.status == (short)AVAILABLE) { 
			/* console is already defined and available */
			odm_close_class(CuDv_CLASS);
			odm_close_class(PdDv_CLASS);
			odm_close_class(CuAt_CLASS);
			odm_terminate();
			exit(0);
		}
	}

	/*****************
	  Get the memory and set the tty_limit.
	 ******************/
	tty_limit = 128;
	strcpy (sstring,"name = 'sys0' AND attribute = 'realmem'");
	if ((rc=(int)odm_get_first(cusatt,sstring,&CuAt))==-1)
		exit(E_ODMGET);
	else if (rc == 0) { /* something weird, we must find it */
		DEBUG_0("starttty: Getting real memory failed!\n");
	} else { /* How much memory we got, set the tty_limit now */
		if (atoi(CuAt.value) <= 8196 ) /* 8 Mb or less */
			tty_limit = 3;
	}
	tty_cnt = 0;

	/********************************
	  Define device on S1 to be a console candidate
	 ********************************/
	class = "tty";
	type = "tty";
	strcpy(subclass,"rs232");
	strcpy(connect,"s1");

	/*
	 * get the name of the adapter controlling S1
	 * find the name by looking it up through it's entries in PdCn
	 */
	which_obj = ODM_FIRST;
	while ((rc = (int)odm_get_obj(PdCn_CLASS,PDCN_SSTRING,&connobj,which_obj)) && (rc != -1)) {

	    /*
	     * Make sure we look look for next object that matches
	     * this criteria  in case we need to loop
	     */
	    which_obj = ODM_NEXT;

	    /*
	     * found the uniquetype for the adapter controlling S1
	     * now get the adapter name
	     */
	    sprintf(sstring,"PdDvLn = %s",connobj.uniquetype);

	    if ((rc = (int)odm_get_first(CuDv_CLASS,sstring,&CuDv)) &&
		(rc != -1)) {
		/*
		 * we have the adapter name
		 * if this is the diagnostics phase,
		 *    then return the name of the adapter
		 * else
		 *    define a tty attached to this adapter
		 */
		if (ipl_phase == DIAG) {
		    /* print the parent (adapter) name to stdout */
		    strcpy(parent,CuDv.name);
		    printf("%s ",parent);
		} else {
		/* check if adapter is available, if not continue loop */
		    if (CuDv.status != (short)AVAILABLE)
			continue;
		    /* define a tty attached to this adapter */
		    strcpy(parent,CuDv.name);
		    if (!defineit(class,subclass,type,parent,connect))
			tty_cnt++;
		}
		/* exit while loop */
		break;
	    }
	}


	/*****************************************************
	  get devices from /etc/consdef and define them.
	 *****************************************************/
	while ( (tty_cnt < tty_limit) && 
	    !get_tty_entry(&bus_eu, &port, &slot, unit, subclass, chgparams)){
		DEBUG_6("eu %d port %d slot %d unit %s subclass %s chgparms %s\n",
			bus_eu , port , slot , unit , subclass , chgparams );
		if (get_parent_attribute(bus_eu, port, slot, unit, parent, connect) ) {
			DEBUG_0 ("starttty: Failed to get sa object\n");
			continue; /* Try next tty */
		}
		if (ipl_phase == DIAG) {
			printf("%s " ,parent);
			continue;
		}
		if (defineit(class, subclass, type, parent, connect) == 0) 
			tty_cnt++;
	}
	
	odm_close_class(PdDv_CLASS);
	odm_close_class(PdDv_CLASS);
	odm_close_class(CuAt_CLASS);
	odm_terminate();
	exit(0);
}


/*
 * NAME: err_exit
 *
 * FUNCTION: Closes any open object classes and terminates ODM.  Used to
 *           back out on an error.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is to be used only within this file.  The device
 *      specific routines for the various device specific config methods
 *      must not call this function.
 *
 * NOTES:
 *
 * void
 *   err_exit( exitcode )
 *      exitcode = The error exit code.
 *
 * RETURNS:
 *               None
 */
void err_exit(int exitcode)
{
	/* Close any open object class */
	odm_close_class(CuDv_CLASS);
	odm_close_class(PdDv_CLASS);
	odm_close_class(CuAt_CLASS);

	/* Terminate the ODM */
	odm_terminate();
	exit(exitcode);
}
/*
 * NAME: save_name
 *
 * FUNCTION: Save the requested tty console target in both tmpcons and
 *           altcons sys0 attributes in the database depending on if 
 *	     defcons is true otherwise save in altcons attribute only.
 *	     If in tmpcons , cfgcon will delete the device if not
 *	     chosen as console.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is to be used only within this file.  The device
 *      specific routines for the various device specific config methods
 *      must not call this function.
 *
 * NOTES:
 *
 * RETURNS:
 * 0 if altcons or tmpcons attribute correctly modified
 * error code otherwise
 */

int save_name(int defcons, char *devname)
{
	int 	rc;
	/***************************************
	  Put name of device into system object 
	  for console finder.
	 ****************************************/

	/**********************************************
	  Create a altcons entry for the temp. device , if not already there
	 **********************************************/
	sprintf (sstring,
		"name = 'sys0' AND attribute = 'altcons' AND value = '%s'",
			devname);
	if ((rc=(int)odm_get_first(cusatt,sstring,&CuAt))==-1)
		return(E_ODMGET);
	else if (rc == 0 ) {
		strcpy(CuAt.value,devname);
		strcpy(CuAt.name,"sys0");
		strcpy(CuAt.type,"R");
		strcpy(CuAt.generic,"");
		strcpy(CuAt.rep,"s");
		CuAt.nls_index = 0;
		strcpy(CuAt.attribute,"altcons");
		if ((rc = odm_add_obj(CuAt_CLASS, &CuAt)) < 0)
			return(E_ODMUPDATE);
	}
	/**********************************************
	  Create a tmpcons entry for the temp. device , if not already there
	 **********************************************/
	if (defcons==0) 
		return(0);
	sprintf (sstring,
		"name = 'sys0' AND attribute = 'tmpcons' AND value = '%s'",
			devname);
	if ((rc=(int)odm_get_first(cusatt,sstring,&CuAt))==-1)
		return(E_ODMGET);
	else if (rc == 0 ) {
		strcpy(CuAt.value,devname);
		strcpy(CuAt.name,"sys0");
		strcpy(CuAt.type,"R");
		strcpy(CuAt.generic,"");
		strcpy(CuAt.rep,"s");
		CuAt.nls_index = 0;
		strcpy(CuAt.attribute,"tmpcons");
		if ((rc = odm_add_obj(CuAt_CLASS, &CuAt)) < 0)
			return(E_ODMUPDATE);
	}
	return (0);
}				
/*
 * NAME: defineit
 *
 * FUNCTION: Given class, subclass, type, parent and connection, it checks
 *	     to see no device exists at the given location. If not then it
 *	     defines the device, sets up the proper change parameters and
 *	     configures the device if required.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is to be used only within this file.  The device
 *      specific routines for the various device specific config methods
 *      must not call this function.
 *
 * NOTES:
 *	Uses global "chgparams" to change device characterestics
 *
 * RETURNS:
 * 0 if device defined successfully
 * error code otherwise
 */

int defineit(char *class, char *subclass, char *type, char *parent, char *connect)
{
	int	tmpptr;			/* temporary index */
	char *errptr, *outptr;
	int rc;
	char	lname[30];		/* logical name of current object */
	char	devname[30];		/* Device name */
	char 	existing_dev;			/* Boolean */

	lname[0] = '\0';
	existing_dev = 0;  /* Device is not already defined */
	sprintf(sstring,"parent = '%s' AND connwhere = '%s'",parent,connect);
	if ((rc = (int)odm_get_obj(CuDv_CLASS,sstring,&CuDv,ODM_FIRST)) == -1) {
		DEBUG_1 ("starttty: get_obj failed, crit=%s\n",sstring)
		err_exit (E_ODMGET);
	} else if (rc != 0) {  /* device exists */
		DEBUG_0 ("starttty: Device already defined on temporary Console location.\n")

		/* some device exists at the port. Is it our tty? */
		sprintf (sstring, "%s/%s/%s", class,subclass,type);
		if (strcmp(sstring, CuDv.PdDvLn_Lvalue) != 0)
			return (1); /* No, not our tty */

		/* Device exists, Select it as an altcons so that
		   display message would come on that device */
		save_name(0, CuDv.name);

		/* Do we want to change some parameters,
		   only possible if we defined this device i.e "tmpcons"
		   attribute exists */

		if (*chgparams) {
			sprintf (sstring,
			   "name = 'sys0' AND attribute = 'tmpcons' AND value = '%s'", 
						CuDv.name);
			if ((rc=(int)odm_get_first(cusatt,sstring,&CuAt))==-1)
				err_exit(E_ODMGET);
			else if (rc != 0) { /* "tmpcons" is defined, we can change it */
				existing_dev =1; /* device is already defined */
				strcpy(devname, CuDv.name);
			} else return(1); /* should not come here */
		} else return (1);
	}

	/*****************************************
	  Get predefined object for a tty device
	 *****************************************/
	sprintf (sstring, "uniquetype = '%s/%s/%s'",
					class,subclass,type);
	if ((rc = (int)odm_get_first(predev,sstring,&PdDv)) == 0) {
		/* error getting object */
		DEBUG_1 ("starttty: PdDv crit=%s found no objects\n",sstring)
		return (E_NOPdDv);
	} else if (rc == -1) {
		/* odm error occurred */
		DEBUG_1 ("starttty: get_obj failed, crit=%s\n",
						sstring)
		err_exit (E_ODMGET);
	}

	if (!existing_dev) {
		/********************************************
		  Invoke the Define method for a tty device
		 ********************************************/
		sprintf (params,"-c %s -s %s -t %s -p %s -w %s",
				class,subclass,type,parent,connect);
		DEBUG_1 ("starttty: define parms are:, %s \n",params)
		if ((int)odm_run_method (PdDv.Define,params,&outptr,
					&errptr) != 0) {
			DEBUG_0("starttty: odm_run_method for define failed \n")
			return (E_ODMRUNMETHOD);
		}
		for (tmpptr=(strlen(outptr))-1;
			isspace(outptr[tmpptr]) && tmpptr >= 0;
			tmpptr--)
			outptr[tmpptr] = '\0';
		strcpy(devname, outptr);
		if (rc = save_name(TRUE, devname) !=0)
		{  
		  DEBUG_0("starttty: adding tmpcons attribute failed\n")
		  err_exit(rc);
		}
		if (ipl_phase != DIAG) {
			printf("%s",devname);
		}
	}

	if (*chgparams) {
	/************************************
	  Run change method for device  with
	  attributes from consdef file
	 ************************************/
		sprintf (params, "-l %s %s", devname,chgparams);
		DEBUG_1 ("starttty: change params=%s\n",params)
		if ((int)odm_run_method (PdDv.Change,params,
				&outptr, &errptr) != 0) {
			DEBUG_0 ("starttty: odm_run_method for change failed\n")
			err_exit (E_ODMRUNMETHOD);
		}
	}
	/************************************
	  Run config method for device 
	  Running config method again won't hurt!
	 ************************************/
	if (ipl_phase == PHASE1 && !existing_dev)
	{
		sprintf (lname, "-l %s  -1", devname);
		DEBUG_1 ("starttty: logical name=%s\n",lname)
		if (rc=(int) odm_run_method(PdDv.Configure,
			      lname,&outptr, &errptr) != 0)
		{
			DEBUG_1 ("starttty: odm_run_method for Configure failed,exit code= %d\n",rc)
			/* return (E_ODMRUNMETHOD); may fail because we are running again */
		}
		DEBUG_0 ("starttty: odm_run_method for configure passed\n")
	}  
	return(0);
}
/*
 * NAME: get_tty_entry
 *
 * FUNCTION: get_tty_entry returns the next valid entry found in /etc/consdef
 *	     file. It opens the file, parses it and returns the next entry.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is to be used only within this file.  The device
 *      specific routines for the various device specific config methods
 *      must not call this function.
 *
 * NOTES:
 *
 * RETURNS:
 * 	bus_eu, port, slot, unit, subclass are output parameters.
 *
 *	chgparms contain a list of "-a attribute=vlaue"	format list of
 *		change attributes.
 * 0 if device defined successfully
 * error code otherwise
 */

int get_tty_entry(int *bus_eu, int *port, int *slot, 
		char *unit, char *subclass, char *chgparms)
{
	static FILE  *fp = (FILE *) 0;
	char entry[256], *keyword, *p, *value;
	char loc, conn, finish_stanza; /* flags */
	
	
	if (!fp) {	/* "/etc/consdef" not opened yet */
		if ( (fp = fopen("/etc/consdef", "r")) == (FILE *) NULL) {
			DEBUG_0 ("Open of /etc/consdef failed\n")
			return (1);
		}
	}

	finish_stanza =loc=conn=0;
	*bus_eu=*port=*slot=*unit=*subclass=*chgparms=0;
	p = chgparms;

	while ( (fgets(entry, 255, fp)) != NULL) {
		DEBUG_1("starttty: Read: \n%s\n", entry)
		if (parse_entry(entry, &keyword, &value))
			continue;
		/* Got a valid entry */
		if (strcmp(keyword, "ALTTTY") == 0) {
			if(finish_stanza) {
				if(loc && conn)
					return(0);
				else { /*OOps! location or connection not there.
				        well ignore this one, try next stanza */
					DEBUG_0("starttty: Missing location or connection")
					finish_stanza =loc=conn=0;
					*bus_eu=*port=*slot=*unit=*subclass=*chgparms=0;
					p = chgparms;
					continue;
				}
			} else {
				finish_stanza = 1;
				continue;
			}
		} else if (strcmp(keyword,"connection") == 0) {
			DEBUG_1 ("starttty: connection=%s\n",value)
			strcpy(subclass,value);
			conn=1;
		} else if (strcmp(keyword,"location") == 0) {
			if ((sscanf(value,"%2d-%2d-%2s-%2d",
				    bus_eu,slot,unit,port)) != 4) 
			{
				DEBUG_0 ("sscanf failed\n")
				*bus_eu=*slot=*unit=*port=loc=0;
				continue;
			}
			DEBUG_3 ("starttty: bus_eu=%d,port=%d,slot=%d\n",*bus_eu,*port,*slot)
			loc=1;
		} else {  /* no special processing required */
			sprintf(p, "-a %s=%s ", keyword, value);
			p += strlen(p);
		}
		finish_stanza=1;
	} /* end while */

	/* Comes here only if EOF (fgets returns null) is encountered */
	if(finish_stanza && loc && conn)
		return(0); 
	else /* we read just an end of file, time to close now */
	{
		fclose(fp);
		fp = (FILE *) NULL;
		return(1);
	}

}
/*
 * NAME: parse_entry
 *
 * FUNCTION: parse_entry takes the line (entry) read by get_tty_entry and 
 *	     into keyword and value. 
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is to be used only within this file.  The device
 *      specific routines for the various device specific config methods
 *      must not call this function.
 *
 * NOTES:
 *	Original string entry is modified! keyword and value point to
 *	a place in the original string itself.
 *
 * RETURNS:
 * 	keyword and value pointers are set.
 *
 * 0 if device defined successfully
 * error code otherwise
 */

int parse_entry(char *entry, char **keyword, char **value) 
{
	char *p;

	if ((*entry == '#')  ||
	    (*entry == '\n') ) {
		DEBUG_0 ("starttty: Skipping Comment line\n")
		return(1);
	}
	/* replace last newline with null */
	p = entry + strlen(entry) -1;
	if (*p == '\n') *p = '\0';

	while(*entry == ' ' || *entry == '\t') entry++;  /* skip blanks and tabs */
	if (*entry == '\n') return (1);

	*keyword = entry++;

	while(*entry != ' ' && *entry != '\t' && *entry != '=' && *entry !=':' 
		&& *entry != '\0') entry++;

	if ( *entry == ':') {
		*entry='\0';
		return(0);
	} else if (*entry == '\0') { /* Error, ignore */
		DEBUG_1 ("starttty: Not a valid Entry - %s\n", entry);
		return(1);
	} else if (*entry == '=') {
		*entry++ = '\0';
	} else if (*entry != '\0') { /* *entry is white space */
		*entry++ = '\0'; 
		while(*entry == ' ' || *entry == '\t') entry++;  /* skip blanks and tabs */
		if(!*entry)	return(1);
		else if (*entry == ':')  return(0);
		else if (*entry != '=') {
			DEBUG_1 ("starttty: Not a valid Entry - %s\n", entry);
			return(1);
		}
		entry++;
	}

	while(*entry == ' ' || *entry == '\t') entry++;  /* skip blanks and tabs */
	if(!*entry)	return(1);
	*value = entry;
	return(0);
		
}
/*
 * NAME: get_parent_attribute
 *
 * FUNCTION: get_parent_attribute returns the parent and connection point 
 *	     for the given device parameters.
 *	     It checks the odm for the existance of the parent adapter.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is to be used only within this file.  The device
 *      specific routines for the various device specific config methods
 *      must not call this function.
 *
 * NOTES:
 *
 * INPUTS:
 *	bus_eu, port, slot, unit.
 * OUTPUTS:
 *	parent, connect
 * GLOBALS USED:
 *	sstring - temporary storage.
 *	ipl_phase - gives the current ipl_phase.
 *	CuDv	  - storage for odm calls.
 *
 * RETURNS:
 * 0 if device defined successfully
 * error code otherwise
 */
int get_parent_attribute(int bus_eu, int port, int slot, char *unit, 
						char *parent, char *connect)
{
	int rc;
	if (*unit == 'S') {
		if (ipl_phase == DIAG)
			sprintf(sstring, "location = '%02d-%02d-%s'",
					bus_eu, slot, unit);
		else
			sprintf(sstring, 
				"location = '%02d-%02d-%s' AND status = '%d'",
					bus_eu, slot, unit, AVAILABLE);
		sprintf(connect, "s%s", &unit[1]);
	} else {
		if (ipl_phase == DIAG)
			sprintf(sstring, "location = '%02d-%02d'",
					bus_eu, slot);
		else
			sprintf(sstring, 
				"location = '%02d-%02d' AND status = '%d'",
					bus_eu, slot, AVAILABLE);
		port += (atoi(unit)-1)*16;
		sprintf(connect, "%d", port);
	}

	DEBUG_1 ("starttty: sstring=%s\n",sstring)
	DEBUG_1 ("starttty: connect=%s\n",connect)

	if ((rc = (int) odm_get_obj(CuDv_CLASS, sstring,
				&CuDv,ODM_FIRST)) == 0) {
		DEBUG_0 ("starttty: Failed to get sa object.\n")
		return (1);
	} else if (rc == -1) {
		DEBUG_0 ("starttty: error getting sa object.\n")
		return (1);
	} else {
		strcpy(parent, CuDv.name);
		DEBUG_1 ("starttty: parent=%s\n", parent)
	}
	return(0);

}
