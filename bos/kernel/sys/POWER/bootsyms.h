/* @(#)01	1.13  src/bos/kernel/sys/POWER/bootsyms.h, sysvmm, bos411, 9428A410j 5/5/94 19:19:53 */
#ifndef _H_BOOTSYMS
#define _H_BOOTSYMS

/*
 * COMPONENT_NAME: (SYSSI) System Initialization
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1990, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

/*
		    IPL DATA LAYOUT

	========================================= (REAL 0)
	|		header			|
    --> ========================================= (ORG 0), &pin_obj_start
   |	|      pinned kernel code + data	|
   | K	----------------------------------------- &page_obj_start, &pin_obj_end
   | E	|     pageable kernel code + data	|
   | R	----------------------------------------- &init_obj_start, &page_obj_end
   | N	|   kernel initialization code + data	|
   | E	========================================= g_toc, &init_obj_end
   | L	|	    Table Of Contents		|
 --+--> ========================================= &low_com_start, endtoc
|  |	|	     loader section		|
|   --> ========================================= header_offset
|	|	       PAD to 4K		|
| LOW	========================================= ram_disk_start
|	|					|
|COMMON |	       RAM Disk 		|
|	|					|
|	========================================= ram_disk_end
|	|		  ...			|
 -----> ========================================= pin_com_start, &low_com_end
|	|	     pinned common		|
|	----------------------------------------- &page_com_start, &pin_com_end
|COMMON |	    pageable common		|
|	----------------------------------------- &init_com_start, &page_com_end
|	|	      init common		|
 -----> ========================================= &init_com_end
	|		   .			|
	|		   .			|
	|		   .			|
	========================================= ipl_cb (HIGHEST ADDRESS
	|	   IPL Control Block		|	  POSSIBLE)
	=========================================
	|		  ...			|
	========================================= (END OF REAL MEMORY)
*/

/*
 * Information saved in page 0 by the tool that creates the IPL data.
 */
extern int	header_size;		/* Size of header	     */
extern int	header_offset;		/* Offset to copy of header  */
extern int	ram_disk_start; 	/* Offset to RAM disk start  */
extern int	ram_disk_end;		/* Offset to RAM disk end    */
extern int	dbg_avail;		/* debugger available flags	 */
extern int	base_conf_start; 	/* Offset to base config start   */
extern int	base_conf_end;		/* Offset to base config end     */
extern int	base_conf_disk;		/* base config area disk address */

/*
 * Information saved by routine 'start1.s'.
 */
extern int	ipl_cb; 		/* Offset to IPL control blk  */

/*
 * Information saved by routine 'start.s'.
 */
extern caddr_t	g_toc;			/* Start of TOC 	     */

/*
 * Symbols that delineate the sections of the IPL data.
 */
extern int	pin_obj_start;		/* Start of pinned kernel    */
extern int	pin_obj_end;		/* End "		     */
extern int	page_obj_start; 	/* Start of pageable kernel  */
extern int	page_obj_end;		/* End "		     */
extern int	init_obj_start; 	/* Start of kernel init      */
extern int	init_obj_end;		/* End "		     */

extern int	endtoc; 		/* End of the TOC	     */

extern int	low_com_start;		/* Start of low common	     */
extern int	low_com_end;		/* End "		     */

extern int	pin_com_start;		/* Start of pinned common    */
extern int	pin_com_end;		/* End "		     */
extern int	page_com_start; 	/* Start of pageable common  */
extern int	page_com_end;		/* End "		     */
extern int	init_com_start; 	/* Start of init common      */
extern int	init_com_end;		/* End "		     */

/*
 * This variable gives the location of a page in the kernel containing
 * code and data which must be executed/accessed in non-privileged mode.
 */
extern int	nonpriv_page;		/* Non-privileged page.      */

/*
 * This variable gives the location of the first page in the kernel
 * which must be fetch protected.
 */
extern int      fetchprot_page;         /* the first fetch protected page */
#endif /* _H_BOOTSYMS */




