static char sccsid[] = "@(#)90  1.7  src/bos/usr/ccs/lib/libcfg/POWER/pt.c, libcfg, bos41J, 9520A_all 5/8/95 09:52:32";
/*
 *   COMPONENT_NAME: (LIBCFG) PRINT TABLES Module
 *
 *   FUNCTIONS:
 *		log_resource_summary
 *		log_message
 *		log_error
 *		log_ok_value
 *		log_conflict
 *		log_postpone
 *		log_wrap
 *		log_increment
 *		log_share
 *		log_resid_summary
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994,1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <cf.h>
#include <memory.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include "adapter.h"
#include "bt.h"

#define PRINT_BOOL(x) ((x) ? "TRUE" : "FALSE")

#define VALUE_FIELD_WIDTH 24

/*------------------*/
/* Global Variables */
/*------------------*/
extern int prntflag;
extern FILE *trace_file;
static char Tabs[80];

/* Keep module independent of definitions for NAMESIZE, ATTRNAMESIZE */
static char Lname[NAMESIZE], Aname[ATTRNAMESIZE]; 

/*----------------------*/
/* Forward declarations */
/*----------------------*/
void log_resource_summary(int, attribute_t *, adapter_t *, int);
void log_message(int, char *, ...);
void log_error(int, int, char *, char *, char *);
void log_ok_value(int, attribute_t *);
void log_conflict(int, attribute_t *, attribute_t *);
void log_postpone(int, attribute_t *);
void log_wrap(int, attribute_t *);
void log_increment(int, attribute_t *, unsigned long);
void log_share(int, attribute_t *);
void log_resid_summary(int, adapter_t *);
static void set_tabs(int, int);
static void sprintf_values(char [], attribute_t *, unsigned long);
static void sprintf_names(char *, char *);
static void log_resource_stanza(attribute_t *, int, int, int);
static void log_resid_stanza(adapter_t *);

/***************************************************************************/
/*                     EXTERNALLY VISIBLE FUNCTIONS                        */
/***************************************************************************/

