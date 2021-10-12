static char sccsid[] = "@(#)22	1.2  src/bos/kernel/io/rampack.c, sysio, bos411, 9428A410j 6/16/90 03:10:20";
/*
 * COMPONENT_NAME: SYSIO
 *
 * FUNCTIONS: Used by ram disk device driver to pack an unpack blocks
 *
 * ORIGINS: 27 26
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <ctype.h>
#include <sys/syspest.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ramdd.h>

BUGXDEF(ramdd_dbg);

#define BITS	12
#define HSIZE	5003

/*
 * a code_int must be able to hold 2**BITS values of type int, and also -1
 */
typedef long code_int;
typedef long count_int;
typedef	unsigned char	char_type;

/* Defines for third byte of header */
#define BIT_MASK	0x1f
#define BLOCK_MASK	0x80
/* Masks 0x40 and 0x20 are free.  I think 0x20 should mean that there is
   a fourth header byte (for expansion).
*/
#define INIT_BITS 9			/* initial number of bits/code */

int n_bits;				/* number of bits/code */
int maxbits = BITS;			/* user settable max # bits/code */
code_int maxcode;			/* maximum code, given n_bits */
code_int maxmaxcode = 1 << BITS;	/* should NEVER generate this code */
#define MAXCODE(n_bits)	((1 << (n_bits)) - 1)


count_int *htab;
unsigned short *codetab;

#define htabof(i)	htab[i]
#define codetabof(i)	codetab[i]
code_int hsize = HSIZE;			/* for dynamic table sizing */
count_int fsize;

char *input_buffer;
char *output_buffer;
int input_size;
int input_point;
int output_size;



/*
 * To save much memory, we overlay the table used by compress() with those
 * used by decompress().  The tab_prefix table is the same size and type
 * as the codetab.  The tab_suffix table needs 2**BITS characters.  We
 * get this from the beginning of htab.  The output stack uses the rest
 * of htab, and contains characters.  There is plenty of room for any
 * possible stack (stack used to be 8000 characters).
 */

#define tab_prefixof(i)	codetabof(i)
#define tab_suffixof(i)	((char_type *)(htab))[i]
#define de_stack		((char_type *)&tab_suffixof(1<<BITS))


code_int getcode();

/*
 * block compression parameters -- after all codes are used up,
 * and compression rate changes, start over.
 */
int block_compress = BLOCK_MASK;
int clear_flg = 0;
long int ratio = 0;
#define CHECK_GAP 10000	/* ratio check interval */
count_int checkpoint = CHECK_GAP;

int offset;
int bytes_out;
int out_count = 0;
int in_count = 1;

code_int free_ent = 0;			/* first unused entry */
int exit_stat = 0;			/* per-file status */
int perm_stat = 0;			/* permanent status */

/*
 * the next two codes should not be changed lightly, as they must not
 * lie within the contiguous general code space.
 */
#define FIRST	257	/* first free entry */
#define	CLEAR	256	/* table clear output code */

int do_decomp = 0;

#ifdef _KERNEL
extern int pinned_heap;
#define MALLOC(size) xmalloc(size, 2 ,pinned_heap)
#define FREE(ptr) xmfree(ptr, pinned_heap)
#else
#define MALLOC(size) malloc(size)
#define FREE(ptr) free(ptr)
#endif


/*
 * NAME: ram_comp_init
 *
 * FUNCTION: allocate data used by compress and decompress routines
 *
 * RETURNS: 0 = successfull	-1 = failure
 */
int
ram_comp_init()
{
	htab = (count_int *) MALLOC(sizeof(count_int) * HSIZE);
	if (htab == NULL)
		return(-1);
	codetab = (unsigned short *) MALLOC(sizeof(unsigned short) * HSIZE);
	if (codetab == NULL)
		return(-1);

	return(0);

}

/*
 * NAME: ram_comp_free
 *
 * FUNCTION: free memory allocated for compress and decompress routines
 *
 * RETURNS: NONE
 */
void
ram_comp_free()
{
	int rc;

	FREE((char *)htab);
	FREE((char *)codetab);
}


/*
 * NAME: ram_compress
 *
 * FUNCTION: compress a block of ram image
 *
 * RETURNS: None
 */
void
ram_compress(ibuf, obuf, size)
char *ibuf;
char *obuf;
int *size;
{
	int i;
	int flag;

	BUGLPR(ramdd_dbg, 10, ("ram_compress:%x %x %x\n", ibuf, obuf, size));

	/*
	 * special case zero blocks since there are a lot of these
	 */
	if (ibuf[0] == 0 && ibuf[PSIZE/3] == 0 && ibuf[PSIZE/2] == 0 &&
			ibuf[2*PSIZE/3] == 0 && ibuf[PSIZE-1] == 0 &&
			zero_block(ibuf)) {

		*size = 0;

	} else {

		z_compress(ibuf, obuf, size);
		if (*size >= PSIZE) {
			bcopy(ibuf, obuf, PSIZE);
			*size = PSIZE;
		}
	}
}

