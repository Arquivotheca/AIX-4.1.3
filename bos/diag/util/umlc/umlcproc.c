static char sccsid[] = "@(#)43  1.23.1.5  src/bos/diag/util/umlc/umlcproc.c, dsaumlc, bos411, 9428A410j 3/14/94 12:30:24";
/*
 * COMPONENT_NAME: (DUTIL) DIAGNOSTIC UTILITY - umlcproc.c
 *
 * FUNCTIONS:
 *              bld_generic(strg,buf)
 *              process_rebuild()
 *              mfg_build()
 *              process_repair()
 *              gen_sys_rec()
 *              au_newparts()
 *              set_install_date()
 *              set_seq_no()
 *              test_4repairs()
 *              log_error(id,name)
 *              chk_extra_vpd(p1,p2)
 *              merge_data()
 *              merge_extras(pn,vp)
 *              remove_parts(pp)
 *              build_mes_hist(pptr)
 *              build_del_hist(pp)
 *              bld_repair_hist(dat,old,new,rflg)
 *              rm_pts(pp)
 *              del_fcs(fp)
 *              del_fbs(bp)
 *              remove_mates()
 *              remove_vmates(pp)
 *              test_add_del()
 *              search_vrec_scsi()
 *              calloc_mem(number, size)
 *              malloc_mem(size)
 *
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <signal.h>
#include <nl_types.h>
#include <sys/cfgdb.h>
#include <memory.h>
#include <sys/errids.h>
#include "umlc_msg.h"
#include "mlc_defs.h"
#include "diag/class_def.h"
#include "diag/diago.h"
#include "diag/diag.h"
#include "diag/tmdefs.h"
#include <macros.h>

static char             date_buf[16];
static int              fcsi_mes_added;

/* EXTERNAL VARIABLES   */
extern char             *keywrds[];
extern char             *lvl_kwds[];
extern char             diagdir[];
extern MACH_REC         machine;
extern VPD_REC          vprec;
extern struct PN_PTR    *pn_ptr;
extern struct FB_PTR    *fb_ptr;
extern struct FC_PTR    *fc_ptr;
extern int              vpcnt;
extern int              pncnt;
extern int              fbcnt;
extern int              fccnt;
extern int              diskette;
extern int              fdd_gone;
extern int              install_yes;
extern nl_catd          fdes;
extern ASL_SCR_TYPE     dm_menutype;
extern char             mfg_sno[];
extern char             mfg_typ[];
extern struct PN_PTR    **loctab;
extern char             outputfile[];
extern int              no_menus;


/* EXTERNAL FUNCTIONS */
extern void build_loc_table(struct PN_PTR *);
extern struct FC_PTR *last_fc();
extern int new_vitem(struct VP_PTR *);
extern int chk_4_scsi_dev(struct VP_PTR *);

/*  ANSI C FUNCTION PROTOTYPING FOLLOWS */
void bld_generic(char *, char *);
int process_rebuild(void);
void mfg_build(void);
void process_repair(void);
void gen_sys_rec(void);
void au_newparts(void);
void set_install_date(void);
void set_seq_no(void);
int test_4repairs(void);
void log_error(int, char *);
LBUFF *chk_extra_vpd(LBUFF *, LBUFF *);
void merge_data(void);
void merge_extras(struct PN_PTR *, V_REC *);
void remove_parts(struct FC_PTR *);
void build_mes_hist(struct PN_PTR *);
void build_del_hist(struct PN_PTR *);
void bld_repair_hist(struct PN_PTR *, LBUFF *, LBUFF *, int);
void rm_pts(struct PN_PTR *);
void del_fcs(struct FC_PTR *);
void del_fbs(struct FB_PTR *);
void remove_mates(void);
void remove_vmates(struct PN_PTR *);
int test_add_del(void);
void search_vrec_scsi(void);
void* calloc_mem(int, int);
void* malloc_mem(int);

/*  */
/*
 * NAME: bld_generic
 *
 * FUNCTION: strip numerical value from a resource name ie. mem1 becomes mem
 *
 * RETURNS: NONE
 */

void bld_generic(char *strg, char *buf)
{
        while((!(isdigit(*strg))) && (*strg != '\0') && (*strg != '\n')) {
                *buf++ = *strg++;
        }
        strcpy(buf,"\n");
}

/*  */
/*
 * NAME: process_rebuild
 *
 * FUNCTION: create a PT_SYS diskette
 *
 * NOTES:
 *      if a hardfile copy of the PT_SYS file is present copy it to diskette
 *      if no files exist generate a system disk from vpd data
 *
 * RETURNS: -1 if an error occurred
 *           0 if the user wants to cancel or exit
 *           1 if everything worked successfully
 */

