static char sccsid[] = "@(#)62        1.2  src/bos/kernext/pse/str_tty_db.c, sysxpse, bos411, 9428A410j 6/10/94 10:17:19";
/*
 *  
 * COMPONENT_NAME: SYSXPSE - STREAMS framework
 *  
 * FUNCTIONS: sh_ttydb_print
 *  
 * ORIGINS: 83
 *  
 */

/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <pse/str_stream.h>
#include <sys/str_tty.h>

#ifdef TTYDBG


#define ttydb_printf    printf

/* function definitions */
int sh_ttydb_print();

int
sh_ttydb_print (s)
	struct	shtty_s	*s;
{
	struct	shtty_s	store;
	struct	shtty_s	*shtty_p = &store;

	if (!s) return;

	tty_read_mem(s, &store, sizeof(struct shtty_s));
	tty_db_nomore(ttydb_printf("shtty_p (%8X):\n", s));
	tty_db_nomore(ttydb_printf("pgrp : %8X sid : %8X\n",
			shtty_p->shtty_pgrp, shtty_p->shtty_sid));
	tty_db_nomore(ttydb_printf("tsm : %8X\n", shtty_p->shtty_tsm));
	tty_db_nomore(ttydb_printf("flags "));
	if (shtty_p->shtty_flags) {
		tty_db_nomore(ttydb_printf("%8X: ", shtty_p->shtty_flags));
		if (shtty_p->shtty_flags & F_SHTTY_TRUST)
			tty_db_nomore(ttydb_printf("TERMINAL TRUSTED "));
		if (shtty_p->shtty_flags & F_SHTTY_SAK)
			tty_db_nomore(ttydb_printf("SAK ENABLED "));
		if (shtty_p->shtty_flags & F_SHTTY_KEP)
			tty_db_nomore(ttydb_printf("KEP ENABLED "));
		if (shtty_p->shtty_flags & F_SHTTY_CONS)
			tty_db_nomore(ttydb_printf("CONSOLE REDIRECTED "));
		tty_db_nomore(ttydb_printf("\n"));
	} else
		tty_db_nomore(ttydb_printf("NONE\n"));
	return(0);
}

#endif /* TTYDBG */
