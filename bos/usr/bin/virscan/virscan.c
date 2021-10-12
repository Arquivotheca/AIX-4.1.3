static char sccsid[] = "@(#)31	1.11  src/bos/usr/bin/virscan/virscan.c, cmdsvir, bos411, 9435D411a 9/2/94 17:21:13";
/*
 *   COMPONENT_NAME: CMDSVIR
 *
 *   FUNCTIONS: pr_usage
 *		pr_short_usage
 *		pr_examples_of_usage
 *		invalid_option
 *		sexit
 *		charhextobyte
 *		pr_hash_tables
 *		init_hash_tables
 *		free_hash_tables
 *		install_sig
 *		output_logfile_line
 *		warning_msg
 *		warn_if_warranted
 *		scan_for_1260
 *		test_memory
 *		get_signatures
 *		scan_via_listfile
 *		non_fatal_error
 *		test_system_boot_sector
 *		test_master_boot_record
 *		test_drivename
 *		main
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Main() and supporting routines which aren't particularly operating
 * system specific are in this module.
 */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#ifdef _AIX
	#include <errno.h>
#endif

#ifdef BOOTS
	#include <conio.h>	/* getch() and kbhit() 		*/
#endif

#ifdef M_I86
	#include <io.h>		/* open(), read(), close() 	*/
	#include <fcntl.h>
#endif

#include "vsdefs.h"
#include "vstypes.h"

#ifdef MSG			/* AIX message catalogs         */
	#include "virscan_msg.h"
	#include <locale.h>
	#include <nl_types.h>

	nl_catd catd;
	#define MSGSTR(Num, Str)        catgets(catd,MS_VIRSCAN, Num, Str)
#else
	#include "vsmsg.h"
	#define MSGSTR(C,D)             message(C)
#endif

#include "default.h"

/*
 * Make sure we retrieve correct the value of "product_version" 
 * from the message file before using it.
 */
#define Product_Version (MSGSTR(VERSION_FLAG, MSG_VERSION_FLAG),product_version)

boolean product_version = TRUE;

static char *copyright = "(C) Copyright IBM Corporation 1989, 1990, 1994\n";

boolean
signature_has_been_found = FALSE;   	/* SIG_FOUND takes precedence over */
					/* other errors */
#ifndef _AIX
boolean
have_tested_master_boot_rec = FALSE;	/* Make sure that master boot record */
					/*   is only tested once. */
#endif

boolean
quiet = FALSE;                      	/* If this boolean is set TRUE */
					/*  via a command line switch, */
					/*  only warning messages (if any) or */
					/*   error messages (if any), */
					/*   the byte and file count */
					/*   messages will be displayed, and */
					/*   'verbose' is forced FALSE. */

boolean
very_quiet = FALSE;                 	/* Don't display *any* warnings about */
					/*   signatures that are found */

boolean
verbose = FALSE;                    	/* If this boolean is set TRUE */
					/*   via a command line switch, */
					/*   file names will be listed as */
					/*   they are processed, and 'quiet' */
					/*   is forced FALSE */
boolean
continue_if_open_error = FALSE;     	/* If this boolean is set TRUE */
					/*  via a command line switch, */
					/*  virscan will continue the scan */
					/*  even if it cant open a particular */
					/*   for scanning */

static boolean
continue_if_access_denied = FALSE;  	/* If set with command line option */
					/*   -cad, virscan will continue the */
					/*   scan even if it isn't allowed to */
					/*   scan down some subdirectory */
					/*   subtree. */

boolean
cr_works = FALSE;                    	/* If this is set to FALSE with the */
					/*  command line option "-np", then */
					/*  virscan reverts to the old */
					/*  summary behavior, without the */
					/*  display of object's names as they */
					/*  are scanned. */
					/* If carriage return doesnt works as */
					/*  it is supposed to, for example */
					/*  in the VM/CMS version, this */
					/*  switch should be initialized to */
					/*  FALSE when compiling that version */

boolean
list_positives = FALSE;             	/* If this boolean is set TRUE */
					/*  via a command line switch, */
					/*  program will generate a list */
					/*  of names of files in which a */
					/*  viral was found */
char *positives_filename = NULL;    	/* Routine 'warning_msg()' writes */
					/*   names of files in which were */
					/*   found viral signatures to this */
					/*   file, if the list_positives  */
					/*   boolean is TRUE */
boolean
create_logfile = FALSE;             	/* If this boolean is set TRUE */
					/*  via a command line switch, */
					/*  program will generate a log file, */
					/*  with at least one line per object */
					/*  scanned. (Multiple lines if there */
					/*   were multiple virus hits in the */
					/*   object */
char *logfilename = NULL;           	/* If create_logfile is TRUE, then a */
					/*   logfile by this name is created */
					/*   and maintained during the scan */

boolean
always_warn = FALSE;               	/* If this boolean is set, always */
					/*   warn if a signature is found, */
					/*   even if the indicated virus is */
					/*   not normally found in */
					/*   executables of the executable's */
					/*   type. */
boolean
explicit_searches_only = FALSE;     	/* If this boolean is set via */
					/*  undocumented option -e, searches */
					/*  are only done of things specified */
					/*   on the command line. */

#ifndef _AIX
boolean
display_bootsector = FALSE;         	/* If this boolean is set via */
					/*  undocumented option -vv, */
					/*  a hex dump of a boot sector is */
					/*  displayed after it is read. */
					/*  Both master boot records and */
					/*  system boot sectors are displayed */
#endif

boolean
mutation_support = FALSE;           	/* If this boolean is set via */
					/*   undocumented option -mf or */
					/*   option -m, */
					/*   crude mutant detection support */
					/*   is enabled. The support involves */
					/*   breaking signatures into 11 byte */
					/*   fragments. */
boolean
tolerance_allowed = TRUE;           	/* If this boolean is set via */
					/*  undocumented option -mt */
					/*  or option -m, */
					/*  some mismatched bytes are allowed */

boolean
more_tolerance_allowed = FALSE;     	/* If this boolean is set via */
					/*   undocumented option -mm */
					/*   even more mismatched bytes are */
					/*   allowed than if -mt is specified */

boolean
default_mutant_detection = TRUE;    	/* Unless this boolean is set FALSE */
					/* with any of the mutant detection */
					/* switches (-m, -mm, -mt, -mf), */
					/* only the default */
					/* mutant detection will be used. */

boolean
beep_if_sig_found = TRUE;           	/* Normally, virscan will beep if a */
					/*   virus signature is found. This */
					/*   beep can be turned off with */
					/*   undocumented command line option */
				 	/*   -nb */

#ifndef _AIX
boolean
do_self_test = TRUE;                	/* Self-test can be turned off with */
					/* command line option "-nst" */
boolean
do_memory_scan = TRUE;             	/* Memory scanning can be turned off */
					/* with command line option "-nms" */
boolean
do_high_memory_scan = TRUE;         	/* Memory scanning above A000 can be */
					/* turned off with command line */
					/* option "-nhms" */
boolean
do_mbr_scan = TRUE;                 	/* Automatic scanning of the master */
					/*   boot record when in DOS mode */
					/*   and a drive letter is >= C: is */
					/*   specified can be turned off with */
					/*  the -NMBRS switch */
#endif /* if not AIX */

boolean
beep_loop_if_sigsfound = FALSE;    	/* If set with command line 'z', */
					/*   instead of terminating normally, */
					/*   virscan will beep every second, */
					/*   waiting for user input, if any */
					/*   virus signatures were */
					/* found during the scan. */

#ifndef _AIX
boolean
display_license_agreement = TRUE;   	/* Can be set false with cmd line */
					/*   option */
#else
boolean
display_license_agreement = FALSE;
#endif

char
file_specification[128] = { 0 };    	/* The default file specification is */
					/*   set if no non-default */
					/*   specification is found on the */
					/*   first pass through the command */
					/*   line. Can be set to something  */
					/*   else with command line option -w */

int sigfound_cnt=0;                 	/* The number of valid viral */
					/*   signatures found. At program */
					/*   termination, the total count of */
					/*   signatures found */

int object_sigfound_cnt=0;          	/* The number of valid viral */
					/*   signatures found in the object */
					/*   that was just scanned.  When */
					/*   scanning an object, */
					/*   it is the running total of */
					/*   signatures found. */

int found_precise_match=FALSE;       	/* Set to TRUE if a signature was */
					/*   matched  precisely during the */
					/*   scan of an object */

sigdat *sig_with_min_diffs=NULL;    	/* Set to point to the signature data */
					/*   structure with the fewest */
					/*   mismatches. */

int min_mismatched_bytes=INT_MAX;   	/* The number of mismatched bytes for */
					/*   the signature with the fewest */
					/*   mismatches. */

long offset_swmd=0l;                	/* Offset into object where the */
					/*   signature data structure with */
					/*   the fewest mismatches was found. */

char *obj_name = NULL;              	/* These are set if a signature is */
int obj_type = T_FILE;              	/*   partially matched. */

boolean obj_message_output = FALSE; 	/* Has a message been output for this */
					/*   object yet? */

int infected_object_cnt=0;          	/* Rough count of the number of */
					/*   infected objects (objects in */
					/*   which signatures were found) */
					/*   count; An object is counted as */
					/*   long as it's name isn't */
					/*   the same as the name of a */
					/*   previous object; this means */
					/*   that if an infected drive is */
					/*   scanned twice, infected objects */
					/*   may be counted */
					/*   twice. */

int prevnamelen = 0;                	/* For pretty printing of filenames */

char tmplinebuf[SIZE_TMP_LINE_BUF] = { 0 };
					/* fgets() reads to this buffer */

char tmpbuf[SIZE_TMP_LINE_BUF/2] = { 0 }; 
					/* unprocessed signatures go here */

/*
 * If PC version, we can use a larger "first_sig_chars" table.
 * (80X8X stores words low-order in low memory byte)
 * Otherwise, use a "first_sig_chars" table the same size as the hash
 * table.
 */
#ifdef M_I86
#define SIZE_FSC 8192
#define FSC_MASK 0x1FFF
#define SIZE_HT  1024
#define HT_MASK  0x03FF
#else
#define SIZE_FSC 256
#define FSC_MASK 0xFF
#define SIZE_HT  256
#define HT_MASK  0xFF
#endif

byte first_sig_chars[SIZE_FSC];     	/* This table is a enables a quick */
					/*   test of whether or not a byte */
					/*   is the first byte of a signature.*/
					/*   It is a performance hack that */
					/*   is only particularly significant */
					/*   if the pointers in 'hash_table' */
					/*   are 4 byte pointers and the */
					/*   machine can compare */
					/*   only 2 bytes at a time */
sigdat *hash_table[SIZE_HT];        	/* 'Hash value' is the first byte of */
					/*   signature. Test for null may be */
					/*   more expensive than test of a */
					/*   byte, hence other table */

byte workbuf[SIZE_WORK_BUF+MAX_SIZE_SIGN-1];
					/* Buffer for file reads. */
					/* Note that the size is larger */
					/* than a power of two by the size */
					/* (max) of a signature. This is */
					/* so that a scan past the end of the */
					/* buffer works */

#ifndef _AIX
int master_brecord_count = 0;       	/* Count master boot records scanned */
int system_bsector_count = 0;       	/* Count system boot sectors scanned */
#endif 

#define OUTPUT_SZ 256

time_t begin_time;                  	/* Time at which scan started */
time_t end_time;                    	/* Time at which scan completed */
time_t scan_starttime;             	/* Time at which actual scan started */

#ifndef _AIX
boolean
have_scanned_system_memory = FALSE; 	/* Set to true when system memory is */
					/*   scanned. */
#endif 

int sig_count = 0;                  	/* Total count of signatures stored */
					/*    in hash table */
int whole_sig_count = 0;            	/* Total count of whole signatures */
int frag_sig_count = 0;            	/* Total count of signatures fragments*/

#if FREQ_COUNTS
long ccounts[256];                  	/* Development use only */
#endif

unsigned long total_scanlength = 0l;	/* keep statistics */
unsigned int filecnt = 0;
unsigned int OS_major_ver = 0;
unsigned int OS_minor_ver = 0;

#ifndef _AIX
boolean  user_spec_removable[26] =
{ 
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
	    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
	    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
	    FALSE, FALSE };
#endif

/*
 * Function prototypes. Prototypes should be set to true.
 */
#if BOOTS
#define getch() my_getch()
int my_getch(void);
#endif

#include "vsutil.h"

void pr_short_usage(void);
void pr_usage(void);
void pr_examples_of_usage(void);
void invalid_option(char *option);
void sexit(int rc);
byte charhextobyte(byte charhex);
int  stringtohex(byte *hexstring, char *bytestring);
void pr_signature(sigdat *sig);
void pr_hash_tables(void);
void init_hash_tables(void);
void free_hash_tables(void);
void install_sig(sigdat *sig, boolean is_frag);
void scan_for_1260(unsigned int scan_length,
		char *filename,
		boolean is_exe_type,
		long base,
		int object_type);
void scanworkbuf(register unsigned int scan_length,
		char *filename,
		boolean lastbuf,
		boolean is_exe_type,
		long base,
		boolean do_scan_for_1260,
		int object_type);
void main(int argc, char **argv);
void output_positive_filename(char *filename);
void output_logfile_line(sigdat *sd,
		char *filename,
		long signature_offset,
		int mismatched_bytes_count);
void warning_msg(sigdat *sd,
		char *filename,
		long signature_offset,
		int mismatched_bytes_count,
		int object_type);
void warn_if_warranted(sigdat *sd,
		char *filename,
		boolean is_exe_type,
		long signature_offset,
		int mismatch_bytes_count,
		int object_type);
void test(char *testfilename);
byte *far_to_near_memcpy(register byte *dst,
		unsigned int src_seg,
		register unsigned int cnt);
void get_signatures(char *sigfilename,
		boolean no_default_dir,
		boolean test_for_modifications,
		boolean fail_if_not_found);
char *message(unsigned int msg_ind);
void scan_via_listfile(char *listfile);

#if BOOTS
void test_system_boot_sector(char *drivename,
		boolean continue_if_error,
		boolean pause_if_removable);
void test_master_boot_record(unsigned int physical_drive,
		boolean continue_if_error);
#endif

void test_drivename(char *drivename);

#if SEARCH_DRIVE_ENABLE
extern void traverse(char *pathname,
		char *filespec,
		void (*action)(char *),
		void (*error_action)(unsigned short));
extern void norm_error(unsigned int rc);
#endif

#if SEARCH_DRIVE_ENABLE /* DOS version only. */
void test_memory(void);
#endif

#if BOOTS
extern void get_system_boot_sector(char *drive,
		char *buffer,
		boolean verbose,
		void (*error_action)(void));
