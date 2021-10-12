static char sccsid[] = "@(#)73	1.3.1.1  src/bos/usr/lpp/jls/dictutil/kjdicadd.c, cmdKJI, bos411, 9428A410j 7/23/92 00:53:28";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * COPYRIGHT:           5756-030 COPYRIGHT IBM CORP 1991
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              User Dictionary Maintenance for AIX 3.2
 *
 * CLASSIFICATION:    OCO Source Material - IBM Confidential.
 *                    (IBM Confidential-Restricted when aggregated)
 *
 * FUNCTION:
 *
 * NOTES:               NA.
 *
 */

/*----------------------------------------------------------------------*
 * Include Standard.
 *----------------------------------------------------------------------*/
#if defined(PARMADD)
#include <stdio.h>      /* Standard I/O Package.                        */
#include <stdlib.h>
#include <string.h>     /* Stirng Package                               */
/*#include <memory.h>*/ /* Memory Package                               */
#include <fcntl.h>      /* File Control Package                         */
#include <sys/stat.h>
#include <sys/lockf.h>

/*----------------------------------------------------------------------*
 * Include Kanji Project.
 *----------------------------------------------------------------------*/
#include "kje.h"
#include "kut.h"        /* Utility Define File                          */

#define U_MSGFLE ("/usr/lpp/jls/etc/kjdict.jmsg")
#define LNGMAX   ( 44 )

ushort user_flg;        /* User Dictionary Error Flag                   */

#define PR(XX)  XX;fflush(stderr);

