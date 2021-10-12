static char sccsid[] = "@(#)18  1.4  src/bos/diag/tu/lega/load_coff.c, tu_lega, bos411, 9428A410j 4/7/92 12:06:27";
/*
 * COMPONENT_NAME: (tu_lega) Low End Graphics Adapter Test Units
 *
 * FUNCTIONS: load_executable_coff(), load_section(), flip_short(),
 *            flip_long(), flip_header_short(), flip_header_long(),
 *            check_dsp_address(), partial_load_executable_coff(),
 *            partial_load_section()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 * MODULE NAME: load_coff.c
 *
 * STATUS: Release 1, EC 00, EVT Version 1
 *
 * DEPENDENCIES:  Pointers to BIM registers properly initialized.
 *
 * RESTRICTIONS:  None.
 *
 * EXTERNAL REFERENCES
 *
 *     OTHER ROUTINES:  fopen, fread, fseek, fclose
 *
 *     DATA AREAS:  Global Variable - bim_base_addr
 *
 *     TABLES:  None.
 *
 *     MACROS:  None.
 *
 * COMPILER / ASSEMBLER
 *
 *     TYPE, VERSION: AIX C Compiler, version 3
 *
 *     OPTIONS:
 *
 * NOTES:  None.
 *
 */
#include <stdio.h>
#include   <fcntl.h>
#include   "bim_defs.h"

#define TRUE 1
#define FALSE 0

/* defines for DSP memory segments accessible through BIM */
#define MEMORY_SEG_MIN 0x0         /* beginning of SRAM */
#define MEMORY_SEG_MAX 0x3FFFF     /* end of SRAM */

/* fseek specifications */
#define WHENCE_ABS      0       /* Seek absolute from the beginning of file */
#define WHENCE_REL      1       /* Seek relative to current position */

/*  C.O.F.F. structure DEFINES used for loading executable code */
#define COFF_MAGIC_NUMBER     0x93  /* magic number indicating COFF */
#define FILE_HEADER_SIZE        20  /* Size of file header, in bytes.    */
#define MAGIC_NUMBER_OFFSET      0  /* Offset into file header for field */
                                    /* containing the file type */
#define NUM_SEC_HEADERS_OFFSET   2  /* Offset into file header for field */
                                    /* containing number of section headers */
#define OPT_HEADER_SIZE_OFFSET  16  /* Offset into file header, for field   */
                                    /* containing optional header size  */
#define SECTION_HEADER_SIZE     40  /* Size of section header, in bytes */
#define PHYSICAL_ADDRESS_OFFSET  8  /* Offset in section header for field */
                                    /* containing physical address    */
#define SECTION_SIZE_OFFSET     16  /* Offset in section header for field */
                                    /* containing section size in words  */
#define RAW_DATA_F_PTR_OFFSET   20  /* Offset in section header for field */
                                  /* containing file pointer to the raw data */

/* Error codes returned by functions in this module */
#define FOPEN_ERROR       0x10 /* Error in opening the file */
#define FH_READ_ERROR     0x11 /* Error in reading file header */
#define SH_READ_ERROR     0x12 /* Error in reading section header */
#define FILE_TYPE_ERROR   0x13 /* Wrong type of file, cannot load */
#define SEEK_ERROR        0x14 /* Error in seek operation */
#define READ_ERROR        0x15 /* Error in read operation */
#define DSP_ADDRESS_ERROR 0x16 /* error in converting DSP address */
#define CRC_ERROR         0x17 /* Error in verifying code's CRC */


/* Note: the following constant used in this file, ties to same in m_hshk.c */
/*       b_boot.asm. Change must be made in all three places                */
#define END_LOAD       0xEEEEEEEE

/* Declare GLOBAL variables */
extern volatile unsigned long *bim_base_addr;    /* Adapter's Base Address */
extern int      dd;

/* LOCAL variables for this file */