/***************************************************************************/
/* Function Name : LOG_RESOURCE_SUMMARY()                                  */
/* Function Type : C function                                              */
/* Purpose       : Log current resource value information to the log file  */
/*                 for all attributes attributes, ordered by bus resource. */
/* Global Vars   :                                                         */
/*   prntflag    - Boolean print if TRUE flag                              */
/*   Tabs        - Tabstring                                               */
/*   trace_file  - FILE struct pointer for logging                         */
/* Arguments     :                                                         */
/*  tabdepth     - tabs into beginning of each line                        */
/*  attr_head    - pointer to head of attributes list                      */
/*  adap_last    - OPTIONAL pointer to last adapter of attributes to print */  
/*  format       - INTRO | INTERIM | FORCE for table format                */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
void log_resource_summary(tabdepth, attr_head, adap_last, format)
	int tabdepth;
	attribute_t *attr_head;
	adapter_t *adap_last;
	int format;
{
	bus_resource_e resource; 
	attribute_t *attr_p, *attr_p2, *attr_last = NULL;
	int sharecnt, groupcnt;

	if (!prntflag)
		return;

	set_tabs(tabdepth, 0);

	/* Find the last attribute to terminate printing and print header string */
	if (adap_last != NULL)
	{

		/* If this adapter has no attributes don't print the summary table */
		if (adap_last->attributes == NULL)
		{
			fprintf(trace_file, "%sThis device has no bus resource attributes\n", Tabs);
			return;
		}

		/* If this adapter is AVAILABLE don't print the summary table unless we force it */
		if (adap_last->status == AVAILABLE && format != FORCE)
		{
			fprintf(trace_file, "%sThis device is AVAILABLE and had no changes made to its attributes\n", Tabs);
			return;
		}

		/* Find the attribute to stop printing at */
		for (attr_last = adap_last->attributes ; attr_last ; attr_last = attr_last->next)
			if (attr_last->adapter_ptr != adap_last)
				break;
	}

	fprintf(trace_file, "\n%s", Tabs);
	if (format == INTRO)
	{
		fprintf(trace_file, "RESRC  ADAPTER         ATTRIBUTE       SHARE GROUP         VALUES\n");
		fprintf(trace_file, "%s", Tabs);
		fprintf(trace_file, "-----  --------------- --------------- ----- ----- ------------------------\n");
	}
	else /* format != INTRO */
	{
		fprintf(trace_file, "RESRC  ADAPTER         ATTRIBUTE       SHARE GROUP         CURRENT\n");
		fprintf(trace_file, "%s", Tabs);
		fprintf(trace_file, "-----  --------------- --------------- ----- ----- -----------------------\n");
	}

	for (resource = MADDR ; resource <= DMALVL ; resource++) 
	{

		groupcnt = 0;
		sharecnt = 0;

		for (attr_p = attr_head ; attr_p != attr_last ; attr_p = attr_p->next)
		{

			if (attr_p->adapter_ptr->unresolved || (attr_p->ignore && format != FORCE))
				continue;

			if (attr_p->resource == GROUP)
			{
				++groupcnt;

				for (attr_p2 = attr_p->group_ptr ; attr_p2 ; attr_p2 = attr_p2->group_ptr)
				{
					if (attr_p2->specific_resource != resource) 
						continue;

					log_resource_stanza(attr_p2, 0, groupcnt, format);
				}

			}
			else if (attr_p->share_head && format != FORCE)
			{
				++sharecnt;

				if (attr_p->specific_resource != resource) 
					continue;

				/* Print the share list head attribute */
				log_resource_stanza(attr_p, sharecnt, 0, format);

				/* Print the subsequent shared attributes */
				for (attr_p2 = attr_p->share_ptr ; attr_p2 ; attr_p2 = attr_p2->share_ptr)
				{
					if (attr_p2->adapter_ptr->unresolved)
						continue;

					/* Sync the subsequent shared attribute for logging */
					if (attr_p->changed)
					{
						if (!attr_p2->changed)
						{
							attr_p2->changed = TRUE;
							if (attr_p2->reprsent == LIST)
								attr_p2->start.list.ptr = attr_p2->values.list.current;
							else /* attr_p2->reprsent == RANGE */
								attr_p2->start.range.value = attr_p2->current;
						}
						attr_p2->current = attr_p->current;
					}
					log_resource_stanza(attr_p2, sharecnt, 0, format);
				}

			}
			else
			{
				if (attr_p->specific_resource != resource)
					continue;

				log_resource_stanza(attr_p, 0, 0, format);
			}

		}
	}

	fflush(trace_file);
	return;
} /* end of log_resource_summary() */

/***************************************************************************/
/* Function Name : LOG_MESSAGE()                                           */
/* Function Type : C function                                              */
/* Purpose       : Prints out general purpose log message with LIMITED     */
/*                 parameter substitution in the style of printf().        */
/* Global Vars   :                                                         */
/*   prntflag    - Boolean print if TRUE flag                              */
/*   trace_file  - FILE struct pointer for logging                         */
/* Arguments     :                                                         */
/*  tabdepth     - Tabs into beginning of each line                        */
/*  fmtstring    - Format string                                           */
/*  arglist      - Substitution parameters (optional)                      */ 
/*                                                                         */
/* Notes         :                                                         */
/*                 1) Maximum formatted log message of 255 bytes + null    */
/*                 2) Format specifiers allowed :                          */
/*                      %s - string substitution                           */
/*                      %x - hexidecimal substitution (0x%08x)             */
/*                      %d - signed decimal substitution                   */
/*                      %u - unsigned decimal substitution                 */
/*                 3) Any other use of % prints a '%' and skips the next   */
/*                    character in the format string.                      */
/*                 4) Number of optional parameters determined by number   */
/*                    of valid format specifiers.                          */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
void log_message(int tabdepth, char *fmtstring, ...)
{
	va_list Argp;
	int bytes_out, str_space, num;
	char *str_p1, *str_p2, *char_p; 
	char str_out[256], tmp_str[30];

	if (!prntflag)
		return;

	if (strlen(fmtstring) == 0)
		return;

	memset(str_out, (int)'\0', sizeof(str_out));
	memset(str_out, (int)'\t', tabdepth);

	/*------------------------------------------------------------------*/
	/* Process arguments on the stack while there are format parameters */ 
	/* in the format string and output string space available.          */  
	/*------------------------------------------------------------------*/

	str_p1 = fmtstring;
	str_p2 = strchr(fmtstring, (int)'%');
	str_space = sizeof(str_out) - 1;
	va_start(Argp, fmtstring);
	for ( ; str_p2 && str_space ; )
	{

		bytes_out = (str_p2 - str_p1 > str_space) ? str_space : str_p2 - str_p1;
		strncat(str_out, str_p1, bytes_out);

		switch ((int)(str_p2[1]))
		{
			case 's'  : 
				char_p = va_arg(Argp, char*);
				strncat(str_out, char_p, str_space);
				break;
			case 'x'  : 
				num = va_arg(Argp, int);
				sprintf(tmp_str, "0x%08x", num);
				strncat(str_out, tmp_str, str_space);
				break;
			case 'd'  :
				num = va_arg(Argp, int);
				sprintf(tmp_str, "%d", num);
				strncat(str_out, tmp_str, str_space);
				break;
			case 'u'  :
				num = va_arg(Argp, int);
				sprintf(tmp_str, "%lu", num);
				strncat(str_out, tmp_str, str_space);
				break;
			default   :
				/* Print the '%' and skip next char */
				strcat(str_out, "%");
				break;
		}

		str_space = sizeof(str_out) - (strlen(str_out) + 1);
		str_p1 = str_p2 = str_p2 + 2;
		str_p2 = strchr(str_p2, (int)'%');
	}
	va_end(Argp);

	/* Pick up any string after the last format parameter */
	strncat(str_out, str_p1, str_space);

	fprintf(trace_file, "%s", str_out); 

	fflush(trace_file);
	return;
} /* end of log_message() */

