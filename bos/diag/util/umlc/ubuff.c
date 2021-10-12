static char sccsid[] = "@(#)39  1.20.1.5  src/bos/diag/util/umlc/ubuff.c, dsaumlc, bos411, 9428A410j 6/28/93 16:27:58";
/*
 * COMPONENT_NAME: (DUTIL) DIAGNOSTIC UTILITY
 *
 * FUNCTIONS:   load_hard(char *fn)
 *              load_sddb(fname)
 *              read_data(fname,nfptr,nbptr,npptr,tmach)
 *              vc_to_int()
 *              proc_frec(pfptr,nfptr,fc,fileptr,link)
 *              proc_brec(pfptr,pbptr,nbptr,fb,fileptr)
 *              proc_prec(pfptr,pbptr,ppptr,npptr,pn,fileptr)
 *              stuff_rec(ptr,cnt)
 *              load_prec(pr,buf,val)
 *              load_frec(fr,buf,val)
 *              load_brec(br,buf,val)
 *              get_kw_val(strg)
 *              test_kw(key,strg)
 *              test_lvl_kw(strg)
 *              load_mach(strg,buf,other)
 *              parse_data(pd_struct)
 *              build_loc_table(p)
 *              build_assoc()
 *              scan_4ax(vstrg)
 *              scan_4sn(vstrg)
 *              scan_4pn(vstrg)
 *              scan_4cd(vstrg)
 *              scan_4ec(vstrg)
 *              new_parts()
 *              lost_parts()
 *              rmvd_parts(pn,flg)
 *              read_vpd()
 *              dev_ID(hex_str)
 *              copy_vpd( vpd,buf,vnum)
 *              proc_vrec(key,index,vptr,lptr,dat,vrec)
 *              load_vrec(vr,buf,val)
 *              load_vpd_mach(vpd,vnum)
 *              update_sysunit_vpd()
 *              add_vpd_entry(keyword, data, vpdptr)
 *              update_vpd_entry(keyword, data, vpdptr)
 *              scan_file(fname,cnt)
 *              build_newrec(vptr,buf)
 *              last_fc()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
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
#include "umlc_msg.h"
#include "mlc_defs.h"
#include "diag/class_def.h"
#include "diag/diago.h"
#include "diag/diag.h"
#include "diag/tmdefs.h"
#include "diag/diagvpd.h"

/* EXTERN VARIABLES     */
extern  char *keywrds[];
extern  char *lvl_kwds[];
extern  char diagdir[];
extern  VPD_REC         vprec;
extern  struct PN_PTR   *pn_ptr;
extern  struct FB_PTR   *fb_ptr;
extern  struct FC_PTR   *fc_ptr;
extern  MACH_REC        machine;
extern  int             pncnt;
extern  int             fbcnt;
extern  int             fccnt;
extern  int             vpcnt;

/* EXTERNAL FUNCTIONS */
extern void* calloc_mem(int, int);
extern void* malloc_mem(int);

/* LOCAL VARIABLES      */
struct PN_PTR **loctab;

/*  ANSI C FUNCTION PROTOTYPING */
int load_hard(char *);
int load_sddb(char *);
int read_data(char *, struct FC_PTR *, struct FB_PTR *,
                struct PN_PTR *, MACH_REC *, ITEM_CNT *,
                short);
int vc_to_int(void);
void proc_frec(struct FC_PTR *, struct FC_PTR *,char *,
                FILE *, int);
void proc_brec(struct FC_PTR *, struct FB_PTR *,
                struct FB_PTR *, char *, FILE *);
void proc_prec(struct FC_PTR *, struct FB_PTR *,
                struct PN_PTR *, struct PN_PTR *,
                char *, FILE *);
void stuff_rec(LBUFF *, int);
void load_prec(P_REC *, LBUFF *, int);
void load_frec(F_REC *, LBUFF *, int);
void load_brec(B_REC *, LBUFF *, int);
int get_kw_val(char *);
int test_kw(char const *, char const *);
int test_lvl_kw(char const *);
int load_mach(LBUFF *, MACH_REC *, LBUFF *);
void parse_data(pd_struct *);
void build_loc_table(struct PN_PTR *);
void build_assoc(void);
int scan_4ax(char *, int);
int scan_4sn(char *);
int scan_4pn(char *);
int scan_4cd(char *);
int scan_4ec(char *);
int new_parts(void);
int lost_parts(void);
int rmvd_parts(struct PN_PTR *, int *);
int read_vpd(void);
int dev_ID(char *);
void copy_vpd(struct CuVPD *, SCRATCH *, int);
void proc_vrec(LBUFF *, char *, V_REC *);
void load_vrec(V_REC *, LBUFF *, int);
void load_vpd_mach(struct CuVPD *, int);
int update_sysunit_vpd(void);
int add_vpd_entry(char *, char *, char *);
int update_vpd_entry(char *, char *, char *);
int scan_file(char *, ITEM_CNT *);
struct PN_PTR *build_newrec(struct VP_PTR *, LBUFF *);
struct FC_PTR *last_fc(void);

/*  */
/*
 * NAME: load_hard
 *
 * FUNCTION: this function sets up the full path name for hardfile reads
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */

int load_hard(char *fn)
{
char fbuff[256];

        sprintf(fbuff, "%s/%s", diagdir, fn);
        return(load_sddb(fbuff)) ;
}

/*  */
/*
 * NAME: load_sddb
 *
 * FUNCTION: reads the data for the given filename and calls numerous
 *              functions to build all the tables
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:  0 - if data read
 *          -1 - if error
 */

int load_sddb(char *fname)
{
int i1;
char *mach_init;
pd_struct pd_str;
ITEM_CNT        tmp;


        tmp.fcs = 0;
        tmp.fbs = 0;
        tmp.parts = 0;
        mach_init = (char *) &machine;
        for (i1=0; i1 < (sizeof(MACH_REC)/sizeof(char)); i1++)
          *(mach_init+i1) = '\0';
        if(!scan_file(fname,&tmp)) return(-1);
        if(!read_data(fname,fc_ptr,fb_ptr,pn_ptr,&machine,&tmp,
                                                (short)0)) return(-1);
        fccnt = tmp.fcs;
        fbcnt = tmp.fbs;
        pncnt = tmp.parts;

        if(!read_vpd()) return(-1);
        loctab = (struct PN_PTR **) calloc_mem(pncnt+1,sizeof(loctab[0]));

        pd_str.dir = 1;
        pd_str.func1 = NOFUNC;
        pd_str.func2 = NOFUNC;
        pd_str.func3 = (void (*)(void))build_loc_table;
        pd_str.str1 = (char *)NULL;
        pd_str.str2 = (char *)NULL;
        pd_str.str3 = (char *)NULL;
        parse_data(&pd_str);

        build_assoc();
        return 0;
}

/*  */
/*
 * NAME: read_data
 *
 * FUNCTION: this function reads the file and stores the data in buffers.
 *
 * NOTES:
 *      all of the file reading functions will clip the read data to
 *      a maximum of 46 characters.
 *      read lines until the first FC code is encountered storing the
 *      data in the machine record structure
 *      read lines until EOF process FC's, FB's and PN's accordingly if
 *      read in the proper sequence. Sequence logic is:
 *      1 = must be an FC ,  4 =  can be an FC because we completed a set
 *      2 = must be an FB ,  4 = can be an FB because multiple FB's for an FC
 *      3 = must be a PN , 4 = can be a PN because multiple PN's for an FB
 *      when finished delete the file if read from the tmp directory
 *
 * RETURNS: total number of FC's + FB's + PN's
 */