int process_rebuild(void)
{
int cnt;
int rval;

        if(!read_vpd()) return(-1);
        if(exist_test()) {
                if((load_hard("PT_SYS")) == -1) {
                        disp_popup_message(INTERNAL_ERR);
                        genexit(4);
                }
        }
        else
                install_yes=TRUE;
        rval=disp_sys_rec(&machine);
        if(rval == 0) {
                if(install_yes==TRUE) {
                        if (check_for_space() != 0) {
                                disp_popup_message(OUT_OF_SPACE);
                                genexit(7);
                        } /* endif */
                        gen_sys_rec();
                }
                write_hardfile();
                if((load_hard("PT_SYS")) == -1) {
                        disp_popup_message(INTERNAL_ERR);
                        genexit(4);
                }
        }
        return ((rval == 0) ? 1 : 0);
}

/*  */
/*
 * NAME: mfg_build
 *
 * FUNCTION: create a PT_SYS diskette in manufacturing with no menus
 *
 * RETURNS:
 */

void mfg_build(void)
{
int cnt;
int rval=0;
char serial[256];
char filename[256];

        read_vpd();

        /* if serial No. exists in vpd use it else use entered number */
        if(vprec.mr.serial[0] != '\0')
                strcpy(machine.serial,vprec.mr.serial);
        else sprintf(machine.serial,"*SE %s",mfg_sno);

        /* if machine type exists in vpd use it else use entered number */
        if(vprec.mr.type[0] != '\0')
                strcpy(machine.type,vprec.mr.type);
        else sprintf(machine.type,"*TM %s",mfg_typ);

        gen_sys_rec();
        write_hardfile();

        /* get rid of the SDDB file and rename the SYS file to serial#.vpd */
        sprintf(filename, "%s/PT_SDDB", diagdir);
        unlink(filename);
        strtok(mfg_sno, "\n");
        sprintf(serial, "%s/%s.vpd", diagdir, mfg_sno);
        sprintf(filename, "%s/PT_SYS", diagdir);
        rval = rename(filename, serial);
        term_dgodm();

        /* exits to the shell, shell uses exit code to determine success */
        exit(rval);
}

/*  */
/*
 * NAME: process_repair
 *
 * FUNCTION:  check for parts that have field entries changed and update
 *      the databases with the new data.
 *
 * RETURNS: NONE
 */

void process_repair(void)
{
int   rval;

        rval = test_add_del();
        if (rval < 0) {            /* If an error occurred */
                diag_asl_msg(diag_cat_gets(fdes, ESET, LOAD_ERR));
                return;
        }

        rval |= test_4repairs();

        if(rval)   /* If there have been changes, realign vpd and database */
                merge_data();

        search_vrec_scsi();  /* Search for SCSI devices without LO keyword */

        merge_data();
        write_hardfile();

        if (strncmp(outputfile,"\0",1) != 0) {  /* If outputfile is set */
                rval=write_flops(WRITE_OUTFILE);
                if(rval != 0)
                        genexit(3);
        }
        else {
                if (!fdd_gone) {
                        rval=write_flops(WRITE_SDDB);
                        if(rval != 0)
                                genexit(3);
                }
        }
}

/*  */
/*
 * NAME: gen_sys_rec
 *
 * FUNCTION: create system records with whatever vpd that's available,
 *      use dummy fc and fb codes when generating this diskette.
 *
 * RETURNS: NONE
 */

void gen_sys_rec(void)
{
struct FC_PTR *cfptr,*nfptr;
struct FB_PTR *cbptr,*nbptr;
struct PN_PTR *cpptr,*npptr;
struct VP_PTR *vrec;
int cnt;

        pn_ptr = (struct PN_PTR *)calloc_mem(vpcnt, sizeof(struct PN_PTR));
        fb_ptr = (struct FB_PTR *)calloc_mem(vpcnt, sizeof(struct FB_PTR));
        fc_ptr = (struct FC_PTR *)calloc_mem(vpcnt, sizeof(struct FC_PTR));

        vrec = vprec.vpd;
        cfptr=nfptr=fc_ptr;
        cbptr=nbptr=fb_ptr;
        cpptr=npptr=pn_ptr;

        for(cnt=0;cnt<vpcnt;cnt++) {
                /* create feature code record for each vpd item */
                nfptr->data = (F_REC *)malloc_mem(sizeof(F_REC));
                memset(nfptr->data,0,sizeof(F_REC));
                strcpy(nfptr->data->fc,"*FC ????????\n");

                /* fc record only has a dummy FC field so size is 1 */
                nfptr->data->rec_size=1;

                /* create bill of materials record */
                nbptr->data = (B_REC *)malloc_mem(sizeof(B_REC));
                memset(nbptr->data,0,sizeof(B_REC));
                strcpy(nbptr->data->fb,"*FB @@@@@@@@\n");

                /* fb record only has a dummy FB field so size is 1 */
                nbptr->data->rec_size=1;

                /* create part record */
                npptr->data = (P_REC *)malloc_mem(sizeof(P_REC));
                memset(npptr->data,0,sizeof(P_REC));

                /* load a dummy PN, it will get replaced if a real one exists */
                strcpy(npptr->data->pn,"*PN 00000000\n");
                npptr->data->rec_size = P_LINES;

                /* set current FC to next FC, then increment next FC */
                cfptr = nfptr;
                ++nfptr;

                /* set link of current FC to next FC */
                cfptr->nextFC=nfptr;

                /* set ptr to first PN for this FC to the PN ptr */
                cfptr->FstPN=npptr;

                /* set ptr to first FB for this FC to the FB ptr */
                cfptr->FstFB=nbptr;

                /* set current FB to next FB, then increment next FB */
                cbptr=nbptr;
                ++nbptr;

                /* set ptr to first PN for this FB to the PN ptr */
                cbptr->FstPN=npptr;

                /* set current PN to next PN, then increment next PN */
                cpptr=npptr;
                ++npptr;

                /* associate the vpd and part through a cross link */
                vrec->p_mate = cpptr;
                vrec= vrec->nextVP;
        }
        cfptr->nextFC = FC_NULL;
        cbptr->nextFB = FB_NULL;
        cpptr->nextPN = PN_NULL;

        merge_data();
}