extern boolean is_valid_drive_letter(char drive_letter);
extern boolean RealMode(void);
extern unsigned int get_master_boot_record(unsigned int physical_drive,
		char *buffer);
extern void get_version(unsigned int *OS_major_ver, unsigned int *OS_minor_ver);
extern void get_pathname(char *pathname);
#endif

char *strdup(char *);

#if WHITESMITHS
char * strupr(char *string);
char * strstr(char *string1, char *string2);
#endif

#if SELF_TEST
boolean file_has_been_modified(char *filename,
		byte workbuf[],
		unsigned int len_workbuf,
		int max_len_overbite);
#endif

/*
 * Detailed usage information.
 */
void
pr_usage(void)
{
	#if BOOTS
	char *press_enter = 	MSGSTR(PRESS_ANY_KEY, MSG_PRESS_ANY_KEY);
	#else
	char *press_enter = 	MSGSTR(PRESS_ENTER, MSG_PRESS_ENTER);
	#define getch getchar
	#endif

	printf(MSGSTR(USAGE_01, MSG_USAGE_01));
	printf(MSGSTR(USAGE_02, MSG_USAGE_02));

	#if SEARCH_DRIVE_ENABLE
	printf(MSGSTR(USAGE_DOPTION, MSG_USAGE_DOPTION));
	#endif

	printf(MSGSTR(USAGE_LOPTION, MSG_USAGE_LOPTION));
	printf(MSGSTR(USAGE_TOPTION, MSG_USAGE_TOPTION));

	#if BOOTS
	printf(MSGSTR(USAGE_BOPTION, MSG_USAGE_BOPTION));
	if (RealMode())
		printf(" || -bXX");
	#endif

	printf("\n");
	printf(MSGSTR(USAGE_SPQOPTION, MSG_USAGE_SPQOPTION));

	#ifndef _AIX
	if (Product_Version)
		printf(" [-nla]");
	#endif

	printf("\n\t");

	#ifdef _AIX
	printf(MSGSTR(USAGE_WAOPTION, MSG_USAGE_WAOPTION));
	#endif

	#if SEARCH_DRIVE_ENABLE
	printf(MSGSTR(USAGE_WAROPTION, MSG_USAGE_WAROPTION));
	#if BOOTS
	printf("[-vv] ");
	#endif
	#endif

	#ifdef _AIX
	printf("[-m]\n\t[-qq]");
	#else
	printf("[-m] [-guru]\n");
	printf("\t[-np] [-nst] [-c]");
	#endif

	#if BOOTS
	printf(" [-z] [-e]");
	if (RealMode())
		printf(" [-nhms] [-nms] [-mem] [-nmbrs]");
	#endif

	#if SEARCH_DRIVE_ENABLE
	printf(" [-cad]");
	#endif

	printf(" [-nmut]");

	printf("\n");
	printf(MSGSTR(USAGE_03, MSG_USAGE_03));

	#if SEARCH_DRIVE_ENABLE
	printf(MSGSTR(USAGE_DRIVE01, MSG_USAGE_DRIVE01));
	#endif

	#if BOOTS
	printf(MSGSTR(USAGE_B01, MSG_USAGE_B01));
	if (RealMode())
		printf(MSGSTR(USAGE_B02, MSG_USAGE_B02));
	#endif

	printf(MSGSTR(USAGE_L, MSG_USAGE_L));
	printf(MSGSTR(USAGE_T, MSG_USAGE_T));

	#ifdef _AIX
	printf(MSGSTR(USAGE_PAIX, MSG_USAGE_PAIX));
	#else
	printf(MSGSTR(USAGE_P, MSG_USAGE_P));
	#endif

	printf(MSGSTR(USAGE_Q, MSG_USAGE_Q));

	#ifdef _AIX
	printf(MSGSTR(USAGE_VAIX, MSG_USAGE_VAIX));
	#else
	printf(MSGSTR(USAGE_V, MSG_USAGE_V));
	#endif

	#ifndef _AIX
	fprintf(stderr, press_enter);
	fprintf(stderr, cr_works ? "\r" : "\n");
	getch();
	#endif

	#ifdef _AIX
	printf(MSGSTR(USAGE_SAIX, MSG_USAGE_SAIX));
	#else
	printf(MSGSTR(USAGE_S, MSG_USAGE_S));
	#endif

	#if SEARCH_DRIVE_ENABLE
	printf(MSGSTR(USAGE_A,  MSG_USAGE_A));
	#endif

	#ifdef _AIX
	printf(MSGSTR(USAGE_AAIX, MSG_USAGE_AAIX));
	#endif

	printf(MSGSTR(USAGE_M, MSG_USAGE_M));

	#if SEARCH_DRIVE_ENABLE
	printf(MSGSTR(USAGE_W, MSG_USAGE_W));
	#endif

	#ifdef _AIX
	printf(MSGSTR(USAGE_WAIX, MSG_USAGE_WAIX));
	#endif

	#ifndef _AIX
	printf(MSGSTR(USAGE_G, MSG_USAGE_G));
	#endif

	#if SEARCH_DRIVE_ENABLE
	if (RealMode())
		printf(MSGSTR(USAGE_GMEM, MSG_USAGE_GMEM));
	#endif

	#if BOOTS
	printf(MSGSTR(USAGE_VV, MSG_USAGE_VV));
	#endif

	#ifndef _AIX
	fprintf(stderr, press_enter);
	fprintf(stderr, cr_works ? "\r" : "\n");
	getch();
	#endif

	#if SEARCH_DRIVE_ENABLE
	printf(MSGSTR(USAGE_R, MSG_USAGE_R));
	#endif

	#ifndef _AIX
	printf(MSGSTR(USAGE_NP, MSG_USAGE_NP));
	#endif

	#if SELF_TEST
	printf(MSGSTR(USAGE_NST, MSG_USAGE_NST));
	#endif

	#ifndef _AIX
	if (Product_Version)
		printf(MSGSTR(USAGE_NLA, MSG_USAGE_NLA));
	#endif

	#if SEARCH_DRIVE_ENABLE
	if (RealMode())
	{
		printf(MSGSTR(USAGE_NHMS, MSG_USAGE_NHMS));
		printf(MSGSTR(USAGE_NMS,  MSG_USAGE_NMS));
		printf(MSGSTR(USAGE_MEM,  MSG_USAGE_MEM));
	}

	fprintf(stderr, press_enter);
	fprintf(stderr, cr_works ? "\r" : "\n");
	getch();
	#endif /* if SEARCH_DRIVE_ENABLE */

	printf(MSGSTR(USAGE_QQ, MSG_USAGE_QQ));

	#ifndef _AIX 
	printf(MSGSTR(USAGE_C, MSG_USAGE_C));
	#endif

	#if BOOTS
	printf(MSGSTR(USAGE_Z, MSG_USAGE_Z));
	printf(MSGSTR(USAGE_E, MSG_USAGE_E));
	if (RealMode())
		printf(MSGSTR(USAGE_NMBRS, MSG_USAGE_NMBRS));
	#endif

	#if BOOTS
	printf(MSGSTR(USAGE_CAD, MSG_USAGE_CAD));
	#endif

	printf(MSGSTR(USAGE_NMUT, MSG_USAGE_NMUT));

}

/*
 * Short usage information.
 */
void
pr_short_usage(void)
{
	if (Product_Version)
	{
		printf("\n");
		printf(MSGSTR(PVERSION_BANNER, MSG_PVERSION_BANNER));

		#ifdef _AIX
		printf(MSGSTR(PSUSAGE_AIX, MSG_PSUSAGE_AIX));
		#else
		if(RealMode())
			printf(MSGSTR(PSUSAGE_01, MSG_PSUSAGE_01));
		else
			printf(MSGSTR(PSUSAGE_02, MSG_PSUSAGE_02));
		#endif
	}
	else
	{
		printf("\n");

		#if BOOTS
		if (RealMode())
			printf(MSGSTR(ISUSAGE_B, MSG_ISUSAGE_B));
		else
			printf(MSGSTR(ISUSAGE_A, MSG_ISUSAGE_A));
		#else
		#ifdef _AIX
		printf(MSGSTR(ISUSAGE_AIX, MSG_ISUSAGE_AIX));
		#else
		printf(MSGSTR(ISUSAGE_NB, MSG_ISUSAGE_NB));
		#endif
		#endif 
	}


	#if SEARCH_DRIVE_ENABLE
	#if BOOTS
	if(RealMode())
		printf(MSGSTR(SUSAGE_B, MSG_SUSAGE_B));
	else
		printf(MSGSTR(SUSAGE_A, MSG_SUSAGE_A));
	#else
	printf(MSGSTR(SUSAGE_NB, MSG_SUSAGE_NB));
	#endif
	#else
	printf("\n");
	printf(MSGSTR(SUSAGE_NDS, MSG_SUSAGE_NDS));
	#endif 

	printf("\n");

	#ifdef _AIX
	printf(MSGSTR(SUSAGE_AIX, MSG_SUSAGE_AIX));
	printf("\n");
	#endif 

	printf(MSGSTR(SUSAGE_01, MSG_SUSAGE_01));
	printf("\n");
	printf(MSGSTR(SUSAGE_02, MSG_SUSAGE_02));
	printf("\n");
}

/*
 * Usage examples.
 */
void
pr_examples_of_usage(void)
{

	#ifdef _AIX
	printf("\n");
	printf(MSGSTR(EUSAGE_AIX, MSG_EUSAGE_AIX));
	printf(MSGSTR(EUSAGE_DSAIX, MSG_EUSAGE_DSAIX));
	printf(MSGSTR(EUSAGE_DS01AIX, MSG_EUSAGE_DS01AIX));
	printf(MSGSTR(EUSAGE_DS03AIX, MSG_EUSAGE_DS03AIX));
	#else
	printf(MSGSTR(EUSAGE,MSG_EUSAGE));
	printf("\n");
	#endif

	#if SEARCH_DRIVE_ENABLE
	printf(MSGSTR(EUSAGE_DS01));
	printf(MSGSTR(EUSAGE_DS02));
	printf("\n");

	#if BOOTS
	if(RealMode())
		printf(MSGSTR(EUSAGE_DSB01, MSG_EUSAGE_DSB01));
	else
		printf(MSGSTR(EUSAGE_DSB02, MSG_EUSAGE_DSB02));
	printf("\n");
	#else
	printf(MSGSTR(EUSAGE_DSNB01, MSG_EUSAGE_DSNB01));
	printf("\n");
	#endif

	printf(MSGSTR(EUSAGE_DS03, MSG_EUSAGE_DS03));
	printf("\n");
	#endif /* SEARCH_DRIVE_ENABLE */

	#if BOOTS
	if (RealMode())
		printf(MSGSTR(EUSAGE_B01, MSG_EUSAGE_01));
	else
		printf(MSGSTR(EUSAGE_B02, MSG_EUSAGE_02));

	printf("\n");
	#endif

	printf(MSGSTR(EUSAGE_03, MSG_EUSAGE_03));
	printf("\n");
}

/*
 * Message displayed for an invalid option.
 */
void
invalid_option(char *option)
{
	#ifdef _AIX
	printf(MSGSTR(INVALID_SW_AIX, MSG_INVALID_SW_AIX), option);
	#else
	printf(MSGSTR(INVALID_SWITCH, MSG_INVALID_SWITCH), option);
	#endif
}


/*
 * Make SURE that a virus warning RC gets set. Scan will continue if
 * a viral signature is found, and other errors may returned subsequently.
 */
void
sexit(int rc)
{
	memset(workbuf, 0, SIZE_WORK_BUF+MAX_SIZE_SIGN-1);
	switch(rc)
	{
		case(SIG_FOUND):
			break;
		case(NORM_ERR):
			printf(MSGSTR(SCAN_TERMINATE, MSG_SCAN_TERMINATE));
			break;
		case(NO_ERR):
			break;
		default:
			printf(MSGSTR(CANT_HAPPEN, MSG_CANT_HAPPEN));
			break;
	}
	if (signature_has_been_found)
		exit(SIG_FOUND);
	else
		exit(rc);
}


/*
 * Interpret character value as a hex char, return numerical value.
 * Slow and portable. Note that it displays a message, using globally
 * available information, if there is an error.
 */
byte
charhextobyte(byte charhex)
{
	switch(toupper(charhex))
	{
		case('0'): 
			return 0x0;
		case('1'): 
			return 0x1;
		case('2'): 
			return 0x2;
		case('3'): 
			return 0x3;
		case('4'): 
			return 0x4;
		case('5'): 
			return 0x5;
		case('6'): 
			return 0x6;
		case('7'): 
			return 0x7;
		case('8'): 
			return 0x8;
		case('9'): 
			return 0x9;
		case('A'): 
			return 0xA;
		case('B'): 
			return 0xB;
		case('C'): 
			return 0xC;
		case('D'): 
			return 0xD;
		case('E'): 
			return 0xE;
		case('F'): 
			return 0xF;
		case('?'): 
			return 0xF;
		default:
			clear_line_if_required();
			printf(MSGSTR(ERR_PARSE_LINE, MSG_ERR_PARSE_LINE));
			printf("\n%s\n", tmplinebuf);
			sexit(NORM_ERR);
	}
	return 0x0;  /* IBM C/2 Compiler complains without this line */
}

/*
 * Convert a string of character data representing a hex string to
 * a hex string. Return the length of the output hex string in bytes.
 * Warning - this may not be portable.
 */
int
stringtohex(byte *hexstring, char *bytestring)
{
	register int i;
	register int len;

	len=strlen(bytestring);

	if (len%2)
	{
		clear_line_if_required();
		printf(MSGSTR(ODD_LEN_SIG, MSG_ODD_LEN_SIG));
	}
	memset(hexstring,0,len/2);
	for (i=0; i<len; ++i)
	{
		if (i%2)
			hexstring[i/2] |= charhextobyte(bytestring[i]);
		else
			hexstring[i/2] |= charhextobyte(bytestring[i]) << 4;
	}
	/*pr_hex(hexstring,len/2+len%2);*/
	return (len/2 + len%2);
}

/*
 * Display a 'sigdat' data structure.
 */
void
pr_signature(sigdat *sig)
{
	pr_masked_hex(sig->signature,
	    (int) sig->len_signature,
	    sig->is_complex_sig);
	printf("\n");
	printf("%s\n", sig->message);
	printf("com_files=%d, exe_files=%d, pause_if_found=%d\n",
	    sig->com_files, sig->exe_files, sig->pause_if_found);
	printf("at_offset=%d, is_sig_frag=%d, is_complex_sig=%d, offset=%ld\n",
	    sig->at_offset, sig->is_sig_frag, sig->is_complex_sig, sig->offset);
}

