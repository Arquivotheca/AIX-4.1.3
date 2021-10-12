/* @(#)17	1.5  src/bos/usr/ccs/bin/ld/bind/ifiles.h, cmdld, bos41B, 9505A 1/23/95 15:58:22 */
#ifndef Binder_IFILES
#define Binder_IFILES
/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

extern IFILE	*first_ifile;

extern void	reserve_new_ifiles(int); /* Force allocation of continguous
					    IFILE entries when the next
					    IFILE is allocated.  */
extern IFILE	*ifile_open_and_map(char *); /* Open and map file (or read file
						into anonymous mapped memory,
						if it can't be mapped).*/
extern RETCODE	ifile_reopen_remap(IFILE *); /* Re-open and re-map a file that
						may have been closed or
						unmapped. */
extern void	ifile_close_for_good(IFILE *);
extern int	ifile_close_one(void);	/* Close any IFILE. */
extern void	ifile_close_all(void);	/* Close all ifiles for good */
extern int	get_file_objects_count(IFILE *);
extern int	free_segments(int);

#ifdef READ_FILE
extern int	fseek_read(IFILE *, off_t, void *, size_t);
extern int	safe_fread(void *, size_t, IFILE *);
extern int	safe_fseek(IFILE *, off_t, int);
#endif

#endif /* Binder_IFILES */