int read_data(char *fname,
        struct FC_PTR *nfptr,
        struct FB_PTR *nbptr,
        struct PN_PTR *npptr,
        MACH_REC *tmach,
        ITEM_CNT *tcnt,
        short flg)
{
FILE *pnew;
char tmp[256];
char *pstr;
int rval = 0;
struct PN_PTR *ppptr,*tmppn,*tpn;
struct FB_PTR *pbptr,*tmpfb,*tfb;
struct FC_PTR *pfptr;
LBUFF *dptr;
int fc_val= 0;
int seq_no;
int bad_seq = 0;
int near_eof;

        pnew = (FILE *) fopen(fname,"r");
        if (pnew == (FILE *) 0) return 0;

        ppptr = npptr;
        pbptr = nbptr;
        pfptr = nfptr;

/* pointer to machine record storage for undefined keywords */
        dptr = (LBUFF *) tmach->d1;

        while ((fgets(tmp,255,pnew)) != ((char *) 0)) {
                if (strlen(tmp) >= FIELD_SIZE-2)
                        strcpy(&tmp[FIELD_SIZE-2],"\n");
                if (test_kw("*FC",tmp)) {
                        fseek(pnew,(long) -(strlen(tmp)),1);
                        seq_no = 1;
                        break;
                }

/* if undefined keyword added, then bump pointer */
                if (load_mach((LBUFF *)tmp, tmach, dptr))
                        ++dptr;
        }

        near_eof = FALSE;
        while ((fgets(tmp,255,pnew)) != ((char *) 0)) {
                if (strlen(tmp) >= FIELD_SIZE-2)
                        strcpy(&tmp[FIELD_SIZE-2],"\n");
                if (test_kw("*FC",tmp)) {
                        if (seq_no == 1 || seq_no == 4) {
                                seq_no = 2;
                                proc_frec(pfptr,nfptr,tmp,pnew,fc_val);
                                fc_val = 1; /* first FC has been processed */
                                pfptr = nfptr;
                                ++nfptr;

                                /* if data block was not allocated, then*/
                                /* no more FB's or PN's left in file    */
                                if (pfptr->data == (F_REC *)0)
                                  near_eof = TRUE;
                                else
                                  pfptr->data->flags |= flg;
                                ++rval;
                        }
                        /* unexpected FC encountered. file is corrupted */
                        else {
                                bad_seq = 1;
                        }
                }
                else if (test_kw("*FB",tmp)) {
                        if (seq_no == 2 || seq_no == 4) {
                                seq_no = 3;
                                proc_brec(pfptr,pbptr,nbptr,tmp,pnew);
                                pbptr = nbptr;
                                ++nbptr;

                                /* if data block was not allocated,     */
                                /* then no more PN's left in file       */
                                if (pbptr->data == (B_REC *)0)
                                  near_eof = TRUE;
                                else
                                  pbptr->data->flags |= flg;
                        }
                        /* unexpected FB encountered. file is corrupted */
                        else {
                                bad_seq = 2;
                                --tcnt->fbs;
                        }
                }
                else if (test_kw("*PN",tmp)) {
                        if (seq_no == 3 || seq_no == 4) {
                                seq_no = 4;
                                proc_prec(pfptr,pbptr,ppptr,npptr,tmp,pnew);
                                ppptr = npptr;
/* if part has a remove temp date code, flag it for special processing */
                                if (!(strncmp(ppptr->data->dc,"*DC RT",6)))
                                        ppptr->data->flags |= RMV_TMP;
                                ++npptr ;
                                ppptr->data->flags |= flg;
                        }
                        /* unexpected PN encountered. file is corrupted */
                        else {
                                bad_seq = 3;
                                --tcnt->parts;
                        }
                }

                if ((bad_seq > 0 && vc_to_int() < 12) || near_eof) {
                        if (pfptr->data != (F_REC *)0)
                          {
                          free(pfptr->data);
                          pfptr->data = (F_REC *)0;
                          }
                        tmpfb = pfptr->FstFB;
                        while (tmpfb != FB_NULL) {
                                tfb = tmpfb;
                                tmppn = tmpfb->FstPN;
                                while (tmppn != PN_NULL) {
                                        tpn = tmppn;
                                        --pncnt;
                                        if (tmppn->data != (P_REC *)0)
                                          {
                                          free(tmppn->data);
                                          tmppn->data = (P_REC *)0;
                                          }
                                        tmppn = tmppn->nextPN;
                                        memset(tpn, 0, sizeof(struct PN_PTR));
                                }

                                --fbcnt;
                                if (tmpfb->data != (B_REC *)0)
                                  {
                                  free(tmpfb->data);
                                  tmpfb->data = (B_REC *)0;
                                  }
                                tmpfb = tmpfb->nextFB;
                                memset(tfb,0,sizeof(struct FB_PTR));
                        }
                        pfptr->FstFB = FB_NULL;
                        pfptr->FstPN = PN_NULL;
                        if (pfptr->prevFC == FC_NULL) fc_val = 0;
                        else pfptr = pfptr->prevFC;
                        pfptr->nextFC = FC_NULL;
                        --rval;
                        --tcnt->fcs;
                        --nfptr;

                        if (near_eof)
                          break;
                        else if (bad_seq > 1)
                          {
                          while ((pstr = fgets(tmp, 255, pnew)) != (char *)NULL)
                            {
                            if (test_kw("*FC", tmp))
                              break;
                            else
                              {
                              if      (test_kw("*FB", tmp)) --tcnt->fbs;
                              else if (test_kw("*PN", tmp)) --tcnt->parts;
                              }
                            }
                          if (pstr == (char *)NULL) break;
                          }

                        fseek(pnew,(long) -(strlen(tmp)),1);
                        bad_seq = 0;
                        seq_no = 1;
                }
                else if (bad_seq > 0  && vc_to_int() >= 12) {
                        disp_popup_message(INTERNAL_ERR);
                        genexit(2);
                }
        }

        fclose(pnew);
        if (strncmp(fname,"/tmp",4) == 0)
                unlink(fname);
        return(rval);
}

/*  */
/*
 * NAME: vc_to_int
 *
 * FUNCTION: this function converts the version code of the
 *           Product Topology data into an integer.  Thus, VC=1.1
 *           is converted to 11 and VC=1.2 => 12.
 *
 * NOTES:
 *
 * RETURNS: The integer equivalent of the VC value
 */
int vc_to_int(void)
{
char tbuf[10];
char *tmp,*ttmp;
int rval;

        tmp = machine.vers;
        if(*tmp == '\0') return(0);

        memset(tbuf,0,10);
        ttmp=tbuf;
        while(!(isdigit(*tmp)) && *tmp != '\0' && *tmp != '\n') ++tmp;
        while(isdigit(*tmp)) *ttmp++ = *tmp++;
        rval=atoi(tbuf)*10;

        memset(tbuf,0,10);
        ttmp=tbuf;
        while(!(isdigit(*tmp)) && *tmp != '\0' && *tmp != '\n') ++tmp;
        while(isdigit(*tmp) && *tmp != '\0' && *tmp != '\n') *ttmp++ = *tmp++;
        rval +=atoi(tbuf)%10;

        return(rval);
}

/*  */
/*
 * NAME: proc_frec
 *
 * FUNCTION: read and store the FC data and updates the links
 *
 * NOTES:
 *      read lines until a PN or FB keyword is detected. A test for
 *      EOF is included, however a file should never have an FC record last
 *      if this happens then the data block will not be allocated and read
 *      data will error out when this function returns.
 *
 * RETURNS: NONE
 */

