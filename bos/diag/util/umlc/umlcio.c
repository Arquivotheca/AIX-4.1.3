static char sccsid[] = "@(#)42  1.18.2.10  src/bos/diag/util/umlc/umlcio.c, dsaumlc, bos41J, 9509A_all 2/27/95 20:09:21";
/*
 * COMPONENT_NAME: (DUTIL) DIAGNOSTIC UTILITY - umlcio.c
 *
 * FUNCTIONS:
 *              write_disk(ptr)
 *              write_hardfile()
 *              write_hrec(dat,fd1,fd2)
 *              write_hist(rec)
 *              add_seq_to_hist()
 *              get_date(buf)
 *              exist_test()
 *              write_flops(which)
 *              strnchr(sptr,ch,len)
 *              gen_logs(max)
 *              get_dsize()
 *              create_update()
 *              process_checkstop(serial)
 *              check_for_space()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <nl_types.h>
#include <sys/cfgdb.h>
#include <sys/errno.h>
#include <time.h>
#include "umlc_msg.h"
#include "mlc_defs.h"
#include "diag/class_def.h"
#include "diag/diago.h"
#include "diag/diag.h"
#include "diag/tmdefs.h"
#include <sys/ioctl.h>
#include <sys/fd.h>
#include <sys/statfs.h>

#define DISKDRIVE       "/dev/rfd0"
#define LOWDISK          700000
#define HIGHDISK        1400000
#define SWHIST_COMM     "/usr/bin/lslpp"
#define SWHIST_PARM     "-ch"
#define SWPID_COMM      "/usr/bin/lslpp"
#define SWPID_PARM      "-ci"

/* EXTERNAL VARIABLES */
extern  nl_catd         fdes;
extern  VPD_REC         vprec;
extern  MACH_REC        machine;
extern  struct PN_PTR   *pn_ptr;
extern  struct FB_PTR   *fb_ptr;
extern  struct FC_PTR   *fc_ptr;
extern  char            diagdir[];
extern  int             output;
extern  int             install_yes;
extern  int             diskette;
extern  int             fdd_gone;
extern  int             fdd_not_supported;
extern  int             no_menus;
extern  int             cstop_found;
extern  int             output_file_open;
extern  char            outputfile[];

/* FUNCTION PROTOTYPES */
int write_disk(char *);
void write_hardfile(void);
void write_hrec(struct PN_PTR *, FILE *, FILE *);
void write_hist(HIST_REC *);
void add_seq_to_hist(void);
void get_date(char *);
int exist_test(void);
int write_flops(int);
char *strnchr(char *, char, int);
void gen_logs(int);
int get_dsize(void);
void create_update(void);
int process_checkstop(char *);
int check_for_space(void);


/*  */
/*
 * NAME: write_disk
 *
 * FUNCTION: this function writes the passed filename PT_SYS or PT_SDDB
 *              along with PT_HIST to a diskette
 *
 * EXECUTION ENVIRONMENT:
 *
 *      get the current directory and save it
 *      change to the /etc/lpp/diagnostics/data directory
 *      if the filename passed in ptr is SYS write the PT_SYS and PT_HIST
 *      else if the filename passed in ptr is SDDB write the PT_SDDB, PT_HIST
 *      error log and diagnostic reports
 *      switch back to the saved directory
 *
 * RETURNS: the return code from odm_run_method
 *
 */

int write_disk(char *ptr)
{
int rval;
char    *obuf, *ebuf;
char    current_dir[512];
char    filename[512];
char    serial[FIELD_SIZE];
char    lsparms[256];
char    out[256];
char    sys_str[512];
int     rc = 0;
int     sddb_file = 0;

        getcwd(current_dir, 512);
        chdir(diagdir);

        strcpy(serial, &vprec.mr.serial[4]);
        strtok(serial, "\n");

        if(strncmp(outputfile,"\0",1)!=0) {
                sddb_file = 1;
                strcpy(out,outputfile);
        }
        else
                strcpy(out,"/dev/rfd0");

        if(cstop_found == TRUE)
                strcpy(lsparms, "PT_SDDB PT_HIST *.log *.rpt *.SWHIST *.SWPID *.ckstp ");
        else
                strcpy(lsparms, "PT_SDDB PT_HIST *.log *.rpt *.SWHIST *.SWPID ");

        strcat(lsparms, "| cpio -oC20 > ");
        strcat(lsparms, out);
        rval = odm_run_method("/bin/ls", lsparms, &obuf, &ebuf);
        /* If writing output to a file, change ownership of the file */
        /* to the "real userid" (not the effective userid).           */
        if((sddb_file) && (rval == 0)) {
                sprintf(sys_str,"%d %s", getuid(), outputfile);
                rc = odm_run_method("/bin/chown", sys_str, &obuf, &ebuf);
                if(rc == 0) {
                        sprintf(sys_str,"%d %s", getgid(), outputfile);
                        rc = odm_run_method("/bin/chgrp", sys_str, &obuf, &ebuf);
                 }
                if(rc == 0) {
                        sprintf(sys_str,"644 %s", outputfile);
                        rc = odm_run_method("/bin/chmod", sys_str, &obuf, &ebuf);
                 }
        }
        if ((diskette == DIAG_FALSE) && ((rval == 0) || (sddb_file))) {
                sprintf(filename, "%s/%s.log", diagdir, serial);
                unlink(filename);
                sprintf(filename, "%s/%s.rpt", diagdir, serial);
                unlink(filename);
                sprintf(filename, "%s/%s.SWHIST", diagdir, serial);
                unlink(filename);
                sprintf(filename, "%s/%s.SWPID", diagdir, serial);
                unlink(filename);
                if(cstop_found == TRUE) {
                        sprintf(filename, "%s/%s.ckstp", diagdir, serial);
                        unlink(filename);
                }
        }

        if(rc!=0)    /* Error changing permissions */
                rval = 3;

        chdir(current_dir);
        free(obuf);
        free(ebuf);
        return rval;
}