/*  */
/*
 * NAME: au_newparts
 *
 * FUNCTION: add parts with an action code of AU
 *
 * RETURNS: NONE
 */

void au_newparts(void)
{
struct VP_PTR *vrec;

        vrec=vprec.vpd;
        while(TRUE) {
        /* any part in vpd without a cross link to a database part is new */
                if(vrec->p_mate == PN_NULL)
                        new_vitem(vrec);
                if(vrec->nextVP == VP_NULL)
                        break;
                vrec=vrec->nextVP;
        }
}

/*  */
/*
 * NAME:  set_install_date
 *
 * FUNCTION: build the install date field
 *
 * RETURNS: NONE
 */

void set_install_date(void)
{
        strcpy(machine.install,"*DC ID ");
        get_date(&machine.install[7]);
}

/*  */
/*
 * NAME: set_seq_no
 *
 * FUNCTION: increment and build the sequence number field
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */

void set_seq_no(void)
{
char *pseq;
int  tmp;

        pseq = machine.seq_no;
        if(*pseq == '\0')
                sprintf(machine.seq_no,"*SQ 1\n");
        else {
                while(!numeric(*pseq)) ++pseq; /* skip to the number */
                tmp =atoi(pseq);
                sprintf(machine.seq_no,"*SQ %d\n",tmp+1);
        }
}

/*  */
/*
 * NAME: test_4repairs
 *
 * FUNCTION: Run comparisons of disk and vpd for parts
 *
 * NOTES:
 *      for each part in vpd with a link to a database entry
 *      test PN, EC, SN, BC, CD, RL, FN, PI, NA, PC, SZ, MF, TM
 *      for a mismatch.  A mismatch occurs only if both the VPD data
 *      and the PT_SYS file have the same keyword but different
 *      keyword values.  If a mismatch is found, the history file
 *      records the repair.
 *
 * RETURNS: TRUE if a repair was detected
 *          FALSE otherwise
 */