void proc_frec(struct FC_PTR *pfptr,
        struct FC_PTR *nfptr,
        char *fc,
        FILE *fileptr,
        int link)
{
F_REC *frec;
int size;
int val;
SCRATCH spad;
LBUFF   *sptr;
char tmp[256];

        memset(tmp,0,sizeof(tmp));
        memset(&spad,0,sizeof(spad));
        frec = (F_REC *) &spad;
        sptr = (LBUFF *) &spad.dat[F_LINES];
        size = F_LINES;
        stuff_rec((LBUFF *)frec->fc,F_LINES);
        strcpy(frec->fc,fc);
        while ((fgets(tmp,255,fileptr)) != ((char *) 0)) {
                if (test_lvl_kw(tmp)) {
                     fseek(fileptr,(long) -(strlen(tmp)),1);
                     nfptr->data = (F_REC *) malloc_mem(sizeof(F_REC)+
                                                   ((size-F_LINES)*FIELD_SIZE));
                     frec->rec_size = size;
                     memcpy(nfptr->data,&spad,((size-F_LINES)*FIELD_SIZE)+
                                                        sizeof(F_REC));
                     break;
                }
                if (strlen(tmp) >= FIELD_SIZE-2)
                        strcpy(&tmp[FIELD_SIZE-2],"\n");
                val = get_kw_val(tmp);

/* if the line is part of the FC structure store it in the structure */
/* otherwise tack it on to the end */

                if (val < FB || val == DATE || val == DSMSG)
                        load_frec(frec, (LBUFF *)tmp, val);
                else {
                        strcpy(sptr->dat,tmp);
                        ++sptr;
                        ++size;
                }
        }
/* except for the very first FC set both forward an backword links */
        if (link) {
                pfptr->nextFC = nfptr;
                nfptr->prevFC = pfptr;
        }
}

/*  */
/*
 * NAME:  proc_brec
 *
 * FUNCTION: read and store the fb data and updates the links
 *
 * NOTES:
 *      see notes under proc_frec similar situation except its a FB
 *
 * RETURNS: NONE
 */

void proc_brec( struct FC_PTR *pfptr,
        struct FB_PTR *pbptr,
        struct FB_PTR *nbptr,
        char *fb,
        FILE *fileptr)
{
B_REC *brec;
int size;
int val;
SCRATCH spad;
char tmp[256];
LBUFF   *sptr;

        memset(tmp,0,sizeof(tmp));
        memset(&spad,0,sizeof(spad));
        brec = (B_REC *) &spad;
        sptr = (LBUFF *) &spad.dat[B_LINES];
        size = B_LINES;
        stuff_rec((LBUFF *)brec->fb, B_LINES);
        strcpy(brec->fb,fb);
        while((fgets(tmp,255,fileptr)) != ((char *) 0)) {
                if(test_lvl_kw(tmp)) {
                     fseek(fileptr,(long) -(strlen(tmp)),1);
                     brec->rec_size=size;
                     nbptr->data= (B_REC *) malloc_mem(sizeof(B_REC)+
                                                   ((size-B_LINES)*FIELD_SIZE));
                     memcpy(nbptr->data,&spad,((size-B_LINES)*FIELD_SIZE)+
                                                        sizeof(B_REC));
/* if this is the first FB for the FC set the pointer in the FC structure */
/* otherwise set the forward and backward FB links */
                     if(pfptr->FstFB == FB_NULL)
                        pfptr->FstFB = nbptr;
                     else {
                        pbptr->nextFB = nbptr;
                        nbptr->prevFB = pbptr;
                     }
                     nbptr->FCown = pfptr;
                     break;
                }
                if(strlen(tmp) >= FIELD_SIZE-2)
                        strcpy(&tmp[FIELD_SIZE-2],"\n");
                val=get_kw_val(tmp);
                if(val < PART || val == DSMSG)
                        load_brec(brec, (LBUFF *)tmp, val);
                else {
                        strcpy(sptr->dat, tmp);
                        ++sptr;
                        ++size;
                }
        }
}

/*  */
/*
 * NAME:  proc_prec
 *
 * FUNCTION: read and store the part data and update the links
 *
 * NOTES:
 *      A PN record is expected to be last so this function includes
 *      some extra code to handle the last PN record in the file.
 *
 * RETURNS: NONE
 */

void proc_prec( struct FC_PTR *pfptr,
        struct FB_PTR *pbptr,
        struct PN_PTR *ppptr,
        struct PN_PTR *npptr,
        char *pn,
        FILE *fileptr)
{
P_REC *prec;
int size;
int val;
SCRATCH spad;
char *ttmp;
char tmp[256];
LBUFF   *sptr;

        memset(&spad,0,sizeof(spad));
        memset(tmp,0,sizeof(tmp));
        prec = (P_REC *) &spad;
        sptr = (LBUFF *) &spad.dat[P_LINES];
        size = P_LINES;
        stuff_rec((LBUFF *)prec->pn, P_LINES);
        strcpy(prec->pn,pn);
        while((ttmp=fgets(tmp,255,fileptr)) != ((char *) 0)) {
                if(test_lvl_kw(tmp)) {
                     fseek(fileptr,(long) -(strlen(tmp)),1);
                     prec->rec_size=size;
                     npptr->data= (P_REC *) malloc_mem(sizeof(P_REC)+
                                                   ((size-P_LINES)*FIELD_SIZE));
                     memcpy(npptr->data,&spad,((size-P_LINES)*FIELD_SIZE)+
                                                        sizeof(P_REC));

/* if this is the first PN of the FC set the pointer in the FC structure */
                     if(pfptr->FstPN == PN_NULL)
                        pfptr->FstPN = npptr;

/* if this is the first PN of the FB set the pointer in the FC structure */
                     if(pbptr->FstPN == PN_NULL)
                        pbptr->FstPN = npptr;
/* otherwise set the forward and backward PN links */
                     else {
                        ppptr->nextPN = npptr;
                        npptr->prevPN = ppptr;
                     }
/* establish ownership of the part */
                     npptr->FCown = pfptr;
                     npptr->FBown = pbptr;
                     break;
                }
                if(strlen(tmp) >= FIELD_SIZE-2)
                        strcpy(&tmp[FIELD_SIZE-2],"\n");
                if((val=get_kw_val(tmp)) > PART)
                        load_prec(prec, (LBUFF *)tmp, val);
                else {
                        strcpy(sptr->dat, tmp);
                        ++sptr;
                        ++size;
                }
        }

        if (ttmp == (char *)0) {
             fseek(fileptr,(long) -(strlen(tmp)),1);
             prec->rec_size=size;
             npptr->data= (P_REC *) malloc_mem(sizeof(P_REC)+
                                           ((size-P_LINES)*FIELD_SIZE));
             memcpy(npptr->data,&spad,((size-P_LINES)*FIELD_SIZE)+
                                                sizeof(P_REC));
             if(pfptr->FstPN == PN_NULL)
                pfptr->FstPN = npptr;
             if(pbptr->FstPN == PN_NULL)
                pbptr->FstPN = npptr;
             else {
                ppptr->nextPN = npptr;
                npptr->prevPN = ppptr;
             }
             npptr->FCown = pfptr;
             npptr->FBown = pbptr;
        }
}


/*  */
/*
 * NAME: stuff_rec
 *
 * FUNCTION: copy the string "$$\n" for each line of a given cnt into the
 *              pointer passed to this function.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */

void stuff_rec(LBUFF *ptr, int cnt)
{
int i;
        for(i = 0; i < cnt; i++, ptr++)
                strcpy(ptr->dat, "$$\n");
}


/*  */
/*
 * NAME: load_prec
 *
 * FUNCTION: copy the line of data into the appropriate member of the P_REC
 *              structure
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */

void load_prec(P_REC *pr, LBUFF *buf, int val)
{
        switch(val) {
                case PART+1:    strcpy(pr->pl, buf->dat);
                                break;
                case PART+2:    strcpy(pr->ax, buf->dat);
                                break;
                case PART+3:    strcpy(pr->ec, buf->dat);
                                break;
                case PART+4:    strcpy(pr->sn, buf->dat);
                                break;
                case PART+5:    strcpy(pr->bc, buf->dat);
                                break;
                case PART+6:    strcpy(pr->si, buf->dat);
                                break;
                case PART+7:    strcpy(pr->cd, buf->dat);
                                break;
                case PART+8:    strcpy(pr->lo, buf->dat);
                                break;
                case PART+9:    strcpy(pr->rl, buf->dat);
                                break;
                case PART+10:   strcpy(pr->ll, buf->dat);
                                break;
                case PART+11:   strcpy(pr->dd, buf->dat);
                                break;
                case PART+12:   strcpy(pr->dg, buf->dat);
                                break;
                case PART+13:   strcpy(pr->fn, buf->dat);
                                break;
                case PART+14:   strcpy(pr->dc, buf->dat);
                                break;
                case PART+15:   strcpy(pr->ds, buf->dat);
                                break;
                default:        break;
        }
}

/*  */
/*
 * NAME: load_frec
 *
 * FUNCTION: copy the line of data into the appropriate member of the F_REC
 *              structure
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */

void load_frec( F_REC *fr, LBUFF *buf, int val)
{
        switch(val) {
                case FEATURE+1: strcpy(fr->t1, buf->dat);
                                break;
                case FEATURE+2: strcpy(fr->s1, buf->dat);
                                break;
                case FEATURE+3: strcpy(fr->ms, buf->dat);
                                break;
                case DATE:      strcpy(fr->dc, buf->dat);
                                break;
                case DSMSG:     strcpy(fr->ds, buf->dat);
                                break;
                default:        break;
        }
}

/*  */
/*
 * NAME: load_brec
 *
 * FUNCTION: copy the line of data into the appropriate member of the B_REC
 *              structure
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */

void load_brec(B_REC *br, LBUFF *buf, int val)
{
        switch(val) {
                case FB+1:      strcpy(br->fd, buf->dat);
                                break;
                case DSMSG:     strcpy(br->ds, buf->dat);
                                break;
                default:        break;
        }
}

/*  */
/*
 * NAME: get_kw_val
 *
 * FUNCTION: scan the keywrds array and return a numerical value for the
 *              keyword passed in strg
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 0 if strg is not in the keywrds array
 *          if strg is in the keywrds array the offset into the array + 1
 */
int get_kw_val(char *strg)
{
int tmp=0;

        while((strncmp(keywrds[tmp],"00",2)) != 0) {
                if((strncmp(keywrds[tmp],strg,3)) == 0) {
                        return(tmp+1);
                }
                ++tmp;
        }
        return 0;
}

/*  */
/*
 * NAME: test_kw
 *
 * FUNCTION: test passed strg against passed key for a 3 character match
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: TRUE if strg matches the kewyword in key
 *          FALSE if no match
 */
int test_kw(char const *key, char const *strg)
{
        if (strncmp(key, strg, 3) == 0)
          return TRUE;
        else
          return FALSE;
}

/*  */
/*
 * NAME: test_lvl_kw
 *
 * FUNCTION:  test passed strg for a level keyword such as *SE *FC *FB *PN
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: TRUE if strg begins with "*SE","*FC","*FB" or "*PN"
 *          FALSE if it doesn't
 */
int test_lvl_kw(char const *strg)
{
        if (test_kw("*SE",strg) ||
            test_kw("*FC",strg) ||
            test_kw("*FB",strg) ||
            test_kw("*PN",strg)) return TRUE;
        else                     return FALSE;
}


/*  */
/*
 * NAME: load_mach
 *
 * FUNCTION: copy the data into the appropriate machine record buffer location
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 0 if keyword is a known machine record keyword
 *          1 if unknown keyword
 */

int load_mach(LBUFF *strg,
        MACH_REC *buf,
        LBUFF   *other)
{
char *dest;
int field;
char junk[80];
int rval = 0;

        dest = junk;
        field = get_kw_val(strg->dat);
        switch(field) {
                case 1:  dest = buf->vers;
                         break;
                case 2:  dest = buf->serial;
                         break;
                case 3:  dest = buf->type;
                         break;
                case 4:  dest = buf->name;
                         break;
                case 5:  dest = buf->street;
                         break;
                case 6:  dest = buf->citystate;
                         break;
                case 7:  dest = buf->zip;
                         break;
                case 8:  dest = buf->contact;
                         break;
                case 9:  dest = buf->phone;
                         break;
                case 10: dest = buf->sysno;
                         break;
                case 11: dest = buf->cust;
                         break;
                case 12: dest = buf->adrs;
                         break;
                case 13: dest = buf->pid;
                         break;
                case 14: dest = buf->seq_no;
                         break;
                case DATE : if((strncmp((char*) strg,"*DC BD",6)) == 0)
                                dest = buf->build;
                            else if((strncmp((char*) strg,
                                        "*DC ID",6)) == 0)
                                dest = buf->install;
                        break;

                default:
                         /* The *SL and *SC keywords are obsolete.  If they */
                         /* are encountered, skip it so it does not get     */
                         /* written out to the system or update diskette.   */
                         /* Otherwise, add the unknown keyword to the list  */
                         /* of unknown keywords.                            */
                         if ((!test_kw("*SL",strg->dat)) &&
                             (!test_kw("*SC",strg->dat)))   {
                                 strcpy(other->dat, strg->dat);
                                 rval =1;
                         }
                         break;
        }
        strcpy(dest, strg->dat);
        return(rval);
}

/*  */
/*
 * NAME: parse_data
 *
 * FUNCTION: call the passed function(s) for every FC, FB and PN.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *      the dir variable processes top down if set and bottom up if not set
 *
 * RETURNS: NONE
 */

void parse_data(pd_struct *pd_str)
{
  struct PN_PTR *part;
  struct FB_PTR *fb;
  struct FC_PTR *fc;
  void (*func1)(struct FC_PTR *, char *, char *, char *);
  void (*func2)(struct FB_PTR *, char *, char *, char *);
  void (*func3)(struct PN_PTR *, char *, char *, char *);

        fc = fc_ptr;
        func1 = (void (*)(struct FC_PTR *, char *, char *, char *))pd_str->func1;
        func2 = (void (*)(struct FB_PTR *, char *, char *, char *))pd_str->func2;
        func3 = (void (*)(struct PN_PTR *, char *, char *, char *))pd_str->func3;

        while (TRUE)
          {
          if ((pd_str->func1 != NOFUNC) && pd_str->dir)
            (*func1)(fc, pd_str->str1, pd_str->str2, pd_str->str3);
          fb = fc->FstFB;
          while (TRUE)
            {
            if ((pd_str->func2 != NOFUNC) && pd_str->dir)
              (*func2)(fb, pd_str->str1, pd_str->str2, pd_str->str3);
            part = fb->FstPN;
            while (TRUE)
              {
              if (pd_str->func3 != NOFUNC)
                (*func3)(part, pd_str->str1, pd_str->str2, pd_str->str3);
              if (part->nextPN == PN_NULL)
                break;
              part = part->nextPN;
              }

            if ((pd_str->func2 != NOFUNC) && !pd_str->dir)
              (*func2)(fb, pd_str->str1, pd_str->str2, pd_str->str3);
            if (fb->nextFB == FB_NULL)
              break;
            fb = fb->nextFB;
            }

          if ((pd_str->func1 != NOFUNC) && !pd_str->dir)
            (*func1)(fc, pd_str->str1, pd_str->str2, pd_str->str3);
          if (fc->nextFC == FC_NULL)
            break;
          fc = fc->nextFC;
          }
}