static char file_header[FILE_HEADER_SIZE];       /* holds file header info */
static char section_header[SECTION_HEADER_SIZE]; /* holds section header info */
static unsigned long crcval;             /* holds results of CRC accumulation */
static pc_compile;


int check_dsp_address(long address);     /* function prototype */
long flip_long(unsigned char *bp);

/*
 * NAME: load_executable_coff
 *
 * FUNCTION: Load the graphics adapter, DSP program memory, with
 *           DSP-executable code from a specified file (C.O.F.F. format).
 *
 * EXECUTION ENVIRONMENT:  AIX operating system
 *
 * NOTES: None.
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: file_header[], section_header[] local to this file
 *
 * RETURNS : see below
 *
 * INPUT:
 *      coffname - String containing name of the file to be loaded.
 * OUTPUT:
 *      returns zero if load was successful, otherwise will return
 *      non-zero.
 */

load_executable_coff(coffname)
char *coffname;       /* name of the file to be loaded */
{
  FILE *coff_file;    /* points to the file to be loaded */
  int filetype;       /* holds the filetype of the file to be loaded */
  int section_count;  /* holds the number of sections to be loaded */
  int opt_header_size; /* holds the size of the optional header */
  int section_header_location; /* holds the location in the file for */
                               /* current section header */
  long raw_data_ptr;
  long addr;
  long size;
  int i;
  int ret_code;

  pc_compile = FALSE; /* assume RIOS compile to begin with */

  *(bim_base_addr + DSP_CONTROL) = RESET_DSP; /* hold DSP in reset state */

  coff_file = fopen(coffname,"r");   /* Open input file, read only */
  if (coff_file==NULL){
    return(FOPEN_ERROR);
  } /* endif */

  if (fseek(coff_file,0,WHENCE_ABS)){
    fclose(coff_file);
    return(SEEK_ERROR);
  } /* endif */

  if ( fread(file_header,sizeof(*file_header),FILE_HEADER_SIZE,coff_file)
            != FILE_HEADER_SIZE ) {
    fclose(coff_file);
    return(FH_READ_ERROR);
  } /* endif */

  /* NOTE : since the file has reverse byte ordering (32 bits per word, */
  /*        least significant byte first), we flip all data before use. */
  filetype = flip_header_short(&file_header[MAGIC_NUMBER_OFFSET]);
  if (filetype != COFF_MAGIC_NUMBER) {
    filetype = flip_short(&file_header[MAGIC_NUMBER_OFFSET]);
    if (filetype != COFF_MAGIC_NUMBER) {
       fclose(coff_file);
       return(FILE_TYPE_ERROR);
    } else {
       pc_compile = TRUE;
    } /* endif */
  } /* endif */

  section_count = flip_header_short(&file_header[NUM_SEC_HEADERS_OFFSET]);

  opt_header_size = flip_header_short(&file_header[OPT_HEADER_SIZE_OFFSET]);

  section_header_location = FILE_HEADER_SIZE + opt_header_size;

  for (i = 0 ; i < section_count ; i++)
  {
    if (fseek(coff_file,section_header_location,WHENCE_ABS)){
      fclose(coff_file);
      return(SEEK_ERROR);
    } /* endif */

    if (fread(section_header,sizeof(*section_header),SECTION_HEADER_SIZE,
              coff_file) != SECTION_HEADER_SIZE){
      fclose(coff_file);
      return(SH_READ_ERROR);
    } /* endif */

    section_header_location += SECTION_HEADER_SIZE;

    raw_data_ptr = flip_header_long(&section_header[RAW_DATA_F_PTR_OFFSET]);

    /* load if the pointer is not NULL */
    if (raw_data_ptr) {
      addr = flip_header_long(&section_header[PHYSICAL_ADDRESS_OFFSET]);
      size = flip_header_long(&section_header[SECTION_SIZE_OFFSET]);
      ret_code = load_section(coff_file,raw_data_ptr,addr,size);
      if (ret_code){
        return(ret_code);
      } /* endif */
    } /* endif */
  }


  fclose(coff_file);

  return(0);    /* if we got here then good return */

} /* end load_executable_coff() */