/***************************************************************************/
/* Function Name : LOG_ERROR()                                             */
/* Function Type : C function                                              */
/* Purpose       : Prints out log message for adapter / attribute errors.  */
/* Global Vars   :                                                         */
/*   prntflag    - Boolean print if TRUE flag                              */
/*   Tabs        - Tabstring                                               */
/*   Lname       - Adapter logical name string                             */
/*   Aname       - Attribute name string                                   */
/*   trace_file  - FILE struct pointer for logging                         */
/* Arguments     :                                                         */
/*  tabdepth     - Tabs into beginning of each line                        */
/*  fail         - resulted in device being marked "unresolved"            */
/*  adap_name    - adapter logical name                                    */
/*  attr_name    - optional attribute name                                 */
/*  str          - error description                                       */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
void log_error(tabdepth, fail, adap_name, attr_name, str)
	int tabdepth, fail;
	char *adap_name, *attr_name, *str;
{
	if (!prntflag) 
		return;

	set_tabs(tabdepth, 0);
	sprintf_names(adap_name, attr_name);

	fprintf(trace_file, "%s", Tabs); 
	if (fail)
  	fprintf(trace_file, "TERMINATED - %s %s %s\n", Lname, Aname, str); 
	else
  	fprintf(trace_file, "ADJUSTED   - %s %s %s\n", Lname, Aname, str);

	fflush(trace_file);
	return;
} /* end of log_error() */

/***************************************************************************/
/* Function Name : LOG_OK_VALUE()                                          */
/* Function Type : C function                                              */
/* Purpose       : To be used for debug output for resolve_conflict().     */
/*                 Prints a single debug line to the logfile identifying   */
/*                 an acceptable (non-conflicting) attribute value.        */
/* Global Vars   :                                                         */
/*   Tabs        - Tabstring                                               */
/*   prntflag    - Boolean print if TRUE flag                              */
/*   trace_file  - FILE struct pointer for logging                         */
/* Arguments     :                                                         */
/*  tabdepth     - Tabs into beginning of each line                        */
/*  attr_p       - Attribute structure to be logged                        */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
void log_ok_value(tabdepth, attr_p)
	int tabdepth;
	attribute_t *attr_p;
{
	char valstr[VALUE_FIELD_WIDTH];
	attribute_t *attr_p1;

	if (!prntflag) 
		return;

	set_tabs(tabdepth, 0);

	/* Loop through the attribute, handling GROUP with attr_p1 */
	if (attr_p->resource == GROUP)
		attr_p1 = attr_p->group_ptr;
	else
		attr_p1 = attr_p;
	for ( ; attr_p1 ; attr_p1 = attr_p1->group_ptr)
	{
		sprintf_names(attr_p1->adapter_ptr->logname, attr_p1->name);
		sprintf_values(valstr, attr_p1, attr_p1->current);
		fprintf(trace_file, "%sValue OK  : %s %s %s\n", Tabs, Lname, Aname, valstr);
	}

	fflush(trace_file);

	return;
} /* end of log_OK_value() */

