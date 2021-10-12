static char sccsid[] = "@(#)00	1.6  src/bos/usr/lib/nls/lsmle/lsmle.c, cmdmle, bos411, 9428A410j 5/13/94 10:06:22";
/*
 *   COMPONENT_NAME: cmdmle 
 *
 *   FUNCTIONS: MSGSTR
 *		all_output
 *		cc_output
 *		find_alternate_keyboard
 *		font_output
 *		keyb_output
 *		lang_output
 *		main
 *		print_error_and_exit
 *		print_usage_and_exit
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* lsmle                                                                      */
/*                                                                            */
/* List various cultural convention things from the ODM.  If no command line  */
/* arguments are given, the ILS file is basically dumped out.  One of the     */
/* following command line arguments may be given, some with an optional       */
/* parameter (if the parameter is not given, information for all possible     */
/* choices is output):                                                        */
/*                                                                            */
/*   -a          Output a multicolumn list of cc, lang and keyboard sets      */
/*   -c <cc>     Output the codeset and description of a cultural convention  */
/*   -l <lang>   Output the codeset and description of a language             */
/*   -k <keyb>   Output the codeset and description of a keyboard             */
/*   -f <font>   Output the codeset and description of a default font         */
/*                                                                            */
/* In addition, one of the following flags may be given to control what is    */
/* output:                                                                    */
/*                                                                            */
/*   -X          Output only the command used to make this setting            */
/*   -P          Output only the packages this setting is dependent on        */
/*   -C          Output only the codeset for the setting                      */
/*   -D          Output the long description for the setting (use with -k)    */
/*                                                                            */
/*----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>     /* getopt */
#include <nl_types.h>   /* message catalog */
#include "ILS.h"        /* odm stuff */

/* external variable defined by getopt() */
extern int optopt;


/* the following variable controls what is printed out. */
static long OUTPUT = 0;
#define IS_ALL    1
#define IS_CC     2
#define IS_LANG   4
#define IS_KEYB   8
#define IS_FONT  16
#define IS_COMM  32
#define IS_PACK  64
#define IS_CODE 128
#define IS_DESC 256

/* the following two variables point to any option parameters */
static char* field1 = NULL;
static char* field2 = NULL;

/* the following variable stores if we printed anything out */
static int DID_FIND=0;

#define NL_CAT          "BosMenus.cat"
#define NL_CAT_SET       2
#define MSGSTR(Num,Str)  catgets(catd, NL_CAT_SET, Num, Str)

#define ERR_CAT         "smit.cat"
#define ERR_CAT_SET      53
#define ERRSTR(Num,Str)  catgets(catd, ERR_CAT_SET, Num, Str)



/*------------------------------------------*/
/* Outputs a line for the -all option       */
/*                                          */
/* The output is the codeset, CC descrip    */
/* language descrip, keyboard descrip and   */
/* cc/language 5 char key.  Fields are      */
/* separated by " - " to aid in parsing by  */
/* SMIT.                                    */
/*------------------------------------------*/
void all_output (
nl_catd catd,
char *codeset,
char *text1,
short text1_id,
char *text2,
short text2_id,
char *descrip,
short descrip_id,
char *nls,
char *language,
char *keyboard
) {
	printf ("%-10s- %-24s- %-24s- %-80s- %-s - %-s - %-s\n",
		codeset, 
		MSGSTR(text1_id,text1),   
		MSGSTR(text2_id,text2),   
		MSGSTR(descrip_id,descrip), 
		nls,language,keyboard);
	DID_FIND = -1;
}


/*------------------------------------------*/
/* Outputs a line for the -cc option        */
/*                                          */
/* Normal output is the codeset, CC descrip */
/* and 5 char key within brackets.          */
/*------------------------------------------*/
void cc_output (
nl_catd catd,
char *codeset,
char *locale,
char *descrip,
short descrip_id,
char *package
) {
	if (OUTPUT & IS_PACK)
		printf ("%s\n", package);
	else if (OUTPUT & IS_CODE)
		printf ("%s\n", codeset);
	else
		printf ("%-10s %-s [%s]\n",
			codeset,
			MSGSTR(descrip_id,descrip),
			locale);
	DID_FIND = -1;
}


