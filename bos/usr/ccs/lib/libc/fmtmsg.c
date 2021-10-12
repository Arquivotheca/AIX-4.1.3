static char sccsid[] = "@(#)83	1.1  src/bos/usr/ccs/lib/libc/fmtmsg.c, libcfmt, bos411, 9428A410j 3/4/94 10:26:52";
/*
 *   COMPONENT_NAME: libcfmt
 *
 *   FUNCTIONS: arg_setup
 *		fmtmsg
 *		formatter
 *		isaverb
 *		label_ok
 *		severity_lookup
 *
 *   ORIGINS: 85
 *
 *                    SOURCE MATERIALS
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fmtmsg.h>
#include <fcntl.h> 
#include <errno.h>

#define MSG_LBL		0x00000001
#define MSG_SEV		0x00000002
#define MSG_TXT		0x00000004
#define MSG_ACT		0x00000008
#define MSG_TAG		0x00000010

extern char *getenv();

static char     ctty[] = "/dev/console";

/*
 *  Definition of message formats
 */
static char 	f0[] = 	"";
static char	f1[] =  "%s%s%s";
static char	f2[] =  "%s%s%s\n";
static char	f3[] = 	"%s: %s: %s\n";	
static char	f4[] = 	"%s: %s%s\n";
static char	f5[] = 	"%s%s\n";
static char	f6[] = 	"TO FIX: %s %s\n";
static char 	f7[] = 	"%s%s";
static char	f8[] =	"%s%s: %s\n";


/*
 * FUNCTION: This routine accepts arguments and produces a standard format
 * 	     output of the form ....
 *
 *			label: severity: text
 *			TO FIX: action tag
 * PARAMETERS: 
 *		class:  where should the message be printed
 *		label:	text for label field of output
 * 		severity: integer value indicating level of severity
 *		text: text for text field of output
 *		action: text for action field of output
 *		tag: text for tag field of output
 *		
 * RETURN VALUE DESCRIPTIONS:
 *		MM_OK: function succeeded
 *		MM_NOTOK: function failed due to an invalid Severity Level
 * 		MM_NOMSG: function could not write to stderr
 *		MM_NOCON: function could not write to system console
 *
 */
char *severity_lookup();
unsigned long isaverb();
void formatter();
int arg_setup();

int
fmtmsg(long class, const char *label, int severity, 
       const char *text, const char *action, const char *tag)
{
	int ret;
	FILE *fd;
	unsigned long verb;
	unsigned long msgvmask = 0;
	unsigned long msgnmask = 0;
	char *msgverb = NULL;
	char *sev_level = NULL;
	char *tmp_ptr,*sev_msg,*token;
	char *args[5], *formats[2];
#ifdef _THREAD_SAFE
	char *nxt_token;
#endif

	tmp_ptr = getenv("MSGVERB");
	if(tmp_ptr != NULL) {
	   if((msgverb = (char *)malloc((size_t)(strlen(tmp_ptr)+1))) == NULL) 
		return(MM_NOTOK);
	   (void) strcpy(msgverb, tmp_ptr);
	}

	tmp_ptr = getenv("SEV_LEVEL");
	if(tmp_ptr != NULL) {
	  if((sev_level = (char *)malloc((size_t)(strlen(tmp_ptr)+1))) == NULL){
		free(msgverb);
		return(MM_NOTOK);
	  }
	  (void) strcpy(sev_level, tmp_ptr);
	}

	if(msgverb == NULL) {
	   msgvmask = MSG_LBL|MSG_SEV|MSG_TXT|MSG_ACT|MSG_TAG;
	} else {
#ifdef _THREAD_SAFE
	  token = strtok_r(msgverb,":",&nxt_token);
#else
	  token = strtok(msgverb,":");
#endif
	  if(token == NULL) msgvmask= MSG_LBL|MSG_SEV|MSG_TXT|MSG_ACT|MSG_TAG;
	  while(token){
		verb = isaverb(token);
		if(verb == 0) { 
		   msgvmask= MSG_LBL|MSG_SEV|MSG_TXT|MSG_ACT|MSG_TAG;
		   break;
		} else
		   msgvmask |= verb;
#ifdef _THREAD_SAFE
		token = strtok_r(nxt_token,":",&nxt_token);
#else
		token = strtok(NULL,":");
#endif
	  }
	}
	free(msgverb);

	if(label != MM_NULLLBL) msgnmask |= MSG_LBL;
	if(severity != MM_NULLSEV) msgnmask |= MSG_SEV;
	if(text != MM_NULLTXT) msgnmask |= MSG_TXT;
	if(action != MM_NULLACT) msgnmask |= MSG_ACT;
	if(tag != MM_NULLTAG) msgnmask |= MSG_TAG;

	if(msgnmask & MSG_SEV) {
	   switch(severity) {
		case MM_HALT: 	sev_msg = "HALT";
				break;
		case MM_ERROR:	sev_msg = "ERROR";
				break;
		case MM_WARNING:sev_msg = "WARNING";
				break;
		case MM_INFO:	sev_msg = "INFO";
				break;
		case MM_NOSEV:	sev_msg = "";
				break;
		default: /*	must search for user defined severity */
				sev_msg = severity_lookup(sev_level, severity);
				if(sev_msg == NULL) {
					free(sev_level);
					return(MM_NOTOK);
				}
				break;
	   }
	}

	if(class & MM_PRINT) {
		formatter((msgvmask & msgnmask), formats);
		ret= arg_setup 
		     ((msgvmask & msgnmask),label,sev_msg,text,action,tag,args);
		if(ret < 0) {
			free(sev_level);
			return(MM_NOTOK);
		}
		ret = fprintf(stderr, formats[0], args[0], args[1], args[2]);
		if(ret < 0) {
			free(sev_level);
			return(MM_NOMSG);
		}
		ret = fprintf(stderr, formats[1], args[3], args[4]);
		if(ret < 0) {
			free(sev_level);
			return(MM_NOMSG);
		} 
	}

	if(class & MM_CONSOLE) {
		formatter(msgnmask, formats);
		ret = arg_setup(msgnmask,label,sev_msg,text,action,tag,args);
		if(ret < 0) {
			free(sev_level);
			return(MM_NOTOK);
		}
		fd = fopen(ctty, "a");
		if(fd == NULL) {
			free(sev_level);
			return(MM_NOCON);
		}
                ret = fprintf(fd,formats[0], args[0], args[1] ,args[2]);
		if(ret < 0) {
			free(sev_level);
			fclose(fd);
			return(MM_NOCON);
		} 
                ret = fprintf(fd, formats[1], args[3], args[4]);
		if(ret < 0) {
			free(sev_level);
			fclose(fd);
			return(MM_NOCON);
		} 
		fclose(fd);
	}
	free(sev_level);
 	return(MM_OK);
}


