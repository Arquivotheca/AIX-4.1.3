/* @(#)03	1.2  src/bos/usr/bin/mh/zotnet/tws.h, cmdmh, bos411, 9428A410j 6/15/90 22:21:56 */
/* 
 * COMPONENT_NAME: CMDMH tws.h
 * 
 * FUNCTIONS: dtime, dtwszone 
 *
 * ORIGINS: 26  28  35 
 *
 */
/* tws.h */

struct tws {
    int     tw_sec;
    int     tw_min;
    int     tw_hour;

    int     tw_mday;
    int     tw_mon;
    int     tw_year;

    int     tw_wday;
    int     tw_yday;

    int     tw_zone;

    long    tw_clock;

    int     tw_flags;
#define	TW_NULL	0x0000
#define	TW_SDAY	0x0007		/* how day-of-week was determined */
#define	  TW_SNIL	0x0000	/*   not given */
#define	  TW_SEXP	0x0001	/*   explicitly given */
#define	  TW_SIMP	0x0002	/*   implicitly given */
#define	TW_DST	0x0010		/* daylight savings time */
#define	TW_ZONE	0x0020		/* use numeric timezones only */
};

void    twscopy ();
int	twsort ();
long	twclock ();
char   *dasctime (), *dtimezone (), *dctime (), *dtimenow ();
struct tws *dgmtime(), *dlocaltime (), *dparsetime (), *dtwstime ();

#ifndef	ATZ
#define	dtime(cl)	dasctime (dlocaltime (cl), TW_ZONE)
#else	ATZ
#define	dtime(cl)	dasctime (dlocaltime (cl), TW_NULL)
#endif	ATZ
#define	dtwszone(tw)	dtimezone (tw -> tw_zone, tw -> tw_flags)


extern char   *tw_dotw[], *tw_ldotw[], *tw_moty[];
