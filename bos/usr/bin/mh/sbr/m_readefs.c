static char sccsid[] = "@(#)78	1.5  src/bos/usr/bin/mh/sbr/m_readefs.c, cmdmh, bos411, 9428A410j 3/27/91 17:48:33";
/* 
 * COMPONENT_NAME: CMDMH m_readefs.c
 * 
 * FUNCTIONS: MSGSTR, m_readefs 
 *
 * ORIGINS: 10  26  27  28  35 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* static char sccsid[] = "m_readefs.c	7.1 87/10/13 17:10:14"; */

/* m_readefs.c - read a profile/context file */

#include "mh.h"
#include <stdio.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 


static struct procs {
	char    *procname;
	char    **procnaddr;
} procs [] = {
	{ "context",	&context	},
	{ "mh-sequences",
			&mh_seq		},
	{ "fileproc",   &fileproc       },
	{ "incproc",	&incproc	},
    	{ "installproc",&installproc	},
	{ "lproc",      &lproc          },
	{ "mailproc",	&mailproc	},
	{ "mhlproc",	&mhlproc	},
	{ "moreproc",	&moreproc	},
	{ "mshproc",	&mshproc	},
	{ "packproc",	&packproc	},
	{ "postproc",   &postproc       },
	{ "rmfproc",	&rmfproc	},
	{ "rmmproc",	&rmmproc	},
	{ "sendproc",   &sendproc       },
	{ "showproc",   &showproc       },
	{ "slocalproc", &slocalproc     },
	{ "vmhproc",    &vmhproc        },
	{ "whatnowproc",
			&whatnowproc	},
	{ "whomproc",	&whomproc	},
	{ NULL,         NULL            }
};

static struct node **opp = NULL;


void	m_readefs (npp, ib, file, ctx)
register struct node **npp;
register FILE   *ib;
register char   *file;
register int     ctx;
{
    register int    state;
    register char  *cp;
    char    name[NAMESZ],
            field[BUFSIZ];
    register struct node   *np;
    register struct procs  *ps;

    if (npp == NULL && (npp = opp) == NULL) {
	admonish (NULLCP, MSGSTR(PUMP, "bug: m_readefs called but pump not primed")); /*MSG*/
	return;
    }

    for (state = FLD;;) {
	switch (state = m_getfld (state, name, field, sizeof field, ib)) {
	    case FLD:
	    case FLDPLUS:
	    case FLDEOF:
		np = (struct node *) malloc (sizeof *np);
		if (np == NULL)
		    adios (NULLCP, MSGSTR(NOAPSTOR, "unable to allocate profile storage")); /*MSG*/
		*npp = np;
		*(npp = &np -> n_next) = NULL;
		np -> n_name = getcpy (name);
		if (state == FLDPLUS) {
		    cp = getcpy (field);
		    while (state == FLDPLUS) {
			state = m_getfld
				    (state, name, field, sizeof field, ib);
			cp = add (field, cp);
		    }
		    np -> n_field = trimcpy (cp);
		    free (cp);
		}
		else
		    np -> n_field = trimcpy (field);
		np -> n_context = ctx;
		for (ps = procs; ps -> procname; ps++)
		    if (strcmp (np -> n_name, ps -> procname) == 0) {
			*ps -> procnaddr = np -> n_field;
			break;
		    }
		if (state == FLDEOF)
		    break;
		continue;

	    case BODY:
	    case BODYEOF:
		adios (NULLCP, MSGSTR(NOBLINES, "no blank lines are permitted in %s"), file); /*MSG*/

	    case FILEEOF:
		break;

	    default:
		adios (NULLCP, MSGSTR(POORFOR, "%s is poorly formatted"), file); /*MSG*/
	}
	break;
    }

    opp = npp;
}