/*  */
/*
 * NAME:  build_loc_table
 *
 * FUNCTION: load an array of pointers with all database entries with a
 *      resource keyword (AX) or from an MES diskette.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */

void build_loc_table(struct PN_PTR *p)
{
int cnt;
        for(cnt=0; cnt < pncnt; cnt++)
                if(loctab[cnt] == PN_NULL)
                        break;
        if(p->data->ax[0] != '$')
                loctab[cnt] = p;
}

/*  */
/*
 * NAME: build_assoc
 *
 * FUNCTION: search vpd finding unique matches between vpd and the database
 *
 * NOTES: this function is a series of loops processing each VPD record.
 *      each loop uses a different keyword for trying to make a match.
 *      when a match is found setup cross link pointers for matches.
 *      Also if a match is made on a part that has been marked as removed
 *      temporarily clear the flag and cancel the remove temp date code.
 *
 * RETURNS: NONE
 */

void build_assoc(void)
{
struct VP_PTR *vrec;
int  loc_match;
char vstrg[FIELD_SIZE];

        vrec=vprec.vpd;
        while(TRUE) {
                if((loc_match=scan_4sn(vrec->data->sn)) != -1) {
                        vrec->p_mate=loctab[loc_match];
                        loctab[loc_match]->v_mate = vrec;
                        if((loctab[loc_match]->data->flags&RMV_TMP)) {
                                strcpy(loctab[loc_match]->data->dc,"$$\n");
                                loctab[loc_match]->data->flags &= ~(RMV_TMP);
                        }
                }
                if(vrec->nextVP == (struct VP_PTR *) 0)
                        break;
                vrec=vrec->nextVP;
        }

        vrec=vprec.vpd;
        while(TRUE) {
                if(vrec->p_mate == PN_NULL) {
                        if((loc_match=scan_4pn(vrec->data->pn)) != -1) {
                                vrec->p_mate=loctab[loc_match];
                                loctab[loc_match]->v_mate = vrec;
                                if((loctab[loc_match]->data->flags&RMV_TMP)) {
                                        strcpy(loctab[loc_match]->data->dc,
                                                                        "$$\n");
                                        loctab[loc_match]->data->flags &=
                                                                ~(RMV_TMP);
                                }
                        }
                }
                if(vrec->nextVP == (struct VP_PTR *) 0)
                        break;
                vrec=vrec->nextVP;
        }

        vrec=vprec.vpd;
        while(TRUE) {
                if(vrec->p_mate == PN_NULL) {
                        if((loc_match=scan_4cd(vrec->data->cd)) != -1) {
                                vrec->p_mate=loctab[loc_match];
                                loctab[loc_match]->v_mate = vrec;
                                if((loctab[loc_match]->data->flags&RMV_TMP)) {
                                        strcpy(loctab[loc_match]->data->dc,
                                                                        "$$\n");
                                        loctab[loc_match]->data->flags &=
                                                                ~(RMV_TMP);
                                }
                        }
                }
                if(vrec->nextVP == (struct VP_PTR *) 0)
                        break;
                vrec=vrec->nextVP;
        }

        vrec=vprec.vpd;
        while(TRUE) {
                if(vrec->p_mate == PN_NULL) {
                        if((loc_match=scan_4ec(vrec->data->ec)) != -1) {
                                vrec->p_mate=loctab[loc_match];
                                loctab[loc_match]->v_mate = vrec;
                                if((loctab[loc_match]->data->flags&RMV_TMP)) {
                                        strcpy(loctab[loc_match]->data->dc,
                                                                        "$$\n");
                                        loctab[loc_match]->data->flags &=
                                                                ~(RMV_TMP);
                                }
                        }
                }
                if(vrec->nextVP == (struct VP_PTR *) 0)
                        break;
                vrec=vrec->nextVP;
        }


        vrec=vprec.vpd;
        while(TRUE) {
                if(vrec->p_mate == PN_NULL) {
                        bld_generic(vrec->data->ax,vstrg);
                        if((loc_match=scan_4ax(vstrg,0)) != -1) {
                                vrec->p_mate=loctab[loc_match];
                                loctab[loc_match]->v_mate = vrec;
                                if((loctab[loc_match]->data->flags&RMV_TMP)) {
                                        strcpy(loctab[loc_match]->data->dc,
                                                                        "$$\n");
                                        loctab[loc_match]->data->flags &=
                                                                ~(RMV_TMP);
                                }
                        }
                }
                if(vrec->nextVP == (struct VP_PTR *) 0)
                        break;
                vrec=vrec->nextVP;
        }
/* rerun the resource check for non unique matches, we're desperate to
   find a mate */
        vrec=vprec.vpd;
        while(TRUE) {
                if(vrec->p_mate == PN_NULL) {
                        bld_generic(vrec->data->ax,vstrg);
                        if((loc_match=scan_4ax(vstrg,1)) != -1) {
                                vrec->p_mate=loctab[loc_match];
                                loctab[loc_match]->v_mate = vrec;
                                if((loctab[loc_match]->data->flags&RMV_TMP)) {
                                        strcpy(loctab[loc_match]->data->dc,
                                                                        "$$\n");
                                        loctab[loc_match]->data->flags &=
                                                                ~(RMV_TMP);
                                }
                        }
                }
                if(vrec->nextVP == (struct VP_PTR *) 0)
                        break;
                vrec=vrec->nextVP;
        }

}


/*  */
/*
 * NAME:  scan_4ax
 *
 * FUNCTION: for each data base item with a part location compare
 *      resource name against passed vpd name
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:     index into array of pointers where match found
 *              -1 for no matches
 */

int scan_4ax(char *vstrg, int uniflg)
{
int cnt;
int matches = 0;
int locval;
char pstrg[FIELD_SIZE];
P_REC *prec;

        for(cnt=0; loctab[cnt] != PN_NULL; ++cnt) {
                if(loctab[cnt]->v_mate != (struct VP_PTR *) 0)
                        continue;

                prec = loctab[cnt]->data;
                bld_generic(prec->ax,pstrg);
                if(!strcmp(vstrg,pstrg)) {
                        ++matches;
                        locval=cnt;
                        if(uniflg) return(locval); /* don't care about others*/
                }
        }
        if(matches == 1) return(locval);
        else return(-1);
}

/*  */
/*
 * NAME:  scan_4sn
 *
 * FUNCTION: for each data base item with a part location compare
 *      serial number against passed vpd name
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:     index into array of pointers (for unqiue matches)
 *              -1 for multiple or no matches
 */

int scan_4sn(char *vstrg)
{
int cnt;
int matches = 0;
int locval;
P_REC *prec;

        for(cnt=0; loctab[cnt] != PN_NULL; ++cnt) {
                if(loctab[cnt]->v_mate != (struct VP_PTR *) 0)
                        continue;
                prec = loctab[cnt]->data;
                if(!strcmp(vstrg,prec->sn)) {
                        ++matches;
                        locval=cnt;
                }
        }
        if(matches == 1) return(locval);
        else return(-1);
}


/*  */
/*
 * NAME:  scan_4pn
 *
 * FUNCTION: for each data base item with a part location compare
 *      part number against passed vpd name
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:     index into array of pointers (for unqiue matches)
 *              -1 for multiple or no matches
 */

