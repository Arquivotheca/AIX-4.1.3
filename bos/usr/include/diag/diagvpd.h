/* @(#)40	1.3  src/bos/usr/include/diag/diagvpd.h, cmddiag, bos411, 9428A410j 12/8/92 08:56:22 */
/*
 *   COMPONENT_NAME: CMDDIAG
 *
 *   FUNCTIONS: Diagnostic header file.
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1992
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef TRUE
	#define	TRUE	1
#endif 

#ifndef FALSE
	#define FALSE	0
#endif 

#define MAX_ENTRIES 	64

typedef struct {
		int 	entries;
		char 	*vdat[MAX_ENTRIES];
		} VPDBUF;

extern int get_byte(int, char *, char *);
	
struct key {
	char	*keyword;
	int	(*func)(void);
} vpd_data[] = {
	{ "AD", NULL },			/* Addressing field -		*/
	{ "AT", NULL },			/* Adapter type -		*/
	{ "CD", NULL },			/* Card Id -			*/
	{ "DC", NULL },			/* Date Code -			*/
	{ "DD", NULL },			/* Device driver level - ascii	*/
	{ "DG", NULL },			/* Diagnostic level - ascii	*/ 
	{ "DL", NULL },			/* Drawer level - ascii		*/
	{ "DS", NULL },			/* Displayable message - ascii	*/
	{ "DU", NULL },			/* Drawer Unit			*/
	{ "EC", NULL },			/* EC level - ascii		*/
	{ "FC", NULL },			/* Feature code - ascii		*/
	{ "FN", NULL },			/* FRU number - ascii		*/
	{ "LA", (int (*)(void))get_byte },/* Loadable microcode pointer	*/
	{ "LL", NULL },			/* Loadable microcode level -asc*/
	{ "LO", NULL },			/* Location - ascii		*/
	{ "MF", NULL },			/* Manufacturer - ascii		*/
	{ "NA", (int (*)(void))get_byte },/* Network address		*/
	{ "NX", (int (*)(void))get_byte },/* VPD data address - 2 byte	*/
	{ "PC", NULL },			/* Processor Component		*/
	{ "PI", NULL },			/* Processor ID - ascii		*/
	{ "PN", NULL },			/* Part number - ascii		*/
	{ "RA", (int (*)(void))get_byte },/* ROS code pointer		*/
	{ "RL", NULL },			/* ROS level/id - ascii		*/
	{ "RN", NULL },			/* Rack name - ascii		*/
	{ "RW", (int (*)(void))get_byte },/* R/W adapter reg ptr	*/
	{ "SL", (int (*)(void))get_byte },/* Slot location 2 bytes	*/
	{ "SN", NULL },			/* Serial number - ascii	*/
	{ "SZ", NULL },			/* Size - ascii			*/
	{ "TM", NULL },			/* Machine type/model - ascii	*/
	{ "US", NULL },			/* User data - ascii		*/
	{ "VE", (int (*)(void))get_byte },/* VPD extension - 2 bytes	*/
};

#define NVPDKEYS (sizeof(vpd_data) / sizeof( struct key))