/*
 * NAME: load_section
 *
 * FUNCTION: Loads one coff section into DSP memory and will verify successful
 *           transfer.
 *
 * EXECUTION ENVIRONMENT:  AIX operating system
 *
 * NOTES: None.
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: None.
 *
 * RETURNS : see below
 *
 * INPUT:
 *      file_ptr - pointer to the file that contains the section to be loaded
 *      seekloc - location in the file, where section begins
 *      addr - physical address in DSP memory where section is to be loaded
 *      size - size of section to be loaded
 * OUTPUT:
 *      returns zero if load was successful, otherwise will return
 *      non-zero.
 */
load_section(file_ptr,seekloc,addr,size)
FILE    *file_ptr;  /* file that contains section to be loaded */
long    seekloc;    /* location in file where section begins */
long    addr;      /* address in DSP memory where section is to be loaded */
long    size;      /* size of section to be loaded */
{
  long ind_addr;
  long data;
  char data_buffer[4];
  int i;
  unsigned long prev_crc;


  crcval = 0;
  if (fseek(file_ptr,seekloc,WHENCE_ABS)){
    return(SEEK_ERROR);
  } /* endif */

  ind_addr = addr;

  if (check_dsp_address(ind_addr)){
    return(DSP_ADDRESS_ERROR);
  } /* endif */

  *(bim_base_addr + IND_CONTROL) = (IND_WRITE  | /* Request write operation  */
                                    IND_DSPMEM | /* Destination = DSP memory */
                                    IND_AUTOINC);/* Auto increment           */

  *(bim_base_addr + IND_ADDRESS) = ind_addr;

  for (i = 0 ; i < size ; i++) {
    if (fread(data_buffer,sizeof(*data_buffer),4,file_ptr) != 4){
      return(READ_ERROR);
    } /* endif */
    data = flip_long(data_buffer);
    *(bim_base_addr + IND_DATA) = data;      /* write to the adapter memory */
    crc32(&data);                      /* Accumulate 32 bit crc            */
  } /* endfor */

  prev_crc = crcval;
  crcval = 0;

  *(bim_base_addr + IND_CONTROL) = (IND_READ   | /* Request read  operation  */
                                    IND_DSPMEM | /* Destination = DSP memory */
                                    IND_AUTOINC);/* Auto increment           */

  *(bim_base_addr + IND_ADDRESS) = ind_addr;

  for (i = 0 ; i < size ; i++) {
    data = *(bim_base_addr + IND_DATA);
    crc32(&data);                /* Accumulate 32 bit crc,  */
                                 /* this should clear previous accumulated  */
                                 /* crcval     */
  } /* endfor */

  if (crcval != prev_crc){
    return(CRC_ERROR);
  } else {
    return(0);
  } /* endif */

} /* end load_section() */



/*
 * NAME: flip_short
 *
 * FUNCTION: flips a 2 byte sequence and forms a short word (16 bit) value.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES: None.
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: None.
 *
 * INPUT: Pointer to character buffer containing the two input bytes.
 *
 * OUTPUT: returns the short value corresponding to the two input bytes.
 */

flip_short(bp)
unsigned char *bp;
{
  return ( *bp | (*(bp+1) << 8) );
} /* end flip_short() */


/*
 * NAME: flip_long
 *
 * FUNCTION: flips a 4 byte sequence and forms a long word (32 bit) value.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:  None.
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: None.
 *
 * INPUT: Pointer to character buffer containing the four input bytes.
 *
 * OUTPUT: returns the long value corresponding to the four input bytes.
 */

long flip_long(unsigned char *bp)
{
  return ( *bp | (*(bp+1) << 8 ) | (*(bp+2) << 16) | (*(bp+3) << 24) );
} /* end flip_long() */

/*
 * NAME: flip_header_short
 *
 * FUNCTION: flips a 2 byte sequence and forms a short word (16 bit) value
 *           if pc_compile==TRUE, otherwise just forms a short word.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES: None.
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: None.
 *
 * INPUT: Pointer to character buffer containing the two input bytes.
 *
 * OUTPUT: returns the short value corresponding to the two input bytes.
 */