int scan_4pn(char *vstrg)
{
int cnt;
int matches = 0;
int locval;
P_REC *prec;

        for(cnt=0; loctab[cnt] != PN_NULL; ++cnt) {
                if(loctab[cnt]->v_mate != (struct VP_PTR *) 0)
                        continue;
                prec = loctab[cnt]->data;
                if(!strcmp(vstrg,prec->pn)) {
                        ++matches;
                        locval=cnt;
                }
        }
        if(matches == 1) return(locval);
        else return(-1);
}


/*  */
/*
 * NAME:  scan_4cd
 *
 * FUNCTION: for each data base item with a part location compare
 *      card descriptor against passed vpd name
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:     index into array of pointers (for unqiue matches)
 *              -1 for multiple or no matches
 */


int scan_4cd(char *vstrg)
{
int cnt;
int matches = 0;
int locval;
P_REC *prec;

        for(cnt=0; loctab[cnt] != PN_NULL; ++cnt) {
                if(loctab[cnt]->v_mate != (struct VP_PTR *) 0)
                        continue;
                prec = loctab[cnt]->data;
                if(!strcmp(vstrg,prec->cd)) {
                        ++matches;
                        locval=cnt;
                }
        }
        if(matches == 1) return(locval);
        else return(-1);
}


/*  */
/*
 * NAME:  scan_4ec
 *
 * FUNCTION: for each data base item with a part location compare
 *      ec level against passed vpd name
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:     index into array of pointers (for unqiue matches)
 *              -1 for multiple or no matches
 */

int scan_4ec(char *vstrg)
{
int cnt;
int matches = 0;
int locval;
P_REC *prec;

        for(cnt=0; loctab[cnt] != PN_NULL; ++cnt) {
                if(loctab[cnt]->v_mate != (struct VP_PTR *) 0)
                        continue;
                prec = loctab[cnt]->data;
                if(!strcmp(vstrg,prec->ec)) {
                        ++matches;
                        locval=cnt;
                }
        }
        if(matches == 1) return(locval);
        else return(-1);
}

/*  */
/*
 * NAME: new_parts
 *
 * FUNCTION:  test for any vpd items not on disk
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: TRUE if new part in vpd
 *          FALSE otherwise
 */

int new_parts(void)
{
struct VP_PTR *vrec;

        vrec=vprec.vpd;
        while(TRUE) {
                if(vrec->p_mate == PN_NULL)
                        return(TRUE);
                if(vrec->nextVP == VP_NULL)
                        break;
                vrec=vrec->nextVP;
        }
        return(FALSE);
}

/*  */
/*
 * NAME: lost_parts;
 *
 * FUNCTION: schedule test for removed parts
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: TRUE if parts have been removed
 *          FALSE otherwise
 */

int lost_parts(void)
{
int flag;
pd_struct pd_str;

        flag = FALSE;
        pd_str.dir = 1;
        pd_str.func1 = NOFUNC;
        pd_str.func2 = NOFUNC;
        pd_str.func3 = (void (*)(void))rmvd_parts;
        pd_str.str1 = (char *)&flag;
        pd_str.str2 = (char *)NULL;
        pd_str.str3 = (char *)NULL;
        parse_data(&pd_str);
        return(flag);
}

/*  */
/*
 * NAME: rmvd_parts
 *
 * FUNCTION: test for parts not in vpd set passed in flag accordingly
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */

int rmvd_parts(struct PN_PTR *pn, int *flg)
{
        if(*pn->data->pl == '*' && pn->v_mate == VP_NULL &&
                                        !(pn->data->flags&RMV_TMP))
                *flg=TRUE;
}


/**/
/*
 * NAME: read_vpd
 *
 * FUNCTION: This unit reads vpd data from the CuVPD object class
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 *              0 - no data found
 *              1 - data read
 *             -1 - error occurred
 */

int read_vpd(void)
{
        V_REC           *vrec;
        struct VP_PTR   *prev;
        struct VP_PTR   *crnt;
        int             count;
        int             num;
        struct CuDv     *cudv;
        struct CuVPD    *cuvpd;
        struct PDiagDev *pddv;
        struct listinfo c_info,v_info,p_info;
        char            criteria[128];

        char            *class,*type,*ptype;
        SCRATCH         spad;

        /* first get all the devices in the CuDv object class */
        cudv = get_CuDv_list(CuDv_CLASS,
         "name like * and chgstatus != 3", &c_info, 30, 2);
        if ( (cudv == (struct CuDv *) -1 ) || ( c_info.num == 0 ) )
                return(0);

        vprec.vpd = (struct VP_PTR *) calloc_mem(c_info.num,sizeof(struct VP_PTR));

        prev=crnt=vprec.vpd;
        vpcnt=0;
        for ( count = 0; count < c_info.num; count++ ) {
                memset(&spad,0,sizeof(spad));
                vrec = (V_REC *)&spad;
                stuff_rec((LBUFF *)vrec->pn, V_LINES);
                vrec->rec_size = V_LINES;
                class=ptype= strtok(cudv[count].PdDvLn_Lvalue,"/");
                while( ptype != '\0') {
                        type = ptype;
                        ptype = strtok('\0',"/");
                }
                if((strlen(cudv[count].location)) == 0)  {
                        sprintf(criteria, "DType = %s",type);

                        pddv = get_PDiagDev_list(PDiagDev_CLASS,criteria,
                                                &p_info,1,1);
                        num = p_info.num;
                        odm_free_list(pddv,&p_info);
                        if(num == 0) continue;
                }

                sprintf(criteria, "name = %s",cudv[count].name);
                cuvpd = get_CuVPD_list(CuVPD_CLASS, criteria, &v_info, 1, 1);
                if (cuvpd == (struct CuVPD *) -1 )
                        return(-1);
                if (strcmp(class,CLASS_SYSUNIT) == 0)
                        load_vpd_mach(cuvpd,v_info.num);

                ++vpcnt;
                NLsprintf(vrec->ax,"*AX %s\n", cudv[count].name);
                if (strlen(cudv[count].location))
                  NLsprintf(vrec->pl,"*PL %s\n",cudv[count].location);
                if (v_info.num > 0)
                  copy_vpd(cuvpd,&spad,v_info.num);
                prev->nextVP = crnt;
                crnt->data = (V_REC *)malloc_mem(sizeof(V_REC) +
                             (vrec->rec_size - V_LINES)*FIELD_SIZE);
                memcpy(crnt->data, &spad, (sizeof(V_REC)+
                        (vrec->rec_size-V_LINES)*FIELD_SIZE));

                /* if the CD value did not come out of vpd then */
                /* get the devid from PdDv for this CuDv device */
                if (((num = dev_ID(cudv[count].PdDvLn->devid)) > 0) &&
                    ((*crnt->data->cd == '\0') ||
                     (*crnt->data->cd ==  '$')))
                  sprintf(crnt->data->cd, "*CD %04x\n", num);

                prev=crnt;
                ++crnt;
                odm_free_list(cuvpd, &v_info);

        }
        odm_free_list(cudv, &c_info);
        return(1);
}

/*  */
/*
 * NAME: dev_ID
 *
 * FUNCTION: Convert a hex string to an integer value.
 *           The string is two bytes long and is byte-reversed.
 *           The hex value represents the device ID of a resource.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 0
 */