/*------------------------------------------*/
/* Outputs a line for the -keyb option      */
/*                                          */
/* Normal output is the codeset, descrip    */
/* and 5 char key within brackets.          */
/*------------------------------------------*/
void keyb_output (
nl_catd catd,
char *codeset,
char *locale,
char *descrip,
short descrip_id,
char *long_descrip,
short long_descrip_id,
char *package,
char *command
) {
	if (OUTPUT & IS_PACK)
		printf ("%s\n", package);
	else if (OUTPUT & IS_CODE)
		printf ("%s\n", codeset);
	else if (OUTPUT & IS_COMM)
		printf ("%s\n", command);
	else if (OUTPUT & IS_DESC)
		printf ("%-10s %-s [%s]\n",
			codeset,
			MSGSTR(long_descrip_id,long_descrip),
			locale);
	else
		printf ("%-10s %-s [%s]\n",
			codeset,
			MSGSTR(descrip_id,descrip),
			locale);
	DID_FIND = -1;
}


/*------------------------------------------*/
/* Outputs a line for the -lang option      */
/*                                          */
/* Normal output is the codeset, descrip    */
/* and 5 char key within brackets.          */
/*------------------------------------------*/
void lang_output (
nl_catd catd,
char *codeset,
char *locale,
char *descrip,
short descrip_id,
char *package
) {
	if (OUTPUT & IS_PACK)
		printf ("%s\n", package);
	else if (OUTPUT & IS_CODE)
		printf ("%s\n", codeset);
	else
		printf ("%-10s %-s [%s]\n",
			codeset,
			MSGSTR(descrip_id,descrip),
			locale);
	DID_FIND = -1;
}


/*------------------------------------------*/
/* Outputs a line for the -font option      */
/*                                          */
/* Normal output is the codeset and command */
/* used to change the font setting.         */
/*------------------------------------------*/
void font_output (
nl_catd catd,
char *codeset,
char *command
) {
	if (OUTPUT & IS_COMM)
		printf ("%s\n", command);
	else
		printf ("%-10s %-s\n", codeset, command);
	DID_FIND = -1;
}


/*------------------------------------------*/
/* Returns an array of alternate keyboards. */
/* Right now, the algorithm looks for names */
/* like <name>@alt - although any algorithm */
/* could be used.                           */
/*                                          */
/* NOTE: caller should NOT free array. This */
/* routine will be responsible for any      */
/* memory it allocates.                     */
/*------------------------------------------*/
struct KEYBOARD* find_alternate_keyboard (
struct KEYBOARD* array,
int n,
struct CC *celem,
int *m    /* return - #of array elements pointed to by return pointer */
) {
	int i;
	char altmap[20];

	sprintf (altmap, "%-.15s@alt", celem->keyboardLink_Lvalue);

	for (i=0; i<n; ++i)
		if (!strcmp (array[i].keyboard_map, altmap))
		{
			*m = 1;
			return (array+i);
		}

	*m = 0;
	return (NULL);
}


/*------------------------------------------*/
/* Error routine to print usage info        */
/*                                          */
/* Prints a summary of how to use this      */
/* command and then exits.                  */
/*------------------------------------------*/
void print_usage_and_exit (
int d
) {
	fprintf (stderr, "usage:\n");
	fprintf (stderr, "  lsmle [-X | -P | -C] [-c<cc> | ");
	fprintf (stderr, "-c\"<keyb> <lang>\" | -l<lang> | -k<keyb> |\n");
	fprintf (stderr, "                        -f<codeset> | -a]\n");

	odm_terminate();
	exit (d);
}


/*------------------------------------------*/
/* Error routine                            */
/*                                          */
/* Prints the given message catalog text    */
/* and exits.                               */
/*------------------------------------------*/
void print_error_and_exit (
nl_catd catd,
int n,
char *s
) {
	ERRSTR(n,s);
	odm_terminate();
	exit (1);
}