void
ram_decompress(ibuf, obuf, size)
char *ibuf;
char *obuf;
int size;
{
	BUGLPR(ramdd_dbg, 10,
		 ("ram_decompress:%x %x %x\n", ibuf, obuf, size));
	ASSERT(size <= PSIZE);

	if (size == 0)
		memset(obuf, 0x00, PSIZE);
	else if (size == PSIZE)
		bcopy(ibuf, obuf, PSIZE);
	else
		z_decompress(ibuf, obuf, size);
}

/*
 * NAME: zero_block
 *
 * FUNCTION: check if a block of memory is zero
 *
 * RETURNS:
 *	1 - if block is zero
 *	0 - if block has any non-zero value
 */
int
zero_block(bp)
char *bp;
{
	static char buff[32] = {0};
	int i;

	for (i = 0 ; i < PSIZE/32 ; i++) {
		if (memcmp(bp, buff, 32))
			return(0);
		bp += 32;
	}

	return(1);
}

/*
 * NAME: open_out
 *
 * FUNCTION: open output buffer
 *
 */
void
open_out(buf)
char *buf;
{
	output_buffer = buf;
	output_size = 0;
}

/*
 * NAME: open_in
 *
 * FUNCTION: open input buffer
 *
 */
void 
open_in(buf, size)
char *buf;
int size;
{
	input_buffer = buf;
	input_point = 0;
	input_size = size;
}

/*
 * NAME: in_char
 *
 * FUNCTION: get a character from input buffer
 *
 * RETURNS: 0 if successful	-1 if end of buffer reached
 */
int
in_char(cp)
char *cp;
{
	if (input_point >= input_size)
		return(-1);

	*cp = input_buffer[input_point];
	input_point++;
	return(0);
}

/*
 * NAME: out_char
 *
 * FUCNTION: place a character in output buffer
 *
 */
void
out_char(ch)
char ch;
{

	if (output_size >= PSIZE) {
		BUGLPR(ramdd_dbg, 10, ("output overflow\n"));
		return;
	}
	output_buffer[output_size] = ch;
	output_size++;
}


/*
 * compress stdin to stdout
 *
 * Algorithm:  use open addressing double hashing (no chaining) on the 
 * prefix code / next character combination.  We do a variant of Knuth's
 * algorithm D (vol. 3, sec. 6.4) along with G. Knott's relatively-prime
 * secondary probe.  Here, the modular division first probe is gives way
 * to a faster exclusive-or manipulation.  Also do block compression with
 * an adaptive reset, whereby the code table is cleared when the compression
 * ratio decreases, but after the table fills.  The variable-length output
 * codes are re-sized at this point, and a special CLEAR code is generated
 * for the decompressor.  Late addition:  construct the table according to
 * file size for noticeable speed improvement on small files.  Please direct
 * questions about this implementation to ames!jaw.
 */

z_compress(inbuf, outbuf, size)
char *inbuf;
char *outbuf;
int *size;
{
	register long fcode;
	register code_int i = 0;
	int c;
	char inchar;
	register code_int ent;
	register int disp;
	register code_int hsize_reg;
	register int hshift;

	open_out(outbuf);
	open_in(inbuf, PSIZE);


	offset = 0;
	bytes_out = 0;		/* includes 3-byte header mojo */
	out_count = 0;
	clear_flg = 0;
	ratio = 0;
	in_count = 1;
	checkpoint = CHECK_GAP;
	maxcode = MAXCODE(n_bits = INIT_BITS);
	free_ent = ((block_compress) ? FIRST : 256 );

	in_char (&inchar);
	ent = inchar;

	hshift = 0;
	for ( fcode = (long) hsize;  fcode < 65536L; fcode *= 2L )
		hshift++;
	hshift = 8 - hshift;		/* set hash code range bound */

	hsize_reg = hsize;
	cl_hash( (count_int) hsize_reg);		/* clear hash table */

	while (in_char(&inchar) != -1 ) {
		c = inchar;
		in_count++;
		fcode = (long) (((long) c << maxbits) + ent);
		i = ((c << hshift) ^ ent);	/* xor hashing */

		if ( htabof (i) == fcode ) {
			ent = codetabof (i);
			continue;
		} else if ( (long)htabof (i) < 0 )	/* empty slot */
			goto nomatch;
		disp = hsize_reg - i;	/* secondary hash (after G. Knott) */
		if ( i == 0 )
			disp = 1;
probe:
		if ( (i -= disp) < 0 )
			i += hsize_reg;

		if ( htabof (i) == fcode ) {
			ent = codetabof (i);
			continue;
		}
		if ( (long)htabof (i) > 0 )
			goto probe;
nomatch:
		output ( (code_int) ent );
		out_count++;
		ent = c;
		if ( free_ent < maxmaxcode ) {
			codetabof (i) = free_ent++;	/* code -> hashtable */
			htabof (i) = fcode;
		}
		else if ( (count_int)in_count >= checkpoint && block_compress )
			cl_block ();
	}
    /*
     * Put out the final code.
     */
	output( (code_int)ent );
	out_count++;
	output( (code_int)-1 );
	*size = bytes_out;
	return;
}