flip_header_short(unsigned char *bp)
{
  if (pc_compile) {
     return ( *bp | (*(bp+1) << 8) );
  } else {
     return ( (*bp << 8) | *(bp+1)  );
  } /* endif */
} /* end flip_header_short() */


/*
 * NAME: flip_header_long
 *
 * FUNCTION: flips a 4 byte sequence and forms a long word (32 bit) value
 *           if pc_compile==TRUE, otherwise just forms a long word.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:  None.
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: None.
 *
 * INPUT: Pointer to character buffer containing the four input bytes.
 *
 * OUTPUT: returns the long value corresponding to the four input bytes.
 */

long flip_header_long(unsigned char *bp)
{
  if (pc_compile) {
     return ( *bp | (*(bp+1) << 8 ) | (*(bp+2) << 16) | (*(bp+3) << 24) );
  } else {
     return ( (*bp << 24) | (*(bp+1) << 16 ) | (*(bp+2) << 8) | *(bp+3) );
  } /* endif */
} /* end flip_header_long() */

/*
 * NAME: check_dsp_address
 *
 * FUNCTION: Checks the address to see if it is within the adapter
 *           memory address range.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:  None.
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: None.
 *
 * RETURNS : zero if legal address, non-zero if illegal address
 *
 * INPUT: address - physical dsp address to be verified
 *
 * OUTPUT: None.
 */

int check_dsp_address(long address)
{

  if ((address >= MEMORY_SEG_MIN) && (address <= MEMORY_SEG_MAX)){
      return(0);
  } else {
      return(-1);
  } /* endif */

} /* end check_dsp_address() */


/*
 * COMPONENT_NAME: crc32
 *
 * ORIGIN: IBM
 *
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1984, 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */

/*
 * NAME: crc32
 *
 * FUNCTION:
 *     crc32 generates a 32 bit "classic" CRC using the following
 *     CRC polynomial:
 *
 *   g(x) = x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11 + x^10 + x^8
 *               + x^7  + x^5  + x^4  + x^2  + x^1  + x^0
 *
 *   e.g. g(x) = 1 04c1 1db7
 *
 * NOTES:
 *
 *    This function has been optimized for speed and size.
 *    By studying the CRC algorithm, we note that
 *    the data byte and the high byte of the accumulator are combined
 *    together to a value between 0 and 255 which can be precalculated in
 *    a table of the 256 possible values.  This table can further be
 *    collapsed by computing a table of values for the high nybble and a
 *    table of values for the low nybble, which are then XOR'ed into the
 *    accumulator.
 *
 *    This function accumulates the crc value in the global variable
 *    crcval.  If crcval is originally initialize to zero before calling
 *    this function, it will contain the correct crc value after this
 *    function has been called for each word of data.  If crcval is
 *    originally initialized to the computed crc value before calling
 *    this function, it will contain zero after this function has been
 *    called for each word of data if there are NO ERRORS in the data.
 *
 */


static unsigned long crctl[16] = {
  0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9,
  0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
  0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61,
  0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD};

static unsigned long crcth[16] = {
  0x00000000, 0x4C11DB70, 0x9823B6E0, 0xD4326D90,
  0x34867077, 0x7897AB07, 0xACA5C697, 0xE0B41DE7,
  0x690CE0EE, 0x251D3B9E, 0xF12F560E, 0xBD3E8D7E,
  0x5D8A9099, 0x119B4BE9, 0xC5A92679, 0x89B8FD09};

crc32(pbuff)
char    *pbuff;
{
  unsigned long   i;
  unsigned long   temp;

  for (i=0; i<4; i++)
  {
    temp = (crcval >> 24) ^ *pbuff++;
    crcval <<= 8;
    crcval ^= crcth[ temp/16 ];
    crcval ^= crctl[ temp%16 ];
  }
} /* end crc32() */