/*
 * Dump portions of the hash table.
 */
void
pr_hash_tables(void)
{
	unsigned int i;
	sigdat *tmp;

	printf("(Hash table follows)\n");
	for (i=0; i<SIZE_HT; ++i)
	{
		tmp = hash_table[i];
		if (tmp != NULL)
		{
			printf("i=%02.2X: ",i);
			while (tmp != NULL)
			{ 
				printf("%p ",tmp); 
				tmp = tmp->next_sibling; 
			}
			printf("\n");
		}
	}
	printf("(End of hash table)\n");
}

/*
 * Initialize the two hash tables. Do it only once. Hash tables
 * will accumulate if more than one signature file is specified.
 * The performance degradation should be negligible.
 */
void
init_hash_tables(void)
{
	static boolean first_time = TRUE;
	register unsigned int sigind;

	if (first_time)
	{
		for (sigind=0; sigind<SIZE_HT; ++sigind)
		{
			hash_table[sigind] = NULL;
		}
		for (sigind=0; sigind<SIZE_FSC; ++sigind)
		{
			first_sig_chars[sigind] = FALSE;
		}
		first_time = FALSE;
	}
}

/*
 * Free the elements in the hash table with malloc-ed elements.
 * Zero the signature field so that virscan doesn't leave a residue in
 * memory.
 */
void free_hash_tables(void)
{
	register unsigned int sigind;
	register sigdat *hte;
	sigdat *prev;

	for (sigind=0; sigind<SIZE_HT; ++sigind)
	{
		hte = hash_table[sigind];
		while (hte != NULL)
		{
			memset(hte->signature,0, (int) hte->len_signature);
			prev = hte;
			hte = hte->next_sibling;
			if (!prev->is_sig_frag)
			{
				free(prev->signature);
				free(prev->message);
			}
			free(prev);
		}
	}
	whole_sig_count = frag_sig_count = sig_count = 0;
}


/*
 * Install a signature object in the hash table. Signature is
 * assumed to be masked during installation.
 * If collision, object goes to end of list.
 * Also count signatures here.
 */
void
install_sig(sigdat *sig, boolean is_frag)
{
	word hval_first_sig_chars;
	word hval_hash_table;
	sigdat *tmp;

	#if M_I86
	hval_first_sig_chars =
			(*((word *) &sig->signature[0]) & FSC_MASK) ^ FSC_MASK;
	hval_hash_table =
			(*((word *) &sig->signature[0]) & HT_MASK)  ^ HT_MASK;
	#else
	hval_first_sig_chars = sig->signature[0] ^ FSC_MASK;
	hval_hash_table =      sig->signature[0] ^ FSC_MASK;
	#endif

	if (first_sig_chars[hval_first_sig_chars] == FALSE)
	{
		first_sig_chars[hval_first_sig_chars] = TRUE;
	}
	if (hash_table[hval_hash_table] == NULL)
	{
		hash_table[hval_hash_table] = sig;
	}
	else
	{
		tmp = hash_table[hval_hash_table];
		while (tmp->next_sibling != NULL) { 
			tmp = tmp->next_sibling; 
		}
		tmp->next_sibling = sig;
	}

	#if DEBUGGERY
	pr_signature(sig);
	#endif
	++sig_count;   /* Count of signatures */
	if (is_frag)
		++frag_sig_count;
	else
		++whole_sig_count;
}

/*
 * Output a filename to the "positives" file, making sure that the same
 * filename isn't output twice in a row.
 *
 * Also bump "infected object count" if a filename isn't the same as a
 * previous filename.
 */
void
output_positive_filename(char *filename)
{
	static boolean first_posfile_open = TRUE;
	FILE *positives_file;
	static char *prev_pos_filename = "";

	if (strcmp(prev_pos_filename,filename))
	{
	   ++infected_object_cnt;
	   if (list_positives)
	   {
		if (first_posfile_open)
		{
		   if ((positives_file = fopen(positives_filename,"w")) == NULL)
		   {
			clear_line_if_required();
			printf(MSGSTR(ERR_OPENING_W, MSG_ERR_OPENING_W),
			   positives_filename);

			if (verbose)
			   printf(MSGSTR(REASON_FOR_ERR, MSG_REASON_FOR_ERR),
			      strerror(errno));
			   sexit(NORM_ERR);
		   }
		   first_posfile_open = FALSE;
		}
		else
		{
		   if ((positives_file = fopen(positives_filename,"a")) == NULL)
		   {
			clear_line_if_required();
			printf(MSGSTR(ERR_OPENING_A, MSG_ERR_OPENING_A),
			   positives_filename);

			if (verbose)
			   printf(MSGSTR(REASON_FOR_ERR, MSG_REASON_FOR_ERR),
			      strerror(errno));

			sexit(NORM_ERR);
		   }
		}
		fprintf(positives_file, "%s\n", filename);
		fclose(positives_file);
	   }
	}

	if (strlen(prev_pos_filename) != 0) 
	   free(prev_pos_filename);

	prev_pos_filename = strdup(filename);
	if (prev_pos_filename == (char *)NULL)
	{
	   clear_line_if_required();
	   printf(MSGSTR(OOM_STRDUP, MSG_OOM_STRDUP));
	   sexit(NORM_ERR);
	}
}

void
output_logfile_line(sigdat *sd,
		char *filename,
		long signature_offset,
		int mismatched_bytes_count)
{
	static FILE *logfile = NULL;
	char *tmpptr;
	char time_string[OUTPUT_SZ+1];		/* locale-formatted time string */
	struct tm *timp, tim;

	/*
  	 * Don't do anything unless we're creating a log file.
 	 */
	if (create_logfile)
	{
	   /*
 	    * If we have finished scan of object and no messages were
	    * output for the object and there were no precise matches 
 	    * and there were signatures found then we update the 
	    * parameters that were passed to this routine.
 	    */
	   if (sd == NULL && !obj_message_output && !found_precise_match &&
	       object_sigfound_cnt != 0)
	   {
		if (sig_with_min_diffs != NULL)
		{
		   sd = sig_with_min_diffs;
		   filename = obj_name;
		   signature_offset = offset_swmd;
		   mismatched_bytes_count = min_mismatched_bytes;
		}
	   }
	   /*
 	    * If the logfile hasn't been opened yet, open it.
 	    */
	   if (logfile == NULL)
	   {
		if ((logfile = fopen(logfilename,"w")) == NULL)
		{
		   clear_line_if_required();
		   printf(MSGSTR(ERR_OPENING_W, MSG_ERR_OPENING_W),logfilename);

		   if (verbose)
			printf(MSGSTR(REASON_FOR_ERR, MSG_REASON_FOR_ERR),
			   strerror(errno));

		   sexit(NORM_ERR);
		}

		fprintf(logfile, MSGSTR(LOGFILE_VMSG, MSG_LOGFILE_VMSG),
		   product_version ? 
		   MSGSTR(PVERSION_BANNER, MSG_PVERSION_BANNER) :
		   MSGSTR(VERSION_BANNER,  MSG_VERSION_BANNER));
		timp = localtime(&scan_starttime);
		tim = *timp;
		strftime(time_string,OUTPUT_SZ,"%c",&tim);
		fprintf(logfile, MSGSTR(LOGFILE_TDMSG, MSG_LOGFILE_TDMSG),
		   time_string);
	   }
	   /*
  	    * If there is a non-null signature parameter, i.e. there
	    * was a valid one passed or there was a valid one copied
	    * above, then output a line logfile to the logfile.
 	    */
	   if (sd != NULL)
	   {
		fprintf(logfile, "%s", filename);
		if ((tmpptr = strstr(sd->message, "%s")) != NULL)
		   fprintf(logfile, " ::: %s", &tmpptr[3]);
		else
		   fprintf(logfile, " ::: %s", sd->message);

		if (mismatched_bytes_count)
		   fprintf(logfile, MSGSTR(MMATCHED_BYTES, MSG_MMATCHED_BYTES),
		       mismatched_bytes_count);

		if (sd->is_sig_frag)
		   fprintf(logfile, MSGSTR(FRAGMENT, MSG_FRAGMENT));

		if (verbose || always_warn)
		   fprintf(logfile, MSGSTR(OFFSET, MSG_OFFSET),
		       signature_offset, signature_offset);

		fprintf(logfile, "\n");
	   }

	   /*
 	    * Otherwise, we are definitely following a scan. If we 
	    * haven't found any viruses, we output a line to the log 
	    * file saying so.
 	    */
	    else
	    {
		if (!obj_message_output)
		{
		   fprintf(logfile, "%s", filename);
		   fprintf(logfile, "\n");
		}
	    }
	}
}

/*
 * Display warning message. If list_positives is set to TRUE, then
 * write the filename to the positives file, if the filename isn't the
 * same as the previous filename.
 *
 * There is also code to warn the user about dangerous viruses like
 * the Dark Avenger, if they are found in potentially dangerous
 * circumstances.
 */
void
warning_msg(sigdat *sd,
		char *filename,
		long signature_offset,
		int mismatched_bytes_count,
		int object_type)
{
	char *typename;

	++sigfound_cnt;

	if (!very_quiet)
	{
	   if (!verbose & cr_works)
	   {
		pr_then_clear_line("");
		printf("\r");
		prevnamelen = strlen("");
	   }
	   if (verbose || always_warn)
	   {
		if (!mismatched_bytes_count && !sd->is_sig_frag)
		   printf(MSGSTR(FOUND_SIG_AO, MSG_FOUND_SIG_AO),
		      filename, signature_offset, signature_offset);
		else
		   if ( mismatched_bytes_count && !sd->is_sig_frag)
			printf(MSGSTR(FOUND_SIG_P_AO, MSG_FOUND_SIG_P_AO),
			   filename, signature_offset, signature_offset);
		   else
			if (!mismatched_bytes_count &&  sd->is_sig_frag)
			   printf(MSGSTR(FOUND_SIG_F_AO, MSG_FOUND_SIG_F_AO),
			      filename, signature_offset, signature_offset);
			else
			   /* Redundant, yes */
			   if ( mismatched_bytes_count && sd->is_sig_frag) 
				printf(MSGSTR(FOUND_SIG_PF_AO,
				   MSG_FOUND_SIG_PF_AO), filename, 
				   signature_offset, signature_offset);
	   }
	   else
	   {
		if (!mismatched_bytes_count && !sd->is_sig_frag)
		   printf(MSGSTR(FOUND_SIG, MSG_FOUND_SIG), filename);
		else
		   if ( mismatched_bytes_count && !sd->is_sig_frag)
			printf(MSGSTR(FOUND_SIG_P, MSG_FOUND_SIG_P), filename);
		   else
			if (!mismatched_bytes_count && sd->is_sig_frag)
			   printf(MSGSTR(FOUND_SIG_F,MSG_FOUND_SIG_F),filename);
			else
			   /* Redundant, yes */
			   if ( mismatched_bytes_count &&  sd->is_sig_frag) 
				printf(MSGSTR(FOUND_SIG_PF, MSG_FOUND_SIG_PF),
				    filename);
	   }
	   printf("\n");

	   if (verbose)
	   {
		if (sd->signature != NULL)
		{
		   pr_masked_hex(sd->signature, (int) sd->len_signature,
	  	   	sd->is_complex_sig);
		}
		else
		   printf(MSGSTR(NO_SIMPLE_SIG,MSG_NO_SIMPLE_SIG));

		printf("\n");
	   }

	   if (mismatched_bytes_count > 0)
	   {
		printf(mismatched_bytes_count > 1 ? MSGSTR(BYTES_DIFFERENT,
		   MSG_BYTES_DIFFERENT) : MSGSTR(BYTE_DIFFERENT,
		   MSG_BYTE_DIFFERENT), mismatched_bytes_count);
	   }

	   if (default_mutant_detection && mismatched_bytes_count != 0)
	   {
		switch(object_type)
		{
		   case(T_FILE):
			typename = MSGSTR(THIS_FILE_MBIV, MSG_THIS_FILE_MBIV);
			break;

		   case(T_MASTER_BOOT_RECORD):
			typename = MSGSTR(THIS_MBR_MBIV, MSG_THIS_MBR_MBIV);
			break;

		   case(T_SYSTEM_BOOT_SECTOR):
			typename = MSGSTR(THIS_SBS_MBIV, MSG_THIS_SBS_MBIV);
			break;

		   case(T_SYSTEM_MEMORY):
			typename = MSGSTR(SM_MCV, MSG_SM_MCV);
			break;

		   default:
			typename = MSGSTR(MBIV, MSG_MBIV);
			break;
		}
	   }
	   else
	   {
		switch(object_type)
		{
		   case(T_FILE):
			typename = MSGSTR(THIS_FILE_MBI, MSG_THIS_FILE_MBI);
			break;

		   case(T_MASTER_BOOT_RECORD):
			typename = MSGSTR(THIS_MBR_MBI, MSG_THIS_MBR_MBI);
			break;

		   case(T_SYSTEM_BOOT_SECTOR):
			typename = MSGSTR(THIS_SBS_MBI, MSG_THIS_SBS_MBI);
			break;

		   case(T_SYSTEM_MEMORY):
			typename = MSGSTR(SM_MC, MSG_SM_MC);
			break;

		   default:
			typename = MSGSTR(MBI, MSG_MBI);
			break;
		}
	   }
	   if ((strstr(filename, "system memory") != NULL) &&
	       (strstr(sd->message, "%s") == NULL))
	   {
	 	printf(MSGSTR(FOUND_IN_MEM, MSG_FOUND_IN_MEM));

		printf("%s\"%s\"\n",		/* \a is a beep */
		   beep_if_sig_found ? "\a" : "", sd->message);   
	   }
	   else
	   {
		printf("%s", beep_if_sig_found ? "\a" : ""); 
		printf(sd->message, typename);
		printf("\n");

		if ((default_mutant_detection && mismatched_bytes_count != 0) &&
		     strstr(sd->message,"%s") == NULL)
		{
		   printf(MSGSTR(MAY_IND_VARIANT, MSG_MAY_IND_VARIANT));
		}
	   }

	   if ((strstr(filename, "system memory") != NULL) &&
	       (strstr(sd->message, "%s") == NULL))
		printf(MSGSTR(FOUND_IN_MEM_03, MSG_FOUND_IN_MEM_03));
	}

	#if BOOTS
	if (RealMode() && sd->pause_if_found &&
	    ((strstr(filename, "system memory") != NULL) || 
	    do_memory_scan == FALSE))
	{
	   char c;

	   printf(MSGSTR(BAD_VIRUS, MSG_BAD_VIRUS));

	   if (!Product_Version)
		printf(MSGSTR(YOU_HAVE_BEEN, MSG_YOU_HAVE_BEEN));

	   printf(MSGSTR(PRESS_C_TO_CONT, MSG_PRESS_C_TO_CONT));

	   sd->pause_if_found = FALSE;
	   c = (char) getch();
	   if (toupper(c) != 'C')
	   {
		printf(MSGSTR(ABORTING_SCAN, MSG_ABORTING_SCAN));
		sexit(SIG_FOUND);
	   }
	}
	#endif

	if (!very_quiet)
	{
	   printf(MSGSTR(CONT_SCAN_MSG, MSG_CONT_SCAN_MSG));
	   printf("\n");
	}


	output_logfile_line(sd, filename, signature_offset,  
	   mismatched_bytes_count);

	output_positive_filename(filename);
	obj_message_output = TRUE;
}


