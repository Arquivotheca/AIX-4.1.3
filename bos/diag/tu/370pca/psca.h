/* src/bos/diag/tu/370pca/psca.h, tu_370pca, bos411, 9428A410j 8/8/91 15:29:22 */
/*************************************************************************
* There have been several revisions of the card.  Some of the code is	 *
* mutually exclusive to a specific revision.  Here I will try to make it *
* clear what #define corresponds to what card.	The rest of the modules	 *
* should include this file and use #ifdef's to compile the correct code. *
*									 *
* CARD_POS	uses POS register bits for int's and ack's (although	 *
*		insignificant to this code, it also uses io-based chip	 *
*		selects rather than mem-based).				 *
* CARD_FIFO	has hardware FIFO's with separate mem-mapped addrs	 *
* CARD_MM	uses mem-mapped addr's to send and clear int's,		 *
*		uses mem-based chip selects, has mem-mapped DMA start	 *
* CARD_OLD_BASE	has 3 bits for addr selection and bad choices of addrs	 *
* CARD_BAD_BASE	has base addr that are close to the spec.		 *
* CARD_BASE	has base addr that faithfully follow the spec.		 *
* CARD_PERFECT	This is the fictitious (so far) card that has everything *
*		right.	This code should reflect the spec or the	 *
*		(current) image of what the final card will be,		 *
*		whichever is more current.  Current code is like	 *
*		CARD_FIFO but with base addr's that match the spec.	 *
*************************************************************************/
#define CARD_PERFECT			/* The card we are really compiling for */
#define NEWCARD				/* The newer fifo status addresses */

#ifdef	CARD_PERFECT			/* this for informational purposes */
#define CARD_FIFO
#define CARD_MM
#define CARD_BASE
#endif
#ifdef	CARD_NUM_2
#define CARD_POS
#define CARD_OLD_BASE
#endif
#ifdef	CARD_NEW
#define CARD_FIFO
#define CARD_MM
#define CARD_BAD_BASE
#endif

/*************************************************************************
* These are the things that are likely to change			 *
*************************************************************************/
#ifdef AIX
#define DIAG_UCODE   "/etc/microcode/fe92d.00.00"  /* diagnostic microcode */
#define FUNC_UCODE   "/etc/microcode/fe92.00.00"   /* functional microcode */
#define PSCA_DEV	"/dev/cat0"
#endif
#ifdef MTEX
#define DIAG_UCODE	"diag.ucd"	/* file containing diagnostic microcode */
#define FUNC_UCODE	"psca.abs"	/* file containing functional microcode */
#endif

/*************************************************************************
* type definitions:  You could probably bet on these, but BITnn is more	 *
* intuitive in some places.						 *
*************************************************************************/
typedef unsigned char	BYTE;			/* an Intel 186 Byte :-) */
typedef unsigned short	BIT16;			/* an Intel 186 Word */
typedef unsigned long	BIT32;			/* an Intel 186 Double Word */
typedef unsigned long	SRAM;			/* an offset in Shared RAM */

/*************************************************************************
* The following #ifdef block is designed to make this module		 *
* plug-compatible into other applications.  If this block is not	 *
* included (i.e. HTX is not defined), another block defining these items *
* must be provided.							 *
*	MSGBUF		A convenient place to build messages		 *
*	VERBOSE		A boolean that determines noisiness		 *
*	RPT_HARD(n, s)	How to report a "Hard" error			 *
*		s	A textual error message				 *
*		n	An error number, usually errno			 *
*	RPT_SOFT(n, s)	How to report a "Soft" error			 *
*		s	A textual error message				 *
*		n	An error number, usually errno			 *
*	RPT_ERR(n, s)	How to report a system error			 *
*		s	A textual error message				 *
*		n	An error number, usually errno			 *
*	RPT_INFO(s)	How to output an informational message		 *
*		s	A textual information message			 *
*	RPT_VERBOSE(s)	How to report noise when VERBOSE is true	 *
*		s	A textual information message			 *
*	RPT_UPDATE(s)	How to issue a status update			 *
*		s	A textual information message			 *
*************************************************************************/
#ifdef HTX
#define MSGBUF			(ps.msg_text)
#define VERBOSE			(ps.run_type[0] == 'O')
#define RPT_HARD(n, s)		{ if (--max_errors > 0) hxfmsg(&ps, n, HARD, s); }
#define RPT_SOFT(n, s)		{ if (--max_errors > 0) hxfmsg(&ps, n, SOFT, s); }
#define RPT_ERR(n, s)		{ if (--max_errors > 0) hxfmsg(&ps, n, SYSERR, s); }
#define RPT_INFO(s)		{ hxfmsg(&ps, 0, INFO, s); }
#define RPT_VERBOSE(str)	{ if (VERBOSE) printf("%s", str); }
/* #define RPT_VERBOSE(str)	{ hxfupdate(INFO, &ps); } */
#define RPT_UPDATE(str)		{ hxfupdate(UPDATE, &ps); }
#define INC_GOOD_OTHERS		{ ps.good_others++; }
#define INC_BAD_OTHERS		{ ps.bad_others++; }