int test_4repairs(void)
{
struct VP_PTR *vrec;
struct PN_PTR *pn;
int rflg;
int repair;
int i1;
char name[80];
char key_word[4];
LBUFF old[60];
LBUFF new[60];
LBUFF *lptr1, *lptr2;
SCRATCH *buf1, *buf2;

        vrec = vprec.vpd;
        repair = FALSE;
        while (vrec != VP_NULL)
          {
          if (vrec->p_mate != PN_NULL)
            {
            rflg = 0;
            pn = vrec->p_mate;
            if ((*vrec->data->pn == '*') &&
                (*pn->data->pn != '$') && (*pn->data->pn != '\0'))
              if (strcmp(pn->data->pn,vrec->data->pn))
                {
                strcpy(old[rflg].dat, pn->data->pn);
                strcpy(new[rflg++].dat, vrec->data->pn);
                }

            if ((*vrec->data->ec == '*') &&
                (*pn->data->ec != '$') && (*pn->data->ec != '\0'))
              if (strcmp(pn->data->ec,vrec->data->ec))
                {
                strcpy(old[rflg].dat, pn->data->ec);
                strcpy(new[rflg++].dat, vrec->data->ec);
                }

            if ((*vrec->data->sn == '*') &&
                (*pn->data->sn != '$') && (*pn->data->sn != '\0'))
              if (strcmp(pn->data->sn,vrec->data->sn))
                {
                strcpy(old[rflg].dat, pn->data->sn);
                strcpy(new[rflg++].dat, vrec->data->sn);
                }

            if ((*vrec->data->bc == '*') &&
                (*pn->data->bc != '$') && (*pn->data->bc != '\0'))
              if (strcmp(pn->data->bc,vrec->data->bc))
                {
                strcpy(old[rflg].dat, pn->data->bc);
                strcpy(new[rflg++].dat, vrec->data->bc);
                }

            if ((*vrec->data->cd == '*') &&
                (*pn->data->cd != '$') && (*pn->data->cd != '\0'))
              if (strcmp(pn->data->cd,vrec->data->cd))
                {
                strcpy(old[rflg].dat, pn->data->cd);
                strcpy(new[rflg++].dat, vrec->data->cd);
                }

            if ((*vrec->data->rl == '*') &&
                (*pn->data->rl != '$') && (*pn->data->rl != '\0'))
              if (strcmp(pn->data->rl,vrec->data->rl))
                {
                strcpy(old[rflg].dat, pn->data->rl);
                strcpy(new[rflg++].dat, vrec->data->rl);
                }

            if ((*vrec->data->fn == '*') &&
                (*pn->data->fn != '$') && (*pn->data->fn != '\0'))
              if (strcmp(pn->data->fn,vrec->data->fn))
                {
                strcpy(old[rflg].dat, pn->data->fn);
                strcpy(new[rflg++].dat, vrec->data->fn);
                }

            /************************************************************
            * Even though vrec->data is of type V_REC, which means      *
            * that the data stored there should assume a size of        *
            * sizeof(V_REC), there could be extra VPD from a resource   *
            * which did not have a corresponding keyword in V_REC, e.g. *
            * there is no vrec->data->tm.  This extra VPD would be      *
            * appended to the vrec->data information (see read_vpd()    *
            * and copy_vpd() subroutines).  This extra information is   *
            * being accessed by overlaying a SCRATCH data structure     *
            * on the V_REC data structure, thereby essentially extending*
            * the amount of data associated with vrec->data.            *
            ************************************************************/
            buf1 = (SCRATCH *)vrec->data;
            lptr1 = &buf1->dat[V_LINES];
            buf2 = (SCRATCH *)pn->data;

            while (*lptr1->dat != '\0')
              {
              if (test_kw("*PI", lptr1->dat))
                {
                if ((lptr2 = chk_extra_vpd(lptr1, &buf2->dat[V_LINES]))
                    != (LBUFF *)NULL)
                  {
                  strcpy(old[rflg].dat, lptr2->dat);
                  strcpy(new[rflg++].dat, lptr1->dat);
                  }
                }

              else if (test_kw("*NA", lptr1->dat))
                {
                if ((lptr2 = chk_extra_vpd(lptr1, &buf2->dat[V_LINES]))
                    != (LBUFF *)NULL)
                  {
                  strcpy(old[rflg].dat, lptr2->dat);
                  strcpy(new[rflg++].dat, lptr1->dat);
                  }
                }

              else if (test_kw("*PC", lptr1->dat))
                {
                if ((lptr2 = chk_extra_vpd(lptr1, &buf2->dat[V_LINES]))
                    != (LBUFF *)NULL)
                  {
                  strcpy(old[rflg].dat, lptr2->dat);
                  strcpy(new[rflg++].dat, lptr1->dat);
                  }
                }

              else if (test_kw("*SZ", lptr1->dat))
                {
                if ((lptr2 = chk_extra_vpd(lptr1, &buf2->dat[V_LINES]))
                    != (LBUFF *)NULL)
                  {
                  strcpy(old[rflg].dat, lptr2->dat);
                  strcpy(new[rflg++].dat, lptr1->dat);
                  }
                }

              else if (test_kw("*MF", lptr1->dat))
                {
                if ((lptr2 = chk_extra_vpd(lptr1, &buf2->dat[V_LINES]))
                    != (LBUFF *)NULL)
                  {
                  strcpy(old[rflg].dat, lptr2->dat);
                  strcpy(new[rflg++].dat, lptr1->dat);
                  }
                }

              else if (test_kw("*TM", lptr1->dat))
                {
                if ((lptr2 = chk_extra_vpd(lptr1, &buf2->dat[V_LINES]))
                    != (LBUFF *)NULL)
                  {
                  strcpy(old[rflg].dat, lptr2->dat);
                  strcpy(new[rflg++].dat, lptr1->dat);
                  }
                }

              else
                {
                for (i1=0; i1<10; i1++)
                  {
                  sprintf(key_word, "*Z%c", (char )(i1 + (int )'0'));
                  if (test_kw(key_word, lptr1->dat))
                    {
                    if ((lptr2 = chk_extra_vpd(lptr1, &buf2->dat[V_LINES]))
                        != (LBUFF *)NULL)
                      {
                      strcpy(old[rflg].dat, lptr2->dat);
                      strcpy(new[rflg++].dat, lptr1->dat);
                      }
                    }
                  }

                for (i1=0; i1<26; i1++)
                  {
                  sprintf(key_word, "*Z%c", (char )(i1 + (int )'A'));
                  if (test_kw(key_word, lptr1->dat))
                    {
                    if ((lptr2 = chk_extra_vpd(lptr1, &buf2->dat[V_LINES]))
                        != (LBUFF *)NULL)
                      {
                      strcpy(old[rflg].dat, lptr2->dat);
                      strcpy(new[rflg++].dat, lptr1->dat);
                      }
                    }
                  }
                }

              lptr1++;
              }

            /************************************************************
            * If a keyword difference was detected then update the      *
            * history file.  If running from hardfile, then alse        *
            * generate an errlog entry using the AX device name (minus  *
            * the extra new-line character).                            *
            ************************************************************/
            if (rflg > 0)
              {
              bld_repair_hist(pn, old, new, rflg);
              if (diskette == DIAG_FALSE)
                {
                if (test_kw("*AX", pn->data->ax))
                  {
                  strcpy(name, &pn->data->ax[4]);
                  name[strlen(name)-1] = '\0';
                  }
                else
                  strcpy(name, " ");
                log_error(ERRID_REPLACED_FRU, name);
                }
              repair = TRUE;
              }
            }
          vrec = vrec->nextVP;
          }

        return(repair);
}

