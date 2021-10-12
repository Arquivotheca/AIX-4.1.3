/* src/bos/diag/tu/370pca/pscatu.h, tu_370pca, bos411, 9428A410j 8/8/91 15:40:40 */
/* pscatu.h      08/8/91	       */
#include <stdio.h>
#ifdef DIAGS

#include <diag/atu.h>  
           
#else
				      /*  Header for Test Unit Control Block */
typedef struct	{
		 long  tu;	   /* The number of the test unit to execute */
		 long  mfg;			     /* Manufacturering mode */
		 long  loop;	      /* The number of times to execute test */
		 long  r1, r2;					 /* Reserved */
	       } tucb_t;
#endif

/*************************************************************************
* Storage area for options selected by a rule stanza			 *
*************************************************************************/
struct ruleinfo {
	char		rule_id[9];	/* rule id			*/
	int		test_unit;	/* test unit number (1-15)	*/
	char		diagcode[80];	/* filename of Diagnostic ucode	*/
	char		funccode[80];	/* filename of Functional ucode	*/
	char		cutbl1[80];	/* filename of Basic CU Table */
	char		cutbl2[80];	/* filename of Extended CU Table */
	int		loop;		/* number of loops in TU	*/
	int		mfg;		/* mfg mode, undocumented feature */
	int		debug;		/* device driver debug level */
	int		intlevel;	/* interrupt level */
	int		arblevel;	/* arbitration level */
	int		maxerrs;	/* maximum errors per TU */
	int		syncw;		/* synchronous writes (TU 26) */
	int		recsize;	/* record size (TU 26) */
	int		chanspeed;	/* channel speed for func ucode */
	int		bufsize;	/* buffer size, func ucode */
	int		numxmit;	/* number of xmit buffers, func ucode*/
	int		numrec;		/* number of rec buffers, func ucode*/
	int		dmasize;	/* size of block to dma */
	int		sram_addr;	/* address to do dma test */
	int		subchan;	/* subchannel address (TU 26) */
	};
/*************************************************************************
* This is the structure for communicating parameters to the Test Units.	 *
* The definition of struct tucb_t is in this file			 *
*************************************************************************/
#ifdef DIAGS
#define TUTYPE struct _pca_tu
struct	_pca_tu {
	struct tucb_t		header;
	struct ruleinfo		pr;
	};
#else

typedef struct {
	tucb_t			header;
	struct ruleinfo		pr;
}				TUTYPE;

#endif
 
FILE *		msg_file;	