#include "hxihtx.h"		/* for the ps structure mostly */
#define HARD 1
#define SOFT 4
#define INFO 7
#define SYSERR 0
#define PATLIB_PATH "../pattern/"
#endif

/*************************************************************************
* This block provides the same interface for the Field Diagnostics.	 *
* There are no reporting mechanisms except return codes when running	 *
* diagnostics, but they might be useful for debugging someday.		 *
*************************************************************************/
#ifdef DIAGS
#define MSGBUF			msg_buf
#define RPT_SOFT(n, s)		{}
#define RPT_HARD(n, s)		{}
#define RPT_ERR(n, s)		{}
#define RPT_INFO(s)		{}
#define RPT_VERBOSE(str)	{}
#define RPT_UPDATE(str)		{}
#define VERBOSE			(0)
#define INC_GOOD_OTHERS		{}
#define INC_BAD_OTHERS		{}
char msg_buf[255];
int max_errors = 1;  /* stop immediately after seeing first error */
#endif

/*************************************************************************
* This block provides the definitions of these reporting macros which    *
* will be used in Systems Manufacturing.                		 *
*************************************************************************/
#ifdef BOXMFG
char msg_buf[255];
int max_errors = 1;  /* stop immediately after seeing first error */

#ifdef TU_DEBUG_MSG

#define MSGBUF			msg_buf
#define RPT_SOFT(num, str)	{					\
	fprintf( tucb_ptr->msg_file, "SOFT ERR %2d:%s", num, str);		\
	}

#define RPT_HARD(num, str)	{					\
	fprintf( tucb_ptr->msg_file, "HARD ERR %2d:%s", num, str);		\
	}

#define RPT_ERR(num, str)	{					\
	fprintf( tucb_ptr->msg_file, "SYS ERR %2d:%s", num, str);		\
	}

#define RPT_INFO(str)		{					\
	fprintf( tucb_ptr->msg_file, "INFO:%s", str);				\
	}

#define RPT_VERBOSE(str)	{					\
	fprintf( tucb_ptr->msg_file, "VERBOSE MSG :%s", str);			\
	}
#define RPT_UPDATE(str)	{						\
	fprintf( tucb_ptr->msg_file, "UPDATE MSG :%s", str);			\
	}
#define VERBOSE			(1)
#define INC_GOOD_OTHERS		{}
#define INC_BAD_OTHERS		{}

#else

#define MSGBUF			msg_buf
#define RPT_SOFT(n, s)		{}
#define RPT_HARD(n, s)		{}
#define RPT_ERR(n, s)		{}
#define RPT_INFO(s)		{}
#define RPT_VERBOSE(str)	{}
#define RPT_UPDATE(str)		{}
#define VERBOSE			(0)
#define INC_GOOD_OTHERS		{}
#define INC_BAD_OTHERS		{}

#endif

#endif
/*************************************************************************
* This is a stab at getting things to compile for MTEX, maximum		 *
* verbosity for debugging the card and the code.			 *
*************************************************************************/
#ifdef MTEX
#define MSGBUF			msg_buf
#define RPT_SOFT(num, str)	{					\
	if (--max_errors) {						\
		printf("SOFT ERR %2d:%s", num, str);			\
		if (dump >= 0)						\
			write(dump, (char far *) str, strlen(str));	\
	}}