/*
 *
 * Output the given code.
 * Inputs:
 * 	code:	A n_bits-bit integer.  If == -1, then EOF.  This assumes
 *		that n_bits =< (long)wordsize - 1.
 * Outputs:
 * 	Outputs code to the file.
 * Assumptions:
 *	Chars are 8 bits long.
 * Algorithm:
 * 	Maintain a BITS character long buffer (so that 8 codes will
 *      fit in it exactly).  When the buffer fills up empty it and start over.
 */

static char buf[BITS];

char_type lmask[9] = {
	0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0x00};
char_type rmask[9] = {
	0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff};

output( code )
code_int  code;
{
	int i;

	register int r_off = offset, bits= n_bits;
	register char * bp = buf;

	if ( code >= 0 ) {
	/*
	 * Get to the first byte.
	 */
		bp += (r_off >> 3);
		r_off &= 7;
	/*
	 * Since code is always >= 8 bits, only need to mask the first
	 * hunk on the left.
	 */
		*bp = (*bp & rmask[r_off]) | (code << r_off) & lmask[r_off];
		bp++;
		bits -= (8 - r_off);
		code >>= 8 - r_off;
	/* Get any 8 bit parts in the middle (<=1 for up to 16 bits). */
		if ( bits >= 8 ) {
			*bp++ = code;
			code >>= 8;
			bits -= 8;
		}
		/* Last bits. */
		if(bits)
			*bp = code;
		offset += n_bits;
		if ( offset == (n_bits << 3) ) {
			bp = buf;
			bits = n_bits;
			bytes_out += bits;
			do
				out_char(*bp++);
			while(--bits);
			offset = 0;
		}

	/*
	 * If the next entry is going to be too big for the code size,
	 * then increase it, if possible.
	 */
		if ( free_ent > maxcode || (clear_flg > 0))
		{
	    /*
	     * Write the whole buffer, because the input side won't
	     * discover the size increase until after it has read it.
	     */
			if ( offset > 0 ) {
				bytes_out += n_bits;
				for (i=0 ; i < n_bits ; i++) {
					out_char(buf[i]);
				}
			}
			offset = 0;

			if ( clear_flg ) {
				maxcode = MAXCODE (n_bits = INIT_BITS);
				clear_flg = 0;
			}
			else {
				n_bits++;
				if ( n_bits == maxbits )
					maxcode = maxmaxcode;
				else
					maxcode = MAXCODE(n_bits);
			}
		}
	} else {
	/*
	 * At EOF, write the rest of the buffer.
	 */
		if ( offset > 0 ) {
			for (i = 0 ; i < (offset + 7) / 8 ; i++) {
				out_char(buf[i]);
				bytes_out++;
			}
		}
		offset = 0;
	}
}

cl_block ()		/* table clear for block compress */
{
	register long int rat;

	checkpoint = in_count + CHECK_GAP;

	if(in_count > 0x007fffff) {	/* shift will overflow */
		rat = bytes_out >> 8;
		if(rat == 0) {		/* Don't divide by zero */
			rat = 0x7fffffff;
		} else {
			rat = in_count / rat;
		}
	} else {
		rat = (in_count << 8) / bytes_out;	/* 8 fractional bits */
	}
	if ( rat > ratio ) {
		ratio = rat;
	} else {
		ratio = 0;
		cl_hash ( (count_int) hsize );
		free_ent = FIRST;
		clear_flg = 1;
		output ( (code_int) CLEAR );
	}
}

cl_hash(hsize)		/* reset code table */
register count_int hsize;
{
	register count_int *htab_p = htab+hsize;
	register long i;
	register long m1 = -1;

	i = hsize - 16;
	do {				/* might use Sys V memset(3) here */
		*(htab_p-16) = m1;
		*(htab_p-15) = m1;
		*(htab_p-14) = m1;
		*(htab_p-13) = m1;
		*(htab_p-12) = m1;
		*(htab_p-11) = m1;
		*(htab_p-10) = m1;
		*(htab_p-9) = m1;
		*(htab_p-8) = m1;
		*(htab_p-7) = m1;
		*(htab_p-6) = m1;
		*(htab_p-5) = m1;
		*(htab_p-4) = m1;
		*(htab_p-3) = m1;
		*(htab_p-2) = m1;
		*(htab_p-1) = m1;
		htab_p -= 16;
	} while ((i -= 16) >= 0);
	for ( i += 16; i > 0; i-- )
		*--htab_p = m1;
}