/*
 * Warn if a warning is warranted.
 * Currently, exclude warning only if file type is exe and virus is not known
 * to infect exe files, and if an offset is specified if the offset of the
 * signature doesn't match the specified offset. Never exclude if the
 * always_warn flag is set, via the command line switch -g
 * If the system memory is being scanned, always warn if the "scan_memory"
 * field of the signature is marked TRUE.
 */
void
warn_if_warranted(sigdat *sd,
		char *filename,
		boolean is_exe_type,
		long signature_offset,
		int mismatched_bytes_count,
		int object_type)
{
	#if DEBUGGERY
	printf("exe_type=%d , exefiles=%d\n",
	    is_exe_type,sd->exe_files);
	#endif

	if (always_warn || (!(is_exe_type && !sd->exe_files) &&
	    (sd->at_offset ? sd->offset == signature_offset : TRUE) &&
	    (strstr(filename, "system memory") == NULL)) ||
	    (sd->scan_memory && (strstr(filename, "system memory") != NULL)))
	{
	   if (!sd->is_sig_frag)
	   {
		if (mismatched_bytes_count == 0)
		{
		   warning_msg(sd, filename, signature_offset, 
		      mismatched_bytes_count, object_type);
		   found_precise_match = TRUE;
		}
		else
		{
		   if (!default_mutant_detection)
		   {
			warning_msg(sd, filename, signature_offset, 
			   mismatched_bytes_count, object_type);
		   }
		   else /* Wait until scan of object finishes and	*/
		   {    /* display message for 'closest' virus 		*/
		        /* found in the signature hash table 		*/

			++object_sigfound_cnt;
			if (mismatched_bytes_count < min_mismatched_bytes)
			{
			   min_mismatched_bytes = mismatched_bytes_count;
		  	   sig_with_min_diffs = sd;
			   offset_swmd = signature_offset;
			   obj_name = filename;
			   obj_type = object_type;
			}
		   }
		}
	   }
	   else
	   {
		if (!default_mutant_detection)
		{
		   warning_msg(sd, filename, signature_offset, 
		      mismatched_bytes_count, object_type);
		}
	   }

	   signature_has_been_found = TRUE;
	}
}

void
reset_object_counters(void)
{
	object_sigfound_cnt = 0;
	found_precise_match = FALSE;
	min_mismatched_bytes = INT_MAX;
	sig_with_min_diffs = NULL;
	offset_swmd = 0l;
	obj_name = NULL;
	obj_type = T_FILE;
	obj_message_output = FALSE;
}

void
messages_if_mutants(void)
{
	if (!found_precise_match && object_sigfound_cnt != 0)
	{
		if (sig_with_min_diffs != NULL)
		{
			warning_msg(sig_with_min_diffs,
			    obj_name,
			    offset_swmd,
			    min_mismatched_bytes,
			    obj_type);
		}
	}
}

/*
 * Documentation for the next few functions is in for repairs.
 * Sorry.
 */
void
inside_scanloop(register unsigned int scan_length,
		char *filename,
		boolean lastbuf,
		boolean is_exe_type,
		long base,
		sigdat *tmp,
		register unsigned int i,
		int object_type)
{
	int mismatched_bytes_count;

	if (!(mycmp(&tmp->signature[2],
	    &workbuf[i+2],
	    (int) tmp->len_signature-2,
	    &mismatched_bytes_count,
	    tmp)))
	{
		if (lastbuf && (int) tmp->len_signature > ((scan_length+3)-i))
		{
			#if DEBUGGERY
			printf("early abort, ls=%d, sl=%d, sig=",
			    (int) tmp->len_signature-2,
			    (scan_length-i)-1);
			pr_masked_hex(tmp->signature,
			    (int) tmp->len_signature,
			    tmp->is_complex_sig);
			printf("\n");
			#endif
			/* Compare total length of signature */
			/* with number bytes really left in buffer */
			/* If we just compared past end of file, */
			/* don't report message */
		}
		else
			warn_if_warranted(tmp,
			    filename,
			    is_exe_type,
			    base+i,
			    mismatched_bytes_count,
			    object_type);
	}
}

/*
 * This can be easily replaced with assembler, and *is* for the PC version.
 * Use of a profiler consistently indicates that most of the time is spent
 * in this loop and in disk I/O.
 */
void
scanloop(register unsigned int scan_length,
		char *filename,
		boolean lastbuf,
		boolean is_exe_type,
		long base,
		int object_type)
#if ASM_SCANLOOP
;      /* Function prototype, nothing else */
#else
{
	register unsigned int i;
	sigdat *tmp;

	for (i=0; i<scan_length; ++i)
	{
	   #if FREQ_COUNTS
	   ++ccounts[workbuf[i]];
	   #endif

	   #if M_I86		/* This works on a little-endian machine */
	   if (first_sig_chars[*((word *)&workbuf[i]) & FSC_MASK])
	   #else		/* This works on a big-endian machine */
	   if (first_sig_chars[workbuf[i] /* & 0xFF */])
	   #endif
	   {
		#if M_I86	/* This works on a little-endian machine */
		tmp = hash_table[*((word *)&workbuf[i]) &  HT_MASK];
		#else		/* This works on a big-endian machine */
		tmp = hash_table[workbuf[i]/*&0xFF*/];
		#endif

		while (tmp != NULL)
		{
		   if ((tmp->second_byte== workbuf[i+1]) && 
		       (tmp->third_byte  == workbuf[i+2] ||
		       (tmp->is_complex_sig && tmp->third_byte == 0xFF)))
		   {
			inside_scanloop(scan_length, filename, lastbuf,
			   is_exe_type, base, tmp, i, object_type);
		   }
		   tmp = tmp->next_sibling;
		}
	   }
	}

}
#endif

/*
 * Control the scanning of a buffer for viruses. Invokes the 1260 scanner,
 * and also the normal scanning loop.
 */
void
scanworkbuf(register unsigned int scan_length,
		char *filename,
		boolean lastbuf,
		boolean is_exe_type,
		long base,
		boolean do_scan_for_1260,
		int object_type)
{
	if (do_scan_for_1260)
		scan_for_1260(scan_length, filename, is_exe_type, base, 
		    object_type);

	total_scanlength += scan_length;     	/* Record total bytes scanned */
	if (lastbuf)              		/* i.e. end-of-file */
	{
		if (scan_length <= 3) 	 	/* No room for signature */
			return;
		scan_length -= 3;      		/*Signatures at least 4 bytes */
	}
	scanloop(scan_length, filename, lastbuf, is_exe_type, base,  
	    object_type);
}

static sigdat dummy_1260_signature =
{
	(byte) 0,   /* Second byte */
	(byte) 0,   /* Third byte */
	NULL,       /* Signature */
	"%s the Washburn-1260, Washburn-Casper, or \nWashburn-V2P2 virus.",
	0,          /* Len signature (No signature) */
	NULL,       /* Next sibling */
	TRUE,       /* Infects COM files */
	FALSE,      /* Doesn't infect EXE files */
	FALSE,      /* Don't pause if found */
	FALSE,      /* Don't scan memory */
	FALSE,      /* Not at any particular offset */
	FALSE,      /* Is not a signature fragment */
	TRUE,       /* Is a complex signature */
	TRUE,       /* Do disable mutant scan */
	0l          /* Dummy offset */
};

/*
 * This closely resembles scanworkbuf. It is a special purpose scanner for
 * the 1260 virus, and will not scan EXE files unless the "always_warn"
 * flag is set, for instance with the "-g" option. EXE files are identified
 * elsewhere by presence of the EXE signature. No viruses that we know of
 * will convert COM files to EXE files, with the possible exception of
 * self extracting archives, which can't be successfully scanned (as
 * binary images) anyway, as they are compressed.
 */
void
scan_for_1260(register unsigned int scan_length,
		char *filename,
		boolean is_exe_type,
		long base,
		int object_type)
{
	register unsigned int i;
	unsigned int j;
	unsigned int k;
	boolean foundBF;
	boolean foundB8;
	boolean foundB9;
	boolean found40;
	boolean found47;
	boolean found90;
	boolean foundE2;
	boolean found_second_XOR = FALSE;

	if (is_exe_type && !always_warn) return;

	for (i= ((base == 0l) ? 9 : (11+14)); i<scan_length+(11+14); ++i)
	{
	   if (workbuf[i] == 0x31)
	   {
		if (workbuf[i+1] == 0x0D)
		{
		   /*
	            * printf("Found 310DH in %s at offset %ld\n",
		    * filename, base+i);
 		    */
		   found_second_XOR = FALSE;
		   for (j=i+2; j<=i+14; ++j)
			if (workbuf[j] == 0x31 &&  workbuf[j+1] == 0x05)
		 	{	
			   found_second_XOR = TRUE;

	/* if (verbose) 
  	 * printf("Found second XOR instruction within range, in %s\n",
	 * filename);
 	 */
			}
		}

		if (workbuf[i+1] == 0x05)
		{
		   /* printf("Found 3105H in %s at offset %ld\n",
		    * filename, base+i);
 		    */
		   found_second_XOR = FALSE;
		   for (j=i+2; j<=i+14; ++j)
			if (workbuf[j] == 0x31 && workbuf[j+1] == 0x0D)
			{
			   found_second_XOR = TRUE;

	/* if (verbose) 
   	 * printf("Found second XOR instruction within range, in %s\n", 
  	 * filename);
	 */
			}
		}

		if (found_second_XOR)
		{
		   foundBF = foundB8 = foundB9 = FALSE;
		   for (i < (11+14) ? (k=0) : (k=i-(11+14)); k<i; ++k)
		   {
			if (workbuf[k] == 0xBF) foundBF = TRUE;
			if (workbuf[k] == 0xB8) foundB8 = TRUE;
			if (workbuf[k] == 0xB9) foundB9 = TRUE;
		   }

		   if (foundBF && foundB8 && foundB9)
		   {

	/* if (verbose) 
	 * printf("Found the 3 required instructions from the first block.\n");
	 */
			found40 = found47 = found90 = foundE2 = FALSE;
			for (k=i+4; k<=i+23; ++k)
			{
			   if (workbuf[k] == 0x40) found40 = TRUE;
			   if (workbuf[k] == 0x47) found47 = TRUE;
			   if (workbuf[k] == 0x90) found90 = TRUE;
			   if (workbuf[k] == 0xE2) foundE2 = TRUE;
			}
			if (found40 && found47 && found90 && foundE2)
			{

	/* if (verbose) 
	 * printf("Found the 3 required instructions from the last block.\n");
	 * if (verbose) 
	 * printf("Found the required loop instruction.\n"); 
	 */
			   warn_if_warranted(&dummy_1260_signature, filename,
			   	is_exe_type, base+i, 0, object_type);

		 	   i += 23;      
			   /* Make sure this degarbler is only reported once. */
			}
		   }

	/* else if (verbose) 
      	 * printf(
         * "Didn't find the 3 required instructions from the first block.\n");
	 */
		   found_second_XOR = FALSE;   /* Reset for next time */
		}
	   }
	}
}

/*
 * Test the parameter filename for viral signatures.
 * Recovers from certain special cases. (Locked OS/2 files)
 *
 * The fopen/fread etc have been replaced with open/read etc when compiled
 * for OS/2, because in IBM C/2 compact (and large and huge) model they
 * are *much* (factor of 2 or more) faster.
 */
void
test(char *testfilename)
{
	#ifdef M_I86
	int testfile;
	#else
	FILE *testfile;
	#endif
	boolean first_buf;
	boolean last_buf;
	boolean is_exe_type;
	unsigned int rv;
	long base = 0l;

	#ifdef M_I86
	testfile = open(testfilename,O_RDONLY|O_BINARY);
	if (testfile == -1)
	#else
		testfile = fopen(testfilename,"rb");
	if (testfile == NULL)
	#endif
	{
	   #ifndef _AIX
	   if (!(strstr(testfilename,"EA DATA. SF") != NULL ||
	         strstr(testfilename,"SWAPPER.DAT") != NULL))
	   {
		clear_line_if_required();
		printf(MSGSTR(UNABLE_TO_OPEN, MSG_UNABLE_TO_OPEN),testfilename);

		if (verbose)  
		   printf(MSGSTR(REASON_FOR_ERR, MSG_REASON_FOR_ERR),
		       strerror(errno));

		if (continue_if_open_error)
		{
		   printf(MSGSTR(CONT_SCAN_MSG, MSG_CONT_SCAN_MSG));
		}
		else
		{
		   printf(MSGSTR(RESTART_SCAN, MSG_RESTART_SCAN));
		   sexit(NORM_ERR);
		}  
	   }
 	   else
	   #endif
	   {
		if (!very_quiet)
		{
		   clear_line_if_required();
		   printf(MSGSTR(UNABLE_TO_OPEN, MSG_UNABLE_TO_OPEN),
		  	testfilename);

		   if (verbose)
			printf(MSGSTR(REASON_FOR_ERR, MSG_REASON_FOR_ERR),
			    strerror(errno));

		   printf(MSGSTR(CONTINUING_SCAN, MSG_CONTINUING_SCAN));
		}
	   }	
	}
	else
	{
	   outp_progress_line(testfilename);
	   reset_object_counters();
	   first_buf = TRUE;

	   #ifdef M_I86
	   while ((rv = read
		  (testfile,workbuf, SIZE_WORK_BUF+MAX_SIZE_SIGN-1)) != 0)
	   #else
	   while ((rv = fread
		  (workbuf,1, SIZE_WORK_BUF+MAX_SIZE_SIGN-1, testfile)) != 0)
	   #endif
	   {
		if (rv == -1)
		{
		   printf(MSGSTR(ERROR_READING_F, MSG_ERROR_READING_F), 
			testfilename);
		   break;
		}
		if (first_buf)
		{
		   is_exe_type = (workbuf[0] == 0x4D && workbuf[1] == 0x5A);

	   #if DEBUGGERY
	   printf("is_exe_type=%d, %02.2x %02.2x %s\n", is_exe_type,
	 	workbuf[0], workbuf[1], testfilename);

	   if ((is_exe_type && strstr(strupr(testfilename),".EXE") == NULL) ||
	       (!is_exe_type && strstr(strupr(testfilename),".COM") == NULL))
	      printf("File type/exe signature mismatch: %s, %d:%02.2x%02.2x\n",
		     testfilename, is_exe_type, workbuf[0], workbuf[1]);
	   #endif

		   first_buf = FALSE;
		}
		last_buf = (rv != SIZE_WORK_BUF+MAX_SIZE_SIGN-1);

		if (!last_buf)
		{
		   #ifdef M_I86
		   lseek(testfile,(long)(1-MAX_SIZE_SIGN), SEEK_CUR);
		   #else
		   {
			int rc;
			rc = fseek(testfile, (long)(1-MAX_SIZE_SIGN), SEEK_CUR);
			if (rc != 0)
			{
			   printf("Impossible error in test(): fseek rc = %d\n",
				 rc);
			   sexit(NORM_ERR);
			}
	   	   }		
	 	   #endif
		   rv -= MAX_SIZE_SIGN-1;
		}
		scanworkbuf(rv, testfilename, last_buf,  is_exe_type, base,
		 	TRUE, T_FILE);
		base += rv;
	   }
	   messages_if_mutants();
	   output_logfile_line(NULL, testfilename, 0l, 0);

	   #ifdef M_I86
	   close(testfile);
	   #else
	   fclose(testfile);
	   #endif
	   ++filecnt;
	}
}

