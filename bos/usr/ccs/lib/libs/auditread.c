static char sccsid[] = "@(#)28	1.12.1.1  src/bos/usr/ccs/lib/libs/auditread.c, libs, bos411, 9428A410j 1/30/92 22:13:34";
/*
 * COMPONENT_NAME: (SYSAUDIT) Auditing Management
 *
 * FUNCTIONS: auditread() libc function
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
#include	<stdio.h>
#include	<sys/errno.h>
#include	<sys/audit.h>
#include	<ctype.h>

static	char	*a_rec;
static	int	a_alloc;
static	int	a_recp;
static	int	a_end;
static	int	a_bin;

static
rd(input, siz)
FILE	*input;
int	siz;{

	/* 
	 * Check for data already available 
 	 */

	if(a_recp + siz < a_end){
		return (0);
	}

	if(!a_bin){

		int	cnt;
		struct	aud_bin	bin;

		/* 
		 * Check that we have enough room in the buffer 
		 * for the data 
		 */

		if(a_recp + siz > a_alloc){
			a_alloc = a_recp + siz;
			a_rec = (char *)realloc(a_rec, a_alloc);

			if(!a_rec){
				fprintf (stderr, "out of memory\n");
				exit (1);
			}
		}
		
		cnt = fread(&(a_rec[a_end]), (size_t) 1,
		(size_t) (a_recp + siz) - a_end, input);

		if (cnt > 0)a_end += cnt;
		else return(-1);
	}
	return (((a_recp + siz) <= a_end) ? 0 : -1);
}

/* 
 * Read a bin from the input stream on return.
 */

static
getbin(input)
FILE	*input;{

	struct	aud_bin	bh;
	struct	aud_bin bt;
	int	sav_recp;
	int	bin_len;
	int	packed;
	char	*unpacked_rec;

	packed = 0;
	sav_recp = a_recp;
	if(rd(input, sizeof (struct aud_bin))){
		goto	fail;
	}

	/* 
	 * Verify (stream of bins) assumption 
	 */

	bcopy(&(a_rec[a_recp]), &bh, sizeof (bh));
	if(bh.bin_magic != AUDIT_MAGIC){
		goto	fail;
	}

	/* skip the bin header */
	a_recp += sizeof (struct aud_bin);

	if(bh.bin_plen && (bh.bin_plen != bh.bin_len)){
		packed++;
		bin_len = bh.bin_plen;
	}
	else bin_len = bh.bin_len;

	if(rd(input, bin_len + sizeof (struct aud_bin))){
		goto	fail;
	};

	bcopy (&(a_rec[a_recp + bin_len]), &bt, sizeof (bt));
	if(bt.bin_magic != AUDIT_MAGIC){
		goto	fail;
	}
	if (!bt.bin_tail){
		goto	fail;
	}
	if((bh.bin_len != bt.bin_len) || (bh.bin_plen != bt.bin_plen)){
		goto	fail;
	}

	/* 
	 * Unpack bin if necessary 
	 */

	if(packed){

		unpacked_rec = (char *)auditpack(1, a_rec+sav_recp);
		bcopy(unpacked_rec, &bh, sizeof (bh));
		a_alloc = bh.bin_len + 2 * sizeof (struct aud_bin);
		a_recp = sizeof (struct aud_bin);
		a_rec = unpacked_rec;
		a_end = a_alloc;

	}

	/* 
	 * Consume the bin tail! 
	 */

	a_end -= sizeof (struct aud_bin);
	return (0);
fail:
	return(-1);
}

static
isevent(s, len)
char	*s;
int	len;{

	int	i;
	char	c;

	for (i = 0; 1; i++){
		if (i >= len){
			return (0);
		}
		c = s[i];
		if (c == '\0')
			break;
		if (!isascii(c))
		{
			return (0);
		}
	}

	for ( ; i < len; i++)
	{
		if (s[i] != '\0')
		{
			return (0);
		}
	}

	return (1);
}

static
isprog(s, len)
char	*s;
int	len;{

	return (1);

}

static
gethd (input, ah)
FILE	*input;
struct	aud_rec	*ah;{

	/* 
	 * Assure have a header's worth of data in the audit 
	 * trail buffer 
	 */

	if(rd(input, sizeof (struct aud_rec))){
		return (-1);
	}
	bcopy (&(a_rec[a_recp]), ah, sizeof (struct aud_rec));

	if(!isevent (ah->ah_event, sizeof (ah->ah_event))){
		return(-1);
	}
	if(!isprog(ah->ah_name, sizeof (ah->ah_name))){
		return (-1);
	}
	return (0);
}

/* 
 * Read audit record from bin/stream ...  
 */

static
char *
getaud(input, ah)
FILE	*input;
struct	aud_rec	*ah;{

	int	sav_recp;

	sav_recp = a_recp;

	/* 
	 * Read next header 
	 */

	if(gethd(input, ah)){
		a_recp = sav_recp;
		return(NULL);
	}
	a_recp += sizeof (struct aud_rec);

	/* 
	 * Read next tail 
	 */

	if(ah->ah_length){

		if(rd(input, ah->ah_length)){
			a_recp = sav_recp;
			return(NULL);
		}
	}

	a_recp += ah->ah_length;

	return (&(a_rec[sav_recp + sizeof (struct aud_rec)]));
}

char *
auditread(input, ah)
FILE *input;
struct	aud_rec	*ah;{

	char	*p;
	int	nskip;

	if(input == (FILE *)NULL){

		errno = EBADF;
		return(NULL);

	}
	
	if(a_rec == NULL){
		a_alloc = 20 * 1024;
		a_rec = (char *)malloc(a_alloc);
		if(!a_rec){
			errno = ENOSPC;
			return(NULL);
		}
		a_recp = a_end = a_bin = 0;
	};
	
	nskip = 0;

	while(1){

		/* 
		 * Reset pointers if buffer is empty 
		 */

		if(a_recp >= a_end){
			a_recp = a_end = 0;
			a_bin = 0;
		}

		/* 
		 * Try to fill the buffer with a bin from the trail 
		 */

		if (!a_bin){

			/* 
			 * Copy data down to allow reading bin 
			 */

			if (a_recp != 0){
				bcopy (&(a_rec[a_recp]), a_rec, 
				a_end - a_recp);

				a_end -= a_recp;
				a_recp = 0;
			}

			if(getbin(input) == 0){
				a_bin = 1;
				if(nskip){
					fprintf(stdout, 
					"%d bytes to bin header\n", 
					nskip);

					fflush (stdout);
					nskip = 0;
				}

				/* 
				 * Normally, we don't want to
			 	 * iterate here but an empty (!) bin 
				 * would cause us to quit.  
				 */

				if (a_recp == a_end)
					continue;
			}
		}

		/* 
		 * if at EOF, return NULL 
		 */

		if(a_recp >= a_end){
			if (nskip){
				fprintf (stdout, "%d bytes to EOF\n", 
				nskip);

				fflush (stdout);
				nskip = 0;
			}
			errno = 0;
			return(NULL);
		}

		/* 
		 * Extract an audit record from the bin/stream 
		 */

		if(p = getaud(input, ah)){
			if (nskip){
				fprintf (stdout, 
				"%d bytes to audit record\n", nskip);

				fflush (stdout);
				nskip = 0;
			}
			return(p);
		}

		/* 
		 * Discard a byte of input data 
		 */

		if (nskip == 0){
			fprintf (stdout, "skipping ... ");
			fflush (stdout);
		}
		nskip++;
		a_recp++;
	}
}