/*  */
/****************************************************************
* NAME: log_error
*
* FUNCTION: Log an error in the errlog file.
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  No other local routines called.
*
* RETURNS:  No return value.
*
****************************************************************/
void log_error(int id, char *name)
{
  struct err_rec0 errbuf;

        errbuf.error_id = id;
        strcpy(errbuf.resource_name, name);
        errlog((char *)&errbuf, ERR_REC_SIZE);
        return;
}

/*  */
/*
 * NAME: chk_extra_vpd
 *
 * FUNCTION: compare VPD values between ODM data and
 *           Product Topology diskettes
 *
 * NOTES:
 *
 * RETURNS: (LBUFF *) = Matching keywords but different keyword values.
 *          NULL      = Matching keywords and matching keyword values,
 *                      OR, no matching keywords.
 */
LBUFF *chk_extra_vpd(
        LBUFF *lptr1,
        LBUFF *lptr2)
{

        /****************************************************************
        * Search the strings that are stored in *lptr2.  If a keyword   *
        * matches between *lptr1 and *lptr2, check to see if both       *
        * complete strings match.  If not, go on to the next string     *
        * in *lptr2.                                                    *
        ****************************************************************/
        while (*lptr2->dat != '\0')
          {
          if (!strncmp(lptr1->dat, lptr2->dat, 3))
            {
            if (!strcmp(lptr1->dat, lptr2->dat)) return (LBUFF *)NULL;
            else                                 return lptr2;
            }
          lptr2++;
          }

        /* If no keyword in *lptr2 matches *lptr1 then return NULL      */
        return (LBUFF *)NULL;
}

/*  */
/*
 * NAME: merge_data
 *
 * FUNCTION: combine disk and vpd data
 *
 * NOTES:
 *      Any vpd data will replace data currently in the database
 *
 * RETURNS: NONE
 */

void merge_data(void)
{
struct VP_PTR *vrec;
struct PN_PTR *pn;
int cnt;
LBUFF *tpptr;
LBUFF *tvptr;

        vrec = vprec.vpd;


        tvptr= (LBUFF *) &vprec.mr;
        tpptr= (LBUFF *) &machine;
        for(cnt=0; cnt < (sizeof(MACH_REC)/FIELD_SIZE); cnt++) {
                if( (*(char *) tpptr) == (char) '\0')
                        strcpy(tpptr->dat, tvptr->dat);
                ++tpptr;
                ++tvptr;
        }

        while(TRUE) {
                if(vrec->p_mate != PN_NULL) {
                        pn = vrec->p_mate;
                        tpptr = (LBUFF *) pn->data->pn;
                        tvptr = (LBUFF *) vrec->data->pn;
                        for(cnt=0; cnt < V_LINES; cnt++,tvptr++,tpptr++) {
                                if(*tvptr->dat == '*')
                                        strcpy(tpptr->dat, tvptr->dat);

                        }

                        /* Don't merge extra lines if item is new       */
                        /* in which case data for vrec and its p_mate   */
                        /* are identical                                */
                        if ((vrec->data != pn->data) &&
                            (vrec->data->rec_size > V_LINES))
                                merge_extras(pn,vrec->data);

                }
                if(vrec->nextVP == VP_NULL) break;
                vrec=vrec->nextVP;
        }
}

/*  */
/*
 * NAME:  merge_extras
 *
 * FUNCTION: add data that is not part of the P_REC struct after the end of the
 *      P_REC struct
 *
 * RETURNS: NONE
 */

void merge_extras(struct PN_PTR *pn, V_REC *vp)
{
int cnt;
int cntr;
int bsiz;
int match;
P_REC *tmp;
LBUFF *pn_ext;
LBUFF *vp_ext;
LBUFF *free_spot;

/* allocate a block that has enough space for all the extra lines */
/* in both the PN record and the VPD record */

        bsiz = (((pn->data->rec_size-P_LINES)+
                (vp->rec_size-V_LINES))*FIELD_SIZE)+sizeof(P_REC);
        tmp = (P_REC *) malloc_mem(bsiz);
        memset(tmp,0,bsiz);
        memcpy(tmp,pn->data,((pn->data->rec_size-P_LINES)*FIELD_SIZE)+
                                sizeof(P_REC));
/* set pointer to first line after the base pn record */
        free_spot=(LBUFF*)tmp->pn+tmp->rec_size;

/* set pointer to first vpd item not in the base vpd record */
        vp_ext = (LBUFF *) vp->pn+V_LINES;

        for(cnt=V_LINES; cnt < vp->rec_size; cnt++,vp_ext++) {
                if(pn->data->rec_size > P_LINES) {

/* since the pn record has extra data items each pn item is compared */
/* against the extra vpd item. if a match is found copy the vpd data */
/* over that entry otherwise add the item as a new one to the end of */
/* the record.  This eliminates duplicate entries for extra data items */

                        match = FALSE;
                        pn_ext = (LBUFF *) tmp->pn+P_LINES;
                        for (cntr=P_LINES; cntr < pn->data->rec_size; cntr++) {
                                if (!strncmp(pn_ext->dat, vp_ext->dat, 3)) {
                                        strcpy(pn_ext->dat, vp_ext->dat);
                                        match=TRUE;
                                        break;
                                }
                                ++pn_ext;
                        }
                        if(!match) {
                                strcpy(free_spot->dat, vp_ext->dat);
                                ++free_spot;
                                ++tmp->rec_size;
                        }
                }
/* the pn record has no extra data items so unconditionally copy */
                else {
                        strcpy(free_spot->dat, vp_ext->dat);
                        ++free_spot;
                        tmp->rec_size++;
                }
        }
        free(pn->data);
        pn->data = tmp;
}