#if SEARCH_DRIVE_ENABLE
/*
 * Scan system memory for memory resident viruses. This code is 
 * only included in the PC version, and in fact is only called when 
 * in real mode. Normally, memory up to E0000 is scanned; if the 
 * "-NHMS" option is specified, or we're running in the OS/2 DOS Box, 
 * memory to A0000 is scanned.
 */
void
test_memory(void)
{
	register unsigned int segment;
	long base = 0l;
	char msgbuf[256];
	byte *ptr;
	register unsigned int scan_length;
	unsigned int seg_top;

	sprintf(msgbuf, MSGSTR(SCANNING_MEM, MSG_SCANNING_MEM), always_warn ? 
	    " " : " dangerous and/or well hidden resident ");

	outp_progress_line(msgbuf);
	reset_object_counters();
	#define SCAN_SIZE 0x8000

	seg_top = (do_high_memory_scan) ? 0xE000 : 0xA000;
	segment = 0x0000;
	do
	{
		if (segment+SCAN_SIZE >> 4 == seg_top)
			scan_length = SCAN_SIZE;
		else
			scan_length = SCAN_SIZE + MAX_SIZE_SIGN;
		ptr = (byte *)(((unsigned long) segment) << 16);
		memmove(workbuf, ptr, scan_length);

		if (verbose)
			printf(MSGSTR(SCANNING_ADDR_R, MSG_SCANNING_ADDR_R),
			    base, base + SCAN_SIZE - 1);

		scanworkbuf(SCAN_SIZE, "system memory", FALSE, FALSE,  
			base, FALSE, T_SYSTEM_MEMORY);
		segment += SCAN_SIZE >> 4;
		base += SCAN_SIZE;
	}   while (segment != seg_top);

	messages_if_mutants();
	output_logfile_line(NULL, "system memory", 0l, 0);
	have_scanned_system_memory = TRUE;
}
#endif

/*
 * Clear the signature hash tables. Get signatures from the specified file
 * and install them in the signature hash tables.
 * If mutant detection by "fragments" is turned on, with command line switches
 * "-m" or "-mf", the signatures are installed whole, and 4 fragments from
 * each signature are extracted and installed as well. This fragment detection
 * is currently disabled for complex signatures, that have one or more
 * wildcard characters.
 */
void
get_signatures(char    *sigfilename,
		boolean no_default_dir,
	    	boolean test_for_modifications, /* Only used if SELF_TEST is
						 * TRUE  
						 */
	    	boolean fail_if_not_found)
{
	FILE *sigfile;
	unsigned int sigind;           	/* index into signature file */
	sigdat *sigtmp; 		/* signature struct assembled here */
	int rv;
	char *at_offset_ptr;		/* to find any offset specification */
	char *working_sigfilename;
	char progress_line[256];
	char *eol_ptr;

	/*
	 * Open the signature file.
	 * Test signature file integrity unless disabled.
	 * Read signatures.
	 * Install them in the hash tables.
	 * Install signature fragments in hash tables if requested.
	 * Close signature file.
	 *
	 * Signatures are masked immediately after conversion to binary, and
	 * henceforth never unmasked except byte-by-byte when doing comparisons.
	 * This prevents signatures from being left in memory and triggering
	 * false positives when other detectors scan memory.
 	 */
	#if BOOTS
	if (no_default_dir)
	{
		working_sigfilename = sigfilename;
	}
	else
	{
		char working_namebuf[256];
		get_pathname(working_namebuf);
		strcpy(strrchr(working_namebuf, '\\') + 1, sigfilename);
		if (OS_major_ver < 3 &&
		    working_namebuf[0] == '.' &&
		    working_namebuf[1] == '\\')
				working_sigfilename = &working_namebuf[2];
		else
				working_sigfilename = &working_namebuf[0];
	}
	strupr(working_sigfilename);
	#else
	working_sigfilename = sigfilename;
	#endif
	sigfile = fopen(working_sigfilename, "r");
	if (sigfile == NULL)
 	{		
		if (!fail_if_not_found) return;

		clear_line_if_required();
		printf(MSGSTR(ERROR_OPEN_SF, MSG_ERROR_OPEN_SF),
	 	    working_sigfilename);
		if (verbose) 
			printf(MSGSTR(REASON_FOR_ERR, MSG_REASON_FOR_ERR),
		            strerror(errno));
 
		printf(MSGSTR(ERROR_O_SF, MSG_ERROR_O_SF));
		sexit(NORM_ERR);
	}
	#if SELF_TEST
	if (test_for_modifications)
	{
	/*
 	 * Here, I'm presuming that the same file can be open twice, in two
 	 * different read modes, "rb" and "r".
   	 */
		sprintf(progress_line, MSGSTR(TESTING_SIGFILE,
		    MSG_TESTING_SIGFILE), working_sigfilename);

		outp_progress_line(progress_line);
		if (file_has_been_modified(working_sigfilename,
			    workbuf,
			    SIZE_WORK_BUF,
			    MAX_SIZE_SIGN-1))
		{
			clear_line_if_required();
			printf(MSGSTR(FILE_HAS_BEEN_M, MSG_FILE_HAS_BEEN_M),
			    working_sigfilename);
			sexit(NORM_ERR);
		}
	}
	#endif

	init_hash_tables();
	sigind = 0;

	sprintf(progress_line, MSGSTR(READING_SIGFILE, MSG_READING_SIGFILE),
	    working_sigfilename);
	outp_progress_line(progress_line);

	while(TRUE)
	{
		byte working_sig[MAX_SIZE_SIGN];

		/* Get signature. Check it for length, then install it in 
	         * structure 
		 */
		if (get_line_from_sig_file(tmplinebuf,SIZE_TMP_LINE_BUF-1,
			sigfile) == NULL)
				break;
		rv = sscanf(tmplinebuf, "%s", tmpbuf);
		if (rv != 1)
		{
			clear_line_if_required();
			printf(MSGSTR(ERR_PARSE_LINE, MSG_ERR_PARSE_LINE));
			printf("\n%s\n", tmplinebuf);
			sexit(NORM_ERR);
		}
		if (strlen(tmpbuf) > (MAX_SIZE_SIGN*2))
		{
			clear_line_if_required();
			printf(MSGSTR(SIG_OVERFLOW_01, MSG_SIG_OVERFLOW_01),
			    sigind+1, working_sigfilename);
			printf(MSGSTR(SIG_OVERFLOW_02, MSG_SIG_OVERFLOW_02),
			    MAX_SIZE_SIGN);
			printf(MSGSTR(SIG_OVERFLOW_03, MSG_SIG_OVERFLOW_03),
			    2*MAX_SIZE_SIGN);
			sexit(NORM_ERR);
		}
		sigtmp = malloc(sizeof(sigdat));
		if (sigtmp == NULL)
		{
			clear_line_if_required();
			printf(MSGSTR(MALLOC_ERROR, MSG_MALLOC_ERROR), 1);
			printf(MSGSTR(OUT_OF_MEMORY, MSG_OUT_OF_MEMORY));
			printf(MSGSTR(TRY_DECREASING, MSG_TRY_DECREASING));
			sexit(NORM_ERR);
		}
		sigtmp->is_complex_sig = strstr(tmpbuf, "??") ? TRUE : FALSE;
		if (tmpbuf[0] == '?' || tmpbuf[1] == '?' ||
		    tmpbuf[2] == '?' || tmpbuf[3] == '?')
		{
			printf(MSGSTR(NO_WILDCARDS_01, MSG_NO_WILDCARDS_01));
			printf(MSGSTR(NO_WILDCARDS_02, MSG_NO_WILDCARDS_02));
			sexit(NORM_ERR);
		}

		sigtmp->len_signature = (byte) stringtohex(working_sig,tmpbuf);
		mask(working_sig, (int) sigtmp->len_signature);

		if((int) sigtmp->len_signature <= 3)
		{
			clear_line_if_required();
			printf(MSGSTR(SHORT_SIG_01, MSG_SHORT_SIG_01));
			printf(MSGSTR(SHORT_SIG_02, MSG_SHORT_SIG_02));
			sexit(NORM_ERR);
		}
		sigtmp->signature = (byte *)malloc((int)sigtmp->len_signature);

		if (sigtmp->signature == NULL)
		{
			clear_line_if_required();
			printf(MSGSTR(MALLOC_ERROR, MSG_MALLOC_ERROR), 5);
			printf(MSGSTR(OUT_OF_MEMORY, MSG_MALLOC_ERROR));
			printf(MSGSTR(TRY_DECREASING, MSG_TRY_DECREASING));
			sexit(NORM_ERR);
		}

		memcpy(sigtmp->signature, working_sig, 
		    (int) sigtmp->len_signature);
		sigtmp->second_byte = sigtmp->signature[1] ^ 0xFF;
		sigtmp->third_byte  = sigtmp->signature[2] ^ 0xFF;

		/*  
		 * Get message to be written on discovery of signature. 
		 */
		if (get_line_from_sig_file(tmplinebuf,SIZE_TMP_LINE_BUF-1,
		    sigfile) == NULL)
		{
			clear_line_if_required();
			printf(MSGSTR(ERR_READING_SML, MSG_ERR_READING_SML),
			    sigind+1, working_sigfilename);
				sexit(NORM_ERR);
		}

		if ((eol_ptr = strchr(tmplinebuf,'\n')) != NULL)
			*eol_ptr = '\0';

		convert_message(tmplinebuf);
		sigtmp->message = strdup(tmplinebuf);
		if (sigtmp->message == (char *)NULL)
		{
			clear_line_if_required();
			printf(MSGSTR(MALLOC_ERROR, MSG_MALLOC_ERROR), 2);
			printf(MSGSTR(OUT_OF_MEMORY, MSG_OUT_OF_MEMORY));
			printf(MSGSTR(TRY_DECREASING, MSG_TRY_DECREASING));
			sexit(NORM_ERR);
		}

		#if DEBUGGERY
		printf("%s%s\n",tmpbuf,sigtmp->message);
		#endif

		/* Get the line describing file types in which signature
		 * might be found 
		 */
	 	if (get_line_from_sig_file(tmplinebuf,SIZE_TMP_LINE_BUF-1, 
			sigfile) == NULL)
		{
			clear_line_if_required();
		 	printf(MSGSTR(ERR_READING_SFT, MSG_ERR_READING_SFT),
			    sigind+1, working_sigfilename);
			sexit(NORM_ERR);
		}
		strupr(tmplinebuf);
		sigtmp->com_files = (strstr(tmplinebuf,"COM") != NULL);
		sigtmp->exe_files = (strstr(tmplinebuf,"EXE") != NULL);

		if ((at_offset_ptr = strstr(tmplinebuf,"OFFSET")) != NULL)
		{
			sigtmp->at_offset = TRUE;
			sscanf(&at_offset_ptr[strlen("OFFSET")], "%ld", 
			    &sigtmp->offset);

			/*printf("offset=%ld\n", sigtmp->offset);*/
		}
		else
		{
			sigtmp->at_offset = FALSE;
			sigtmp->offset = 0l;
		}

		sigtmp->pause_if_found = 
		    (strstr(tmplinebuf,"PAUSE IF FOUND") != NULL);

		sigtmp->scan_memory =(strstr(tmplinebuf,"SCAN MEMORY") != NULL);
		sigtmp->disable_mutant_scan = 
		    (strstr(tmplinebuf,"NO MUTANTS") != NULL);
		sigtmp->is_sig_frag = FALSE;

		#if DEBUGGERY
		printf("com_files=%d, exe_files=%d\n", sigtmp->com_files, 
		    sigtmp->exe_files);
		#endif

		/* Install signature in the hash table. */
		sigtmp->next_sibling = NULL;
		install_sig(sigtmp, FALSE);
		++sigind;

		/* The following is crude "mutant detection" support.
 		 * Each signature longer than SIZEFRAG is installed in 
	    	 * its entirety in the hash table, but in addition, 
		 * NUMFRAGS chunks of size SIZEFRAG are randomly extracted
		 * from the signature and installed as signatures
  		 * in the signature hash table, with the rest of the
		 * signature information the same as for the full signature. 
		 * SIZEFRAG = 11 was chosen as the minimum size consistent
		 * with no false alarms. SIZEFRAG 8 produced many
 		 * false alarms.
 		 *
 		 * This support supplements the "tolerance" approach, where
		 * a number of mismatched bytes are allowed when matching 
		 * a signature; the number is of mismatches allowed is 
		 * linear with the length of the signature, 11
 		 * byte signatures must be matched completely, and the first
		 * 3 bytes of
  		 * all signatures must be matched completely.
 		 *
 		 * In conjunction (using the -M command line switch) these 
	         * two approaches seem to be adequately powerful to capture
		 * the trivial variants designed to elude normal string 
		 * scanners, at least most of the time. For instance,
 		 * a common attack is to reverse the order of several 
		 * instructions in the virus, thereby not changing the 
		 * virus semantically but changing its appearance enough 
		 * to fool a simple string scanner.
 		 * Often, viruses that are large variations on known viruses 
		 * are also detected, but this is not nearly as certain.
 		 */
		if (mutation_support &&
		    !sigtmp->is_complex_sig &&
		    !sigtmp->disable_mutant_scan &&
		    (int) sigtmp->len_signature > SIZEFRAG)
		{
		   register unsigned int i;
		   register unsigned int j;
		   sigdat *ssigtmp;
		   unsigned int frag_ind;
		   static boolean first_time = TRUE;

		   if (first_time)
		   {
			time_t timeval;

			srand((int) time(&timeval));
			first_time = FALSE;
		   }

		   for (i=0; i<4; ++i)
		   {
			ssigtmp = malloc(sizeof(sigdat));
			if (ssigtmp == NULL)
			{
			   clear_line_if_required();
			   printf(MSGSTR(MALLOC_ERROR, MSG_MALLOC_ERROR), 3);
			   printf(MSGSTR(OUT_OF_MEMORY, MSG_OUT_OF_MEMORY));
			   printf(MSGSTR(OOM_MSG2, MSG_OOM_MSG2));
			   sexit(NORM_ERR);
			}
			memcpy(ssigtmp,sigtmp,sizeof(sigdat));
			for (j=0; j<100; ++j)
			{ 
			   /* Forbid zero and a few other chars
			    * as starting character of a fragment
			    */
			   frag_ind = rand() % ((int) sigtmp->len_signature - 
				(SIZEFRAG - 1) );

			   if (((sigtmp->signature[frag_ind]^0xFF) != 0x00) &&
			       ((sigtmp->signature[frag_ind]^0xFF) != 0xFF) &&
			       ((sigtmp->signature[frag_ind]^0xFF) != 0x20))
				break;
			}
			/*
             		 * Message and signature are shared with the 
			 * parent signature.
             		 */
			ssigtmp->signature = &sigtmp->signature[frag_ind];

			ssigtmp->len_signature = (byte) SIZEFRAG;
			ssigtmp->next_sibling = NULL;
			ssigtmp->second_byte = ssigtmp->signature[1] ^ 0xFF;
			ssigtmp->third_byte = ssigtmp->signature[2] ^ 0xFF;

			if (ssigtmp->at_offset) 
			   ssigtmp->offset += frag_ind;

			ssigtmp->is_sig_frag = TRUE;
			install_sig(ssigtmp, TRUE);
			++sigind;
		   }
		}
	}
	fclose(sigfile);
	/*pr_hash_tables();*/
}