#define RPT_HARD(num, str)	{					\
	if (--max_errors) {						\
		printf("HARD ERR %2d:%s", num, str);			\
		if (dump >= 0)						\
			write(dump, (char far *) str, strlen(str));	\
	}}

#define RPT_ERR(num, str)	{					\
	if (--max_errors) {						\
		printf("SYS ERR %2d:%s", num, str);			\
		if (dump >= 0)						\
			write(dump, (char far *) str, strlen(str));	\
	}}

#define RPT_INFO(str)		{					\
	printf("INFO:%s", str);						\
	if (dump >= 0)							\
		write(dump, (char far *) str, strlen(str));		\
	}

#define RPT_VERBOSE(str)	{printf("VERBOSE MSG  : %s", str);}
#define RPT_UPDATE(str)
#define VERBOSE			(1)
#define INC_GOOD_OTHERS		{}
#define INC_BAD_OTHERS		{}
#define NOSWAP
#endif

/*************************************************************************
* This is a stab at getting TUMAIN to compile for AIX, maximum		 *
* verbosity for debugging the card and the code.			 *
*************************************************************************/
#ifdef TUMAIN
#define MSGBUF			msg_buf
#define RPT_SOFT(num, str)	{					\
	if (--max_errors) {						\
		printf("SOFT ERR %2d:%s", num, str);			\
		if (dump)						\
			fwrite((char *) str, 1, strlen(str), dump);	\
	}}

#define RPT_HARD(num, str)	{					\
	if (--max_errors) {						\
		printf("HARD ERR %2d:%s", num, str);			\
		if (dump)						\
			fwrite((char *) str, 1, strlen(str), dump);	\
	}}

#define RPT_ERR(num, str)	{					\
	if (--max_errors) {						\
		printf("SYS ERR %2d:%s", num, str);			\
		if (dump)						\
			fwrite((char *) str, 1, strlen(str), dump);	\
	}}

#define RPT_INFO(str)		{					\
	printf("INFO:%s", str);						\
	if (dump)							\
		fwrite((char *) str, 1, strlen(str), dump);		\
	}

#define RPT_VERBOSE(str)	{printf("VERBOSE MSG  : %s", str);}
#define RPT_UPDATE(str)
#define VERBOSE			(1)
#define INC_GOOD_OTHERS		{}
#define INC_BAD_OTHERS		{}
#endif



/*************************************************************************
* Constants to enhance readability					 *
*************************************************************************/
#define SRAMSIZE	0x3f800
#define MATCH		0		/* strcmp return value on match */
#define CMD_PART2	0x20		/* Bit to tell ucode to run part2 of a cmd */
#define TU_NUM		(cb->header.tu)
#define TU_NAME		(tu[cb->header.tu].name)

/*************************************************************************
* These TU return codes are translated to appropriate action by exectu() *
*************************************************************************/
#define TU_GOOD		0		/* Test Unit succeeded */
#define TU_SOFT		1		/* Test Unit encountered a "soft" error */
#define TU_HARD		2		/* Test Unit encountered a "hard" error */
#define TU_SYS		3		/* Test Unit encountered a system error */
#define TU_SYS_ERRNO	4		/* ditto, and errno is valid */



/*************************************************************************
* The following are peculiarities of the PSCA hardware			 *
*************************************************************************/
#define VPD_SIZE	256			/* max size of VPD EPROM */
#ifdef CARD_MM					/* if card has mem-mapped stuff */
#endif

/*************************************************************************
* Addresses in the communication area of the Static RAM			 *
*************************************************************************/
#define OPERLVL		((SRAM) 0x18)	/* SRAM offset: board status */
#define READY		((SRAM) 0x19)	/* SRAM offset: microcode status */
#define INT186		((SRAM) 0x1B)	/* SRAM offset: 186 WANTS INTERRUPTS */
#define INTMCI		((SRAM)	0x1a)	/* SRAM offset: MCI wants interrupts */