/***************************************************************************/
/* Function Name : LOG_CONFLICT()                                          */
/* Function Type : C function                                              */
/* Purpose       : To be used for debug output for resolve_conflict().     */
/*                 Prints a single debug line to the logfile identifying   */
/*                 a conflicting attribute value condition.                */
/* Global Vars   :                                                         */
/*   Tabs        - Tabstring                                               */
/*   prntflag    - Boolean print if TRUE flag                              */
/*   trace_file  - FILE struct pointer for logging                         */
/* Arguments     :                                                         */
/*  tabdepth     - Tabs into beginning of each line                        */
/*  attr_p1      - Attribute to be logged (current - may be GROUP)         */
/*  attr_p2      - Attribute to be logged (conflict - NEVER a GROUP)       */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
void log_conflict(tabdepth, attr_p1, attr_p2)
	int tabdepth;
	attribute_t *attr_p1, *attr_p2;
{
	char valstr[VALUE_FIELD_WIDTH];

	if (!prntflag) 
		return;

	set_tabs(tabdepth, 0);

	/* First half string (might be group)... */
	sprintf_names(attr_p1->adapter_ptr->logname, attr_p1->name);
	fprintf(trace_file, "%sConflict  : %s %s ", Tabs, Lname, Aname);

	/* ...second half string (never a group) */
	sprintf_values(valstr, attr_p2, attr_p2->current);
	sprintf_names(attr_p2->adapter_ptr->logname, attr_p2->name);
	fprintf(trace_file, "%s %s %s\n", Lname, Aname, valstr);

	fflush(trace_file);

	return;
} /* end of log_conflict() */

/***************************************************************************/
/* Function Name : LOG_POSTPONE()                                          */
/* Function Type : C function                                              */
/* Purpose       : To be used for debug output for resolve_conflict().     */
/*                 Prints a single debug line to the logfile identifying   */
/*                 an attribute for which we are postponing assignment of  */
/*                 attribute value.                                        */
/* Global Vars   :                                                         */
/*   Tabs        - Tabstring                                               */
/*   prntflag    - Boolean print if TRUE flag                              */
/*   trace_file  - FILE struct pointer for logging                         */
/* Arguments     :                                                         */
/*  tabdepth     - Tabs into beginning of each line                        */
/*  attr_p       - Attribute to be logged (current - may be GROUP)         */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
void log_postpone(tabdepth, attr_p)
	int tabdepth;
	attribute_t *attr_p;
{
	if (!prntflag) 
		return;

	set_tabs(tabdepth, 0);

	sprintf_names(attr_p->adapter_ptr->logname, attr_p->name);
	fprintf(trace_file, "%sPostpone  : %s %s Postpone assignment of value\n", 
					Tabs, Lname, Aname);

	fflush(trace_file);

	return;
	} /* end of log_postpone() */

/***************************************************************************/
/* Function Name : LOG_WRAP()                                              */
/* Function Type : C function                                              */
/* Purpose       : To be used for debug output for resolve_conflict().     */
/*                 Prints a single debug line to the logfile identifying   */
/*                 a wrapped attribute value condition.                    */
/* Global Vars   :                                                         */
/*   Tabs        - Tabstring                                               */
/*   prntflag    - Boolean print if TRUE flag                              */
/*   trace_file  - FILE struct pointer for logging                         */
/* Arguments     :                                                         */
/*  tabdepth     - Tabs into beginning of each line                        */
/*  attr_p       - Attribute structure to be logged                        */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
void log_wrap(tabdepth, attr_p)
	int tabdepth;
	attribute_t *attr_p;
{
	if (!prntflag) 
		return;

	set_tabs(tabdepth, 0);
	sprintf_names(attr_p->adapter_ptr->logname, attr_p->name);

	fprintf(trace_file, "%sWrapped   : %s %s\n", Tabs, Lname, Aname);

	fflush(trace_file);

	return;
} /* end of log_wrap() */