#define NUM_MESSAGES 340
char *messages[NUM_MESSAGES] = { NULL };

void
init_messages(void)
{
	register unsigned int msg_ind;

	for (msg_ind = 0; msg_ind < NUM_MESSAGES; ++msg_ind)
		messages[msg_ind] = "Message file not yet loaded\n";
}

void
get_messages(void)
{
	char           *working_msgfilename;
	char           *msgfilename = "virscan.msg";
	FILE           *msg_file;
	register unsigned int msg_ind;
	static char     progress_line[256];
	char           *tok;
	register int    i;

	#if BOOTS
	{
		static char     working_namebuf[256];

		get_pathname(working_namebuf);
		strcpy(strrchr(working_namebuf, '\\') + 1, msgfilename);
		if (OS_major_ver < 3 &&
		    working_namebuf[0] == '.' &&
		    working_namebuf[1] == '\\')
			working_msgfilename = &working_namebuf[2];
		else
			working_msgfilename = &working_namebuf[0];
		strupr(working_msgfilename);
	}
	#else
	working_msgfilename = msgfilename;
	#endif

	msg_file = fopen(working_msgfilename, "r");
	if (msg_file == NULL)
	{
		clear_line_if_required();
		printf("Error opening message file (%s)\n", 
		    working_msgfilename);

		if (verbose)
			printf("Reason for error : %s\n", strerror(errno));
 
		printf("The message file could not be read. ");
		printf("Make sure that it exists\n");
		sexit(NORM_ERR);
	}

	#if SELF_TEST
	if (do_self_test)
	{

		/*
		 * Here, I'm presuming that the same file can be open twice,
		 * in two different read modes, "rb" and "r". 
		 */
		sprintf(progress_line, 
		     "Testing message file (%s) for modifications.",
		     working_msgfilename);

		if (verbose)
			outp_progress_line(progress_line);
		if (file_has_been_modified(working_msgfilename,
					   workbuf,
					   SIZE_WORK_BUF,
					   MAX_SIZE_SIGN - 1))
		{
			clear_line_if_required();
			printf("The file (%s) has been modified.\n", 
			    working_msgfilename);
			sexit(NORM_ERR);
		}
	}
	#endif
	sprintf(progress_line, "Reading message file (%s)",  
	    working_msgfilename);

	if (verbose)
		outp_progress_line(progress_line);

	msg_ind = 0;
	while (TRUE)
	{
		/* Get message, install it in message array. */
		if (get_line_from_sig_file(progress_line,
		    sizeof(progress_line) - 1, msg_file) == NULL)
			break;

		#if 1
		tok = progress_line;
		while (*(tok++) != '\"')
		{
			;
		}		/* Point tok to char immediately after first
				 * '"' 
				 */
		for (i = 0; tok[i] != '\"'; ++i)
		{
			;
		}
		tok[i] = '\0';
		#else
		strtok(progress_line, "\"");
		tok = strtok(NULL, "\"");
		#endif

		if (msg_ind >= NUM_MESSAGES)
		{
			printf("Error reading message file (%s):", 
			    working_msgfilename);
			printf("Too many messages.\n");
			sexit(NORM_ERR);
		}
		convert_message(tok);
		if ((messages[msg_ind] = strdup(tok)) == (char *)NULL)
		{
		       printf("Error allocating memory for message number %d\n",
			   msg_ind);
		       sexit(NORM_ERR);
		}
		/* sprintf("%s", messages[msg_ind]); */
		++msg_ind;
	}
	fclose(msg_file);
	if (strstr(messages[VERSION_FLAG], "VERSION=PRO") != NULL)
	{
		product_version = TRUE;
	}
}


char           *
message(unsigned int    msg_ind)
{
	static boolean  have_loaded_messages = FALSE;

	if (msg_ind >= NUM_MESSAGES)
	{
		printf("Invalid message number: %d\n", msg_ind);
	}
	if (!have_loaded_messages)
	{
		init_messages();
		get_messages();
		have_loaded_messages = TRUE;
	}
	return messages[msg_ind];
}

/*
 * Parameter filename is a file containing a list of files to scan. Open this
 * file and scan each file named for signatures. 
 */
void
scan_via_listfile(char           *listfilename)
{
	FILE           *listfile;
	int             lenline;
	char            testfilename[256];
	unsigned int    i;

	/*
	 * Open the list file. The first string of each line is a filename.
	 * Open each file, read it up to a buffer (+spill room) at a time and
	 * scan the data read for viral signatures. 
	 */
	listfile = fopen(listfilename, "r");
	if (listfile == NULL)
	{
		clear_line_if_required();
		printf(MSGSTR(SCAN_VIA_LIST, MSG_SCAN_VIA_LIST), listfilename);

		if (verbose)
			printf(MSGSTR(REASON_FOR_ERR, MSG_REASON_FOR_ERR),
			    strerror(errno));

		printf(MSGSTR(FILELIST_ERR, MSG_FILELIST_ERR));
		sexit(NORM_ERR);
	}
	while (fgets(tmplinebuf, SIZE_TMP_LINE_BUF - 1, listfile) != NULL)
	{

		/*
		 * Can't use simple sscanf, as must handle VM filenames. 
		 */
		lenline = strlen(tmplinebuf);
		for (i = 0; i < lenline; ++i)
			if (!(isprint(tmplinebuf[i])))
			{
				tmplinebuf[i] = '\0';
				break;
			}
		strcpy(testfilename, tmplinebuf);

		#if DEBUGGERY
		printf("%s\n", testfilename);
		#endif

		test(testfilename);
	}
	fclose(listfile);
}

#if BOOTS
#include <setjmp.h>
static jmp_buf  mark;

void
non_fatal_error(void)
{
	longjmp(mark, NORM_ERR);
}

/*
 * Scan system boot sector of named drive for known viruses. 
 */
void
test_system_boot_sector(char           *drivename,
		boolean continue_if_error,
		boolean pause_if_removable)
{
	char            locdrivename[3];
	char            boot_sector_name[40];
	char            msgbuf[256];
	char            c;

	if (setjmp(mark))
	{
		clear_line_if_required();
		printf(MSGSTR(BOOT_SCAN_ERR, MSG_BOOT_SCAN_ERR), locdrivename);
		printf(MSGSTR(NETWORK_DRIVE, MSG_NETWORK_DRIVE), locdrivename);
		if (continue_if_error)
		{
			printf(MSGSTR(CONT_SCAN_MSG, MSG_CONT_SCAN_MSG));
			return;
		} else
		{	/* Oops, it really *was* a fatal error! */
			sexit(NORM_ERR);
		}
	}
	locdrivename[0] = toupper(drivename[0]);
	locdrivename[1] = ':';
	locdrivename[2] = '\0';
	do
	{
		if (pause_if_removable &&
		    user_spec_removable[locdrivename[0] - 'A'])
		{
			sprintf(msgbuf, MSGSTR(INS_DISK_MSG, MSG_INS_DISK_MSG),
			    locdrivename);
			if (!verbose)
			{
				pr_then_clear_line(msgbuf);
				prevnamelen = strlen(msgbuf);
			} else
			{
				printf(msgbuf);
			}
			getch();
			printf(cr_works ? " \r" : "\n");
		}
		get_system_boot_sector(locdrivename,
				       workbuf,
				       verbose,
				       non_fatal_error);
		if (display_bootsector)
			dump_bsect(workbuf, 512);
		sprintf(msgbuf, MSGSTR(SCANNING_SBS, MSG_SCANNING_SBS),
		    toupper(drivename[0]));
		outp_progress_line(msgbuf);
		sprintf(boot_sector_name, MSGSTR(BS_NAME, MSG_BS_NAME),
		    locdrivename);

		reset_object_counters();
		scanworkbuf(512,		/* 512 bytes in sector */
			    boot_sector_name,	
			    TRUE,		/* is last buffer to scan */
			    FALSE,		/* is *not* a .exe */
			    0l,			/* starting at 0 */
			    TRUE,		/* Scan for 1260. (Why not..) */
			    T_SYSTEM_BOOT_SECTOR);     /* What we're scanning */
		messages_if_mutants();
		output_logfile_line(NULL, boot_sector_name, 0l, 0);
		++system_bsector_count;
		if (pause_if_removable &&
		    user_spec_removable[locdrivename[0] - 'A'])
		{
			if (cr_works)
			{
				pr_then_clear_line("");
				printf("\r");
			}
			printf(MSGSTR(SCAN_ANOTHER, MSG_SCAN_ANOTHER));
			c = (char) getch();
			printf(cr_works && !verbose ? " \r" : "\n");
			if (toupper(c) != 'Y')
				break;
		}
	} while (pause_if_removable &&
		 user_spec_removable[locdrivename[0] - 'A']);
}

/*
 * Scan master boot record of specified for known viruses. 
 */
void
test_master_boot_record(unsigned int physical_drive,
			boolean continue_if_error)
{
	int             rc;
	char            boot_sector_name[40];
	char            msgbuf[256];

	/* Define big strings only once */

	rc = get_master_boot_record(physical_drive, workbuf);
	if (rc != 0)
	{
		clear_line_if_required();

		if (physical_drive >= 0x80)
			printf(MSGSTR(MASTER_ERR_HARD, MSG_MASTER_ERR_HARD),
			   physical_drive, rc);
		else
			printf(MSGSTR(MASTER_ERR, MSG_MASTER_ERR),
			   physical_drive, rc);
		
		printf(MSGSTR(MASTER_NOT_SCANNED, MSG_MASTER_NOT_SCANNED));
		if (continue_if_error)
		{
			printf(MSGSTR(CONT_SCAN_MSG, MSG_CONT_SCAN_MSG));
			return;
		} else
			sexit(NORM_ERR);
	}
	if (display_bootsector)
		dump_bsect(workbuf, 512);

	sprintf(msgbuf, MSGSTR(SCANNING_MBR), physical_drive);
	outp_progress_line(msgbuf);
	sprintf(boot_sector_name, MSGSTR(MBR_NAME), physical_drive);

	reset_object_counters();
	scanworkbuf(512,	  	/* 512 bytes in sector */
		    boot_sector_name,	
		    TRUE,		/* is last buffer to scan */
		    FALSE,		/* is *not* a .exe */
		    0l,			/* starting at 0 */
		    TRUE,		/* Scan for 1260 (Why not?) */
		    T_MASTER_BOOT_RECORD);	/* What we're scanning */

	messages_if_mutants();
	output_logfile_line(NULL, boot_sector_name, 0l, 0);
	++master_brecord_count;
}

#endif /* BOOTS */

#if SEARCH_DRIVE_ENABLE

/*
 * Scan all .EXE and .COM files in the subdirectory tree specified by the
 * parameter pathname, which can be simply a drive letter. Note that the
 * traversal routine wants the pathname parameter terminated by a backslash. 
 *
 * If boot sector support is enabled, scan the system boot sector for the
 * specified drive. Also try to scan the master boot record if a: or c: are
 * specified. 
 */
void
test_drivename(char *drivename)
{
	char            local_drivename[256];
	char            c;
	char            msgbuf[256];

	strupr(strcpy(local_drivename, drivename));
	do
	{
		if (is_valid_drive_letter(local_drivename[0]) &&
		    user_spec_removable[local_drivename[0] - 'A'])
		{
			char            short_dname[3];

			memcpy(short_dname, local_drivename, 2);
			short_dname[2] = '\0';
			sprintf(msgbuf, MSGSTR(INS_DISK_MSG, MSG_INS_DISK_MSG),
			    short_dname);
			outp_progress_line(msgbuf);
			if (!verbose)
			{
				pr_then_clear_line(msgbuf);
				prevnamelen = strlen(msgbuf);
			}
			getch();
			printf(cr_works ? " \r" : "\n");
		}
		if (local_drivename[strlen(local_drivename) - 1] != '\\')
		{
			strcat(local_drivename, "\\");
		}
		if (verbose)
			printf(MSGSTR(SCANNING, MSG_SCANNING), local_drivename);

		#if DEBUGGERY
		printf("File specification=%s\n", file_specification);
		#endif

		traverse(local_drivename, file_specification, test, norm_error);

		#if BOOTS
		if (!explicit_searches_only &&
		    local_drivename[1] == ':' &&
		    is_valid_drive_letter(local_drivename[0]))
		{
			test_system_boot_sector(local_drivename, TRUE, FALSE);
			if (RealMode())
			{

				/*
				 * Now test the master boot record once only,
				 * iff a drive letter >= C: is specified. 
				 */
				#if 0
				switch (toupper(local_drivename[0]))
				{
				case ('A'):
					test_master_boot_record(0x00, TRUE);
					break;
				case ('C'):
					test_master_boot_record(0x80, TRUE);
					break;
				default:
					break;
				}
				#else

				/*
				 * Test the master boot record of drive 80H
				 * only once. 
				 */
				if (toupper(local_drivename[0]) >= 'C')
				{
					if (do_mbr_scan &&  
					    !have_tested_master_boot_rec)
					{
					   test_master_boot_record(0x80, TRUE);
					   have_tested_master_boot_rec = TRUE;
					}
				}
				#endif
			}
		}
		if (is_valid_drive_letter(local_drivename[0]) &&
		    user_spec_removable[local_drivename[0] - 'A'])
		{
			if (cr_works)
			{
				pr_then_clear_line("");
				printf("\r");
			}
			printf(MSGSTR(SCAN_ANOTHER, MSG_SCAN_ANOTHER));

			c = (char) getch();
			printf(cr_works && !verbose ? " \r" : "\n");
			if (toupper(c) != 'Y')
				break;
		}
	} while (isalpha(local_drivename[0]) &&
		 user_spec_removable[local_drivename[0] - 'A']);

		#endif  /* BOOTS */
}