/*
 * NAME: partial_load_executable_coff
 *
 * FUNCTION: Loads an executable COFF file into the DSP program memory,
 *           starting at a given address, in the following format :
 *                           Section Address,
 *                           Size,
 *                           Raw Data(0),
 *                           ... ,
 *                           Raw Data(Size-1).
 *                           Section Address,
 *                           Size,
 *                           Raw Data(0),
 *                           ... ,
 *                           Raw Data(Size-1).
 *                           END_LOAD.            (This is a special value)
 *
 * EXECUTION ENVIRONMENT:  AIX operating system
 *
 * NOTES: None.
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: file_header[], section_header[] local to this file
 *
 * RETURNS : see below
 *
 * INPUT:
 *      coffname - String containing name of the file to be loaded.
 *      start_address - starting address of area on the adapter to store to
 * OUTPUT:
 *      returns zero if load was successful, otherwise will return
 *      non-zero.
 */

int partial_load_executable_coff(char *coffname,int start_address)
{
  FILE *coff_file;    /* points to the file to be loaded */
  int filetype;       /* holds the filetype of the file to be loaded */
  int section_count;  /* holds the number of sections to be loaded */
  int opt_header_size; /* holds the size of the optional header */
  int section_header_location; /* holds the location in the file for */
                               /* current section header */
  long buff_address;
  long raw_data_ptr;
  long addr;
  long size;
  int i;
  int ret_code;

  pc_compile = FALSE; /* assume RIOS compile to begin with */

  coff_file = fopen(coffname,"r");   /* Open input file, read only */
  if (coff_file==NULL){
    return(FOPEN_ERROR);
  } /* endif */

  if (fseek(coff_file,0,WHENCE_ABS)){
    fclose(coff_file);
    return(SEEK_ERROR);
  } /* endif */

  if ( fread(file_header,sizeof(*file_header),FILE_HEADER_SIZE,coff_file)
            != FILE_HEADER_SIZE ) {
    fclose(coff_file);
    return(FH_READ_ERROR);
  } /* endif */

  /* NOTE : since the file has reverse byte ordering (32 bits per word, */
  /*        least significant byte first), we flip all data before use. */
  filetype = flip_header_short(&file_header[MAGIC_NUMBER_OFFSET]);
  if (filetype != COFF_MAGIC_NUMBER) {
    filetype = flip_short(&file_header[MAGIC_NUMBER_OFFSET]);
    if (filetype != COFF_MAGIC_NUMBER) {
       fclose(coff_file);
       return(FILE_TYPE_ERROR);
    } else {
       pc_compile = TRUE;
    } /* endif */
  } /* endif */

  section_count = flip_header_short(&file_header[NUM_SEC_HEADERS_OFFSET]);

  opt_header_size = flip_header_short(&file_header[OPT_HEADER_SIZE_OFFSET]);

  section_header_location = FILE_HEADER_SIZE + opt_header_size;

  buff_address = start_address;


  for (i = 0 ; i < section_count ; i++)
  {
    if (fseek(coff_file,section_header_location,WHENCE_ABS)){
      fclose(coff_file);
      return(SEEK_ERROR);
    } /* endif */

    if (fread(section_header,sizeof(*section_header),SECTION_HEADER_SIZE,
              coff_file) != SECTION_HEADER_SIZE){
      fclose(coff_file);
      return(SH_READ_ERROR);
    } /* endif */

    section_header_location += SECTION_HEADER_SIZE;

    raw_data_ptr = flip_header_long(&section_header[RAW_DATA_F_PTR_OFFSET]);

    /* load if the pointer is not NULL */
    if (raw_data_ptr) {
      addr = flip_header_long(&section_header[PHYSICAL_ADDRESS_OFFSET]);
      size = flip_header_long(&section_header[SECTION_SIZE_OFFSET]);
      ret_code = partial_load_section(coff_file,raw_data_ptr,addr,size,
                                      buff_address);
      buff_address+=size;
      buff_address+=2;

      if (ret_code){
        fclose(coff_file);
        return(ret_code);
      } /* endif */
    } /* endif */
  } /* endfor */

  *(bim_base_addr + IND_CONTROL) = (IND_WRITE  | /* Request write operation  */
                                    IND_DSPMEM | /* Destination = DSP memory */
                                    IND_AUTOINC);/* Auto increment           */

  *(bim_base_addr + IND_ADDRESS) = buff_address;
  *(bim_base_addr + IND_DATA) = END_LOAD;   /* write to the adapter memory */

  fclose(coff_file);

  return(0);    /* if we got here then good return */

} /* end partial_load_executable_coff() */


