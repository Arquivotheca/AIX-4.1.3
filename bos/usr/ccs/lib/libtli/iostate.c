static char sccsid[] = "@(#)96  1.1.1.4  src/bos/usr/ccs/lib/libtli/iostate.c, libtli, bos411, 9428A410j 7/12/94 12:19:47";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: iostate_lookup, _txstate
 *
 *   ORIGINS: 18 27 63
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */

/*
 * OSF/1 Release 1.2
 */
/** Copyright (c) 1990  Mentat Inc.
 ** iostate.c 1.1, last change 6/4/90
 **/


#include "common.h"
#include <sys/tihdr.h>
#include <sys/timod.h>

#define	FD_HASH_LIST_LEN	32

static	struct tli_st	*iostate[FD_HASH_LIST_LEN];

#ifdef _THREAD_SAFE
extern lib_data_functions_t     __t_data_funcs;
extern lib_mutex_t              __t_mutex;
#endif /* _THREAD_SAFE */

#define xx (-1)
char tli_state[TLI_NUM_EVENTS][TLI_NUM_STATES] = {
/*                 0     1     2     3     4      5        6     7      8     */
/* STATES-------UNINIT UNBND IDLE OUTCON INCON DATAXFER OUTREL INREL STATECHG-*/
/* EVENTS   */
/* 0 OPEN   */  {  1,   xx,   xx,   xx,   xx,    xx,      xx,   xx,      8    },
/* 1 BIND   */  { xx,    2,   xx,   xx,   xx,    xx,      xx,   xx,      8    },
/* 2 OPTMGMT*/  { xx,    1,    2,    3,    4,     5,       6,    7,      8    },
/* 3 UNBIND */  { xx,   xx,    1,   xx,   xx,    xx,      xx,   xx,      8    },
/* 4 CLOSE  */  { xx,    0,    0,    0,    0,     0,       0,    0,      8    },
/* 5 CONNECT1*/ { xx,   xx,    5,   xx,   xx,    xx,      xx,   xx,      8    },
/* 6 CONNECT2*/ { xx,   xx,    3,   xx,   xx,    xx,      xx,   xx,      8    },
/* 7 RCVCONN*/  { xx,   xx,   xx,    5,   xx,    xx,      xx,   xx,      8    },
/* 8 LISTEN */  { xx,   xx,    4,   xx,    4,    xx,      xx,   xx,      8    },
/* 9 ACCEPT1*/  { xx,   xx,   xx,   xx,    5,    xx,      xx,   xx,      8    },
/*10 ACCEPT2*/  { xx,   xx,   xx,   xx,    2,    xx,      xx,   xx,      8    },
/*11 ACCEPT3*/  { xx,   xx,   xx,   xx,    4,    xx,      xx,   xx,      8    },
/*12 SND    */  { xx ,  xx,   xx,   xx,   xx,     5,      xx,    7,      8    },
/*13 RCV    */  { xx,   xx,   xx,   xx,   xx,     5,       6,   xx,      8    },
/*14 SNDDIS1*/  { xx,   xx,   xx,    2,    2,     2,       2,    2,      8    },
/*15 SNDDIS2*/  { xx,   xx,   xx,   xx,    4,    xx,      xx,   xx,      8    },
/*16 RCVDIS1*/  { xx,   xx,   xx,    2,   xx,     2,       2,    2,      8    },
/*17 RCVDIS2*/  { xx,   xx,   xx,   xx,    2,    xx,      xx,   xx,      8    },
/*18 RCVDIS2*/  { xx,   xx,   xx,   xx,    4,    xx,      xx,   xx,      8    },
/*19 SNDREL */  { xx,   xx,   xx,   xx,   xx,     6,      xx,    2,      8    },
/*20 RCVREL */  { xx,   xx,   xx,   xx,   xx,     7,       2,   xx,      8    },
/*21 PASSCON*/  { xx,    5,    5,   xx,   xx,    xx,      xx,   xx,      8    },
/*22 SNDUDATA*/ { xx,   xx,    2,   xx,   xx,    xx,      xx,   xx,      8    },
/*23 RCVUDATA*/ { xx,   xx,    2,   xx,   xx,    xx,      xx,   xx,      8    },
/*24 RCVUDATA*/ { xx,   xx,    2,   xx,   xx,    xx,      xx,   xx,      8    }
		};
#undef xx

