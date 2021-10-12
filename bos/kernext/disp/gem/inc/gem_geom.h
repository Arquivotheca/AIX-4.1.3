/* @(#)23	1.6.2.3  src/bos/kernext/disp/gem/inc/gem_geom.h, sysxdispgem, bos411, 9428A410j 1/19/93 12:42:51 */
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		CLEAR_AND_SET_COLOR
 *		FREEZE_3D
 *		GM_GUARD
 *		GM_LOCK_LDAT
 *		GM_UNGUARD
 *		GM_UNLOCK_LDAT
 *		SET_WINDOW_COMP
 *		START_OTHR_FIFO
 *		STOP_OTHR_FIFO
 *		THAW_3D
 *		UNDRAW_HWID
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


  /*
   * RCM change flags for upd_geom
   */
#define rStealHwid	(1L<<0)		/* HWID needs to be stolen */
#define rTravFifo  	(1L<<1)		/* SE's will be put down Trav Fifo */
#define rImmFifo  	0		/* SE's will be put down Imm Fifo */
#define rSetWinOrgSize	(1L<<2)		/* Set hardware window attributes */
#define rCxtIsActive	(1L<<3)		/* Client's cxt is currently active */
#define rNewHwid	(1L<<4)		/* HWID is a new one */
#define	rZBuf		(1L<<5)		/* Set Zbuffer Protect bits */

  /*
   * RCM change flags for upd_win
   */
#define rSetWinAttrs	(1L<<0)		/* Set window attrs for new WA */
  
  /*
   * Set Window Attrributes SE mask values
   */
#define waColorTableID	0x10		/* Don't set Color Table ID */
#define waColorMode  	0x08		/* Don't set Color Mode */
#define waObscurity  	0x04		/* Don't set Obscurity Indicator */
#define waOrigin	0x02		/* Don't set Window Origin */
#define waSize		0x01		/* Don't set Window Size */

  /*
   * Display Buffer Sync defines
   */
#define DISP_BUF_OFFSET	0x3c		/* Offset of display buf info	*/
#define IMM_SYNC_CNTR ((volatile ulong *)(&((GM_MMAP *)pDevP->gmbase)->disp_buf_ind[0]))
#define TRV_SYNC_CNTR ((volatile ulong *)(&((GM_MMAP *)pDevP->gmbase)->disp_buf_ind[4]))
#define MAX_SYNC_CNTR	0xffffffff
#define DISP_BUFFS	(&((GM_MMAP *)pDevP->gmbase)->disp_buf_ind[8])
  
  /*
   * RS-GCP interrupt defines
   */
#define	RS_GCP_INT_BLK_ofst	0x8
#define RS_GCP_INT_FLAG_ofst	0x10
#define	GCP_INT_BUSY_BIT	0x0100
#define	C25IV_PEND		0x04000000
#define	RESET_HI_LOW		0x01000100

  /*
   * Stop Other Fifo defines
   */
#define ACKNOWLEDGMENT	2

