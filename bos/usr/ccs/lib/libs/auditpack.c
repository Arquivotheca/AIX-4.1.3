static char sccsid[] = "@(#)72	1.7.1.3  src/bos/usr/ccs/lib/libs/auditpack.c, libs, bos411, 9428A410j 11/30/93 16:36:11";
/*
 * COMPONENT_NAME: (LIBS) security library functions
 *
 * FUNCTIONS: auditpack() libs function
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1993
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include	<stdio.h>
#include	<stdlib.h>
#include	<fcntl.h>
#include	<sys/audit.h>
#include	<sys/stat.h>
#include	<sys/errno.h>
#include	<string.h>

#define  	IOSIZE		(32*1024)
#define		COMPRESS_CMD	"/usr/bin/compress -f "
#define		UNCOMPRESS_CMD	"/usr/bin/uncompress -f "

#define		DOT_Z		".Z"
#define		TMP_FILENAME	"/audit/tempfile.%08d"

/*
 * Notes:
 *	1) 32 bit integers are 11 characters long:  -2147483648
 *	2) TMP_FILESIZE and CMDSIZE are larger than they need to be
 *	   (just to be safe).
 */
#define		TMP_FILESIZE	(sizeof(TMP_FILENAME) + 11 + sizeof(DOT_Z))
#define		CMDSIZE		(sizeof(COMPRESS_CMD) + sizeof(UNCOMPRESS_CMD) \
								+ TMP_FILESIZE)


/*
 * NAME:     auditpack
 *
 * FUNCTION: Compresses or uncompresses audit bins.  The beginning of the
 *           input audit bin is pointed at by ibuf.
 *
 * RETURNS:   NULL    - Failed with errno set.
 *           !NULL    - Pointer to the resulting compressed/uncompressed audit
 *                      bin located in allocated memory.
 */
