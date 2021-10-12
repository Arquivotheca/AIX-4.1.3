/* @(#)14     1.1  src/bos/usr/include/mirror.h, cmdmirror, bos411, 9428A410j 12/15/93 10:06:58 */
/*
 * COMPONENT_NAME: CMDMIRROR: Console mirroring
 * 
 * FUNCTIONS:
 * 
 * ORIGINS: 83 
 * 
 */
/*
 *  LEVEL 1, 5 Years Bull Confidential Information
 */


#ifndef _H_MIRROR
#define _H_MIRROR



#define ECHO_OFF  		0         /* the mode echo is off   (no mirror)   */
#define ECHO_ON   		1        /* the mode echo is on   (mirror)        */


#define MIR_PUT_WQ1 		0x100
#define MIR_PUT_WQ2 		0x200
#define MIR_ON_ECHO		0x300
#define MIR_OFF_ECHO		0x400
#define MIR_WRITE_IN_PROCESS	0x500
#define ECHO_OFF_IN_PROCESS	0x600
#define	M_HANGUP_S1		"m_hangup_s1"
#define	M_HANGUP_S2		"m_hangup_s2"

#endif  /* _H_MIRROR   */