/***************************************************************************/
/* Function Name : LOG_INCREMENT()                                         */
/* Function Type : C function                                              */
/* Purpose       : To be used for debug output for resolve_conflict().     */
/*                 Prints a single debug line to the logfile identifying   */
/*                 an attribute increment.                                 */
/* Global Vars   :                                                         */
/*   Tabs        - Tabstring                                               */
/*   prntflag    - Boolean print if TRUE flag                              */
/*   trace_file  - FILE struct pointer for logging                         */
/* Arguments     :                                                         */
/*  tabdepth     - Tabs into beginning of each line                        */
/*  attr_p       - Attribute structure to be logged                        */
/*  oldval       - Current value of attribute before the increment         */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
void log_increment(tabdepth, attr_p, oldval)
	int tabdepth;
	attribute_t *attr_p;
	unsigned long oldval;
{
	char oldstr[VALUE_FIELD_WIDTH], newstr[VALUE_FIELD_WIDTH];

	if (!prntflag) 
		return;

	set_tabs(tabdepth, 0);
	sprintf_values(oldstr, attr_p, oldval);
	sprintf_values(newstr, attr_p, attr_p->current);
	sprintf_names(attr_p->adapter_ptr->logname, attr_p->name);

	fprintf(trace_file, "%sIncrement : %s %s %s    ->   %s\n", Tabs, Lname, Aname, oldstr, newstr);

	fflush(trace_file);

	return;
} /* end of log_increment() */

/***************************************************************************/
/* Function Name : LOG_SHARE()                                             */
/* Function Type : C function                                              */
/* Purpose       : To be used for debug for assign_shared_interrupts().    */
/*                 Prints a single debug line to the logfile identifying   */
/*                 a successful shareing of a sharable interrupt level.    */
/* Global Vars   :                                                         */
/*   Tabs        - Tabstring                                               */
/*   prntflag    - Boolean print if TRUE flag                              */
/*   trace_file  - FILE struct pointer for logging                         */
/* Arguments     :                                                         */
/*  tabdepth     - Tabs into beginning of each line                        */
/*  attr_p       - Attribute structure to be logged                        */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
void log_share(tabdepth, attr_p)
	int tabdepth;
	attribute_t *attr_p;
{
	char valstr[VALUE_FIELD_WIDTH];

	if (!prntflag) 
		return;

	set_tabs(tabdepth, 0);

	/* Cannot be a GROUPed attribute */

	sprintf_names(attr_p->adapter_ptr->logname, attr_p->name);
	sprintf_values(valstr, attr_p, attr_p->current);
	fprintf(trace_file, "%s Shared   : %s %s %s\n", Tabs, Lname, Aname, valstr);

	fflush(trace_file);

	return;
} /* end of log_share() */

/***************************************************************************/
/* Function Name : LOG_RESID_SUMMARY()                                     */
/* Function Type : Internal C function                                     */
/* Purpose       : Log a single residual data stanza.                      */
/* Global Vars   :                                                         */
/*   Lname       - Adapter logical name string                             */
/*   Aname       - Attribute name string                                   */
/*   Tabs        - Tabstring                                               */
/*   trace_file  - FILE struct pointer for logging                         */
/* Arguments     :                                                         */
/*  tabdepth     - tabs into beginning of each line                        */
/*  adap_head    - head of the adapter list                                */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
void log_resid_summary(tabdepth, adapter_head)
	int tabdepth;
	adapter_t *adapter_head;
{
	adapter_t *adap_p;

	if (!prntflag)
		return;

	set_tabs(tabdepth, 0);

	fprintf(trace_file, "%s", Tabs);
	fprintf(trace_file, "LOGNAME         PARENT          PARENT_BUS      INDEX\n");
	fprintf(trace_file, "%s", Tabs);
	fprintf(trace_file, "--------------- --------------- --------------- ------------------------\n");

	for (adap_p = adapter_head ; adap_p ; adap_p = adap_p->next)
		log_resid_stanza(adap_p);

	fflush(trace_file);
	return;
} /* end of log_resid_summary() */

