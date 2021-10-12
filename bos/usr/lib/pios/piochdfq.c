static char sccsid[] = "@(#)92  1.3  src/bos/usr/lib/pios/piochdfq.c, cmdpios, bos41J, 9510A_all 3/1/95 13:36:33";
/*
 *   COMPONENT_NAME: CMDPIOS
 *
 *   FUNCTIONS: main
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <locale.h>
#include <fcntl.h>
#include <nl_types.h>
#include <unistd.h>
#include <piobe_msg.h> 

#define STRGZARG(a)	#a
#define STRGZS(a)	STRGZARG(a)
#define QCONFIG		"/etc/qconfig"
#define CATALOG		"piobe.cat"
#define MSG_SET		(8)
#define SCRIPT "/bin/ksh -c  ' \
function qdef {   \n\
dir=/tmp  \n\
qconfig_new=/etc/qconfig  \n\
qdef=${1%%:*}  \n\
  \n\
# Check for non-existent queue \n\
/usr/bin/enq -P $qdef -Y 2>/dev/null \n\
if [ $? -ne 0 ]  \n\
then  \n\
	/usr/bin/dspmsg -s 8 piobe.cat " STRGZS(MSG_CDQ_INVQ) " \"piochdfq: Print queue %s does not exist.\n\" $qdef >&2 \n\
	exit 1  \n\
fi \n\
# \n\
# awk variables: \n\
# qconfig_new  : Comments and blank lines from top of old qconfig \n\
# qconfig_save : New default queue and queue device stanzas \n\
# qconfig_chop : Everything else \n\
# save  = -1   :  Before default queue is found  \n\
# save  =  0   :  After default queue found, but before processing devices  \n\
# save  >  0   :  While processing devices  \n\
/usr/bin/awk \"BEGIN {  \n\
	# user-input queue to become new default \n\
	queue   = ARGV[1];  \n\
	# getline reads ARGV[1] : re-map so it works \n\
	ARGV[1] = ARGV[2];  \n\
	ARGV[2] = \\\"\\\"; \n\
	ARGV[3] = \\\"\\\";  \n\
	save = -1;  \n\
	past_comments = 0;  \n\
}  \n\
  \n\
(\\$0 ~ \\\"^[\\*]\\\" || \\$0 ~ \\\"^[ 	]*\\$\\\") && past_comments == 0 {   \n\
	if (qconfig_new == \\\"\\\") {  \n\
		qconfig_new = \\$0  \n\
	} else {  \n\
		qconfig_new = qconfig_new \\\"\\n\\\" \\$0  \n\
	}  \n\
	next;  \n\
}  \n\
  \n\
\\$0 ~ /^.*:/ {  \
	past_comments = 1;  \n\
	unget = 0;  \n\
	do {  \n\
	# Strip off colon from name \n\
	obj_name = substr(\\$1,0,index(\\$1,\\\":\\\")-1)  \n\
	if (obj_name == queue) {  \n\
		# This is either default queue  \n\
		# OR a device in some other queue.  \n\
		done = 0;  \n\
		tmp = \\$0;  \n\
		while ( getline > 0 && !done) {  \n\
			if (match(\\$0, /^[ 	]*device[ 	]*=/)) {  \n\
				qd_num = split(\\$3,qd_list,\\\",\\\")   \n\
				save = 0;    \n\
			} else { \n\
				if (match(\\$0, /^[^\\*]*:/)) {  \n\
					done = 1;  \n\
					# We have read an extra line  \n\
					# save it for processing during next iteration  \n\
					replay = \\$0;  \n\
					unget = 1;  \n\
					break;  \n\
				}  #endif \n\
			} #endif \n\
			tmp = tmp \\\"\\n\\\" \\$0  \n\
		} #endwhile \n\
		if (save == 0) {  \n\
			# We found the default queue,  \n\
			# save off device stanzas for the default queue  \n\
			# Current line is processed here making replay \n\
			# obselete.  Thats Ok because we overwrite it \n\
			# in the while loop. \n\
			qconfig_save = tmp  \n\
			while (save < qd_num && save >=0) {  \n\
				old_save = save  \n\
				for (i in qd_list) {  \n\
					device = substr(\\$1,0,index(\\$1,\\\":\\\")-1)  \n\
					if (device == qd_list[i]) {  \n\
						save++;  \n\
						# Found device in default q  \n\
						# step through stanza	  \n\
						done = 0;  \n\
						qconfig_save = qconfig_save \\\"\\n\\\" device \\\":\\\"  \n\
						while ((rc=getline) > 0 && !done) {  \n\
							if (match(\\$0,/^[^\\*]*:/)) {  \n\
								done = 1;  \n\
								# We have read an extra line  \n\
								# save it for processing during next iteration  \n\
								replay = \\$0;  \n\
								unget = 1;  \n\
								break;  \n\
							} #endif \n\
							qconfig_save = qconfig_save \\\"\\n\\\" \\$0				  \n\
						} #endwhile \n\
						if (rc <= 0) {  \n\
							exit;  \n\
						} #endif \n\
					} #endif \n\
				} #endfor \n\
				if (old_save == save) {  \n\
					save = -1;  \n\
				} #endif \n\
			} #endwhile \n\
		} else {  \n\
			# this is a device not part of default queue  \n\
			qconfig_chop = qconfig_chop \\\"\\n\\\" tmp  \n\
		} #endif \n\
	} else {  \n\
		# Not part of default queue  \n\
		if (qconfig_chop == \\\"\\\") {  \n\
			qconfig_chop = \\$0  \n\
		} else {  \n\
			qconfig_chop = qconfig_chop \\\"\\n\\\" \\$0  \n\
		} #endif \n\
	} #endif \n\
	# if we read an extra line   \n\
	# without processing it, put it back  \n\
	if (unget == 1) {  \n\
		\\$0 = replay;  \n\
		unget = 0;  \n\
	} else {  \n\
		rc = getline	  \n\
	} #endif \n\
	} while (rc > 0)   \n\
} \n\
  \n\
END {  \n\
	print qconfig_new \\\"\\n\\\" qconfig_save \\\"\\n\\\" qconfig_chop   \n\
}\" $qdef /etc/qconfig > $dir/qdefault.sorted.$$  \n\
  \n\
/usr/lib/lpd/digest $dir/qdefault.sorted.$$ $dir/qdefault.bin.$$  \n\
if [ $? -eq 0 ]  \n\
then  \n\
	/usr/bin/mv -f $dir/qdefault.sorted.$$ $qconfig_new  \n\
	/usr/bin/chown root.printq $qconfig_new \n\
	/usr/bin/chmod 664 $qconfig_new \n\
else  \n\
	/usr/bin/dspmsg -s 8 piobe.cat " STRGZS(MSG_CDQ_DIGFAIL) " \"piochdfq: Could not make queue %s the default.\" $qdef  >&2 \n\
fi  \n\
/usr/bin/rm -f $dir/qdefault*.$$ 2>/dev/null  \n\
}; qdef "

/* Translatable messages */
#define USAGE_MSG    "Usage: piochdfq -q default_queue\n"
#define OPEN_ERROR   "piochdfq:  Could not open /etc/qconfig\n"
#define LOCK_ERROR   "piochdfq:  Could not lock /etc/qconfig\n"
#define SYSTEM_ERROR "piochdfq:  System error - %s"