/*
 * Decompress stdin to stdout.  This routine adapts to the codes in the
 * file building the "string" table on-the-fly; requiring no table to
 * be stored in the compressed file.  The tables used herein are shared
 * with those of the compress() routine.  See the definitions above.
 */

z_decompress(ibuf, obuf, size)
char *ibuf;
char *obuf;
int size;
{
	register char_type *stackp;
	register int finchar;
	register code_int code, oldcode, incode;

	open_in(ibuf, size);
	open_out(obuf);

    /*
     * As above, initialize the first 256 entries in the table.
     */
	maxcode = MAXCODE(n_bits = INIT_BITS);
	for ( code = 255; code >= 0; code-- ) {
		tab_prefixof(code) = 0;
		tab_suffixof(code) = (char_type)code;
	}
	free_ent = ((block_compress) ? FIRST : 256 );

	finchar = oldcode = getcode();
	if(oldcode == -1)	/* EOF already? */
		return;			/* Get out of here */
	out_char( (char)finchar );	/* first code must be 8 bits = char */
	stackp = de_stack;

	while ( (code = getcode()) > -1 ) {

		if ( (code == CLEAR) && block_compress ) {
			for ( code = 255; code >= 0; code-- )
				tab_prefixof(code) = 0;
			clear_flg = 1;
			free_ent = FIRST - 1;
			if ( (code = getcode ()) == -1 ) /* untimely death! */
				break;
		}
		incode = code;
	/*
	 * Special case for KwKwK string.
	 */
		if ( code >= free_ent ) {
			*stackp++ = finchar;
			code = oldcode;
		}

	/*
	 * Generate output characters in reverse order
	 */
		while ( code >= 256 ) {
			*stackp++ = tab_suffixof(code);
			code = tab_prefixof(code);
		}
		*stackp++ = finchar = tab_suffixof(code);

	/*
	 * And put them out in forward order
	 */
		do
			out_char ( *--stackp );
		while ( stackp > de_stack );

	/*
	 * Generate the new entry.
	 */
		if ( (code=free_ent) < maxmaxcode ) {
			tab_prefixof(code) = (unsigned short)oldcode;
			tab_suffixof(code) = finchar;
			free_ent = code+1;
		}
	/*
	 * Remember previous code.
	 */
		oldcode = incode;
	}
}

/*
 * NAME:getcode
 *
 * FUNCTION: Read one code from the standard input.  If EOF, return -1.
 * Inputs:
 * 	stdin
 * Outputs:
 * 	code or -1 is returned.
 */

code_int
getcode() {
	register code_int code;
	static int offset = 0, size = 0;
	static char_type buf[BITS];
	register int r_off, bits;
	register char_type *bp = buf;

	if ( clear_flg > 0 || offset >= size || free_ent > maxcode ) {
	/*
	 * If the next entry will be too big for the current code
	 * size, then we must increase the size.  This implies reading
	 * a new buffer full, too.
	 */
		if ( free_ent > maxcode ) {
			n_bits++;
			if ( n_bits == maxbits )
				/* won't get any bigger now */
				maxcode = maxmaxcode;
			else
				maxcode = MAXCODE(n_bits);
		}
		if ( clear_flg > 0) {
			maxcode = MAXCODE (n_bits = INIT_BITS);
			clear_flg = 0;
		}
		for(size = 0 ; size < n_bits ; size++) {
			if (in_char(&buf[size]) == -1) {
				if (size == 0) {
					return(-1);
				} else {
					break;
				}
			}
		}
		offset = 0;
		/* Round size down to integral number of codes */
		size = (size << 3) - (n_bits - 1);
	}
	r_off = offset;
	bits = n_bits;
	/*
	 * Get to the first byte.
	 */
	bp += (r_off >> 3);
	r_off &= 7;
	/* Get first part (low order bits) */
	code = (*bp++ >> r_off);
	bits -= (8 - r_off);
	r_off = 8 - r_off;		/* now, offset into code word */
	/* Get any 8 bit parts in the middle (<=1 for up to 16 bits). */
	if ( bits >= 8 ) {
		code |= *bp++ << r_off;
		r_off += 8;
		bits -= 8;
	}
	/* high order bits. */
	code |= (*bp & rmask[bits]) << r_off;
	offset += n_bits;

	return code;
}