/***************************************************************************/
/*                         STATIC FUNCTIONS                                */
/***************************************************************************/

/***************************************************************************/
/* Function Name : SET_TABS()                                              */
/* Function Type : Internal C function                                     */
/* Purpose       : Set up global tabstring                                 */
/* Global Vars   :                                                         */
/*   Tabs        - Tabstring                                               */
/* Arguments     :                                                         */
/*   tabdepth    - Tabs into beginning of each line (external parameter)   */
/*   align       - Internal format alignment tabs                          */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void set_tabs(tabdepth, align)
	int tabdepth, align;
{

	memset(Tabs, (int)'\t', sizeof(Tabs));

	tabdepth = (tabdepth > sizeof(Tabs) - align - 1) ? sizeof(Tabs) - align - 1 : tabdepth;
	Tabs[tabdepth + align] = '\0';

	return;
} /* end of set_tabs() */

/***************************************************************************/
/* Function Name : SPRINTF_NAMES()                                         */
/* Function Type : Internal C function                                     */
/* Purpose       : Set up global logical and attribute name strings, which */
/*                 are used to keep this module independent of the compiler*/
/*                 definitions for NAMESIZE and ATTRNAMESIZE.              */ 
/* Global Vars   :                                                         */
/*   Lname       - Adapter logical name string                             */
/*   Aname       - Attribute name string                                   */
/*   Tabs        - Tabstring                                               */
/* Arguments     :                                                         */
/*   lname       - Adapter logical name                                    */
/*   aname       - Attribute name                                          */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void sprintf_names(lname, aname)
	char *lname, *aname;
{

	if (lname != NULL)
	{
		memset(Lname, (int)' ', sizeof(Lname) - 1);
 		Lname[sizeof(Lname) - 1] = '\0';

		if (strlen(lname) < sizeof(Lname))
			memcpy(Lname, lname, strlen(lname));
		else
			memcpy(Lname, lname, sizeof(Lname) - 1);
	}

	if (aname != NULL)
	{
		memset(Aname, (int)' ', sizeof(Aname) - 1);
 		Aname[sizeof(Aname) - 1] = '\0';

		if (strlen(aname) < sizeof(Aname))
			memcpy(Aname, aname, strlen(aname));
		else
			memcpy(Aname, aname, sizeof(Aname) - 1);
	}

	return;
} /* end of sprintf_names() */

/***************************************************************************/
/* Function Name : SPRINTF_VALUES()                                        */
/* Function Type : Internal C function                                     */
/* Purpose       : Log current resource value information to the log file  */
/* Arguments     :                                                         */
/*  attr_p       - pointer to attributes struct                            */
/*  string       - VALUE_FIELD_WIDTH byte string to print values to        */
/*  value        - current value to use                                    */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void sprintf_values(string, attr_p, value)
	char string[VALUE_FIELD_WIDTH];
	attribute_t *attr_p;
	unsigned long value;
{

	switch (attr_p->specific_resource)
	{
		case MADDR    :
		case BADDR    :
		case IOADDR   :
			if (attr_p->width > 1)
				sprintf(string, "0x%08x - 0x%08x", value, value + attr_p->width - 1);
			else
				sprintf(string, "      0x%08x       ", value);
			break;
		case INTLVL   :
		case NSINTLVL :
		case SINTLVL  :
			switch ((int)INTR_CNTRLR(value))
			{
				case AT_BYTE   :
					sprintf(string, "        %s%d.%lu       ", "AT", INTR_INSTNC(value), INTR_PRINT(value));
					break;
				case MPIC_BYTE :
					sprintf(string, "       %s%d.%lu      ", "MPIC", INTR_INSTNC(value), INTR_PRINT(value));
					break;
				default        :
					sprintf(string, "         %3lu          ", INTR_PRINT(value));
			}
			break;
		case DMALVL   :
			sprintf(string, "          %2lu          ", value);
		}
	
	return;
} /* end of sprintf_values() */

