/* @(#)61  	1.5.1.3  src/bos/kernext/disp/ped/inc/midddfgets.h, peddd, bos411, 9428A410j 12/2/93 09:49:21 */
#ifndef _H_MIDDDFGETS
#define _H_MIDDDFGETS

/*
 * COMPONENT_NAME: PEDDD
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/*---------------------------------------------------------------------------
  Interrupt Status Register bit settings and masks for DDF
  ---------------------------------------------------------------------------*/

#define GETCOLOR_MASK			0x1000
#define GETCPOS_MASK			0x2000
#define SWAPBUFFERS_MASK		0x3000
#define GETCONDITION_MASK		0x4000
#define GETMODMATRIX_MASK		0x5000
#define GETPROJMATRIX_MASK		0x6000
#define GETTEXTFONTINDEX_MASK		0x7000
#define ENDRENDER_MASK			0x8000


#define INITIAL_CORRELATOR_VALUE   1    /* init correlator value is arbitrary */


/*---------------------------------------------------------------------------
  End Rendering SE flag setting
  ---------------------------------------------------------------------------*/

#define MID_INTERRUPT_REQUESTED		0x00008000


/*---------------------------------------------------------------------------
  Flag used to communicate between DDF functions and their interrupt handlers
  ---------------------------------------------------------------------------*/

#define	MID_DO_NOT_SLEEP		0x10


/*--------------------------------------------------------------------------
  Watchdog timer constants and structures for DDF.

  NOTE:  60 seconds is an unreasonably large value for watchdog period.
         However, this was set here for Defect 37452 since we were seeing
         regular watchdog pops at 2 seconds.  When performance improves in
         the microcode, this value should be changed back (to 2-5 seconds).
  --------------------------------------------------------------------------*/

#define	GETCPOS_WATCHDOG_PERIOD			30	/* seconds	*/
#define	GETCOLOR_WATCHDOG_PERIOD		30	/* seconds	*/
#define	GETCONDITION_WATCHDOG_PERIOD		30	/* seconds 	*/
#define	GETTEXTFONTINDEX_WATCHDOG_PERIOD	30	/* seconds	*/
#define	ENDRENDER_WATCHDOG_PERIOD		30	/* seconds 	*/

typedef	struct _mid_watch_getcpos {
	struct watchdog 	dog;		/* watchdog timer structure   */
	caddr_t 		*sleep_addr; 	/* wakeup addr for w-dog      */
} mid_watch_getcpos_t;

typedef	struct _mid_watch_getcolor {
	struct watchdog 	dog;		/* watchdog timer structure   */
	caddr_t 		*sleep_addr; 	/* wakeup addr for w-dog      */
} mid_watch_getcolor_t;

typedef	struct _mid_watch_getcondition {
	struct watchdog 	dog;		/* watchdog timer structure  */
	caddr_t 		*sleep_addr; 	/* wakeup addr for w-dog     */
} mid_watch_getcondition_t;

typedef	struct _mid_watch_gettextfontindex {
	struct watchdog 	dog;		/* watchdog timer structure  */
	caddr_t 		*sleep_addr; 	/* wakeup addr for w-dog     */
} mid_watch_gettextfontindex_t;

/*---------------------------------------------------------------------------
  The following structure is a generic watchdog data structure that should
  eventually replace the DDF specific watchdog data structures defined above.
  Currently, the only DDF function that uses this structure is end render.
  ---------------------------------------------------------------------------*/