/************************************************************************/
/*                                                                      */
/*  FUNCTION NAME: UNDRAW_HWID              (macro)                     */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      Erases the hwid from where it is used on the window planes.     */
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      UNDRAW_HWID( BUF, PWG, HWID, PROTECT_ID )                       */
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*      BUF	- The buffer to put SE's into (must be char *)		*/
/*	PWG	- Pointer to the window geometry with the clip regions	*/
/*		  to be "undrawn"; if NULL, undraw full screen		*/
/*	HWID	- The window id being cleared - used as a compare	*/
/*		  value when filling					*/
/*	PROTECT_ID - The hwid to replace HWID with - used as the	*/
/*		  color to fill with					*/
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/************************************************************************/
#define UNDRAW_HWID( BUF, PWG, HWID, PROTECT_ID )                       \
{									\
									\
  ((frame_buf_cmp *)BUF)->len    = sizeof(frame_buf_cmp);               \
  ((frame_buf_cmp *)BUF)->opcode = CE_FBC;                              \
  ((frame_buf_cmp *)BUF)->flags  = 0;				        \
  ((frame_buf_cmp *)BUF)->mask   = GAI_WIN_PLANES;		        \
  ((frame_buf_cmp *)BUF)->value  = (HWID);				\
  BUF += sizeof(frame_buf_cmp);						\
									\
  ((generic_se *)BUF)->len    = sizeof(generic_se);			\
  ((generic_se *)BUF)->opcode = CE_INCI;				\
  ((generic_se *)BUF)->data   = PROTECT_ID;				\
  BUF += sizeof(generic_se);						\
									\
  ((rectangle *)BUF)->len         = sizeof(rectangle);                  \
  ((rectangle *)BUF)->opcode      = CE_IPLG;				\
  ((rectangle *)BUF)->flags       = CONVEX;				\
  ((rectangle *)BUF)->length      = 0x18;				\
  if (PWG)								\
  { ((rectangle *)BUF)->pt[0].x = PWG->wg.winOrg.x;			\
    ((rectangle *)BUF)->pt[0].y = PWG->wg.winOrg.y;			\
    ((rectangle *)BUF)->pt[1].x = PWG->wg.winOrg.x+(PWG->wg.width-1);	\
    ((rectangle *)BUF)->pt[1].y = PWG->wg.winOrg.y;			\
    ((rectangle *)BUF)->pt[2].x = PWG->wg.winOrg.x+(PWG->wg.width-1);	\
    ((rectangle *)BUF)->pt[2].y = PWG->wg.winOrg.y+(PWG->wg.height-1);	\
    ((rectangle *)BUF)->pt[3].x = PWG->wg.winOrg.x;			\
    ((rectangle *)BUF)->pt[3].y = PWG->wg.winOrg.y+(PWG->wg.height-1);	\
    ((rectangle *)BUF)->pt[4].x = PWG->wg.winOrg.x;			\
    ((rectangle *)BUF)->pt[4].y = PWG->wg.winOrg.y;			\
  }									\
  else									\
  { ((rectangle *)BUF)->pt[0].x = 0;					\
    ((rectangle *)BUF)->pt[0].y = 0;					\
    ((rectangle *)BUF)->pt[1].x = GM_WIDTH-1;				\
    ((rectangle *)BUF)->pt[1].y = 0;					\
    ((rectangle *)BUF)->pt[2].x	= GM_WIDTH-1;				\
    ((rectangle *)BUF)->pt[2].y = GM_HEIGHT-1;				\
    ((rectangle *)BUF)->pt[3].x = 0;					\
    ((rectangle *)BUF)->pt[3].y = GM_HEIGHT-1;				\
    ((rectangle *)BUF)->pt[4].x = 0;					\
    ((rectangle *)BUF)->pt[4].y = 0;					\
  }									\
  BUF += sizeof(rectangle);	         				\
}

/************************************************************************/
/*                                                                      */
/*  FUNCTION NAME: SET_WINDOW_COMP          (macro)                     */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      Sets the frame buffer comparison for the window planes.		*/
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      SET_WINDOW_COMP( BUF, MASK, VALUE, TYPE, RCMASK )               */
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*      BUF	- The buffer to put SE's into (must be char *)		*/
/*	MASK	- What to set the compare mask to			*/
/*	VALUE	- What to set the compare value to			*/
/*	TYPE	- type of rcx, immediate or traversal			*/
/*	RCMASK	- RCM change mask passed to upd_geom			*/
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/************************************************************************/

#define SET_WINDOW_COMP( BUF, MASK, VALUE, TYPE, RCMASK )               \
{                                                                       \
                                                                        \
/* set frame buffer comparison up for window planes		*/	\
                                                                        \
  ((frame_buf_cmp *)BUF)->len    = sizeof(frame_buf_cmp);               \
  ((frame_buf_cmp *)BUF)->opcode = CE_FBC;                              \
  if (TYPE == IMM_RCXTYPE || RCMASK & rTravFifo)			\
    ((frame_buf_cmp *)BUF)->flags = 0x00000008;				\
  else									\
    ((frame_buf_cmp *)BUF)->flags = 0x00000009;				\
  ((frame_buf_cmp *)BUF)->mask   = MASK;				\
  ((frame_buf_cmp *)BUF)->value  = VALUE;				\
  BUF += sizeof(frame_buf_cmp);                                         \
                                                                        \
}