/*------------------------------------------*/
/* Main routine.                            */
/*------------------------------------------*/
main(
int argc,
char *argv[]
) {
	struct listinfo c_info, k_info, m_info, f_info;
	struct CC *cc = NULL;
	struct KEYBOARD *kb = NULL, *ktmp;
	struct MESSAGES *ms = NULL;
	struct FONT     *fn = NULL;
	int c, i, j, m, num;
	char *odmpath;
	nl_catd catd;
	nl_catd err_catd;
	char *tmp;
	int ret_val = 0;

        setlocale(LC_ALL,"");

	/* initialize ODM */
	odm_initialize();
/* Hardcoding ODMDIR to /usr/lib/objrepos because thats where mle databases is
	odmpath = getenv ("ODMDIR");
	if (odmpath != NULL)
		odm_set_path (odmpath);
*/
	odm_set_path ("/usr/lib/objrepos");

	/* open message catalogs */
	catd     = catopen (NL_CAT, NL_CAT_LOCALE);
	err_catd = catopen (ERR_CAT, NL_CAT_LOCALE);

	/* parse command args */
	if (argc > 1)
	{
		while ((c = getopt (argc, argv, ":ahc:f:k:l:XPCD")) != EOF)
		{
			if (c == ':')
				c = optopt;

			switch (c)
			{
				case 'a':
					OUTPUT |= IS_ALL;
					break;
				case 'c':
					OUTPUT |= IS_CC;
					break;
				case 'l':
					OUTPUT |= IS_LANG;
					break;
				case 'k':
					OUTPUT |= IS_KEYB;
					break;
				case 'X':
					OUTPUT |= IS_COMM;
					break;
				case 'P':
					OUTPUT |= IS_PACK;
					break;
				case 'C':
					OUTPUT |= IS_CODE;
					break;
				case 'D':
					OUTPUT |= IS_DESC;
					break;
				case 'f':
					OUTPUT |= IS_FONT;
					break;
				case 'h':
				default:
					print_usage_and_exit(1);
					break;
			}	

			if (optarg != NULL && *optarg != '-')
			{
				if (field1 == NULL)
					field1 = optarg;
				else
					print_usage_and_exit(1);
			}
		}
	}

	/*--------------------------------------------*/
	/* Check for dual parameters                  */
	/*                                            */
	/* This program allows an option argument to  */
	/* consist of two words separated by a space. */
	/* For example:   -c "En_US en_US"            */
	/* Note that this is NOT the same as:         */
	/*                -c En_US en_US              */
	/*--------------------------------------------*/
	if (OUTPUT&IS_CC && (tmp=strchr(field1,' ')) != NULL)
	{
		*tmp = '\0';
		field2 = tmp+1;
	}

	/* get lists from ODM */
	if (!OUTPUT || (OUTPUT&IS_ALL) || (OUTPUT&IS_CC))
	{
		/*--------------------------------------------------------*/
		/* NOTE: the last parameter is how many levels to search. */
		/* If there are more than one that match, will it return  */
		/* an array?  Who knows?  But it will not find the alt-   */
		/* ernate keyboards, since they have a different Lvalue.  */
		/*--------------------------------------------------------*/
		cc = odm_get_list (CC_CLASS,"", &c_info, 43, 2); 
		kb = odm_get_list (KEYBOARD_CLASS, "", &k_info, 43, 1);

		if (kb == NULL || !k_info.num || cc == NULL || !c_info.num)
			print_error_and_exit (err_catd, 56, "Error:  Cultural convention or keyboard information missing in ODM.\n");

		num = c_info.num;
	}
	else if (OUTPUT&IS_LANG)
	{
		ms = odm_get_list (MESSAGES_CLASS, "", &m_info, 43, 1);

		if (ms == NULL || !m_info.num)
			print_error_and_exit (err_catd, 57, "Error:  Language information missing in ODM.\n");

		num = m_info.num;
	}
	else if (OUTPUT&IS_KEYB)
	{
		kb = odm_get_list (KEYBOARD_CLASS, "", &k_info, 43, 1);

		if (kb == NULL || !k_info.num)
			print_error_and_exit (err_catd, 58, "Error:  Keyboard information missing in ODM.\n");

		num = k_info.num;
	}

	if (OUTPUT & IS_FONT)
	{
		fn = odm_get_list (FONT_CLASS, "", &f_info, 4, 1);

		if (fn == NULL || !f_info.num)
			print_error_and_exit (err_catd, 59, "Error:  Font information missing in ODM.\n");

		num = f_info.num;
	}

	/*----------------*/
	/* Loop and print */
	/*----------------*/
	for (i = 0; i < num; i++)
	{
		switch (OUTPUT & 31)
		{
			case IS_ALL:
				all_output (catd,
					cc[i].codeset, 
					cc[i].text_string, 
                    cc[i].text_string_id,
					cc[i].messageLink->text_string,
					cc[i].messageLink->text_string_id,
					cc[i].keyboardLink->key_text,
					cc[i].keyboardLink->key_text_id,
					cc[i].locale,
					cc[i].messageLink->locale,
					cc[i].keyboardLink->keyboard_map);

				if ((ktmp=find_alternate_keyboard(kb,k_info.num,cc+i,&m)) 
					!= NULL)
				{
					all_output (catd,
						cc[i].codeset, 
						cc[i].text_string, 
                    	cc[i].text_string_id,
						cc[i].messageLink->text_string,
						cc[i].messageLink->text_string_id,
						ktmp->key_text, 
						ktmp->key_text_id,
						cc[i].locale,
						cc[i].messageLink->locale,
						ktmp->keyboard_map);
				}
				break;

			case IS_CC:
				if (field1 == NULL || 
					(field2 == NULL && !strcmp(cc[i].locale,field1)) ||
					(field2 != NULL && !strcmp(cc[i].keyboardLink->keyboard_map,
					 field1) && !strcmp(cc[i].messageLink->locale,field2)))
					cc_output (catd,
						cc[i].codeset,
						cc[i].locale,
						cc[i].text_string,
						cc[i].text_string_id,
						cc[i].package);
				break;
	
			case IS_LANG:
				if (field1 == NULL || !strcmp(ms[i].locale,field1))
					lang_output (catd,
						ms[i].codeset,
						ms[i].locale,
						ms[i].text_string,
						ms[i].text_string_id,
						ms[i].package);
				break;

			case IS_KEYB:
				if (field1 == NULL || !strcmp(kb[i].keyboard_map,field1))
				{
					keyb_output (catd,
						kb[i].codeset,
						kb[i].keyboard_map,
						kb[i].text_string,
						kb[i].text_string_id,
						kb[i].key_text,
						kb[i].key_text_id,
						kb[i].package,
						kb[i].keyboard_cmd);

					/* break out of loop to avoid alternate keyboards */
					if (field1 != NULL)
						i = num;
				}
				break;

			case IS_FONT:
				if (field1 == NULL || !strcmp(fn[i].codeset,field1))
					font_output (catd,
						fn[i].codeset,
						fn[i].font_cmd);
				break;

			default:
				printf ("CC:\n");
				printf ("  locale:         \"%s\"\n", cc[i].locale);
				printf ("  text_string:    \"%s\"\n", cc[i].text_string);
				printf ("  text_string_id: %d\n", (int) cc[i].text_string_id);
				printf ("  codeset:        \"%s\"\n", cc[i].codeset);
				printf ("  messages:       \"%s\"\n", cc[i].messages);
				printf ("  keyboards:      \"%s\"\n", cc[i].keyboards);
				printf ("  package:        \"%s\"\n", cc[i].package);
				printf ("  variables:      \"%s\"\n", cc[i].variables);
				printf ("  sbcs_variables: \"%s\"\n", cc[i].sbcs_variables);
				printf ("  bosinst_menu:   \"%s\"\n", cc[i].bosinst_menu);
				printf ("  icon_path:      \"%s\"\n", cc[i].icon_path);
				printf ("  menu:           \"%s\"\n", cc[i].menu);

				/* print keyboard link */
				if (cc->keyboardLink != NULL)
				{
					printf ("\n  Keyboard Descriptions (%d):\n", 
						cc[i].keyboardLink_info->num);
					printf ("  (1) locale:         \"%s\"\n", 
						cc[i].keyboardLink->keyboard_map);
					printf ("  (1) keyboard_map:   \"%s\"\n", 
						cc[i].keyboardLink->keyboard_map);
					printf ("  (1) text_string:    \"%s\"\n", 
						cc[i].keyboardLink->text_string);
					printf ("  (1) text_string_id: %d\n", 
						(int) cc[i].keyboardLink->text_string_id);
					printf ("  (1) codeset:        \"%s\"\n", 
						cc[i].keyboardLink->codeset);
					printf ("  (1) package:        \"%s\"\n", 
						cc[i].keyboardLink->package);
					printf ("  (1) variables:      \"%s\"\n", 
						cc[i].keyboardLink->variables);
					printf ("  (1) keyboard_cmd:   \"%s\"\n", 
						cc[i].keyboardLink->keyboard_cmd);
					printf ("  (1) key_text:       \"%s\"\n", 
						cc[i].keyboardLink->key_text);
					printf ("  (1) key_text_id:    %d\n", 
						(int) cc[i].keyboardLink->key_text_id);
					printf ("  (1) bosinst_menu:   \"%s\"\n", 
						cc[i].keyboardLink->bosinst_menu);
				}

				/* look for alternate keyboard */
				if ((ktmp = find_alternate_keyboard (kb, k_info.num, 
					cc+i, &m)) != NULL)
				{
					printf ("\n  Alternate Keyboard Description:\n");
					printf ("  (1) locale:         \"%s\"\n", 
						ktmp->keyboard_map);
					printf ("  (1) keyboard_map:   \"%s\"\n", 
						ktmp->keyboard_map);
					printf ("  (1) text_string:    \"%s\"\n", 
						ktmp->text_string);
					printf ("  (1) text_string_id: %d\n", 
						(int) ktmp->text_string_id);
					printf ("  (1) codeset:        \"%s\"\n", 
						ktmp->codeset);
					printf ("  (1) package:        \"%s\"\n", 
						ktmp->package);
					printf ("  (1) variables:      \"%s\"\n", 
						ktmp->variables);
					printf ("  (1) keyboard_cmd:   \"%s\"\n", 
						ktmp->keyboard_cmd);
					printf ("  (1) key_text:       \"%s\"\n", 
						ktmp->key_text);
					printf ("  (1) key_text_id:    %d\n", 
						(int) ktmp->key_text_id);
					printf ("  (1) bosinst_menu:   \"%s\"\n", 
						ktmp->bosinst_menu);
				}
	
				/* print message info */
				if (cc->messageLink != NULL)
				{
					printf ("\n  Message Descriptions (%d):\n", 
						cc[i].messageLink_info->num);
					printf ("  (1) message Lvalue: \"%s\"\n", 
						cc[i].messageLink->locale);
					printf ("  (1) message string: \"%s\"\n", 
						cc[i].messageLink->text_string);
					printf ("  (1) codeset:        \"%s\"\n", 
						cc[i].messageLink->codeset);
					printf ("  (1) package:        \"%s\"\n", 
						cc[i].messageLink->package);
					printf ("  (1) variables:      \"%s\"\n", 
						cc[i].messageLink->variables);
				}

				printf ("\n");
				break;
	
		}
	}

	/* now check if we printed anything out */
	if (!DID_FIND && !(OUTPUT&IS_PACK) && !(OUTPUT&IS_CODE) && !(OUTPUT&IS_COMM))
	{
		/* If we didn't print anything out, in the following */
		/* cases, we want to print out something to tell the */
		/* user he asked for an invalid entry                */
		/* This is for SMIT, because the user may have a     */
		/* bogus environment variable set ...                */
		ret_val=2;
		switch (OUTPUT & 31)
		{
			case IS_CC:
				cc_output (catd,
					"???????",
					field1,
					"???????",
					9999,
					"???????");
				break;
	
			case IS_LANG:
				lang_output (catd,
					"???????",
					field1,
					"???????",
					9999,
					"???????");
				break;

			case IS_KEYB:
				keyb_output (catd,
					"???????",
					field1,
					"???????",
					9999,
					"???????",
					9999,
					"???????",
					"???????");
				break;
		}
	}

	odm_terminate ();
	exit(ret_val);
}