/*  */
/*
 * NAME: remove_parts
 *
 * FUNCTION:  test for disk records that are not in vpd anymore and remove them
 *
 * NOTES:
 *      first determine the total number of parts in this feature code
 *      for each FB in this feature code
 *              determine the total number of parts in this FB
 *              scan through the parts for a record that does not match
 *              a vpd record and has not been flagged as a temporary removal
 *              if a missing part is detected set the detected flag and start
 *              over to actual remove the parts.
 *              get the action code for the removed part
 *              if the action code is remove temp set a flag to avoid removing
 *              non detectable parts from this FB.
 *              else flag the part for deletion and increment the deleted parts
 *              counters.
 *              if a non detectable part exists for this FB test that there was
 *              a detectable part deleted permanently before flagging for
 *              deletion.
 *              if flagged for deletion increment the deleted parts counters.
 *      if all of the parts for this FB were deleted delete the FB
 *      go to the next FB
 *      if all the parts for this FC were deleted delete the FC
 *
 * RETURNS:
 *
 */

void remove_parts(struct FC_PTR *fc)
{
struct PN_PTR *pp;
struct FB_PTR *bptr;
int rc;
int total_parts=0;
int total_dels=0;
int fb_parts;
int fb_dels;
char buf[80];
char danda[80];
int detect_del;
int tmp_rmv;
MD_2LINES mpnts;

        mpnts.baseptr = (int *)fc;
        mpnts.type = FEATURE;
        bptr = fc->FstFB;
        while (bptr != FB_NULL) {
                pp = bptr->FstPN;
                while (pp != PN_NULL) {
                        ++total_parts;
                        pp = pp->nextPN;
                }
                bptr = bptr->nextFB;
        }

        bptr = fc->FstFB;
        while(bptr != FB_NULL) {
                fb_parts=0;
                fb_dels=0;
                pp = bptr->FstPN;
                while(pp != PN_NULL) {
                        ++fb_parts;
                        pp = pp->nextPN;
                }
                pp = bptr->FstPN;
                detect_del=FALSE;
                tmp_rmv=FALSE;
                while(pp != PN_NULL) {
                        if(*pp->data->pl == '*' &&  pp->v_mate == VP_NULL &&
                                                !(pp->data->flags&RMV_TMP)) {
                                if( detect_del == FALSE ) {
                                        detect_del = TRUE;
                                        pp = bptr->FstPN;
                                        continue;
                                }
                                if (strncmp(pp->data->dc,"*DC RT",6) == 0) {
                                        rc = 4;  /* Already marked RT */
                                        get_date(date_buf);
                                        strcpy(danda,"*DC RT ");
                                        strcat(danda,date_buf);
                                        strcpy(pp->data->dc,danda);
                                        tmp_rmv = TRUE;
                                }
                                else {
                                        rc = 5;  /* Force *DC RM */
                                        pp->action = 0x10 | rc;
                                        ++total_dels;
                                        ++fb_dels;
                                 }
                        }
                        else if(*pp->data->pl != '*' && detect_del == TRUE
                                                        && tmp_rmv == FALSE) {
                                pp->action = 0x10 | rc;
                                ++total_dels;
                                ++fb_dels;
                        }
                        pp = pp->nextPN;
                }
                if(fb_parts == fb_dels)
                        bptr->action = 0x10 | rc;
                bptr = bptr->nextFB;
        }
        if(total_parts == total_dels) {
                fc->action = 0x10;
        }
}

/*  */
/*
 * NAME: build_mes_hist
 *
 * FUNCTION:  generate a mes add history record
 *
 * RETURNS: NONE
 */

void build_mes_hist(struct PN_PTR *pptr)
{
HIST_REC tmp;

        memset(&tmp,0,sizeof(tmp));
        get_date(date_buf);

        strcpy(tmp.fcode,pptr->FCown->data->fc);
        strcpy(tmp.p_num,pptr->data->pn);
        strcpy(tmp.eclvl,pptr->data->ec);
        strcpy(tmp.s_num,pptr->data->sn);
        strcpy(tmp.bar_c,pptr->data->bc);
        strcpy(tmp.danda,"*DC AU ");

        strcat(tmp.danda,date_buf);
        strcpy(pptr->FCown->data->dc,tmp.danda);
        write_hist(&tmp);
}