/***************************************************************************/
/* Function Name : LOG_RESOURCE_STANZA()                                   */
/* Function Type : Internal C function                                     */
/* Purpose       : Log a single resource allocation summary stanza.        */
/* Global Vars   :                                                         */
/*   Lname       - Adapter logical name string                             */
/*   Aname       - Attribute name string                                   */
/*   Tabs        - Tabstring                                               */
/*   trace_file  - FILE struct pointer for logging                         */
/* Arguments     :                                                         */
/*  attr_p       - attribute structure pointer                             */
/*  share        - share index number                                      */
/*  group        - group index number                                      */ 
/*  format       - INTRO | INTERIM | FORCE for table format                */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void log_resource_stanza(attr_p, share, group, format)
	attribute_t *attr_p;
	int share, group, format;
{
	char resrc, sharestr[] = "     ", groupstr[] = "     ", valstr[VALUE_FIELD_WIDTH];

	sprintf_names(attr_p->adapter_ptr->logname, attr_p->name);

	switch (attr_p->specific_resource)
	{
		case MADDR    : resrc = 'M'; break;
		case BADDR    : resrc = 'B'; break;
		case IOADDR   : resrc = 'O'; break;
		case INTLVL   : resrc = 'I'; break; 
		case NSINTLVL : resrc = 'N'; break;
		case SINTLVL  : resrc = 'S'; break;
		case DMALVL   : resrc = 'A'; break;
		default       : resrc = '?';
	}
	
	if (share)
		sprintf(sharestr, "%3d  ", share);
	if (group)
		sprintf(groupstr, "%3d  ", group);

	fprintf(trace_file, "%s", Tabs);

  fprintf(trace_file, "  %-4c %s %s %s %s ", resrc, Lname, Aname, sharestr, groupstr);

	if (format == INTRO)
	{

		if (attr_p->reprsent == LIST)
		{
			value_list_t *vlist_p = attr_p->values.list.head;
			char _val_str[36], vals[60], *val_p;
 
			for (val_p = vals + 1 , vals[0] = '\0' ; vlist_p ; vlist_p = vlist_p->next)
			{
				switch (attr_p->specific_resource)
				{
					case MADDR    :
					case BADDR    :
					case IOADDR   : 
						sprintf(_val_str, "0x%x, ", vlist_p->value);
						break;
					case INTLVL   :
					case NSINTLVL : 
					case SINTLVL  : 
						switch ((int)INTR_CNTRLR(vlist_p->value))
						{
							case AT_BYTE   :
								sprintf(_val_str, "%s%d.%lu, ", "AT", 
									INTR_INSTNC(vlist_p->value), INTR_PRINT(vlist_p->value));
								break;
							case MPIC_BYTE :
								sprintf(_val_str, "%s%d.%lu, ", "MPIC", 
									INTR_INSTNC(vlist_p->value), INTR_PRINT(vlist_p->value));
								break;
							default        :
								sprintf(_val_str, "%lu, ", INTR_PRINT(vlist_p->value));
						}
						break;
					case DMALVL   :
						sprintf(_val_str, "%lu, ", vlist_p->value);
				}
				if ( val_p - vals + strlen(_val_str) < sizeof(vals) )
				{
					strcat(vals, _val_str);
					val_p += strlen(_val_str);
				}
				else
				{
		                               /*RESRC  ADAPTER         ATTRIBUTE       SHARE GROUP         VALUES         */
		                               /*-----  --------------- --------------- ----- ----- -----------------------*/
					fprintf(trace_file, "%s\n%s                                                   ", vals, Tabs);
					val_p = vals + 1;
					vals[0] = '\0';
					strcat(vals, _val_str);
					val_p += strlen(_val_str);
				}
	
				if (! vlist_p->next) 
				{
					vals[strlen(vals) - 2] = ' ';
					fprintf(trace_file, "%s\n", vals);
				}
			}
		}
		else /* attr_p->reprsent == RANGE */
		{
			value_range_t *rlist_p = attr_p->values.range.head;
			char _val_str[37];

			for ( ; rlist_p ; rlist_p = rlist_p->next)
			{

				memset(_val_str, 0, sizeof(_val_str));

				switch (attr_p->specific_resource)
				{
					case MADDR    :
					case BADDR    :
					case IOADDR   : 
						sprintf(_val_str, "0x%x - 0x%x , 0x%x", rlist_p->lower, rlist_p->upper, rlist_p->incr);
						break;
					case INTLVL   :
					case NSINTLVL : 
					case SINTLVL  : 
						switch ((int)INTR_CNTRLR(rlist_p->lower))
						{
							case AT_BYTE   :
								sprintf(_val_str, "%s%d.%lu - %lu , %lu", "AT", INTR_INSTNC(rlist_p->lower),
								        INTR_PRINT(rlist_p->lower), INTR_PRINT(rlist_p->upper), rlist_p->incr);
								break;
							case MPIC_BYTE :
								sprintf(_val_str, "%s%d.%lu - %lu , %lu", "MPIC", INTR_INSTNC(rlist_p->lower),
								        INTR_PRINT(rlist_p->lower), INTR_PRINT(rlist_p->upper), rlist_p->incr);
								break;
							default        :
								sprintf(_val_str, "%lu - %lu , %lu", INTR_PRINT(rlist_p->lower), 
												INTR_PRINT(rlist_p->upper), rlist_p->incr);
						}
						break;
					case DMALVL   :
						sprintf(_val_str, "%lu - %lu , %lu", rlist_p->lower, rlist_p->upper, rlist_p->incr);
				}

				fprintf(trace_file, "%s", _val_str);

				if (rlist_p->next)
				{
					fprintf(trace_file, ";");
					fprintf(trace_file, "\n");
					fprintf(trace_file, "%s", Tabs);
                             /*RESRC  ADAPTER         ATTRIBUTE       SHARE GROUP         VALUES         */
                             /*-----  --------------- --------------- ----- ----- -----------------------*/
					fprintf(trace_file, "                                                   ");
				}
				else
				{
					fprintf(trace_file, "\n");
				}
			}
		}
	}
	else /* format != INTRO */
	{
		sprintf_values(valstr, attr_p, attr_p->current);
		fprintf(trace_file, "%s\n", valstr);
	}

	return;
} /* end of log_resource_stanza() */

