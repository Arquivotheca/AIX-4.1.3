/* @(#)51	1.5  src/bos/usr/include/xcoff.h, cmdld, bos411, 9428A410j 3/24/93 21:40:22 */
#ifndef _H_XCOFF
#define _H_XCOFF
/*
 * COMPONENT_NAME: (CMDLD) XCOFF object file format definition
 *
 * FUNCTIONS: xcoff.h 
 *
 * ORIGINS: 3, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 /*		EXTENDED COMMON OBJECT FILE FORMAT

	File Organization:

	_________________________________________	INCLUDE FILE
	|_______________HEADER_DATA_____________|
	|					|
	|	File Header			|	"filehdr.h"
	|.......................................|
	|					|
	|	Auxilliary Header Information	|	"aouthdr.h"
	|_______________________________________|
	|	".text" section header		|	"scnhdr.h"
	|.......................................|
	|	".pad" section header		|	''
	|.......................................|
	|	".data" section header		|	''
	|.......................................|
	|	".bss" section header		|	''
	|.......................................|
	|	".loader" section header	|	''
	|.......................................|
	|	".typchk" section header	|	''
	|.......................................|
	|	".debug" section header		|	''
	|.......................................|
	|	".except" section header	|	''
	|.......................................|
	|	".info" section header		|	''
	|_______________________________________|
	|______________RAW_DATA_________________|
	|	".text" section data		|
	|		(rounded to 4 bytes)	|
	|.......................................|
	|	".pad" section data		|
	|		(file alignment)	|
	|.......................................|
	|	".data" section data		|
	|		(rounded to 4 bytes)	|
	|.......................................|
	|	".loader" section data		|	"loader.h"
	|.......................................|
	|	".typchk" section data		|	"typchk.h"
	|.......................................|
	|	".debug" section data		|	"dbug.h"
	|.......................................|
	|	".except" section data		|	"exceptab.h"
	|.......................................|
	|	".info" section data		|
	|_______________________________________|
	|____________RELOCATION_DATA____________|
	|					|
	|	".text" section relocation data	|	"reloc.h"
	|					|
	|.......................................|
	|					|
	|	".data" section relocation data	|	''
	|					|
	|_______________________________________|
	|__________LINE_NUMBER_DATA_____________|
	|					|
	|	".text" section line numbers	|	"linenum.h"
	|					|
	|_______________________________________|
	|________________SYMBOL_TABLE___________|
	|					|
	|	".text", ".data" and ".bss"	|	"syms.h"
	|		section symbols		|	"storclass.h"
	|					|
	|_______________________________________|
	|________________STRING_TABLE___________|
	|					|
	|	long symbol names		|
	|_______________________________________|



		OBJECT FILE COMPONENTS

	STANDARD FILE:
			/usr/include/xcoff.h	

	HEADER FILES:				(included by <xcoff.h>)
			/usr/include/filehdr.h
			/usr/include/aouthdr.h
			/usr/include/scnhdr.h
			/usr/include/loader.h
			/usr/include/typchk.h
			/usr/include/dbug.h
			/usr/include/exceptab.h
			/usr/include/reloc.h
			/usr/include/linenum.h
			/usr/include/syms.h
			/usr/include/storclass.h

	COMPATABLE FILE:
			/usr/include/a.out.h	(alternate to <xcoff.h>
						  includes <xcoff.h>)

 */

#include <filehdr.h>
#include <aouthdr.h>
#include <scnhdr.h>
#include <loader.h>
#include <typchk.h>
#include <dbug.h>
#include <exceptab.h>
#include <reloc.h>
#include <linenum.h>
#include <syms.h>

struct xcoffhdr
{
	struct filehdr	filehdr;
	struct aouthdr	aouthdr;
};
#endif	/* _H_XCOFF */