/*  */
/*
 * NAME: write_hardfile
 *
 * FUNCTION: this function writes the machine record and sets up the data
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */

void write_hardfile(void)
{
FILE *psysf,*psddbf;
LBUFF *plbuf;
int cnt;
char filename[256];
pd_struct pd_str;
int   mode = 0;
struct   stat  buf;                          /* Holds file status info */


        /* If /etc/lpp/diagnsotics/data doesn't exist, create it */
        if(stat(diagdir, &buf) != 0) {
                if(errno == ENOENT)
                        if(mkdir(diagdir, mode)) {
                                disp_popup_message(NO_DIR_ERR);
                                genexit(8);
                        }
        }

        output_file_open=TRUE;
        sprintf(filename, "%s/PT_SYS", diagdir);
        psysf = (FILE *)fopen(filename, "w");
        sprintf(filename, "%s/PT_SDDB", diagdir);
        psddbf = (FILE *)fopen(filename, "w");

        set_seq_no();
        strcpy(machine.vers,"*VC 3.0\n");
        plbuf = (LBUFF *) &machine;
        for(cnt = 0;cnt < (sizeof(MACH_REC)/FIELD_SIZE); cnt++) {
                if((*(char *) plbuf) != (char) '\0') {
                        fputs((char *) plbuf,psysf);
                        fputs((char *) plbuf,psddbf);
                }
                ++plbuf;
        }

        add_seq_to_hist();

        pd_str.dir = 1;
        pd_str.func1 = (void (*)(void))write_hrec;
        pd_str.func2 = (void (*)(void))write_hrec;
        pd_str.func3 = (void (*)(void))write_hrec;
        pd_str.str1 = (char *)psysf;
        pd_str.str2 = (char *)psddbf;
        pd_str.str3 = (char *)NULL;
        parse_data(&pd_str);

        fclose(psysf);
        fclose(psddbf);
        output_file_open=FALSE;
}

/*  */
/*
 * NAME: write_hrec
 *
 * FUNCTION: write the data to the hardfile
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */

void write_hrec(struct PN_PTR *dat, FILE *fd1, FILE *fd2)
{
int cnt;
LBUFF *plbuf;
        plbuf= (LBUFF *) dat->data->pn;
        for(cnt=0; cnt < dat->data->rec_size; cnt++,plbuf++) {
                if(plbuf->dat[0] == '$' || plbuf->dat[0] == '\0') continue;
                fputs((char *) plbuf,fd1);
                fputs((char *) plbuf,fd2);
        }
}

/*  */
/*
 * NAME: write_hist
 *
 * FUNCTION:  this function appends a history record to the end of the
 *              PT_HIST file.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */

void write_hist(HIST_REC *rec)
{
FILE *phistf;
LBUFF *plbuf;
int cnt;
char filename[256];

        sprintf(filename, "%s/PT_HIST", diagdir);
        phistf = (FILE *) fopen(filename, "a");

        plbuf = (LBUFF *) rec;
        for (cnt=0; cnt < H_LINES; cnt++, plbuf++)
          {
          if ((plbuf->dat[0] == '$') || (plbuf->dat[0] == '\0')) continue;
          fputs((char *)plbuf, phistf);
          }

        fclose(phistf);
}

/*  */
/*
 * NAME: add_seq_to_hist
 *
 * FUNCTION:  this function appends the sequence number to the end of the
 *              PT_HIST file.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */

void add_seq_to_hist(void)
{
FILE *phistf;
char filename[256];
char date[16];
char dc_key[50];

        sprintf(filename, "%s/PT_HIST", diagdir);
        phistf = (FILE *) fopen(filename, "a");

        if (install_yes)
          {
          get_date(date);
          sprintf(dc_key, "*DC ID %s", date);
          fputs(dc_key, phistf);
          }

        if (diskette == DIAG_TRUE)
          fputs("*SM DISKETTE\n", phistf);
        else
          fputs("*SM HARDFILE\n", phistf);

        fputs((char *)machine.seq_no, phistf);

        fclose(phistf);

}

/*  */
/*
 * NAME: get_date
 *
 * FUNCTION: this function gets the current date and time and stores the
 *              formatted string yyyymmdd into the passed pointer.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */

void get_date(char *buf)
{
long ticks;
struct tm *tim;

        ticks = time((long *) 0);
        tim = (struct tm *) localtime(&ticks);
        sprintf(buf,"%d%02d%02d%02d%02d%02d\n",
                tim->tm_year+1900,tim->tm_mon+1,tim->tm_mday,
                tim->tm_hour,tim->tm_min,tim->tm_sec);
}

/*  */
/*
 * NAME: exist_test
 *
 * FUNCTION:  this function tests to see if the file PT_SYS exists
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: TRUE if file exists
 *          FLASE if file does not exist
 */

int exist_test(void)
{
FILE *fil;
int rval;
char filename[256];

        sprintf(filename, "%s/PT_SYS", diagdir);
        fil = (FILE *) fopen(filename, "r");
        if(fil == (FILE *) 0)
                rval = FALSE;
        else  {
                fclose(fil);
                rval = TRUE;
        }
        return(rval);
}

/*  */
/*
 * NAME:  write_flops
 *
 * FUNCTION: this function is used to setup calls to write_disk
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *      the variable which is used to indicate the files to be written to
 *      floppy; SYS, SDDB or BOTH.
 *      if which == SYS or BOTH {
 *              try writting the diskette until successful or user aborted
 *      }
 *      if which == SDDB or BOTH {
 *              prompt for the update diskette
 *              get the size of the diskette
 *              get error log summary
 *              try writting the diskette until successful or user aborted
 *      }
 *
 * RETURNS: NONE
 */

int write_flops(int which)
{
char *obuf,*ebuf;
char *tbuf;
int match = TRUE;
int size;
int rc;

        if (which == WRITE_SDDB) {
                /* prompt for SDDB diskette */
                if (disp_load_sys("SDDB"))
                        return 1;

                /* write the new data back to the SDDB diskette */
                output = TRUE;
                if (diskette == DIAG_FALSE)  {
                        while ((size = get_dsize()) == -1)
                                if (disp_retry_format(1)) {
                                        output = FALSE;
                                        return 1;
                                }
                        gen_logs(size);
                }
                while (write_disk("SDDB") != 0)
                        if (disp_retry_format(1)) {
                                output = FALSE;
                                return 1;
                        }
                return 0;       /* Everything worked */
        }
        if (which == WRITE_OUTFILE) {
                gen_logs(HIGHDISK);    /* Force "High Capacity" Disk */
                rc = write_disk("SDDB");
                return(rc);
        }
}

/*  */
/*
 * NAME: strnchr
 *
 * FUNCTION: this function is the same as the strchr library function except
 *              a length variable is used to indicate maximum length of scan.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: pointer to the location of ch if found.
 *      a null pointer if ch is not found within len bytes
 */

char *strnchr(char *sptr, char ch, int len)
{
int cnt;

        for(cnt=0; cnt < len; cnt++,sptr++) {
                if(*sptr == ch) return(sptr);
        }
        return(((char *) '\0'));
}

/*  */
/*
 * NAME: gen_logs
 *
 * FUNCTION:  create diagnostic report summary and error log
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:  NONE
 */