char *
severity_lookup(char *sev_level, int severity)
{
#ifdef _THREAD_SAFE
	char *nxt_field, *nxt_list;
#endif
	char *field, *list, *ret_val;
	int dcount,match;

	ret_val = NULL;
	match = FALSE;
	if(sev_level != NULL) {
#ifdef _THREAD_SAFE
	   field = strtok_r(sev_level,":",&nxt_field);
#else
	   field = strtok(sev_level,":");
#endif
	   while(field) {
		dcount = 0;
#ifdef _THREAD_SAFE
		list = strtok_r(field,",",&nxt_list);
#else
		list = strtok(field,",");
#endif
		while(list) {
			dcount++;
			if(dcount == 2) 
				if(atoi(list) == severity) match = TRUE;
			if((dcount == 3) && (match == TRUE)) ret_val = list;
#ifdef _THREAD_SAFE
			list = strtok_r(nxt_list,",",&nxt_list);
#else
			list = strtok(NULL,",");
#endif
		}
		if((dcount == 3) && (match == TRUE)) break;
#ifdef _THREAD_SAFE
		field = strtok_r(nxt_field,":",&nxt_field);
#else
		field = field + strlen(field) + 1;
		field = strtok(field,":");
#endif
	   }
	}
	return(ret_val);
}


unsigned long
isaverb(char *token)
{
	if(strcmp("label",token) == 0) return(MSG_LBL);
	else if(strcmp("severity",token) == 0) return(MSG_SEV);
	else if(strcmp("text",token) == 0) return(MSG_TXT);
	else if(strcmp("action",token) == 0) return(MSG_ACT);
	else if(strcmp("tag",token) == 0) return(MSG_TAG);
	else return(0L);
}

int
arg_setup(unsigned long msgmask, char *label, char *sev_msg,
	char *text, char *action, char *tag, char *args[])
{
	int i;
	int ret_val = -1;
	
	if(label_ok(label) == 0) {
			ret_val = 0;
			for(i=0; i < 5; i++) args[i] = f0;
			if(msgmask & MSG_LBL) args[0]=(char *)label;
			if(msgmask & MSG_SEV) args[1]=(char *)sev_msg;
			if(msgmask & MSG_TXT) args[2]=(char *)text;
			if(msgmask & MSG_ACT) args[3]=(char *)action;
			if(msgmask & MSG_TAG) args[4]=(char *)tag;
	}
	return(ret_val);
}

int
label_ok(char *label)
{
	int ret_val = -1;
	char *loc_label,*label1, *label2;
#ifdef _THREAD_SAFE
	char *nxt_label;
#endif
	if(label == MM_NULLLBL) 
		ret_val = 0;
	else {
	   if((loc_label= (char *) malloc((size_t)(strlen(label)+1))) == NULL)
		return(ret_val);
	   (void) strcpy(loc_label, label);

	   if(strlen(loc_label) <= 25)
#ifdef _THREAD_SAFE
	      if((label1 = (char *)strtok_r(loc_label,":",&nxt_label)) != NULL) 
#else
	      if((label1 = (char *)strtok(loc_label,":")) != NULL) 
#endif
	       if(strlen(label1) <= 10) 
#ifdef _THREAD_SAFE
	        if((label2=(char *)strtok_r(nxt_label,":",&nxt_label)) != NULL) 
#else
	        if((label2 = (char *)strtok(NULL,":")) != NULL) 
#endif
		  if(strlen(label2) <= 14)  ret_val = 0;
			
	   free(loc_label);
	}
	return(ret_val);
}


void
formatter(unsigned long msgmask, char *formats[])
{
	switch(msgmask & (MSG_LBL|MSG_SEV|MSG_TXT)) {
		case 0: formats[0]= f1;
			break;
		case MSG_LBL: formats[0]= f2;
			break;
		case MSG_SEV: formats[0]= f2;
			break;
		case MSG_TXT: formats[0]= f2;
			break;
		case MSG_LBL|MSG_SEV: formats[0]= f3;
			break;
		case MSG_LBL|MSG_TXT: formats[0]= f4;
			break;
		case MSG_SEV|MSG_TXT: formats[0]= f8;
			break;
		case MSG_LBL|MSG_SEV|MSG_TXT: formats[0]= f3;
			break;
		default: formats[0]=f1;
			break;
	}
	
	switch(msgmask & (MSG_ACT|MSG_TAG)) {
		case 0: formats[1]= f7;
			break;
		case MSG_ACT: formats[1]= f6;
			break;
		case MSG_TAG: formats[1]= f5;
			break;
		case MSG_ACT|MSG_TAG: formats[1]= f6;
			break;
		default: formats[1]=f1;
			break;
	}
}
