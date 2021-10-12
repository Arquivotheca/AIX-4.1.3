static char sccsid[] = "@(#)81	1.1  src/bos/kernext/jfsc/comp.uext.c, sysxjfsc, bos411, 9428A410j 3/29/94 17:31:39";
/*
 * COMPONENT_NAME: SYSXJFSC (JFS Compression)
 *
 * FUNCTIONS:   getcompent
 *		encode_decode
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/malloc.h>
#include <sys/syspest.h>
#include <sys/device.h>
#include <sys/jfsc.h>

/* 
 * table of random integers used as a table of shorts
 * with high-order 4-bits = 0.
 */
static unsigned int rbytes[128] = {
   0x0C200DB8  ,0x001709FA  ,0x0FF30FDF  ,0x08E10712
  ,0x006809DB  ,0x04410115  ,0x0EE0042C  ,0x0E210CA7
  ,0x097B038A  ,0x04AB042D  ,0x09F507DF  ,0x01DA0F65
  ,0x03170134  ,0x06DF0AC5  ,0x0C770CD1  ,0x05880842
  ,0x0B8D0236  ,0x07D10587  ,0x0FCD0C47  ,0x0A6B08EA
  ,0x025201B8  ,0x0AAD0149  ,0x0BB30202  ,0x06BB07B4
  ,0x0C890273  ,0x06280CB6  ,0x01A70C13  ,0x09D40256
  ,0x014B0099  ,0x0213008E  ,0x09B30A79  ,0x0A6B0DCD
  ,0x016208F2  ,0x07510409  ,0x02D10E4C  ,0x0E5B0E92
  ,0x0F8B0C53  ,0x0D3F0BE5  ,0x0DA50A0D  ,0x07960EFC
  ,0x09C50CBB  ,0x03380CF2  ,0x06580B6A  ,0x041202E9
  ,0x0A920BA3  ,0x0882084D  ,0x0432028D  ,0x039504DA
  ,0x08730106  ,0x0FED065F  ,0x0B6003A9  ,0x033F0E05
  ,0x0DBE0D94  ,0x070C0D9F  ,0x0B490C86  ,0x0BEB05E9
  ,0x0768021F  ,0x01350852  ,0x0269095C  ,0x026804DC
  ,0x0D070392  ,0x0DC801E2  ,0x0A3E0EBF  ,0x0CA0065C
  ,0x089B0D58  ,0x0102068B  ,0x00110CF6  ,0x0F780D72
  ,0x00030D0A  ,0x0A710575  ,0x0BB30EAC  ,0x080C0577
  ,0x06880CC5  ,0x0DC704C2  ,0x047A0EA5  ,0x05DC0063
  ,0x0D1101A7  ,0x06EF0723  ,0x04EF0C65  ,0x0B1B0C2D
  ,0x0A0D0740  ,0x0ADF0916  ,0x0E210A3E  ,0x078D050D
  ,0x03B1098A  ,0x08D307B4  ,0x07570314  ,0x08C205E1
  ,0x03500181  ,0x0BFE0E34  ,0x09170FAC  ,0x0552045E
  ,0x06EB0E2C  ,0x051E0434  ,0x04140813  ,0x0A7E0826
  ,0x0E9408F1  ,0x08A303D8  ,0x078B0FBF  ,0x0E1C0FDF
  ,0x00D5058F  ,0x07A403F0  ,0x061A0458  ,0x006705EB
  ,0x06CB04E7  ,0x021E09C5  ,0x03620C9D  ,0x09AB0727
  ,0x02810B39  ,0x0B970C93  ,0x04C307B6  ,0x09770243
  ,0x015D07BA  ,0x08EA05A6  ,0x097B0962  ,0x05BE0458
  ,0x0A6107E3  ,0x03920AE8  ,0x0E4C0108  ,0x05240A73
  ,0x01900D4C  ,0x088B04B3  ,0x0DA70E69  ,0x006007CC
  ,0x046505CA  ,0x0E9D0B19  ,0x088E0355  ,0x0D5004C6 };


#define NHASHTAB 4096
#define NHENTRY 4096

static struct hentry h[NHENTRY * 8];
static short  htab[NHASHTAB * 2];

static struct compmethod comp = {
	"LZ",				/* name                          */
	 LZ,				/* numeric type                  */
        (struct compmethod *)NULL,      /* next algorithm (only LZ now) */
};

/*
 * NAME:        getcompent(cptr)
 *
 * FUNCTION:    Initializes compmethod structure to available method functions.
 *		This is an entry point to the user level portion of the
 *		compression extension.  It returns a structure of available
 *		algorithm names and their numeric types.
 *
 * RETURNS:      0
 */
int
getcompent(struct compmethod **cptr)  /* Compression method functions */
{
        *cptr = &comp;
        return(0);
}


/*
 * NAME: 	encode_decode 
 *
 * FUNCTION:	Compression/Decompression function which the kernel function
 *		pointer is set to.  This function switches on the compression
 *		algorithm and calls the appropriate encoder or decoder.
 * 
 * RETURNS:	-1	- Unsupported algorithm
 *		 0	- Supported algorithm queried
 *		Size 	- Size of compressed data
 */
int
encode_decode(int     	op,	    /* operation type: QUERY, ENCODE, DECODE */
	      int	algorithm,  /* compression algorithm	             */
       	      caddr_t 	inputbuf,   /* source buffer address	   	     */	
       	      size_t 	inputsize,  /* source buffer size 		     */
       	      caddr_t	outputbuf,  /* output buffer address		     */
       	      size_t	outputsize) /* output buffer size		     */
{
	unsigned int *endoutput;
	int 	 *overlay, 
		 k, 
		 rc = -1;

	if (algorithm == LZ)
	{
		if (op == COMP_ENCODE)
		{
			/* init list of hash table entries */
			overlay = (int *) htab;
			for (k = 0; k < NHASHTAB/2; k += 4)
			{
				overlay[k] = 0;
				overlay[k+1] = 0;
				overlay[k+2] = 0;
				overlay[k+3] = 0;
			}

			endoutput = (unsigned int *)outputbuf + 
						(inputsize - outputsize) / 4;
			rc = lzencode(inputbuf, outputbuf, endoutput, htab, 
						h, rbytes);
		}
		else if (op == COMP_DECODE)
			rc = lzdecode(inputbuf, outputbuf, outputsize);
		else if (op = COMP_QUERY)
			rc = 0;
	}
	return(rc);
}