/***************************************************************************/
/* Function Name : LOG_RESID_STANZA()                                      */
/* Function Type : Internal C function                                     */
/* Purpose       : Log a single residual data stanza.                      */
/* Global Vars   :                                                         */
/*   Lname       - Adapter logical name string                             */
/*   Aname       - Attribute name string                                   */
/*   Tabs        - Tabstring                                               */
/*   trace_file  - FILE struct pointer for logging                         */
/* Arguments     :                                                         */
/*  adap_p       - adapter structure pointer                               */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void log_resid_stanza(adap_p)
	adapter_t *adap_p;
{
	char lname[16]; 

	fprintf(trace_file, "%s", Tabs);

	memset(lname, (int)' ', sizeof(lname) - 1);
	lname[sizeof(lname) - 1] = '\0';
	if (strlen(adap_p->logname) < sizeof(lname))
		memcpy(lname, adap_p->logname, strlen(adap_p->logname));
	else
		memcpy(lname, adap_p->logname, sizeof(lname) - 1);
	fprintf(trace_file, "%s ", lname);

	memset(lname, (int)' ', sizeof(lname) - 1);
	lname[sizeof(lname) - 1] = '\0';
	if (strlen(adap_p->parent) < sizeof(lname))
		memcpy(lname, adap_p->parent, strlen(adap_p->parent));
	else
		memcpy(lname, adap_p->parent, sizeof(lname) - 1);
	fprintf(trace_file, "%s ", lname);

	memset(lname, (int)' ', sizeof(lname) - 1);
	lname[sizeof(lname) - 1] = '\0';
	if (adap_p->parent_bus != NULL)
	{
		if (strlen(adap_p->parent_bus->logname) < sizeof(lname))
			memcpy(lname, adap_p->parent_bus->logname, strlen(adap_p->parent_bus->logname));
		else
			memcpy(lname, adap_p->parent_bus->logname, sizeof(lname) - 1);
	}
	fprintf(trace_file, "%s ", lname);

	if (adap_p->resid_index == NO_RESID_INDEX)
		fprintf(trace_file, "%s", "NO RESID INDEX");
	else
		fprintf(trace_file, "%d", adap_p->resid_index);

	fprintf(trace_file, "\n");

	fflush(trace_file);
	return;
} /* end of log_resid_stanza() */