main(int ac, char **av) 
{
   int		qconfig;        /* file descriptor for /etc/qconfig */
   struct flock	lck;
   nl_catd	smit_cat;
   char		*qdef;
   int 		i,qflg=0;
   extern char	*optarg;
   extern int	optind, opterr;

   (void) setlocale(LC_ALL, "");

   smit_cat = catopen(CATALOG, NL_CAT_LOCALE);

   for (opterr = 0, optind = 1; (i = getopt((int)ac, av, "q:")) != EOF; ) {
	switch (i) {
	case 'q':
		qflg++;
		/* Allocate space for entire shell script & arg     */
		/* +1 for closing quote after user-input queue-name */
		/* and +1 for NULL delimiter character.             */
  		qdef = (char *)malloc(strlen(optarg)+strlen(SCRIPT)+2);
		(void) strcpy(qdef,SCRIPT); 
		(void) strcat(qdef,optarg); 
		(void) strcat(qdef,"'");
		break;

	case '?':
		(void) fprintf(stderr, catgets(smit_cat,MSG_SET,MSG_CDQ_USAGE,USAGE_MSG));
		(void) catclose(smit_cat);
		return (1);
	} /* endswitch */
   } /* endfor */
   if (qflg == 0) {
	/* q flag not specified on the command-line */
	(void) fprintf(stderr, catgets(smit_cat,MSG_SET,MSG_CDQ_USAGE,USAGE_MSG));
	(void) catclose(smit_cat);
	return (1);
   } /* endif */	

   /* open qconfig */
   if ((qconfig = open(QCONFIG, O_RDWR)) == -1) {
   	(void) fprintf(stderr, catgets(smit_cat,MSG_SET,MSG_CDQ_OPEN_ERROR,OPEN_ERROR)); 
   	return (1);
   } /* endif */

   /* lock qconfig */
   lck.l_whence = lck.l_start = lck.l_len = 0;
   lck.l_type = F_WRLCK;
   if (fcntl(qconfig, F_SETLKW, &lck) != 0) {
	(void) fprintf(stderr, catgets(smit_cat,MSG_SET,MSG_CDQ_LOCK_ERR,LOCK_ERROR), strerror(errno));
	return (2);
   } /* endif */

   /* sort qconfig to make new default */
   if (system(qdef) != 0) {
 	(void) fprintf(stderr, catgets(smit_cat,MSG_SET,MSG_CDQ_SYS_ERR,SYSTEM_ERROR), strerror(errno)); 
	return (2);
   } /* endif */

   /* unlock qconfig */
   /* Not necessary, since close() will unlock the file anyway.
   lck.l_type = F_UNLCK;
   (void) fcntl(qconfig, F_SETLKW, &lck);
    */

   (void) close(qconfig);
   (void) free(qdef);
   (void) catclose(smit_cat);

   return (0);

} /*end main*/