typedef	struct _mid_watchdog_data {
	struct watchdog 	dog;		/* watchdog timer structure  */
	caddr_t 		*sleep_addr; 	/* wakeup addr for w-dog     */
} mid_watchdog_data_t;


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*  These structures (the gets through the swapbuffer structure) are used  */
/*  to communicate correlator information and function data between the    */
/*  DDF function and its associated interrupt handler.                     */
/*                                                                         */
/*  These typedefs are used within the midddf structure below to reserve   */
/*  actual space for the associated data.                                  */
/*									   */
/*  int_rcvd and sleep_flg have been added to getcpos, getcolor, 	   */
/*  getcondition and gettextfontindex.  These were added to allow the 	   */
/*  a process to enable interrupts before writing to FIFO and yet not miss */
/*  the interrupt, if it got back before the process went to sleep         */
/*									   */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef struct _ddf_data_getcpos {
        mid_getcpos_t                   ddf_getcpos;
        ushort                          correlator;
        ulong                           wakeup_address; /* wake addr-e_sleep()*/
        struct _ddf_data_getcpos        *next;
        void                            *user_data;
	mid_watch_getcpos_t		watchdog_data;
	ushort				int_rcvd;
	ushort				sleep_flg;
} ddf_data_getcpos_t;


typedef struct _ddf_data_getcolor {
        mid_getcolor_t                  ddf_getcolor;
        ushort                          correlator;
        ulong                           wakeup_address; /* wake addr-e_sleep()*/
        struct _ddf_data_getcolor       *next;
        void                            *user_data;
	mid_watch_getcolor_t		watchdog_data;
	ushort				int_rcvd;
	ushort				sleep_flg;
} ddf_data_getcolor_t;


typedef struct _ddf_data_get_modelling_matrix {
        mid_get_modelling_matrix_t              ddf_get_modelling_matrix;
        ushort                                  correlator;
        ulong                                   wakeup_address;
        struct _ddf_data_get_modelling_matrix   *next;
        void                                    *user_data;
} ddf_data_get_modelling_matrix_t;


typedef struct _ddf_data_get_projection_matrix {
        mid_get_projection_matrix_t             ddf_getprojection_matrix;
        ushort                                  correlator;
        ulong                                   wakeup_address;
        struct _ddf_data_get_projection_matrix  *next;
        void                                    *user_data;
} ddf_data_get_projection_matrix_t;


typedef struct _ddf_data_getcondition {
        mid_getcondition_t              ddf_getcondition;
        ushort                          correlator;
        ulong                           wakeup_address; /* wake addr-e_sleep()*/
        struct _ddf_data_getcondition   *next;
        void                            *user_data;
	mid_watch_getcondition_t	watchdog_data;
	ushort				int_rcvd;
	ushort				sleep_flg;
} ddf_data_getcondition_t;


typedef struct _ddf_data_gettextfontindex {
        mid_gettextfontindex_t          ddf_gettextfontindex;
        ushort                          correlator;
        ulong                           wakeup_address; /* wake addr-e_sleep()*/
        struct _ddf_data_gettextfontindex   *next;
        void                            *user_data;
	mid_watch_gettextfontindex_t	watchdog_data;
	ushort				int_rcvd;
	ushort				sleep_flg;
} ddf_data_gettextfontindex_t;


/*---------------------------------------------------------------------------
  The following structure is a generic DDF data structure that should
  eventually replace the DDF specific data structures defined above.
  Currently, the only DDF function that uses this structure is end render.
  ---------------------------------------------------------------------------*/

typedef struct _ddf_data {
        ushort                  correlator;
        ulong                   wakeup_address; /* wake addr-e_sleep()*/
        struct _ddf_data	*next;
        void                    *user_data;
	mid_watchdog_data_t	watchdog_data;
	ushort			int_rcvd;
	ushort			sleep_flg;
	union {
		mid_getcpos_t			ddf_getcpos;
		mid_getcolor_t			ddf_getcolor;
		mid_get_modelling_matrix_t	ddf_get_modelling_matrix;
		mid_get_projection_matrix_t	ddf_getprojection_matrix;
		mid_getcondition_t		ddf_getcondition;
		mid_gettextfontindex_t		ddf_gettextfontindex;
	} proc_data;
} ddf_data_t;

#endif /* _H_MIDDDFGETS */
