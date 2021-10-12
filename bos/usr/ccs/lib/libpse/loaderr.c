static char sccsid[] = "@(#)45	1.1  src/bos/usr/ccs/lib/libpse/loaderr.c, cmdpse, bos411, 9434B411a 8/19/94 16:57:24";
/*
 *   COMPONENT_NAME: cmdpse
 *
 *   FUNCTIONS: loaderr
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * loaderr - display cause of load failure
 *
 * should be called immediately after a failed load attempt
 */

#include <string.h>
#include <sys/types.h>
#include <sys/ldr.h>
#include "execerr_msg.h"

static struct {
	int	code;
	char	*text;
} msgs[] = {
	{ SYSTEM,  "System error - error data is: %s\n"			      },
	{ 0,       "no error\n"						      },
	{ TOOMANY, "Additional errors occurred but are not reported\n"	      },
	{ NOLIB1,  "Could not load library %s[%s]\n"			      },
	{ UNDEF,   "Symbol %s in %s is undefined\n"			      },
	{ RLDBAD,  "Relocation entry number %s is defective in %s\n"	      },
	{ FORMAT,  "%s is not executable or not in correct XCOFF format\n"    },
	{ EWAS,    "Error code %s\n"					      },
	{ MEMBER,  "Member %s not found or file not an archive\n"	      },
	{ TYPE,    "Symbol %s used in %s type does not match exported type\n" },
	{ ALIGN,   "Alignment of text does not match required alignment\n"    },
	{ 0 }
};

void
loaderr(fp)
	FILE *fp;
{
	char buf[4096];
	char **cpp = (char **)buf;
	int code;
	char *arg1, *arg2;
	nl_catd xcatd;

	if (!loadquery(L_GETMESSAGES, buf, sizeof buf)) {
		xcatd = catopen(MF_EXECERR, NL_CAT_LOCALE);
		while (*cpp) {
			code = atoi(strtok(*cpp++, " "));
			arg1 = strtok(0, " ");
			arg2 = strtok(0, " ");
			fprintf(fp, catgets(xcatd, MS_EXECERROR,
				msgs[code+1].code, msgs[code+1].text),
				arg1,arg2);
		}
		catclose(xcatd);
	}
}