void gen_logs(int max_size)
{
int size=0;
FILE *infile,*outfile;
struct stat sbuf;
char oname[256];
char ibuf[512];
char command[256];
char catparms[256];
char serial[FIELD_SIZE];
char *obuf,*ebuf;
char filename[256];

        strcpy(serial, &vprec.mr.serial[4]);
        strtok(serial, "\n");

        sprintf(oname, "%s/%s.rpt", diagdir, serial);
        sprintf(catparms, "%s/*.dat > %s", diagdir, oname);
        odm_run_method("/bin/cat", catparms, &obuf, &ebuf);
        sprintf(catparms, " -a > %s/tlog", diagdir);
        odm_run_method("/usr/bin/errpt", catparms, &obuf, &ebuf);

        sprintf(catparms, "%s > %s/%s.SWHIST", SWHIST_PARM, diagdir, serial);
        odm_run_method(SWHIST_COMM, catparms, &obuf, &ebuf);
        sprintf(catparms, "%s > %s/%s.SWPID", SWPID_PARM, diagdir, serial);
        odm_run_method(SWPID_COMM, catparms, &obuf, &ebuf);

        process_checkstop(&serial[0]);

        sprintf(filename, "%s/PT_SDDB", diagdir);
        stat(filename, &sbuf);
        size += sbuf.st_size;
        sprintf(filename, "%s/PT_HIST", diagdir);
        stat(filename, &sbuf);
        size += sbuf.st_size;
        sprintf(filename, "%s/%s.SWHIST", diagdir, serial);
        stat(filename, &sbuf);
        size += sbuf.st_size;
        sprintf(filename, "%s/%s.SWPID", diagdir, serial);
        stat(filename, &sbuf);
        size += sbuf.st_size;
        sprintf(filename, "%s/%s.ckstp", diagdir, serial);
        stat(filename, &sbuf);
        size += sbuf.st_size;
        stat(oname, &sbuf);
        size += sbuf.st_size;

        sprintf(oname, "%s/%s.log", diagdir, serial);
        sprintf(filename, "%s/tlog", diagdir);
        stat(filename, &sbuf);

        if ((sbuf.st_size + size)  > max_size)  {
                infile = (FILE *) fopen(filename, "r");
                outfile = (FILE *) fopen(oname, "w");
                while (size < max_size) {
                        fgets(ibuf, 512, infile);
                        fputs(ibuf, outfile);
                        size += strlen(ibuf);
                }
                fclose(infile);
                fclose(outfile);
                unlink(filename);
        }
        else
                rename(filename, oname);
        free(obuf);
        free(ebuf);
}

/*  */
/*
 * NAME: get_dsize
 *
 * FUNCTION:  determines diskette capacity
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: size of disk in bytes (not actual size but write size)
 */

int get_dsize(void)
{
        int Dfd;
        struct fdparms parms;

        if ((Dfd = open(DISKDRIVE, O_RDONLY, NULL)) < 0)
                return(-1);
        if (ioctl(Dfd, FDIOCGETPARMS, &parms) < 0)
                return(-1);
        close(Dfd);
        if(parms.diskette_type == 0xa2)
                return(HIGHDISK);
        else return(LOWDISK);

}

/*  */
/*
 * NAME: create_update
 *
 * FUNCTION: generate a PT_SDDB "diskette" on the hardfile
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */

void create_update(void)
{
char    current_dir[512];
int     rc;

        if(!exist_test())
                genexit(2);

        getcwd(current_dir, 512);
        chdir(diagdir);

        if (load_hard("PT_SYS") == -1)
                genexit(4);

        /* If parts were added or deleted from the system configuration, */
        /* log them in the history and database with an action code of */
        /* AU or RM and write the hardfile and the OUTPUTFILE */
        if ( ( test_add_del() ) || ( test_4repairs() ) ) {
                merge_data();
                write_hardfile();
        }

        rc=write_flops(WRITE_OUTFILE);
        chdir(current_dir);
        if(rc != 0)
                genexit(3);
        else
                genexit(0);

}

/*  */
/*
 * NAME: process_checkstop
 *
 * FUNCTION: create a file in the diagdir directory containing all of the
 *           machine's checkstop information.  This file will be in "backup"
 *           format.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: return code from odm_run_method
 */

int process_checkstop(char *serial)
{
int   rval=0;
char  *obuf, *ebuf;
char  ckstp_dir[13];
char  current_dir[512];
char  command[512];

        getcwd(current_dir, 512);
        strcpy(ckstp_dir,"/usr/lib/ras");
        chdir(ckstp_dir);
        /* Check for a checkstop file with read permissions */
        if (access("checkstop.A",4) ==0) {
                sprintf(command, " checkstop.* | backup -if- | dd of=%s/%s.ckstp",diagdir,serial);
                rval = odm_run_method("/bin/ls", command, &obuf, &ebuf);
                cstop_found=TRUE;
        } /* endif */
        chdir(current_dir);
        free(obuf);
        free(ebuf);
        return(rval);
}

/*  */
/*
 * NAME: check_for_space
 *
 * FUNCTION: Checks that the root filesystem has enough available space to
 *           write the Product Topology files.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 0 space is available
 *          1 no space is available or error
 */

int check_for_space(void)
{
        char Path[25];
        struct statfs StatusBuffer;

        strcpy(Path,"/");
        if (statfs(&Path, &StatusBuffer) != 0)
                return(1);

        if (StatusBuffer.f_bfree < MIN_FREE)
                return(1);

        return(0);
}