char *
auditpack(int flag, char *ibuf)
{
	struct	aud_bin	*bin = (struct aud_bin *) ibuf;
	struct	stat	sbuf;
	char		*new_bin = (char *) NULL;
	char		*bufp;
	char		*ifile;
	char		*ofile;
	char		*Command;
	ulong_t		ilen;
	ulong_t		size;
	int		ibin = -1;
	int		obin = -1;
	int		cnt;
	int		tmp_errno;
	char		FileName [TMP_FILESIZE];
	char		ZFileName[TMP_FILESIZE];
	char		CmdString[CMDSIZE];


	if (flag == AUDIT_UNPACK)
	{
		ifile   = ZFileName;		/* Compressed */
		ofile   = FileName;		/* Uncompressed */
		Command = UNCOMPRESS_CMD;

		ilen    = bin->bin_plen;	/* Compressed bin len */
		/*
		 * Note: This "if" code is from the original file.  It is
		 * flawed in that there are conditions when the compressed
		 * and uncompressed files may be the same size (but, very
		 * unlikely).  It is a faulty assumption of the design.
		 */
		if (!ilen || (ilen && (bin->bin_len == ilen)))
		{
			errno = EINVAL;
			goto error_exit;
		}
	}
	else
	if (flag == AUDIT_PACK)
	{
		ifile   = FileName;		/* Uncompressed */
		ofile   = ZFileName;		/* Compressed */
		Command = COMPRESS_CMD;

		ilen    = bin->bin_len;		/* Uncompressed bin len */
		/*
		 * Note: This "if" code is from the original file.  It is
		 * flawed in that there are conditions when the compressed
		 * and uncompressed files may be the same size (but, very
		 * unlikely).  It is a faulty assumption of the design.
		 */
		if (bin->bin_plen && (ilen != bin->bin_plen))
		{
			errno = EINVAL;
			goto error_exit;
		}
	}
	else
	{
		errno = EINVAL;
		goto error_exit;
	}

	/*
	 * Create the uncompressed (FileName) and compressed (ZFileName)
	 * file names.
	 */
	sprintf(FileName, TMP_FILENAME, getpid());	/* Uncompressed */
	strcpy(ZFileName, FileName);			/* Compressed */
	strcat(ZFileName, DOT_Z);

	/*
	 * Create the compressed(AUDIT_UNPACK)/uncompressed(AUDIT_PACK) file.
	 */
	if ((ibin = open(ifile, O_CREAT|O_WRONLY|O_RSHARE|O_TRUNC, 0640)) < 0)
	{
		errno = EACCES;
		goto error_exit;
	}

	/*
	 * Write the contents of the input bin (skipping the aud_bin header
	 * and trailer) to the file.  The following code is written for the
	 * case where a filesystem's maximum file size is larger than what
	 * write() can write in a single invocation.
	 */
	bufp = ibuf + sizeof(struct aud_bin); 
	for (size = ilen; size > 0; size-=cnt, bufp+=cnt)
	{
		cnt = (size < IOSIZE) ? size : IOSIZE;
		if ((cnt = write(ibin, bufp, cnt)) <= 0)
		{
			errno = EACCES;
			goto error_exit;
		}
	}

	close(ibin);
	ibin = -1;

	/*
	 * Uncompress(AUDIT_UNPACK)/Compress(AUDIT_PACK) the file
	 * that we just created.
	 */
	strcpy(CmdString, Command);
	strcat(CmdString, FileName); /* Always use the uncompressed file name */

	if (system(CmdString))
	{
		unlink(ifile);	/* Don't know what the file is currently */
		unlink(ofile);	/* named, so unlink both.                */
		errno = EINVAL;
		goto error_exit;
	}

	/*
	 * Open the uncompressed(AUDIT_UNPACK)/compressed(AUDIT_PACK) file.
	 */
	if ((obin = open(ofile, O_RDONLY|O_RSHARE)) < 0 || fstat(obin, &sbuf))
	{
		unlink(ofile);
		errno = EACCES;
		goto error_exit;
	}

	/*
	 * If AUDIT_UNPACK, check the unpacked length with the original length.
	 */
	if (flag == AUDIT_UNPACK && sbuf.st_size != bin->bin_len)
	{
		errno = EINVAL;
		goto error_exit;
	}

	/*
	 * Allocate enough memory to hold the new file.  Normally a
	 * compressed file is smaller than an uncompressed.  But, if the
	 * uncompressed data is a small amount (e.g., 0 - 32 bytes),
	 * it is possible for the compressed data to be larger than the
	 * uncompressed data.  A 0 byte uncompressed file produces a
	 * 3 byte compressed file.
	 */
	new_bin = (char *) malloc(((size_t) sbuf.st_size) +
						2*sizeof(struct aud_bin));
	if (!new_bin)
	{
		errno = ENOSPC;
		goto error_exit;
	}

	/*
	 * Copy the file into new_bin (skipping the aud_bin header).
	 * The following code is written for the case where a filesystem's
	 * maximum file size is larger than what read() can read in a
	 * single invocation.  It also prevents new_bin overrun.
	 */
	bufp = new_bin + sizeof(struct aud_bin); 
	for (size = sbuf.st_size; size > 0; size-=cnt, bufp+=cnt)
	{
		cnt = (size < IOSIZE) ? size : IOSIZE;
		if ((cnt = read(obin, bufp, cnt)) <= 0)
		{
			errno = EACCES;
			goto error_exit;
		}
	}

	close(obin);
	obin = -1;
	unlink(ofile);

	/*
	 * Copy the original header into new_bin.
	 */
	memcpy(new_bin, ibuf, sizeof(struct aud_bin));

	/*
	 * Copy the original trailer into new_bin.
	 */
	bin = (struct aud_bin *)(ibuf + sizeof(struct aud_bin) + ilen);
	memcpy(bufp, bin, sizeof(struct aud_bin));

	/*
	 * Update both the header and trailer bin_plen values in new_bin.
	 */
	((struct aud_bin *) new_bin)->bin_plen =
	((struct aud_bin *)    bufp)->bin_plen =
				(flag == AUDIT_UNPACK) ? 0 : sbuf.st_size;

	errno = 0;		/* Because the auditcat command expects it. */
	return(new_bin);

error_exit:
	tmp_errno = errno;
	if (ibin >= 0)
	{
		close(ibin);
		unlink(ifile);
	}
	if (obin >= 0)
	{
		close(obin);
		unlink(ofile);
	}
	if (new_bin)
		free((void *) new_bin);

	errno = tmp_errno;
	return((char *) NULL);
}
