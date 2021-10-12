#ifndef lint
static char sccsid[] = "@(#)57 1.3 src/bos/kernext/tty/snls_db.c, sysxtty, bos411, 9428A410j 6/10/94 10:05:18";
#endif
/*
 * COMPONENT_NAME: sysxtty (nls_db extension for tty debugging)
 * 
 * FUNCTIONS: nls_print(),
 *	      nls_line_print(), nls_prstr()
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include "snls.h"

/* function definitions of nls_db extension for tty debugging */

#ifdef IN_TTYDBG
int nls_print();
#endif  /* IN_TTYDBG */

#if defined(_KERNEL) && defined(IN_TTYDBG)
#define	nls_db_printf	tty_db_printf
#else
#define	nls_db_printf	printf
#endif  /* _KERNEL && IN_TTYDBG */

int nls_line_print(); 
static void nls_prstr(); 



/*
 * NAME:     nls_prstr
 *
 * FUNCTION: Service function for nls_print. 
 *
 * RETURNS:  No return value
 *
 */

static void
nls_prstr(char *s, int num, int max)
{
	int c;

	if (num > max)
		num = max;

	while (--num >= 0) {
		if ((c = *s++) >= 0x80) {
			nls_db_printf("M");
			c &= ~0x80;
		}
		if (c < ' ' || c == 0x7f) {
			nls_db_printf("^");
			c ^= 0x40;
		}
		nls_db_printf("%c", c);
	}
}

/*
 * NAME:     nls_line_print
 *
 * FUNCTION: Print an nls line (== nls structure). 
 *
 * RETURNS:  0 on success, return code from tty_db_nomore() macro otherwise
 *
 */

int
nls_line_print(STRNLSP nlsp, struct ttmap *tm_mapin, struct ttmap *tm_mapout)
{
	register infop_t info;

	tty_db_nomore(nls_db_printf("nls_line:\n"));

	tty_db_nomore(nls_db_printf("   "));
	tty_db_nomore(nls_db_printf("devno=0x%08X oldpri=%d nlsmap=0x%08X\n",
		nlsp->devno, nlsp->oldpri, nlsp->nlsmap));

	tty_db_nomore(nls_db_printf("   "));
	tty_db_nomore(nls_db_printf("wbid=%d rbid=%d wtid=%d rtid=%d\n",
		nlsp->wbid , nlsp->rbid , nlsp->wtid , nlsp->rtid ));

	/* info about input map */
	info = &nlsp->nls_mapin;
	if (info->tm_map) {
		tty_db_nomore(nls_db_printf("   "));
		tty_db_nomore(nls_db_printf("input map:\n"));
		tty_db_nomore(nls_db_printf("      "));
		tty_db_nomore(nls_db_printf("map=0x%08X state=%d bufindx=%d rulindx=%d\n",
			info->tm_map, info->tm_state, info->tm_bufindx,
			info->tm_rulindx));
		tty_db_nomore(nls_db_printf("      "));
		tty_db_nomore(nls_db_printf("buffer=``"));
			nls_prstr(info->tm_buffer, TTMAP_BUFMAX, TTMAP_BUFMAX);
		tty_db_nomore(nls_db_printf("''\n"));

		tty_db_nomore(nls_db_printf("      "));
		tty_db_nomore(nls_db_printf("tm_len=%d tm_count=%d tm_flags=0x%02X tm_num_rules=%d\n",
			tm_mapin->tm_len, tm_mapin->tm_count,
			tm_mapin->tm_flags, tm_mapin->tm_num_rules));
		tty_db_nomore(nls_db_printf("      "));
		tty_db_nomore(nls_db_printf("tm_default=%d tm_first_wild=%d tm_mapname=%s\n",
			tm_mapin->tm_default, tm_mapin->tm_first_wild,
			&tm_mapin->tm_mapname));
		tty_db_nomore(nls_db_printf("      "));
		tty_db_nomore(nls_db_printf("tm_next=0x%08X tm_hash=0x%08X tm_rule=0x%08X\n",
			tm_mapin->tm_next, tm_mapin->tm_hash,
			tm_mapin->tm_rule));
	} else {
		tty_db_nomore(nls_db_printf("   "));
		tty_db_nomore(nls_db_printf("null input map\n"));
	}
	if (info->tm_trouble) {
		tty_db_nomore(nls_db_printf("   "));
		tty_db_nomore(nls_db_printf("trouble=%d\n", info->tm_trouble));
	}

	/* info about output map */
	info = &nlsp->nls_mapout;
	if (info->tm_map) {
		tty_db_nomore(nls_db_printf("   "));
		tty_db_nomore(nls_db_printf("output map:\n"));
		tty_db_nomore(nls_db_printf("      "));
		tty_db_nomore(nls_db_printf("map=0x%08X state=%d bufindx=%d rulindx=%d\n",
			info->tm_map, info->tm_state, info->tm_bufindx,
			info->tm_rulindx));
		tty_db_nomore(nls_db_printf("      "));
		tty_db_nomore(nls_db_printf("buffer=``"));
		nls_prstr(info->tm_buffer, TTMAP_BUFMAX, TTMAP_BUFMAX);
		tty_db_nomore(nls_db_printf("''\n"));

		tty_db_nomore(nls_db_printf("      "));
		tty_db_nomore(nls_db_printf("tm_len=%d tm_count=%d tm_flags=0x%02X tm_num_rules=%d\n",
			tm_mapout->tm_len, tm_mapout->tm_count,
			tm_mapout->tm_flags, tm_mapout->tm_num_rules));

		tty_db_nomore(nls_db_printf("      "));
		tty_db_nomore(nls_db_printf("tm_default=%d tm_first_wild=%d tm_mapname=%s\n",
			tm_mapout->tm_default, tm_mapout->tm_first_wild,
			&tm_mapout->tm_mapname));
		tty_db_nomore(nls_db_printf("      "));
		tty_db_nomore(nls_db_printf("tm_next=0x%08X tm_hash=0x%08X tm_rule=0x%08X\n",
			tm_mapout->tm_next, tm_mapout->tm_hash,
			tm_mapout->tm_rule));
	} else {
		tty_db_nomore(nls_db_printf("   "));
		tty_db_nomore(nls_db_printf("null output map\n"));
	}
	if (info->tm_trouble) {
		tty_db_nomore(nls_db_printf("   "));
		tty_db_nomore(nls_db_printf("trouble=%d\n", info->tm_trouble));
	}
	return(0);
}

#ifdef IN_TTYDBG
/*
 * NAME:     nls_print
 *
 * FUNCTION: For debug purpose = print out informations about input and output
 *	     maps in use. 
 *
 * RETURNS:  0 on success, -1 or return code from nls_line_print() on failure
 *
 */

int
nls_print(STRNLSP nlsp, int argv)
{
	STRNLS nls;
	struct ttmap ttmapin, ttmapout;
	struct ttmap *tm_map;
	int res;

	if (tty_read_mem(nlsp, &nls, sizeof(struct nls)) == 0) {
		tm_map = nls.nls_mapin.tm_map;
		if (tm_map != 0) {
			if (tty_read_mem(tm_map, &ttmapin,
						sizeof(struct ttmap)) != 0) {
				return(-1);
			}
		}
		tm_map = nls.nls_mapout.tm_map;
		if (tm_map != 0) {
			if (tty_read_mem(tm_map, &ttmapout,
						sizeof(struct ttmap)) != 0) {
				return(-1);
			}
		}
		if (res = nls_line_print(&nls, &ttmapin, &ttmapout))
			return(res);
	} else {
		return(-1);
	}

	return (0);
}
#endif  /* IN_TTYDBG */