/*  */
/*
 * NAME:  build_del_hist
 *
 * FUNCTION:  generate a delete history record
 *
 * RETURNS: NONE
 */

void build_del_hist(struct PN_PTR *pp)
{
HIST_REC tmp;

        get_date(date_buf);
        memset(&tmp,0,sizeof(tmp));

        strcpy(tmp.fcode,pp->FCown->data->fc);
        if(*pp->data->pn == '*') strcpy(tmp.p_num,pp->data->pn);
        if(*pp->data->ec == '*') strcpy(tmp.eclvl,pp->data->ec);
        if(*pp->data->sn == '*') strcpy(tmp.s_num,pp->data->sn);
        if(*pp->data->bc == '*') strcpy(tmp.bar_c,pp->data->bc);
        strcpy(tmp.danda,"*DC RM ");
        strcat(tmp.danda,date_buf);
        write_hist(&tmp);
}

/*  */
/*
 * NAME: bld_repair_hist
 *
 * FUNCTION:  generate a repair history record
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

void bld_repair_hist(struct PN_PTR *dat,
                     LBUFF *old,
                     LBUFF *new,
                     int cnt)
{
  HIST_REC tmp;
  int i1;

        get_date(date_buf);
        memset(&tmp,0,sizeof(tmp));
        strcpy(tmp.fcode,dat->FCown->data->fc);

        if (*dat->data->pn == '*')
          strcpy(tmp.p_num,dat->data->pn);

        if (*dat->data->ec == '*')
          strcpy(tmp.eclvl,dat->data->ec);

        if (*dat->data->sn == '*')
          strcpy(tmp.s_num,dat->data->sn);

        if (*dat->data->bc == '*')
          strcpy(tmp.bar_c,dat->data->bc);

        for (i1=0; i1<cnt; i1++)
          {
          strcpy(tmp.chang[i1].dat, old[i1].dat);
          tmp.chang[i1].dat[0] = '#';
          }

        strcpy(tmp.danda, "*DC RR ");
        strcat(tmp.danda, date_buf);
        write_hist(&tmp);

        memset(&tmp,0,sizeof(tmp));
        strcpy(tmp.fcode,dat->FCown->data->fc);
        for (i1=0; i1<cnt; i1++)
          {
          strcpy(tmp.chang[i1].dat, new[i1].dat);
          }

        strcpy(tmp.danda, "*DC AR ");
        strcat(tmp.danda, date_buf);
        write_hist(&tmp);
        strcpy(dat->data->dc, tmp.danda);
}

/*  */
/*
 * NAME: rm_pts
 *
 * FUNCTION: null out pointers for deleted parts, adjust links and
 *      free up memory used for the data
 *
 * RETURNS: NONE
 */

void rm_pts(struct PN_PTR *pp)
{
        if((pp->action&0x10) == 0) return;
        build_del_hist(pp);
        free(pp->data);
        pp->data = (P_REC *) 0;
        if(pp->prevPN == PN_NULL && pp->nextPN == PN_NULL) {
                pp->FCown->FstPN = PN_NULL;
                pp->FBown->FstPN = PN_NULL;
        }
        else if( pp->prevPN != PN_NULL && pp->nextPN == PN_NULL)
                pp->prevPN->nextPN = PN_NULL;
        else if( pp->prevPN == PN_NULL && pp->nextPN != PN_NULL) {
                pp->FCown->FstPN = pp->nextPN;
                pp->FBown->FstPN = pp->nextPN;
                pp->nextPN->prevPN = PN_NULL;
        }
        else {        /* pp->prevPN != PN_NULL && pp->nextPN != PN_NULL */
                pp->prevPN->nextPN = pp->nextPN;
                pp->nextPN->prevPN = pp->prevPN;
        }
        --pncnt;
}

/*  */
/*
 * NAME:  del_fcs
 *
 * FUNCTION: null out pointers for deleted fcs, adjust links and
 *      free up memory used for the data
 *
 * RETURNS: NONE
 */

void del_fcs(struct FC_PTR *fp)
{
        if((fp->action&0x10) == 0) return;
        free(fp->data);
        fp->data = (F_REC *) 0;
        if(fp->prevFC == FC_NULL) {
                fc_ptr = fp->nextFC;
                fp->nextFC->prevFC = FC_NULL;
        }
        else if(fp->prevFC != FC_NULL  && fp->nextFC != FC_NULL) {
                fp->prevFC->nextFC = fp->nextFC;
                fp->nextFC->prevFC = fp->prevFC;
        }
        else if(fp->prevFC != FC_NULL && fp->nextFC == FC_NULL)
                fp->prevFC->nextFC = FC_NULL;
        --fccnt;
}

/*  */
/*
 * NAME:  del_fbs
 *
 * FUNCTION: null out pointers for deleted fbs, adjust links and
 *      free up memory used for the data
 *
 * RETURNS: NONE
 */