int dev_ID(char *hex_str)
{
  char ID_str[7];
  char temp[3];
  int i1;

        /****************************************************************
        * The string containing the device ID is assumed to be a 6      *
        * characters hex value beginning with "0x" of "0X".  It is also *
        * byte swapped.  Calculate the correct ID value and return it.  *
        ****************************************************************/
        i1 = 0;
        if (strlen(hex_str) == 6)
          {
          strncpy(temp, hex_str, 2);
          temp[2] = '\0';
          if (!strcmp(temp, "0x") || !strcmp(temp, "0X"))
            {
            strcpy(ID_str, hex_str);
            strncpy(temp, ID_str + 2, 2);
            strncpy(ID_str + 2, ID_str + 4, 2);
            strncpy(ID_str + 4, temp, 2);
            i1 = (int) strtol(ID_str, (char **)NULL, 16);
            }
          }
        return i1;
}

/*  */
/*
 * NAME: copy_vpd
 *
 * FUNCTION: Construct vpd data for each device
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 0
 */

void copy_vpd(struct CuVPD *vpd,
        SCRATCH *buf,
        int     vnum)
{
        int     cnt;
        int     val;
        V_REC   *vrec;
        LBUFF   *lptr;
        VPDBUF  vbuf;

        vrec = (V_REC *) buf;
        lptr = &buf->dat[V_LINES];
        while (vnum) {
                if (parse_vpd(vpd->vpd,&vbuf,1) == -1) {
                        disp_popup_message(MEM_ERR);
                        exit(202);
                }

                for (cnt=0; cnt < vbuf.entries; cnt++) {
                        val=vrec->rec_size;
                        proc_vrec(lptr,vbuf.vdat[cnt],vrec);
                        if (val != vrec->rec_size) ++lptr;
                }
                --vnum;
                ++vpd;
        }
        free_vbuf(&vbuf);
}

/*  */
/*
 * NAME: proc_vrec
 *
 * FUNCTION: store data in structure
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */

void proc_vrec(LBUFF *lptr,
        char *dat,
        V_REC *vrec)
{
int val;

        while(dat[strlen(dat)-1] == ' ')
                dat[strlen(dat)-1] = '\0';

        strcat(dat, "\n");
        if((val = get_kw_val(dat)) >= PART)
                load_vrec(vrec, (LBUFF *)dat, val);
        else {
                if(strlen(dat) < FIELD_SIZE-2)
                        strcpy(lptr->dat, dat);
                else {
                        strncpy(lptr->dat, dat, FIELD_SIZE-2);
                        strcpy(&lptr->dat[FIELD_SIZE-2], "\n");
                }
                vrec->rec_size++;
                if (test_kw("*PI",lptr->dat))
                        strcpy(vprec.mr.pid, lptr->dat);
        }
}

/*  */
/*
 * NAME: load_vrec
 *
 * FUNCTION: copy the line of data into the appropriate member of the V_REC
 *              structure
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */

void load_vrec(V_REC *vr,
        LBUFF *buf,
        int val)
{
        if(strlen((char *) buf) >= FIELD_SIZE-2)
                strcpy(&buf->dat[FIELD_SIZE-2],"\n");
        switch(val) {
                case PART:      strcpy(vr->pn, buf->dat);
                                break;
                case PART+1:    strcpy(vr->pl, buf->dat);
                                break;
                case PART+2:    strcpy(vr->ax, buf->dat);
                                break;
                case PART+3:    strcpy(vr->ec, buf->dat);
                                break;
                case PART+4:    strcpy(vr->sn, buf->dat);
                                break;
                case PART+5:    strcpy(vr->bc, buf->dat);
                                break;
                case PART+7:    strcpy(vr->cd, buf->dat);
                                break;
                case PART+8:    strcpy(vr->lo, buf->dat);
                                break;
                case PART+9:    strcpy(vr->rl, buf->dat);
                                break;
                case PART+10:   strcpy(vr->ll, buf->dat);
                                break;
                case PART+11:   strcpy(vr->dd, buf->dat);
                                break;
                case PART+12:   strcpy(vr->dg, buf->dat);
                                break;
                case PART+13:   strcpy(vr->fn, buf->dat);
                                break;
                case PART+15:   strcpy(vr->ds, buf->dat);
                                break;
                default:        break;
        }
}

/*  */
/*
 * NAME: load_vpd_mach
 *
 * FUNCTION:  this function finds the TM and SN (SE on DISK) entries for
 *              vpd object sysunit and copies the data into vpd machine
 *              record fields.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */

void load_vpd_mach(struct CuVPD *vpd, int vnum)
{
int len;
char *vpdptr;
        if(vnum == 0) {
                /* copy machine rec type and sn to vrec */
                if ( strlen (machine.type))
                        strcpy(vprec.mr.type, machine.type);
                if ( strlen (machine.serial))
                        strcpy(vprec.mr.serial, machine.serial);
                return;
        }

        while(vnum) {
                --vnum;
                vpdptr = vpd->vpd;
                ++vpd;
                while(vpdptr = (char *)strchr(vpdptr, '*' ))  {
                        if ( (strspn (vpdptr, "*TM")) == 3 ) {
                                len=(*(vpdptr+3)*2)-4;
                                sprintf(vprec.mr.type,"*TM ");
                                if(len>0) strncat(vprec.mr.type,(vpdptr+4),len);
                                while(vprec.mr.type[strlen(vprec.mr.type)-1]
                                                                        == ' ')
                                         vprec.mr.type[strlen(vprec.mr.type)-1]
                                                                 = '\0';
                                strcat(vprec.mr.type,"\n");
                        }
                        else if ( ( strspn (vpdptr, "*SN")) == 3 ) {
                                len=(*(vpdptr+3)*2)-4;
                                sprintf(vprec.mr.serial,"*SE ");
                                if( len > 0 )
                                        strncat(vprec.mr.serial,(vpdptr+4),len);
                                while(vprec.mr.serial[
                                      strlen(vprec.mr.serial)-1] == ' ')
                                          vprec.mr.serial[
                                           strlen(vprec.mr.serial)-1] = '\0';

                                strcat(vprec.mr.serial,"\n");
                        }
                        vpdptr++;
                }
        }
}

/**/
/*
 * NAME: update_sysunit_vpd
 *
 * FUNCTION: This unit updates the vpd data for the sysunit
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 *              0 - no data found
 *             -1 - error occurred
 */

int update_sysunit_vpd(void)
{
        int             rc;
        int             found_tm, found_sn;
        int             new_vpd_entry = FALSE;
        struct CuVPD    *cuvpd;
        struct listinfo c_info;
        char            *dname;
        char            *vpdptr;
        char            model_type[FIELD_SIZE+1];
        char            serial_num[FIELD_SIZE+1];
        char            criteria[128];
        char            vpdbuf[VPDSIZE+1];

        dname = "sysunit0";

        /* search CuVPD for any user entered vpd data */
        sprintf(criteria, "name = %s and vpd_type = %d", dname, USER_VPD);
        cuvpd = get_CuVPD_list(CuVPD_CLASS, criteria, &c_info, 1, 1);
        if ( cuvpd == (struct CuVPD *) -1 )
                return(-1);

        /* if no entries - create one */
        if ( c_info.num == 0 ) {
                new_vpd_entry = TRUE;
                cuvpd = (struct CuVPD *) calloc_mem(1, sizeof(struct CuVPD));
                strncpy(cuvpd->name, dname, NAMESIZE);
                cuvpd->vpd_type = USER_VPD;
                cuvpd->vpd[0] = '\0';
        }

        strncpy(model_type, machine.type, FIELD_SIZE);
        strcat(model_type,"\n");
        strncpy(serial_num, machine.serial, FIELD_SIZE);
        strcat(serial_num,"\n");
        strncpy(vpdbuf, cuvpd->vpd, VPDSIZE);
        vpdbuf[strlen(vpdbuf)] = '\0';
        found_tm = found_sn = 0;
        vpdptr = vpdbuf;
        while(vpdptr = (char *)strchr(vpdptr, '*' ))
                if ( (found_tm = strspn (vpdptr, "*TM")) == 3 )
                        break;
                else
                        vpdptr++;

        if ( found_tm == 3 )
                update_vpd_entry("*TM", model_type, vpdbuf);
        else
                add_vpd_entry("*TM", model_type, vpdbuf);

        vpdptr = vpdbuf;
        while(vpdptr = (char *)strchr(vpdptr, '*' ))
                if ( (found_sn = strspn (vpdptr, "*SN")) == 3 )
                        break;
                else
                        vpdptr++;

        if ( found_sn == 3 )
                update_vpd_entry("*SN", serial_num, vpdbuf);
        else
                add_vpd_entry("*SN", serial_num, vpdbuf);

        strncpy(cuvpd->vpd, vpdbuf,VPDSIZE);
        if (new_vpd_entry) {
                rc = odm_add_obj(CuVPD_CLASS, cuvpd);
                free ( cuvpd );
        }
        else {
                rc = odm_change_obj(CuVPD_CLASS, cuvpd);
                odm_free_list ( cuvpd, &c_info );
        }

        return(rc);
}