int kjdicadd( argc, argv, usrdic, parm )
int     argc;           /* Parameter Counter                        (i) */
char    *argv[];        /* Parameter String                         (i) */
char    *usrdic;        /* User Dictionary                          (i) */
char    *parm;          /* Temporary Store Name Parameter           (i) */
{
    struct  stat    stbuf;      /* File Status Buffer                   */
    UDCB    udcptr;             /* User Dictionary File Control Block   */
    SDCB    sdcptr[MAX_SYSDICT];/* System Dictionary File Control Block */
    char    *sysname;           /* System Dictionary File Name          */
    char    *usrname;           /* User Dictionary File Name            */
    char    *ch_y;              /* Yomi Character Pointer               */
    char    *ch_g;              /* Goku Character Pointer               */
    uchar   tmpname[257];       /* Temporary File Name                  */
    char    wrtsts = 0xf1;      /* Write Status Code Buffer             */
    char    cmpsts = 0x00;      /* Complate Status Code Buffer          */
    uint    snelem;             /* System Dictionary File Size Buffer   */
    uint    mnelem;             /* Message Size Buffer                  */
    int     sfldes;             /* System Dictionary File Descripter    */
    int     ufldes;             /* User Dictionary File Descripter      */
#if !defined(MSGFILE)
    int     mfldes;             /* Message File Descripter              */
#endif
    int     tmpdes;             /* Temporary File Descripter            */
    int     tmpsts;             /* Fatal Error Temporary Flag           */
    int     ev;                 /* Extend Curses Event and Break Switch */
    int     i;                  /* Loop Counter                         */
    int     j;                  /* Loop Counter                         */
    int     fatal_f = IUSUCC;   /* Fatal Error Flag                     */
    int     ku_ret;             /* Utility Return Value                 */
    int     rc = U_FOF;         /* Return Value (Updatede Flag)         */
    struct flock flck;          /* flock structure for fcntl()          */
    short   msgid;
    int	    errflg=0;

    /*------------------------------------------------------------------*
     * Initialize
     *------------------------------------------------------------------*/
    udcptr.updflg = U_FOF;

    do {

    	/*--------------------------------------------------------------*
    	 * Initialized Error Flag ON
    	 *--------------------------------------------------------------*/
    	user_flg = U_FON;

    	/*--------------------------------------------------------------*
    	 * User Dictionary File Open
    	 *--------------------------------------------------------------*/
	fatal_f = kuusrrd( usrdic, &udcptr, &msgid );
	if ( fatal_f == IUFAIL || fatal_f == IURECOV ) {
	    if ( msgid == U_AGMSGN )
	    	(void)printf("%s\n", CU_MSGUAE);
	    else if ( msgid == U_FMSGN )
	    	(void)printf("%s\n", CU_MSGSER);
	    else if ( msgid == U_DMSGN )
	    	(void)printf("%s\n", CU_MSGURC);
	    errflg++;
	    break;
	}
	else if ( fatal_f == IUUPDAT ) {
	    (void)printf("%s\n", CU_MSGUUP);
	    (void)printf("%s\n", CU_MSGESE);
	    initscr();                  /* Curses Initialize Screen     */
	    noecho();                   /* Set No Echo Mode             */
	    raw();                      /* Set Raw Mode                 */
	    keypad(TRUE);               /* Definetion Keypad            */
	    for ( ;; ) {                /* Event Get Loop               */
	    	ev = kugetc();          /* Event Get                    */
	    	if ( ev == KEY_NEWL ) { /* Check Enter Key      	*/
		    ev = 0;             /* Set Enter Key Code           */
		    break;              /* Event Loop Break             */
	    	} else if ( ev == KEY_F(12) ) { /* Check PF12 Key       */
		    ev = 1;             /* Set PF12 Key Code            */
		    break;              /* Event Loop Break             */
	    	} else {                /* Default Code Type            */
		    continue;           /* Next Event                   */
	    	}
	    }
	    echo();                     /* Reset Echo Mode              */
	    noraw();                    /* Reset No Raw Mode            */
	    endwin();                   /* Curses End Screen            */
	    if ( ev == 1 ) {            /* Check PF12 Key               */
	    	break;                  /* Subroutine Break             */
	    }
    	}
	ufldes = udcptr.orgfd;

    	/*--------------------------------------------------------------*
    	 * Open Backup File
    	 *--------------------------------------------------------------*/
   	fatal_f = kuopbkf( usrdic, &udcptr, &msgid );
   	if ( fatal_f == IUFAIL ) {
	    if ( msgid == U_AGMSGN )
	    	(void)printf("%s\n", CU_MSGUAE);
	    else if ( msgid == U_FMSGN )
	    	(void)printf("%s\n", CU_MSGSER);
	    errflg++;
	    break;
	}

    	/*---------------------------------------------------------------*
    	 * User Dictionary File Name Convert
    	 *---------------------------------------------------------------*/
    	switch( *parm ) {
      	case '1' :
	    usrname = "./usrdict";
	    break;

      	case '2' :
	    usrname = "$HOME/.usrdict";
	    break;

      	default  :
	    usrname = usrdic;
	    break;
    	};

    	/*--------------------------------------------------------------*
    	 * System Dictionary File Open
    	 *--------------------------------------------------------------*/
    	tmpsts = fatal_f;  	/* Store Status to Temporary Bu		*/
	fatal_f = kusysrd( sdcptr, &msgid );
	if ( fatal_f == IUFAIL ) {
	    if ( msgid == U_FMSGN ) {
	    	(void)printf("%s\n", CU_MSGSER);
	    	close(ufldes);  /* Close User Dictionary File   	*/
	    }
	    else if ( msgid == U_AHMSGN ) {
	    	(void)printf("%s\n", CU_MSGSAE);
	    	close(ufldes);  /* Close User Dictionary File   	*/
	    }
	    errflg++;
	    break;
	}
	fatal_f = tmpsts;       /* Restore Normal Code          	*/

#if !defined(MSGFILE)
    	/*--------------------------------------------------------------*
    	 * Message File Open
    	 *--------------------------------------------------------------*/
        if ((mfldes = open(U_MSGFLE,(O_RDONLY|O_NDELAY))) == U_FILEE) {
	    fatal_f = INOMSGFL;         /* Set Fatal Error              */
    	} else {
	    fstat(mfldes,&stbuf);       /* Get Message File Status      */
	    mnelem = stbuf.st_size;     /* Set Message File Size        */
	    udcptr.erptr = (UECB *)calloc(mnelem,sizeof(uchar));
	    if ( udcptr.erptr == NULL ) {
	    	fatal_f = INOMSGFL;     /* Set Fatal Error              */
	    } else if ((read(mfldes,(uchar *)udcptr.erptr,mnelem)) == U_FILEE) {
	    	fatal_f = INOMSGFL;     /* Set Fatal Error              */
	    };
	    close(mfldes);              /* Close Message File           */
    	};
#endif /* !defined(MSGFILE) */

	if ( 1 ) {
	    int     len;
    	    for ( i = 1, ch_y = (char *)NULL, ch_g = (char *)NULL; 
			i < argc; i++ ) {
		len = strlen(argv[i]);
		if ( strncmp(argv[i],U_PRMYMI,2) == 0 ) {
	    	    if ( len > 2 ) {
			ch_y = (argv[i]+2);
	    	    } else {
			ch_y = argv[++i];
	    	    }
		} else if ( strncmp(argv[i],U_PRMGKU,2) == 0 ) {
	    	    if ( len > 2 ) {
			ch_g = (argv[i]+2);
	    	    } else {
			ch_g = argv[++i];
	    	    }
		} else {
	    	    continue;
		}
		if ( ch_y == (char *)NULL ||
	     		ch_g == (char *)NULL ) {
			continue;
		}

		/*----- registaration ---------------------------------*/
		ku_ret = kjdicpad ( &udcptr, &sdcptr, ch_y, ch_g );
		if ( ku_ret != 0 )
			errflg++;

		ch_y = (char *)NULL;
		ch_g = (char *)NULL;
    	    }
	}

    	user_flg = U_FOF;

    	if ( (fatal_f != IUSUCC  && fatal_f != IUFAIL
		&& fatal_f != INOMSGFL) ) {
	    close(ufldes);
	    errflg++;
	    break;
    	}

    	/*--------------------------------------------------------------*
    	 * Write backup User Dictionary
    	 *--------------------------------------------------------------*/
    	if ( udcptr.updflg == U_FON ) {
	    ku_ret = kutmwrt( udcptr.tmpfd, udcptr.thdbuf, udcptr.ufilsz );
	    if ( ku_ret != IUSUCC ) {
	    	rc = U_FOF;
	    	(void)printf("%s\n", CU_MSGUAE);
		errflg++;
	    	break;
	    }
	    rc = U_FON;
	    udcptr.updflg = U_FOF;
    	}
	else {
	    if ( stat( udcptr.tmpname, &stbuf ) == 0 ) {
		if ( stbuf.st_size == 0 )
		    unlink( udcptr.tmpname );
	    }
	}

    	if ( (lseek(ufldes, (long)U_STATUS, 0)) == U_FILEE ) {
	    perror("File Seek Error ");
	    (void)printf("%s\n", CU_MSGUAE);
	    close(ufldes);        /* Close User Dictionary File   	*/
	    errflg++;
	    break;
    	}
    	if ( (write(ufldes,&cmpsts,1)) == U_FILEE ) {
	    perror("File Write Error ");
	    (void)printf("%s\n", CU_MSGUAE);
	    close(ufldes);        /* Close User Dictionary File   	*/
	    errflg++;
	    break;
    	}
    	close( ufldes );          /* Close User Dictionary File   	*/
    	close( udcptr.tmpfd );    /* Close User Dictionary File   	*/

    } while ( 0 );

    /*------------------------------------------------------------------*
     * Memory Area Free
     *------------------------------------------------------------------*/
    if ( udcptr.dcptr != NULL ) {
	free( udcptr.dcptr );
    }
    if ( udcptr.secbuf != NULL ) {
	free( udcptr.secbuf );
    }
    if ( udcptr.thdbuf != NULL ) {
	free( udcptr.thdbuf );
    }
    for ( i=0; i<MAX_SYSDICT; i++ ) {
    	if ( sdcptr[i].dcptr != NULL )
	    free( sdcptr[i].dcptr );
    }
    if ( udcptr.erptr != NULL ) {
	free( udcptr.erptr );
    }
    if ( udcptr.tmpname != NULL ) {
	free( udcptr.tmpname );
    }

    if ( errflg )
	rc = -1;
    return( rc );

}
#endif