/************************************************************************/
/*                                                                      */
/*  FUNCTION NAME: CLEAR_AND_SET_COLOR             (macro)              */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      Draws 0's into a rectangle of origin (0,0) and width and height	*/
/*	as indicated, then sets the interior color to be that passed.	*/
/*	NB: Assumes that interior style has already been set to solid.	*/
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      CLEAR_AND_SET_COLOR( BUF, WIDTH, HEIGHT, COLOR )                */
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*      BUF	- The buffer to put SE's into (must be char *)		*/
/*	WIDTH	- Width of rectangle to fill with 0's			*/
/*	HEIGHT	- Height of rectangle to fill with 0's			*/
/*	COLOR	- Color to se interior color to after the clear		*/
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/************************************************************************/

#define CLEAR_AND_SET_COLOR( BUF, WIDTH, HEIGHT, COLOR )		\
{									\
  ((generic_se *)BUF)->len = sizeof(generic_se);			\
  ((generic_se *)BUF)->opcode = CE_INCI;				\
  ((generic_se *)BUF)->data = 0;					\
  BUF += sizeof(generic_se);						\
									\
  ((rectangle *)BUF)->len = sizeof(rectangle);				\
  ((rectangle *)BUF)->opcode = CE_IPLG;					\
  ((rectangle *)BUF)->flags = CONVEX;					\
  ((rectangle *)BUF)->length = 0x18;					\
  ((rectangle *)BUF)->pt[0].x = 0;					\
  ((rectangle *)BUF)->pt[0].y = 0;					\
  ((rectangle *)BUF)->pt[1].x = (WIDTH) - 1;				\
  ((rectangle *)BUF)->pt[1].y = 0;					\
  ((rectangle *)BUF)->pt[2].x = (WIDTH) - 1;				\
  ((rectangle *)BUF)->pt[2].y = (HEIGHT) - 1;				\
  ((rectangle *)BUF)->pt[3].x = 0;					\
  ((rectangle *)BUF)->pt[3].y = (HEIGHT) - 1;				\
  ((rectangle *)BUF)->pt[4].x = 0;					\
  ((rectangle *)BUF)->pt[4].y = 0;					\
  BUF += sizeof(rectangle);						\
									\
  ((generic_se *)BUF)->len = sizeof(generic_se);			\
  ((generic_se *)BUF)->opcode = CE_INCI;				\
  ((generic_se *)BUF)->data = COLOR;					\
  BUF += sizeof(generic_se);						\
}

/************************************************************************/
/*                                                                      */
/*  FUNCTION NAME: START/STOP_OTHR_FIFO     (macro)                     */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      These macros generate an enable/disable other fifo when doing	*/
/*      things on behalf of a traversal rcx                             */
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      STOP_OTHER_FIFO( BUF, ID, ACK)					*/
/*      START_OTHER_FIFO( BUF, ID)					*/
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*      BUF	- The buffer to put SE's into (must be char *)		*/
/*	ID	- Identifier for who's issuing start/stop (0-255)	*/
/*	ACK	- Ask for acknowledgment on stop fifo?			*/
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/************************************************************************/

#define START_OTHR_FIFO( BUF, ID)                                       \
{                                                                       \
  ((generic_se *)BUF)->len    = sizeof(generic_se);                     \
  ((generic_se *)BUF)->opcode = CE_EDOF;                                \
  ((generic_se *)BUF)->data   = ENABLE_FIFO | (ID) << 24;		\
  BUF += sizeof(generic_se);                                            \
}

#define STOP_OTHR_FIFO( BUF, ID, ACK)					\
{                                                                       \
  ((generic_se *)BUF)->len    = sizeof(generic_se);                     \
  ((generic_se *)BUF)->opcode = CE_EDOF;                                \
  ((generic_se *)BUF)->data   = DISABLE_FIFO | (ID) << 24;		\
  if (ACK)								\
    ((generic_se *)BUF)->data |= ACKNOWLEDGMENT;			\
  BUF += sizeof(generic_se);                                            \
}