void del_fbs(struct FB_PTR *bp)
{
        if((bp->action&0x10) == 0) return;
        free(bp->data);
        bp->data = (B_REC *) 0;
        if(bp->prevFB == FB_NULL && bp->nextFB == FB_NULL)
                bp->FCown->FstFB = FB_NULL;
        else if( bp->prevFB !=FB_NULL && bp->nextFB == FB_NULL)
                bp->prevFB->nextFB = FB_NULL;
        else if( bp->prevFB == FB_NULL && bp->nextFB != FB_NULL) {
                bp->FCown->FstFB = bp->nextFB;
                bp->nextFB->prevFB = FB_NULL;
        }
        else {        /* bp->prevFB != FB_NULL && bp->nextFB != FB_NULL */
                bp->prevFB->nextFB = bp->nextFB;
                bp->nextFB->prevFB = bp->prevFB;
        }
        --fbcnt;
}

/*  */
/*
 * NAME: remove_mates
 *
 * FUNCTION: null out cross pointers between vpd and database
 *
 * RETURNS: NONE
 */

void remove_mates(void)
{
struct VP_PTR *vp;
pd_struct pd_str;

        pd_str.dir = 1;
        pd_str.func1 = NOFUNC;
        pd_str.func2 = NOFUNC;
        pd_str.func3 = (void (*)(void))remove_vmates;
        pd_str.str1 = (char *)NULL;
        pd_str.str2 = (char *)NULL;
        pd_str.str3 = (char *)NULL;
        parse_data(&pd_str);

        vp = vprec.vpd;

        while(TRUE) {
                vp->p_mate = PN_NULL;
                if(vp->nextVP == VP_NULL)
                        break;
                vp = vp->nextVP;
        }
}

/*  */
/*
 * NAME: remove_vmates
 *
 * FUNCTION: null out cross pointers between database and vpd
 *
 * RETURNS: NONE
 */

void remove_vmates(struct PN_PTR *pp)
{
        pp->v_mate = VP_NULL;
}

/*  */
/*
 * NAME: test_add_del
 *
 * FUNCTION: test for added or deleted parts, then log entries in the PT_SYS
 *           and the PT_HIST files with an action code of AU or RM for their
 *           addition or removal.
 *
 * RETURNS: 0 if nothing needed updating
 *          >0 if database was updated
 *          <0 if an error was detected
 */

int test_add_del(void)
{
int        rval=0;
int        tmp;
pd_struct  pd_str;

        if (tmp = new_parts())
                au_newparts();
        rval |= tmp;
        if (tmp = lost_parts()) {
                pd_str.dir = 1;
                pd_str.func1 = (void (*)(void))remove_parts;
                pd_str.func2 = NOFUNC;
                pd_str.func3 = NOFUNC;
                pd_str.str1 = (char *)NULL;
                pd_str.str2 = (char *)NULL;
                pd_str.str3 = (char *)NULL;
                parse_data(&pd_str);
                pd_str.dir = 0;
                pd_str.func1 = (void (*)(void))del_fcs;
                pd_str.func2 = (void (*)(void))del_fbs;
                pd_str.func3 = (void (*)(void))rm_pts;
                parse_data(&pd_str);
        }
        rval |= tmp;

        if (rval) {
                free(loctab);
                loctab = (struct PN_PTR **) calloc_mem (pncnt+1, sizeof(loctab[0]));
                pd_str.dir = 1;
                pd_str.func1 = NOFUNC;
                pd_str.func2 = NOFUNC;
                pd_str.func3 = (void (*)(void))build_loc_table;
                pd_str.str1 = (char *)NULL;
                pd_str.str2 = (char *)NULL;
                pd_str.str3 = (char *)NULL;
                parse_data(&pd_str);
                remove_mates();
                build_assoc();
        }

        return(rval);
}
/*  */
/*
 * NAME: search_vrec_scsi
 *
 * FUNCTION: search VPD for SCSI devices.
 *
 * RETURNS: NONE
 */

void search_vrec_scsi(void)
{
struct VP_PTR *vrec;

        vrec=vprec.vpd;
        while(TRUE) {
                /* check all VPD for SCSI device, make sure it has LO keyword */
                chk_4_scsi_dev(vrec);
                if(vrec->nextVP == VP_NULL)
                        break;
                vrec=vrec->nextVP;
        }
}
/*  */
/*
 * NAME: calloc_mem
 *
 * FUNCTION: allocate and clear memory.  If there is an error, then terminate.
 *
 * RETURNS: Pointer to allocated memory
 */

void* calloc_mem(int number, int size)
{
void *tdat;

        if ((tdat = calloc(number, size)) == (void *) 0) {
                if(!no_menus)
                        disp_popup_message(MEM_ERR);
                exit(202);
        }
        return(tdat);
}
/*  */
/*
 * NAME: malloc_mem
 *
 * FUNCTION: allocate and clear memory.  If there is an error, then terminate.
 *
 * RETURNS: Pointer to allocated memory
 */

void* malloc_mem(int size)
{
void *tdat;

        if ((tdat = malloc(size)) == (void *) 0) {
                if(!no_menus)
                        disp_popup_message(MEM_ERR);
                exit(202);
        }
        return(tdat);
}