/*  */
/*
 * NAME: add_vpd_entry
 *
 * FUNCTION: this function adds a new entry to the end of the vpd list
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */

int add_vpd_entry(char *keyword,
                char *data,
                char *vpdptr)
{
        int     length;
        char    *newptr;
        char    *nlptr;
        newptr = vpdptr + strlen(vpdptr);

        /* remove ending newline */
        nlptr = (char *) strnchr(data, '\n',FIELD_SIZE);
        if(nlptr != '\0') *nlptr = '\0';

        length = strlen(data) / 2;
        if ( strlen(data)%2 ) {
                NLsprintf(newptr,"%s %s ", keyword, data+4);
                ++length;
        }
        else
                NLsprintf(newptr,"%s %s", keyword, data+4);
        newptr[3] = length;
        return(0);
}


/*  */
/*
 * NAME: update_vpd_entry
 *
 * FUNCTION: this function updates the current value for an existing item
 *              in vpd.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */

int update_vpd_entry(char *keyword,
        char    *data,
        char    *vpdptr)
{
        int     length;
        char    *savptr;
        char    tmpbuff[VPDSIZE], *tmpptr;
        char    *nlptr;
        tmpptr = tmpbuff;
        savptr = vpdptr;

        /* copy all vpd data over except for the one that changed */
        while(savptr = (char *)strchr(savptr, '*' )) {
                if ( strspn(savptr, keyword) != 3 ) {
                        NLstrncpy(tmpptr, savptr, savptr[3]*2 );
                        tmpptr = tmpptr + savptr[3]*2;
                }
                savptr++;
        }

        nlptr = (char *) strnchr(data, '\n',FIELD_SIZE);
        if(nlptr != '\0') *nlptr = '\0';

        length = strlen(data) / 2;
        if ( strlen(data)%2 ) {
                NLsprintf(tmpptr,"%s %s ", keyword, data+4);
                ++length;
        }
        else
                NLsprintf(tmpptr,"%s %s", keyword, data+4);
        tmpptr[3] = length;

        NLstrcpy(vpdptr,tmpbuff);
        return(0);
}

/*  */
/*
 * NAME: scan file
 *
 * FUNCTION: this function scans the passed file name and counts level
 *              keywords FC FB PN , if flg is set it also allocates
 *              memory
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:  0 if file error
 *           1 otherwise
 */

int scan_file(char *fname,
        ITEM_CNT *cnt)
{
FILE *pnew;
char tmp[256];

        pnew = (FILE *) fopen(fname,"r");
        if(pnew == (FILE *) 0) return 0;
        while((fgets(tmp,255,pnew)) != ((char *) 0)) {
                if(test_kw("*FC",tmp)) ++cnt->fcs;
                else if(test_kw("*FB",tmp)) ++cnt->fbs;
                else if(test_kw("*PN",tmp)) ++cnt->parts;
        }

        /* Can't calloc() 0 bytes.  Return an error code */
        if((cnt->fcs == 0) || (cnt->fbs == 0) || (cnt->parts == 0)) {
                close(pnew);
                return 0;
        }

        pn_ptr=(struct PN_PTR *) calloc_mem(cnt->parts,sizeof(struct PN_PTR));
        fb_ptr=(struct FB_PTR *) calloc_mem(cnt->fbs,sizeof(struct FB_PTR));
        fc_ptr=(struct FC_PTR *) calloc_mem(cnt->fcs,sizeof(struct FC_PTR));
        close(pnew);
        return 1;
}

/*  */
/*
 * NAME:  build_newrec
 *
 * FUNCTION: setup structures and links for new vpd items
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: pointer to calloc'd part structure
 */

struct PN_PTR *build_newrec(struct VP_PTR *vptr, LBUFF *buf )
{
struct FC_PTR *fptr,*tfptr;
struct FB_PTR *bptr;
struct PN_PTR *pptr;

        pptr = (struct PN_PTR *) calloc_mem(1,sizeof(struct PN_PTR));
        bptr = (struct FB_PTR *) calloc_mem(1,sizeof(struct FB_PTR));
        fptr = (struct FC_PTR *) calloc_mem(1,sizeof(struct FC_PTR));

        ++fccnt;
        ++fbcnt;
        ++pncnt;

        tfptr = last_fc();
        tfptr->nextFC = fptr;
        fptr->prevFC = tfptr;

        fptr->data = (F_REC *) malloc_mem(sizeof(F_REC));
        memset(fptr->data,0,sizeof(F_REC));
        strcpy(fptr->data->fc, "*FC ????????\n");
        fptr->data->rec_size=F_LINES;

        bptr->data = (B_REC *) malloc_mem(sizeof(B_REC));
        memset(bptr->data,0,sizeof(B_REC));
        strcpy(bptr->data->fb, "*FB @@@@@@@@\n");
        bptr->data->rec_size=B_LINES;

        pptr->data = vptr->data;
        pptr->v_mate = vptr;
        vptr->p_mate = pptr;
        pptr->FCown = fptr;
        pptr->FBown = bptr;
        bptr->FCown = fptr;
        bptr->FstPN = pptr;
        fptr->FstFB = bptr;
        fptr->FstPN = pptr;

        if(*buf->dat != '\0')
                strcpy(vptr->data->pn, buf->dat);
        else
                strcpy(vptr->data->pn, "*PN 00000000\n");
        ++buf;

        if(*buf->dat != '\0')
                strcpy(vptr->data->pl, buf->dat);
        ++buf;

        if(*buf->dat != '\0')
                strcpy(vptr->data->ax, buf->dat);
        ++buf;

        if(*buf->dat != '\0')
                strcpy(vptr->data->ec, buf->dat);
        ++buf;

        if(*buf->dat != '\0')
                strcpy(vptr->data->sn, buf->dat);
        ++buf;

        if(*buf->dat != '\0')
                strcpy(vptr->data->lo, buf->dat);
        ++buf;

        if(*buf->dat != '\0')
                strcpy(vptr->data->ds, buf->dat);

        return(pptr);

}
/*  */
/*
 * NAME: last_fc
 *
 * FUNCTION: traverse fc links to end of list
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: pointer to last fc structure
 */

struct FC_PTR *last_fc(void)
{
struct FC_PTR *fc;

        fc=fc_ptr;

        while(fc->nextFC != FC_NULL)
                fc=fc->nextFC;

        return(fc);
}