/************************************************************************/
/*                                                                      */
/*  FUNCTION NAME: FREEZE_3D                (macro)                     */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      This macro generates code to stop the ucode from processing	*/
/*      out of the 3D fifo and allow us to check later on whether or	*/
/*	not the 3D fifo has quiesced yet.				*/
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      FREEZE_3D(pDevP)              					*/
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*      pDevP	- Pointer to the rcm private area			*/
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/************************************************************************/
#define FREEZE_3D(pDevP)                                                \
{ uint oldlevel;							\
  volatile ulong *garp;							\
  volatile ulong ogar; 							\
  volatile ulong *rgibp;						\
  volatile ulong *dp;							\
									\
  ((GM_MMAP *)pDevP->gmbase)->gm_ucflags.freeze_3d = 1;		        \
									\
  /* set pointer to geographical address register		*/	\
  garp = (volatile ulong *)(pDevP->gmbase + GAR_ofst);			\
									\
  /* set pointer to Rios-GCP interupt block			*/	\
  rgibp = (volatile ulong *)(pDevP->gmbase + RS_GCP_INT_BLK_ofst);	\
									\
  while (TRUE)								\
  { /* disable interrupts					*/	\
    oldlevel = i_disable(INTMAX);					\
									\
    /* set geographical address to GCP, saving old geo address	*/	\
    ogar = *garp;							\
    *garp = pDevP->gcp_geo;						\
									\
    /* check for busy bit to clear				*/	\
    if (!(*rgibp & GCP_INT_BUSY_BIT))					\
      break;								\
									\
    /* reset geographical address register			*/	\
    *garp = ogar;							\
									\
    /* re-enable interrupts					*/	\
    i_enable(oldlevel);							\
									\
    delay(5);								\
  }									\
									\
  /* set interrupt to stop traversal fifo			*/	\
  *rgibp = 0x00000102;							\
  dp = (volatile ulong *)(pDevP->gmbase + RS_GCP_INT_FLAG_ofst);	\
  *dp = 0x00000002;							\
  dp = (volatile ulong *)(pDevP->gmbase + SCR_ofst);			\
  *dp = C25IV_PEND | RESET_HI_LOW;					\
									\
  /* reset geographical address register			*/	\
  *garp = ogar;								\
									\
  /* re-enable interrupts					*/	\
  i_enable(oldlevel);							\
}

/************************************************************************/
/*                                                                      */
/*  FUNCTION NAME: THAW_3D                  (macro)                     */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      This macro generates a structure element that restarts the	*/
/*      traversal fifo after it was stopped by FREEZE_3D.		*/
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      THAW_3D(BUF)							*/
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*      BUF - pointer to buffer                                		*/
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/************************************************************************/
#define THAW_3D(BUF)	START_OTHR_FIFO( BUF, 2)


/************************************************************************/
/*                                                                      */
/*  FUNCTION NAME: UN/GUARD                 (macro)                     */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      This macro generates a calls to hft to guard and unguard        */
/*      a domain                                      			*/
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      GM_GUARD(PDOM, PPROC)                                           */
/*      GM_UNGUARD(PDOM)                                                */
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*      PDOM - Pointer to domain to be guarded                 		*/
/*      PPROC - ptr to proc using domain to be guarded                  */
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*      None                                                            */
/************************************************************************/
#ifdef FAST_LOCK

#define GM_GUARD(PDOM, PPROC)                                        \
  (gdp->devHead.pCom->rcm_callback->guard_domain) (PDOM,PPROC)

#define GM_UNGUARD(PDOM)                                               \
  (gdp->devHead.pCom->rcm_callback->unguard_domain) (PDOM)

#else

#define GM_GUARD(PDOM, PPROC)
#define GM_UNGUARD(PDOM)

#endif


/************************************************************************/
/*                                                                      */
/*  FUNCTION NAME: UN/LOCK_LDAT             (macro)                     */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      This macro generates a calls to un/lockl to ensure serial       */
/*      execution of upd_geom.  If a routine also does a guard this	*/
/*      lock must come after to prevent deadlock                        */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      GM_LOCK_LDAT(PDEVP)                                             */
/*      GM_UNLOCK_LDAT(PDEVP)                                           */
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*      PDEVP Pointer to rcm private area of LDAT             		*/
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*      None                                                            */
/************************************************************************/
#define GM_LOCK_LDAT(PDEVP)				\
  while ((lockl (&PDEVP->lock, LOCK_SIGRET)) != LOCK_SUCC);

#define GM_UNLOCK_LDAT(PDEVP) unlockl(&(PDEVP)->lock);