/*
 * NAME: partial_load_section
 *
 * FUNCTION: Loads one coff section into DSP memory and will verify successful
 *           transfer.
 *
 * EXECUTION ENVIRONMENT:  AIX operating system
 *
 * NOTES: None.
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: None.
 *
 * RETURNS : see below
 *
 * INPUT:
 *      file_ptr - pointer to the file that contains the section to be loaded
 *      seekloc - location in the file, where section begins
 *      addr - physical address in DSP memory where section is to be loaded
 *      size - size of section to be loaded
 * OUTPUT:
 *      returns zero if load was successful, otherwise will return
 *      non-zero.
 */
partial_load_section(file_ptr,seekloc,addr,size,buffer)
FILE    *file_ptr;  /* file that contains section to be loaded */
long    seekloc;    /* location in file where section begins */
long    addr;      /* address in final DSP memory */
long    size;      /* size of section to be loaded */
long    buffer;    /* address where section is to be temporarily stored */
{
  long ind_addr;
  long data;
  char data_buffer[4];
  int i;
  unsigned long prev_crc;


  crcval = 0;
  if (fseek(file_ptr,seekloc,WHENCE_ABS)){
    return(SEEK_ERROR);
  } /* endif */


  if (check_dsp_address(buffer)){
    return(DSP_ADDRESS_ERROR);
  } /* endif */

  *(bim_base_addr + IND_CONTROL) = (IND_WRITE  | /* Request write operation  */
                                    IND_DSPMEM | /* Destination = DSP memory */
                                    IND_AUTOINC);/* Auto increment           */

  *(bim_base_addr + IND_ADDRESS) = buffer;
  *(bim_base_addr + IND_DATA) = addr;   /* write to the adapter memory */
  data = addr;
  crc32(&data);                      /* Accumulate 32 bit crc            */
  *(bim_base_addr + IND_DATA) = size;     /* write to the adapter memory */
  data = size;
  crc32(&data);                      /* Accumulate 32 bit crc            */

  for (i = 0 ; i < size ; i++) {
    if (fread(data_buffer,sizeof(*data_buffer),4,file_ptr) != 4){
      return(READ_ERROR);
    } /* endif */
    data = flip_long(data_buffer);
    *(bim_base_addr + IND_DATA) = data;      /* write to the adapter memory */
    crc32(&data);          /* Accumulate 32 bit crc */
  } /* endfor */

  prev_crc = crcval;
  crcval = 0;

  *(bim_base_addr + IND_CONTROL) = (IND_READ   | /* Request read  operation  */
                                    IND_DSPMEM | /* Destination = DSP memory */
                                    IND_AUTOINC);/* Auto increment           */

  *(bim_base_addr + IND_ADDRESS) = buffer;
  data = *(bim_base_addr + IND_DATA);
  crc32(&data);                /* Accumulate 32 bit crc,  */
  data = *(bim_base_addr + IND_DATA);
  crc32(&data);                /* Accumulate 32 bit crc,  */

  for (i = 0 ; i < size ; i++) {
    data = *(bim_base_addr + IND_DATA);
    crc32(&data);                /* Accumulate 32 bit crc,  */
  } /* endfor */

  if (crcval != prev_crc){
    return(CRC_ERROR);
  } else {
    return(0);
  } /* endif */

} /* end partial_load_section() */