#endif	/* SEARCH_DRIVE_ENABLE */


/*
 * Process the command line, display outcome messages. The command line could
 * be quite complicated. The command line is first scanned for arguments
 * which control processing, then it is scanned for arguments specifying
 * *what* to scan. The command line is processed from left to right, twice,
 * unless command line processing is aborted for some reason, such as the
 * display of a help message. 
 *
 * Various progress messages are displayed during the actual scan, and then a
 * summary of results is displayed and the program exits. 
 */
void
main(int  argc,
     char **argv)
{
	unsigned int    i;	/* loops */
	char           *sigfilename = NULL;
	char           *listfilename = NULL;
	char           *singlefilename = NULL;
	char           *drivename = NULL;
	int             stuff_to_process_cnt = 0;	/* Count of things to
							 * process */
     char time_string[OUTPUT_SZ+1];		/* locale-formatted time string */
	struct tm *timp, tim;

	/*
	 * Get the start time. 
	 */
	time(&begin_time);

	#if FREQ_COUNTS
	/*
	 * If we are counting character frequencies, erase count table. 
	 */
	memset(ccounts, 0, sizeof(ccounts));
	#endif

	#if BOOTS
	/*
	 * Get the OS version. Defaults to 0.0 if this code isn't compiled. 
	 */
	get_version(&OS_major_ver, &OS_minor_ver);
	if (RealMode() && OS_major_ver >= 10)
		do_high_memory_scan = FALSE;

	#if DEBUGGERY
	printf("%d %d\n", OS_major_ver, OS_minor_ver);
	#endif
	#endif /* BOOTS */

	/*
	 * Make DOS 2.1 work. Slows down execution in some other
	 * environments. I have no idea why this works (or why I even tried
	 * it), but it does. 
	 */
	#if BOOTS
	if (OS_major_ver <= 2)
	{
		setbuf(stdout, NULL);
		setbuf(stderr, NULL);
	}
	#endif

	#ifdef _AIX
	/*
	 * Set the collating and time for this language. 
	 */
	(void) setlocale(LC_ALL, "");

	catd = catopen(MF_VIRSCAN, NL_CAT_LOCALE);
	#endif

	#ifndef _AIX
	/*
 	 * Very rough pass over command line arguments for presence of the -NST
 	 * switch. 
 	 */
	for (i = 1; i < argc; ++i)
	{
		if (!strcmp(strupr(strcpy(tmplinebuf, argv[i])), "-NST"))
		{
			do_self_test == FALSE;
			break;
		}
	}
	#endif

    /*
     * First pass over command line arguments, for switches that indirectly
     * control processing. 
     */
    for (i = 1; i < argc; ++i)
    {
	switch (argv[i][0])
	{
	   case ('-'):
	   #ifndef _AIX                  
	   case ('/'):
	   #endif
		switch (tolower(argv[i][1]))
		{
	 	   #if SEARCH_DRIVE_ENABLE
		   case ('r'):
			if (strlen(&argv[i][2]) == 0)
			{
			   printf(MSGSTR(RDL_NOT_SPEC, MSG_RDL_NOT_SPEC));
			   invalid_option(argv[i]);
			   sexit(NO_ERR);
			} else
			{
		   	   if (!is_valid_drive_letter(argv[i][2]))
			   {
				printf(MSGSTR(INVALID_RDL, MSG_INVALID_RDL));
				invalid_option(argv[i]);

				/* Blat out command line */
				argv[i][0] = '\0'

				/* sexit(NO_ERR); */
		  	   } else
		   	   {
				user_spec_removable[toupper(argv[i][2])-'A'] = 
					TRUE;
		  	   }
			}
			break;

	  	   case ('a'):
			strcpy(file_specification, "*.*");
			break;

	  	   case ('w'):
			if (strlen(file_specification) != 0)
			   strcat(file_specification, ",");
		  	   strcat(file_specification, &argv[i][2]);
			break;

		   case ('d'):
			#if BOOTS
			++stuff_to_process_cnt;
			if (argv[i][3] == ':')
			{
		 	   if (!is_valid_drive_letter(argv[i][2]))
		  	   {
				printf(MSGSTR(NO_SUCH_DRIVE, MSG_NO_SUCH_DRIVE),
				   toupper(argv[i][2]));

				/* Print command line */
				argv[i][0] = '\0'

		 		--stuff_to_process_cnt;
		 		/* sexit(NORM_ERR); */
			   }
		 	}
			break;

		   case ('b'):
			++stuff_to_process_cnt;
			if (isalpha(toupper(argv[i][2])) && argv[i][3] == ':')
			{
		  	   if (!is_valid_drive_letter(argv[i][2]))
		   	   {
				printf(MSGSTR(NO_SUCH_DRIVE, MSG_NO_SUCH_DRIVE),
				   toupper(argv[i][2]));

				/* Print command line */
				argv[i][0] = '\0'

				--stuff_to_process_cnt;
			   }
			}
			break;
			#endif /* endif BOOTS */
		   #endif /* endif SEARCH_DRIVE_ENABLE */

		   #ifdef _AIX
	   	   /*
             	    * Scan all files on indicated filesystem.
             	    */
         	   case ('a'):
            		++stuff_to_process_cnt;
			strcpy(file_specification, SCAN_ALL);
            		break;

         	   /*
             	    * Process a wildcard specification of 
	  	    * what files to process.
              	    */
         	   case ('w'):
            		++stuff_to_process_cnt;
            		if (strlen(file_specification) != 0)
               			strcat(file_specification, ",");

            		strcat(file_specification, &argv[i][2]);
            		break;
	  	   #endif /* if AIX */

	 	   case ('l'):
			if (strlen(&argv[i][2]) == 0)
			{
		  	   printf(MSGSTR(NO_FN_FILELIST, MSG_NO_FN_FILELIST));
			   invalid_option(argv[i]);
			   sexit(NO_ERR);
			}
			++stuff_to_process_cnt;
			break;

		   case ('t'):
			if (strlen(&argv[i][2]) == 0)
			{
			   printf(MSGSTR(NO_FN_FILE, MSG_NO_FN_FILE));
			   invalid_option(argv[i]);
		   	   sexit(NO_ERR);
			}	
			++stuff_to_process_cnt;
			break;

		   case ('g'):
			always_warn = TRUE;
			break;

		   case ('e'):
			explicit_searches_only = TRUE;
			break;

	  	   case ('m'):
			if (strlen(&argv[i][1]) > 1)
			{
			   switch (tolower(argv[i][2]))
			   {
			      	case ('t'):
				   tolerance_allowed= TRUE;
				   default_mutant_detection=FALSE;
			     	   break;

			        case ('f'):
		 		   mutation_support = TRUE;
				   default_mutant_detection=FALSE;
			  	   break;

				case ('m'):
				   tolerance_allowed = TRUE;
				   more_tolerance_allowed = TRUE;
				   mutation_support = TRUE;
				   default_mutant_detection=FALSE;
		           	   break;

				#if SEARCH_DRIVE_ENABLE
				case ('e'):
				   if (tolower(argv[i][3]) == 'm')
				   {
					++stuff_to_process_cnt;
					break;
				   } else
			 	   {
					invalid_option(argv[i]);
					sexit(NO_ERR);
				   }
				   break;
				#endif

			        default:
				   invalid_option(argv[i]);
				   sexit(NO_ERR);
				   break;
			   } /* end switch */
			} 
			else
			{
			   tolerance_allowed = TRUE;
			   mutation_support = TRUE;
		 	   default_mutant_detection=FALSE;
			}
			break;

		   case ('s'):
			break;

		   case ('q'):
			quiet = TRUE;
			verbose = FALSE;
			if (strlen(&argv[i][1]) > 1 && tolower(argv[i][2])=='q')
			   very_quiet = TRUE;

			#ifndef _AIX
		   	display_bootsector = FALSE;
			#endif
			break;

		   case ('v'):
			if (strlen(&argv[i][1]) > 1)
			   switch (tolower(argv[i][2]))
			   {
				/*
				 * For each file, we want a report.
				 * If the file has no positives, say
				 * so, else one line (including
				 * filename) per hit. This may mean a
				 * special routine that updates this
				 * file once per call, which is
				 * called every time a positive is
				 * found in an object, or if no
				 * positives were found in the
				 * object, then after the object scan
				 * is finished. 
				 */
				case ('l'):
				   create_logfile = TRUE;
				   logfilename = &argv[i][3];
				   if (strlen(logfilename) == 0)
					logfilename = "virscan.lgf";
	  			   break;

 				case ('v'):
				   #ifndef _AIX
				   display_bootsector = TRUE;
				   #endif

			    	default:
				   quiet = FALSE;
				   very_quiet = FALSE;
				   verbose = TRUE;
				   cr_works = FALSE;

			   } /* end switch */
			else
			{
			   quiet = FALSE;
			   very_quiet = FALSE;
			   verbose = TRUE;
			   cr_works = FALSE;
			}
			break;

		   #ifndef _AIX
		   case ('c'):
			if (strlen(&argv[i][1]) >= 3 &&
			    tolower(argv[i][2]) == 'a' &&
			    tolower(argv[i][2]) == 'd')
				continue_if_access_denied=TRUE;
			else
				continue_if_open_error = TRUE;
			break;
		   #endif

		   case ('n'):
			if (strlen(&argv[i][1]) > 1) 
			   switch (tolower(argv[i][2]))
			   {
				case ('p'):
				   cr_works = FALSE;
				   break;

				#ifdef _AIX
                                case ('m'):
                                   if (tolower(argv[i][3]) == 'u' &&
                                       tolower(argv[i][4]) == 't')
                                   {
                                        tolerance_allowed=FALSE;
                                        more_tolerance_allowed = FALSE;
                                        mutation_support=FALSE;
                                        default_mutant_detection= TRUE;
                                   }
                                   else
                                   {
                                        invalid_option(argv[i]);
                                        sexit(NO_ERR);
                                   }
                                   break;
                                #endif


				#if SELF_TEST
				case ('s'):
				   if (tolower(argv[i][3]) == 't')
					do_self_test = FALSE;
				   else
				   {
				   	invalid_option(argv[i]);
					sexit(NO_ERR);
				   }
				   break;
				#endif

				#if SEARCH_DRIVE_ENABLE
				case ('h'):
				   if (tolower(argv[i][3])=='m' &&
				       tolower(argv[i][4]) == 's')
				    	do_high_memory_scan = FALSE;
				   else
				   {
				    	invalid_option(argv[i]);
				    	sexit(NO_ERR);
				   }
				   break;

				case ('m'):
			  	   if (tolower(argv[i][3]) == 's')
					do_memory_scan = FALSE;
				   else
				   {
				   	if (tolower(argv[i][3])=='b' && 
					    tolower(argv[i][4]) == 'r' &&
					    tolower(argv[i][5] =='s')
						do_mbr_scan = FALSE;
				   	else
					{
				       	   if (tolower(argv[i][3]) == 'u' &&
					       tolower(argv[i][4]) == 't')
					   {
						tolerance_allowed=FALSE;
						more_tolerance_allowed = FALSE;
						mutation_support=FALSE;
						default_mutant_detection= TRUE;
					   }
					   else
				           {
						invalid_option(argv[i]);
						sexit(NO_ERR);
				       	   }
				   	}
				   }
	   		           break;
				#endif /* if SEARCH_DRIVE_ENABLE */

			     	case ('l'):
				   if (Product_Version && 
				       tolower(argv[i][3])=='a')
			       		display_license_agreement =FALSE;
				   else
				   {
					invalid_option(argv[i]);
					sexit(NO_ERR);
				   }
				   break;

				case ('b'):
				   beep_if_sig_found=FALSE;
				   break;

			  	default:
				   invalid_option(argv[i]);
				   sexit(NO_ERR);
				   break;
	  		   }
			else
			{
			   invalid_option(argv[i]);
			   sexit(NO_ERR);
			}
			break;

		   case ('p'):
			list_positives = TRUE;
			positives_filename = &argv[i][2];
			if (strlen(positives_filename) == 0)
			   positives_filename = "positive.vir";
			break;

		   #if BOOTS
		   case ('z'):
			beep_loop_if_sigsfound = TRUE;
			break;
		   #endif

		   case ('?'):
		   case ('h'):
			if (!strcmp(&argv[i][1], "??") ||
			    !strcmp(&argv[i][1], "hh")) 	
			{
			   pr_examples_of_usage();
			   sexit(NO_ERR);
			}
		      	pr_usage();
		      	sexit(NO_ERR);
		      	break;
		   default:
		      	pr_short_usage();
		      	sexit(NO_ERR);
		      	break;
		}
		break;

		case ('?'):
		   if (!strcmp(argv[i], "?"))
		   {
			pr_usage();
			sexit(NO_ERR);
		   } else
		   if (!strcmp(argv[i], "??"))
	 	   {
			pr_examples_of_usage();
			sexit(NO_ERR);
		   } else
		   {
			pr_short_usage();
			sexit(NO_ERR);
		   }
		   break;

	   	default:
		   #ifdef _AIX
		   if (strchr(argv[i], '/') != NULL ||
		       strchr(argv[i], '.') != NULL)
		   {
		   	++stuff_to_process_cnt;
		   	if (!is_valid_directory(argv[i]))
		   	{
			   fprintf(stderr, MSGSTR(INVALID_DIR,MSG_INVALID_DIR),
			       argv[i]);
			   argv[i][0] = (char) NULL;
			   --stuff_to_process_cnt;
			}
			break;
		   }
		   #endif  /* end if AIX */

		   #if SEARCH_DRIVE_ENABLE
		   if (strchr(argv[i], ':') != NULL ||
		       strchr(argv[i], '\\') != NULL ||
		       strchr(argv[i], '.') != NULL)
		   {
			++stuff_to_process_cnt;
		 	#if BOOTS
			if (argv[i][1] == ':')
			{
			   if (!is_valid_drive_letter(argv[i][0]))
			   {
				printf(MSGSTR(NO_SUCH_DRIVE, MSG_NO_SUCH_DRIVE),
				   toupper(argv[i][0]));
				argv[i][0] = '\0';   /* Blat out command line */
	
				--stuff_to_process_cnt;
				/* sexit(NORM_ERR); */
			   }
			}
			#endif
			break;
		   }
		   #endif /* if SEARCH_DRIVE_ENABLE */

		   pr_short_usage();
		   sexit(NO_ERR);
		   break;
		}
	   }

	   #ifndef _AIX
	   if (Product_Version)
	   {
		if (argc != 1 && display_license_agreement && !quiet && 
		    !very_quiet)
		{
		   char            c;

		   printf(MSGSTR(PVERSION_BANNER, MSG_PVERSION_BANNER));
		   printf(MSGSTR(PLA_02, MSG_PLA_02));
		   c = (char) getch();
		   if (toupper(c) != 'Y')
			sexit(0);
		}
	   } else
	   {
	        if (!quiet)
	   	{
		   printf(MSGSTR(COPYRIGHT, MSG_COPYRIGHT));
		   printf(MSGSTR(IUO_BANNER, MSG_IUO_BANNER));
		   printf(MSGSTR(VERSION_BANNER, MSG_VERSION_BANNER));
		}
	   }
	   #endif

	   if (argc == 1)
	   {
		pr_short_usage();
		sexit(NO_ERR);
	   }

	   /*
	    * Second pass over command line arguments. Only does loading of
	    * signature files specified with the -S option. This pass is broken
	    * out into a separate loop so that the -M and -V (and related)
	    * modifiers will modify the loading of signature files regardless of
	    * the order of arguments on the command line. 
	    */
	   for (i = 1; i < argc; ++i)
	   {
		switch (argv[i][0])
		{
		   case ('-'):
		   #ifndef _AIX
		   case ('/'):
		   #endif
			if (tolower(argv[i][1]) == 's')
			{
			   sigfilename = &argv[i][2];
			   if (strlen(sigfilename) == 0)
			   {
				printf(MSGSTR(NO_FN_SIGFILE,MSG_NO_FN_SIGFILE));
				invalid_option(argv[i]);
				sexit(NO_ERR);
			   } else
			   {
				 get_signatures(sigfilename, TRUE, FALSE, TRUE);
			   }
			   break;
			}
			break;

		   default:
			break;
		}
	   }

	   #if SELF_TEST
	   if (do_self_test)
	   {
		char           *nameptr;
		char            progress_line[256];

		get_pathname(tmplinebuf);
		if (OS_major_ver < 3 && tmplinebuf[0] == '.' && 
		    tmplinebuf[1] == '\\')
	  	   nameptr = &tmplinebuf[2];
		else
		   nameptr = tmplinebuf;

		sprintf(progress_line, MSGSTR(TESTING_FILE,MSG_TESTING_FILE), 
		   nameptr);

		outp_progress_line(progress_line);
		if (file_has_been_modified(nameptr,
					   workbuf,
					   SIZE_WORK_BUF,
					   MAX_SIZE_SIGN - 1))
		{
		   clear_line_if_required();
		   printf(MSGSTR(FILE_MOD_MSG, MSG_FILE_MOD_MSG), nameptr);
		   sexit(NORM_ERR);
		}
	   }
	   if (!Product_Version)
	   {
		extern boolean  six_months_elapsed(void);

		if (six_months_elapsed() && !quiet)
		{
		   printf("\n");
		   printf(MSGSTR(SIX_MONTHS, MSG_SIX_MONTHS), __DATE__);
		   printf("\n");
		}
	   }
	   #endif

	   if (stuff_to_process_cnt == 0)
	   {
		clear_line_if_required();
		printf(MSGSTR(NOTHING_TO_SCAN, MSG_NOTHING_TO_SCAN));
		sexit(NO_ERR);
	   } else
	   {
		if (!quiet)
		{
		   time(&scan_starttime);
		   timp = localtime(&scan_starttime);
		   tim = *timp;
		   clear_line_if_required();
		   strftime(time_string,OUTPUT_SZ,"%c",&tim);
		   printf(MSGSTR(STARTING_VSCAN, MSG_STARTING_VSCAN),
		       time_string);
		   printf("\n");
		}
	   }

	   /*
	    * Get the signature database. 
	    */
	   #ifdef _AIX
	   if (sigfilename == (char *)NULL)
		get_signatures(VIRSIG_PATH, FALSE, FALSE, TRUE);
	   if (sigfilename == (char *)NULL)
		get_signatures(ADDENDA_PATH, FALSE, FALSE, FALSE);
	   #else
	   if (sigfilename == (char *)NULL)
		get_signatures("virsig.lst", FALSE, do_self_test, TRUE);
	   if (sigfilename == (char *)NULL)
		get_signatures("addenda.lst", FALSE, FALSE, FALSE);
	   #endif

	   /*
	    * At this point, signatures should have been loaded. If none were
	    * loaded, abort with a message. 
	    */
	   if (sig_count == 0)
	   {
		clear_line_if_required();
		printf(MSGSTR(NO_SIGNATURES, MSG_NO_SIGNATURES));
		sexit(NO_ERR);
	   }
	   if (verbose)
	   {
		printf(MSGSTR(LOADED_N_SIGS, MSG_LOADED_N_SIGS),
		    whole_sig_count);
		printf(MSGSTR(LOADED_N_SIGFS, MSG_LOADED_N_SIGFS),
		    frag_sig_count);
	   }
	   /* pr_hash_tables(); */

	   /*
	    * Test memory for signatures of viruses, presumably resident
	    * viruses. Only do this if in real mode, and certainly only in
	    * PC-DOS version. 
	    */
           #if SEARCH_DRIVE_ENABLE
	   if (do_memory_scan && RealMode())
		test_memory();
           #endif

	   /*
	    * Set the default file specification, if a file specification has
	    * not been specified on the command line. 
	    */
	   if (strlen(file_specification) == 0)
		#ifdef _AIX
		strcpy(file_specification, SCAN_EXE);
		#else
		strcpy(file_specification, 
		    "*.exe,*.com,*.ov?,*.ini,*.sys,*.bin");
		#endif

	   /*
	    * Process command line arguments which specify what to test for
	    * signatures. 
	    */
	   for (i = 1; i < argc; ++i)
	   {
		switch (argv[i][0])
		{
		    case ('\0'): /* Argument was deleted in previous pass */
			break;

		    case ('-'):
		    #ifndef _AIX
		    case ('/'):
		    #endif
			switch (tolower(argv[i][1]))
			{
			   case ('s'):
			   case ('g'):
			   case ('q'):
			   case ('v'):
		 	   case ('c'):
			   case ('n'):
			   case ('e'):
			   case ('p'):
			   case ('?'):
			   case ('h'):
			   case ('a'):
			   case ('w'):
			   case ('z'):
			   case ('r'):
				/* Have already been processed. */
				break;
			   case ('m'):
			   	#if SEARCH_DRIVE_ENABLE
				if (tolower(argv[i][2]) == 'e' &&  
				    tolower(argv[i][3]) == 'm')
				{
				   if (RealMode())
				   {
					if (!have_scanned_system_memory)
					   test_memory();

					/*
					 * Even if -NMS is specified!
					 * If -NHMS is specified, 
					 * then test_memory will scan
					 * only the low 640K 
					 * 
					 * If not in real mode,
					 * should never get this far, 
					 * but we check just to be sure 
					 */
				   } else
				   {
					clear_line_if_required();
					printf(MSGSTR(SSM_NOT_SUPPORT, 
						MSG_SSM_NOT_SUPPORT));
					invalid_option(argv[i]);
					sexit(NO_ERR);
				   }
				}
				#endif
				break;

			   case ('l'):
				scan_via_listfile(&argv[i][2]);
				break;
			   case ('t'):
				test(&argv[i][2]);
				break;

			   #if SEARCH_DRIVE_ENABLE
			   case ('d'):
				test_drivename(&argv[i][2]);
				break;
			   #endif

			   #if BOOTS
			   case ('b'):
				/*
				 * Argument for physical drive is hex, but
				 * the first hex digit will be 0-9 
				 */
				if (isdigit(argv[i][2]))
				{
				   if (RealMode())
				   {
					unsigned int    physical_drive;

					sscanf(&argv[i][2], "%2x", 
					   &physical_drive);
					test_master_boot_record(physical_drive,
					   FALSE);

					if (physical_drive = 0x80)
					   have_tested_master_boot_rec = TRUE;
				   } else
				   {
					clear_line_if_required();
					printf(MSGSTR(MBRS_NOT_SUPP, 
					   MSG_MBRS_NOT_SUPP));
					invalid_option(argv[i]);
					sexit(NORM_ERR);
				   }
				} else
				if (isalpha(toupper(argv[i][2])) && 
				    argv[i][3] == ':')
				{
				   /*
				    * Have already tested drive letter
				    * for validity 
				    */
				   test_system_boot_sector(&argv[i][2], FALSE,
					 TRUE);
				} else
				{
				   clear_line_if_required();
				   invalid_option(argv[i]);
				   sexit(NO_ERR);
				}
				break;
		   	   #endif

			   default:
				clear_line_if_required();
				printf("Impossible error # 1...\n");
				sexit(NORM_ERR);
				break;
			}
			break;

		   default:
		       	#if SEARCH_DRIVE_ENABLE
			/* Have already tested invalid argument */
			test_drivename(argv[i]);
			#endif

			#ifdef _AIX
         		drivename = (char *)malloc(strlen(argv[i]) * 
				sizeof(char) + 3);
         		strcpy (drivename, argv[i]);
         		strcat (drivename, "/");
         		traverse(drivename,file_specification,test, sexit);
			#endif

			break;
		}	
	   }


	   /*
	    * Completion. Output completion messages. Free tables. (Not
	    * necessary) Exit with correct error level. 
	    */

	   if (!quiet)
	   {
		if (cr_works)
		{
		   if (!verbose)
		   {
			pr_then_clear_line(MSGSTR(SCAN_COMPLETE01, 
			   MSG_SCAN_COMPLETE01));
			printf("\n");
		   } else
			printf(MSGSTR(SCAN_COMPLETE02, MSG_SCAN_COMPLETE02));
		} else
		{
		   printf(MSGSTR(SCAN_COMPLETE02, MSG_SCAN_COMPLETE02));
		}

		#ifdef _AIX
		if (filecnt == 0)
		#else 
		if (filecnt == 0 && system_bsector_count == 0 &&
		    master_brecord_count == 0 && !have_scanned_system_memory)
		#endif
		   printf(MSGSTR(NOTHING_SCANNED, MSG_NOTHING_SCANNED));

		if (filecnt != 0)
		{
		   printf(filecnt == 1 ? MSGSTR(FILE_SCANNED,MSG_FILE_SCANNED) :
		       MSGSTR(FILES_SCANNED, MSG_FILES_SCANNED), filecnt);
		}
		#ifndef _AIX
		if (system_bsector_count != 0)
		{
		   printf(system_bsector_count == 1 ?
			MSGSTR(SBS_SCANNED, MSG_SBS_SCANNED) :
		       	MSGSTR(SBSS_SCANNED, MSG_SBSS_SCANNED),
			 system_bsector_count);
		}
		if (master_brecord_count != 0)
		{
		   printf(master_brecord_count == 1 ?
		       MSGSTR(MBR_SCANNED, MSG_MBR_SCANNED) :
		       MSGSTR(MBRS_SCANNED, MSG_MBRS_SCANNED),
		     	master_brecord_count);
		}
		#endif

		#if SEARCH_DRIVE_ENABLE
		if (have_scanned_system_memory)
		{
		   printf(always_warn ? MSGSTR(SM_SCANNED, MSG_SM_SCANNED) :
			 MSGSTR(SM_PART_SCANNED, MSG_SM_PART_SCANNED));
		}
		#endif

		time(&end_time);
		printf((end_time - begin_time) == 1l ?
		       MSGSTR(TOTAL_B_SCAN01, MSG_TOTAL_B_SCAN01) :
		       MSGSTR(TOTAL_B_SCAN02, MSG_TOTAL_B_SCAN02),
		       total_scanlength,
		       end_time - begin_time);

		if (sigfound_cnt != 0)
		{
		   if (sigfound_cnt == 1 && infected_object_cnt == 1)
			printf(MSGSTR(VSF_COUNT_11, MSG_VSF_COUNT_11),
			   sigfound_cnt, infected_object_cnt);
		   else
			if (sigfound_cnt > 1 && infected_object_cnt == 1)
			   printf(MSGSTR(VSF_COUNT_21, MSG_VSF_COUNT_21),
			      sigfound_cnt, infected_object_cnt);
			else
			   if (sigfound_cnt == 1 && infected_object_cnt > 1)
				printf(MSGSTR(VSF_COUNT_12, MSG_VSF_COUNT_12),
				   sigfound_cnt, infected_object_cnt);
			   else
				if (sigfound_cnt > 1 && infected_object_cnt > 1)
				   printf(MSGSTR(VSF_COUNT_22,MSG_VSF_COUNT_22),
				      sigfound_cnt, infected_object_cnt);
		} else
		{
		   printf(MSGSTR(NO_VIRUS_FOUND, MSG_NO_VIRUS_FOUND));
		}
	   }
	   free_hash_tables();

 	   #if FREQ_COUNTS
	   printf("Character counts:\n");
	   for (i = 0; i < 256; ++i)
		printf("%02X:%08ld\n", i, ccounts[i]);
	   printf("total bytes counted=%ld\n", total_scanlength);
	   #endif

	   if (!Product_Version)
	   {
		if (sigfound_cnt && !very_quiet)
		{
		   printf(MSGSTR(PLEASE_REPORT01, MSG_PLEASE_REPORT01));
		}
	   }

	   #if BOOTS
	   if (signature_has_been_found && beep_loop_if_sigsfound)
		beep_until_key_pressed();
	   #endif

	   if (signature_has_been_found)
		sexit(SIG_FOUND);
	   else
		sexit(NO_ERR);
}