struct tli_st *
iostate_lookup (fd, cmd)
	int	fd;
	int	cmd;
{
	static	boolean	initialized = false;
	struct	tli_st		**tlipp;
	struct	tli_st		*tli, *tlip;
	struct	T_info_ack 	tinfo;

	IOSTATE_LOCK();
	tli = nilp(struct tli_st);
	if (fd < 0) {
		t_errno = TBADF;
		IOSTATE_UNLOCK();
		return tli;
	}
	if ( !initialized ) {
		for ( tlipp = iostate; tlipp < A_END(iostate); tlipp++ )
			*tlipp = nilp(struct tli_st);
		initialized = true;
	}
	/* Try to find fd in our current hash list. */
	for (tlipp = &iostate[fd % FD_HASH_LIST_LEN]; tlip = *tlipp; 
		tlipp = &tlip->tlis_next) {
		if (tlip->tlis_fd == fd) {
			tli = tlip;
			break;
		}
	}

	switch(cmd) {
	case IOSTATE_VERIFY:
		if (!tli)
			t_errno = TBADF;
		else
			TLI_LOCK(tli);
		IOSTATE_UNLOCK();
		break;

	case IOSTATE_SYNC:
		tinfo.PRIM_type = T_INFO_REQ;
		if (tli_ioctl(fd, TI_GETINFO, (char *)&tinfo, sizeof(tinfo)) == -1) {
			IOSTATE_UNLOCK();
			errno = EBADF;
			t_errno = TBADF;
			tli = 0;
			break;
		}
		if (!tli) {
			tli = (struct tli_st *)malloc(sizeof(struct tli_st) +
				_DEFAULT_STRCTLSZ);
			if (!tli) {
				IOSTATE_UNLOCK();
				t_errno = TSYSERR;
				errno = ENOMEM;
				break;
			}
			if (!TLI_LOCKCREATE(tli)) {
				free(tli);
				tli = 0;
				IOSTATE_UNLOCK();
				t_errno = TSYSERR;
				errno = ENOMEM;
				break;
			}
			tli->tlis_next = nilp(struct tli_st);
			tli->tlis_fd = fd;
			*tlipp = tli;
			tli->tlis_flags = 0;
			tli->tlis_sequence = 0;
			tli->tlis_proto_buf = (char *)&tli[1];
		}
		TLI_LOCK(tli);
		IOSTATE_UNLOCK();
		tli->tlis_servtype = tinfo.SERV_type;
		tli->tlis_etsdu_size = tinfo.ETSDU_size;
		tli->tlis_tsdu_size = tinfo.TSDU_size;
		tli->tlis_tidu_size = tinfo.TIDU_size;
		tli->tlis_state = _txstate(tinfo.CURRENT_state);
		break;

	case IOSTATE_FREE:
		if (!tli) {
			IOSTATE_UNLOCK();
			errno = EBADF;
			t_errno = TBADF;
			break;
		}
		TLI_LOCK(*tlipp);
		TLI_LOCK(tli);
		*tlipp = tli->tlis_next;
		if (*tlipp)
			TLI_UNLOCK(*tlipp);
		TLI_UNLOCK(tli);
		TLI_LOCKDELETE(tli);
		IOSTATE_UNLOCK();
		free(tli);
		break;
	default:
		IOSTATE_UNLOCK();
		break;
	}
	return tli;
}

int
_txstate(int st)
{
	switch (st) {
	case TS_UNBND:
		return T_UNBND;
	case TS_IDLE:
		return T_IDLE;
	case TS_WCON_CREQ:
		return T_OUTCON;
	case TS_WRES_CIND:
		return T_INCON;
	case TS_DATA_XFER:
		return T_DATAXFER;
	case TS_WIND_ORDREL:
		return T_OUTREL;
	case TS_WREQ_ORDREL:
		return T_INREL;
	case TS_WACK_BREQ:
	case TS_WACK_CREQ:
	case TS_WACK_CRES:
	case TS_WACK_DREQ6:
	case TS_WACK_DREQ7:
	case TS_WACK_DREQ9:
	case TS_WACK_DREQ10:
	case TS_WACK_DREQ11:
	case TS_WACK_OPTREQ:
	case TS_WACK_ORDREL:
	case TS_WACK_UREQ:
		t_errno = TSTATECHNG;
		break;
	default:
		break;
	}
	return -1;
}

