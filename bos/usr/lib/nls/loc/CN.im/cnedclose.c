static char sccsid[] = "@(#)06	1.1  src/bos/usr/lib/nls/loc/CN.im/cnedclose.c, ils-zh_CN, bos41B, 9504A 12/19/94 14:33:23";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: CNedClose
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATION ***************************/
/*                                                                            */
/* MODULE NAME:         CNedClose                                             */
/*                                                                            */
/* DESCRIPTIVE NAME:    Chinese Input Method Editor Close                     */
/*                                                                            */
/* FUNCTION:            CNedClose : Free Internal Used Data Structure.        */
/*                                                                            */
/* MODULE TYPE:         C                                                     */
/*                                                                            */
/* COMPILER:            AIX C                                                 */
/*                                                                            */
/* AUTHOR:              Tang Bosong, WuJian                                   */
/*                                                                            */
/* STATUS:              Chinese Input Method Version 1.0                      */
/*                                                                            */
/* CHANGE ACTIVITY:                                                           */
/*                                                                            */
/*                                                                            */
/************************ END OF SPECIFICATION ********************************/
/*----------------------------------------------------------------------------*/
/*                      include files                                         */
/*----------------------------------------------------------------------------*/
  #include "cned.h"
  #include "cnedinit.h"
  #include "cnedud.h"
/*----------------------------------------------------------------------------*/
/*                      external reference                                    */
/*----------------------------------------------------------------------------*/
  extern  RcFreeCandidates();
  extern  PyFreeCandidates();
  extern  PyLFreeCandidates();
  extern  EnFreeCandidates();
  extern  FssFreeCandidates();
  extern  FsFreeCandidates();
  extern  AbcFreeCandidates();
  extern  UdFreeCandidates();
/*----------------------------------------------------------------------------*/
/*                      Beginning of procedure                                */
/*----------------------------------------------------------------------------*/

int      CNedClose(fepcb)
FEPCB   *fepcb ;
{
         free(fepcb->rcinbuf);    /* free internal structure pointer space */
         free(fepcb->pyinbuf);
         free(fepcb->eninbuf);
         free(fepcb->fssinbuf);
         free(fepcb->fsinbuf);
         free(fepcb->abcinbuf);
         free(fepcb->udinbuf);
         CNedFreeDictionaryName(fepcb);
         if(fepcb->mi.pysysmi != NULL) free(fepcb->mi.pysysmi);
         if(fepcb->mi.pyusrmi != NULL) free(fepcb->mi.pyusrmi);
         if(fepcb->mi.lesysmi != NULL) free(fepcb->mi.lesysmi);
         if(fepcb->mi.leusrmi != NULL) free(fepcb->mi.leusrmi);
         if(fepcb->mi.ensysmi != NULL) free(fepcb->mi.ensysmi);
         if(fepcb->mi.enusrmi != NULL) free(fepcb->mi.enusrmi);
         if(fepcb->mi.fsssysmi != NULL) free(fepcb->mi.fsssysmi);
         if(fepcb->mi.fssjmsysmi != NULL) free(fepcb->mi.fssjmsysmi);
         if(fepcb->mi.fssusrmi != NULL) free(fepcb->mi.fssusrmi);
         if(fepcb->mi.fssysmi != NULL) free(fepcb->mi.fssysmi);
         if(fepcb->mi.fsusrmi != NULL) free(fepcb->mi.fsusrmi);
         if(fepcb->mi.fsphsysmi != NULL) free(fepcb->mi.fsphsysmi);
         if(fepcb->mi.fsphusrmi != NULL) free(fepcb->mi.fsphusrmi);
         if(fepcb->mi.abcsysmi != NULL) free(fepcb->mi.abcsysmi);
         if(fepcb->mi.abcusrmi != NULL) free(fepcb->mi.abcusrmi);
         if(fepcb->fd.pysysfd != NULL) fclose(fepcb->fd.pysysfd);
         if(fepcb->fd.lesysfd != NULL) fclose(fepcb->fd.lesysfd);
         if(fepcb->fd.ensysfd != NULL) fclose(fepcb->fd.ensysfd);
         if(fepcb->fd.fsssysfd != NULL) fclose(fepcb->fd.fsssysfd);
         if(fepcb->fd.fssjmsysfd != NULL) fclose(fepcb->fd.fssjmsysfd);
         if(fepcb->fd.fssysfd != NULL) fclose(fepcb->fd.fssysfd);
         if(fepcb->fd.fsphsysfd != NULL) fclose(fepcb->fd.fsphsysfd);
         if(fepcb->fd.abcsysfd[ABCCWD] != NULL) fclose(fepcb->fd.abcsysfd[ABCCWD]);
         if(fepcb->fd.abcsysfd[ABCOVL] != NULL) fclose(fepcb->fd.abcsysfd[ABCOVL]);
         if(fepcb->fd.pyusrfd != NULL) fclose(fepcb->fd.pyusrfd);
         if(fepcb->fd.leusrfd != NULL) fclose(fepcb->fd.leusrfd);
         if(fepcb->fd.enusrfd != NULL) fclose(fepcb->fd.enusrfd);
         if(fepcb->fd.fssusrfd != NULL) fclose(fepcb->fd.fssusrfd);
         if(fepcb->fd.fsusrfd != NULL) fclose(fepcb->fd.fsusrfd);
         if(fepcb->fd.fsphusrfd != NULL) fclose(fepcb->fd.fsphusrfd);
         if(fepcb->fd.abcusrfd[USRREM] != NULL) fclose(fepcb->fd.abcusrfd[USRREM]);
         if(fepcb->fd.abcusrfd[USR] != NULL) fclose(fepcb->fd.abcusrfd[USR]);

/**************************************************************************/
/* The User defined Input Method Module don't provide temporarily         */
/**************************************************************************/
         if(udimcomm->ret == UD_FOUND)
            (*udimcomm->UserDefinedClose)();

/* Free Candidates structure, and free control block    */
         if(fepcb->pystruct.cand != NULL) free(fepcb->pystruct.cand);
         if(fepcb->pystruct.curptr != NULL) free(fepcb->pystruct.curptr);

         if(fepcb->lestruct.cand != NULL) free(fepcb->lestruct.cand);
         if(fepcb->lestruct.curptr!= NULL) free(fepcb->lestruct.curptr);

         if(fepcb->enstruct.cand != NULL) free(fepcb->enstruct.cand);
         if(fepcb->enstruct.curptr != NULL) free(fepcb->enstruct.curptr);

         if(fepcb->fssstruct.cand != NULL) free(fepcb->fssstruct.cand);
         if(fepcb->fssstruct.curptr != NULL) free(fepcb->fssstruct.curptr);

         if(fepcb->fsstruct.cand != NULL) free(fepcb->fsstruct.cand);
         if(fepcb->fsstruct.curptr != NULL) free(fepcb->fsstruct.curptr);

         if(fepcb->abcstruct.cand != NULL) free(fepcb->abcstruct.cand);
         if(fepcb->abcstruct.curptr!= NULL) free(fepcb->abcstruct.curptr);

/**************************************************************************/
/* The User defined Input Method Module don't provide temporarily         */
/**************************************************************************/
         if(fepcb->udstruct.cand != NULL) free(fepcb->udstruct.cand);
         if(fepcb->udstruct.curptr != NULL) free(fepcb->udstruct.curptr);

         free(fepcb);              /* free control block                  */

         return( OK );
}