#define ERRDBGBLK	((SRAM) 0x62)	/* SRAM offset: debug info block */
#define TESTNUMBER	((SRAM) 0x62)	/* SRAM offset: current subtest # */
#define ERRLOG		((SRAM) 0x64)	/* SRAM offset: current TU # */
#define ERRCODE		((SRAM) 0x66)	/* SRAM offset: TU return code */
#define BADOFFS		((SRAM) 0x68)	/* SRAM offset: bad location found */
#define BADSEGM		((SRAM) 0x6a)	/* SRAM offset: segment of location */
#define EXPECTED	((SRAM) 0x6c)	/* SRAM offset: what was expected */
#define RECEIVED	((SRAM) 0x6e)	/* SRAM offset: what was gotten */



/*************************************************************************
* Arbitrary constants used by the PSCA microcode			 *
*************************************************************************/
#define DL_WAITING	0x01	/* in OPERLVL:	Board is waiting for a download */
#define DL_CMD		0x02	/* in OPERLVL:	cmd for download */
#define DL_CONFIRM	0x03	/* in OPERLVL:	confirmation of DL_CMD */
#define DIAG_WAITING	0x01	/* in READY:	Diagnostic Microcode is waiting */
#define DIAG_CMD	0x02	/* in READY:	cmd to run TU */
#define DIAG_RUNNING	0x03	/* in READY:	TU is now running */
#ifndef MTEX			/* avoid redefinition from pscadefs.h */
#define UCIRQ186	((SRAM) 0x3f806)	/* addr to cause int to OBP */
#endif
#define GOT_INTR	0x99	/* in ERRCODE:	when interrupt rcvd */

/*************************************************************************
* This is the format of the result block from a memory test, i.e.  the	 *
* POST's memory test.  This block usually resides at ERRDBGBLK.		 *
*************************************************************************/
struct errdbgblk {
	BIT16		testnumber;
	BIT16		errlog;
	BIT16		errcode;
	BIT16		badoffs;
	BIT16		badsegm;
	BIT16		expected;
	BIT16		received;
};



/*************************************************************************
* General macros and definitions for the exerciser and TU's		 *
*************************************************************************/

/*************************************************************************
* This macro will perform a subtest or lower-level function.  If it	 *
* fails an immediate return is performed.  Requires a local int rc.	 *
*************************************************************************/
#define ROF(x)	{ if ((rc = (x)) != TU_GOOD) return(rc); }	/* Return On Failure */

/*************************************************************************
* The type for the array of Test Units.					 *
*************************************************************************/
struct tu_type {
	int		ucode;		/* what microcode is required for the test */
	int		(*func)();	/* where is the function to do the test */
	char		*name;		/* a descriptive name of the test */
};

/*************************************************************************
* Possible values of tu_type.ucode					 *
*************************************************************************/
#define NONE	0			/* no ucode at all (hold reset) */
#define FUNC	1			/* functional microcode only */
#define	DIAG	3			/* diagnostic microcode only */
#define NOCARE	4			/* Doesn't matter at all */
#define UNKNOWN	5			/* Microcode is in unknown state */

/*************************************************************************
* Let's check even the system calls we know will succeed		 *
*************************************************************************/
#define CHECK(scall)	{ if ((scall) == -1) {					\
				sprintf(MSGBUF,					\
				  "System call error at line %d of %s: %s\n",	\
				  __LINE__, __FILE__, sys_errlist[errno]);	\
				RPT_ERR(errno, MSGBUF);				\
			} }



/*************************************************************************
* Operating System Portability macros					 *
*************************************************************************/

/*************************************************************************
* ADD_SPRINTF		use sprintf to generate string, accumulate len.	 *
*	str		pointer to string to add length of string to.	 *
*	sp_cmd		sprintf command to generate string at str.	 *
*************************************************************************/
#ifdef MTEX
#define ADD_SPRINTF(str, sp_cmd)	{ sp_cmd; str += strlen(str); }
#endif
#ifdef AIX
#define ADD_SPRINTF(str, sp_cmd)	{ str += sp_cmd; }
#endif
