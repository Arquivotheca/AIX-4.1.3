static char sccsid[] = "@(#)55	1.1  src/bos/usr/lib/nls/loc/ZH.im/zhedclose.c, ils-zh_CN, bos41B, 9504A 12/19/94 14:38:41";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: ZHedClose
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
/* MODULE NAME:         ZHedClose                                             */
/*                                                                            */
/* DESCRIPTIVE NAME:    Chinese Input Method Editor Close                     */
/*                                                                            */
/* FUNCTION:            ZHedClose : Free Internal Used Data Structure.        */
/*                                                                            */
/************************ END OF SPECIFICATION ********************************/
/*----------------------------------------------------------------------------*/
/*                      include files                                         */
/*----------------------------------------------------------------------------*/
  #include "zhed.h"
  #include "zhedinit.h"
  #include "zhedud.h"

/* extern Ud_Switch     ud_switch;    */

/*----------------------------------------------------------------------------*/
/*                      external reference                                    */
/*----------------------------------------------------------------------------*/
  extern  RcFreeCandidates();
  extern  PyFreeCandidates();
  extern  TjFreeCandidates();
  extern  PyLFreeCandidates();
  extern  EnFreeCandidates();
  extern  AbcFreeCandidates();
  extern  UdFreeCandidates();
/*----------------------------------------------------------------------------*/
/*                      Beginning of procedure                                */
/*----------------------------------------------------------------------------*/

int      ZHedClose(fepcb)
FEPCB   *fepcb ;
{
         free(fepcb->rcinbuf);    /* free internal structure pointer space */
         free(fepcb->pyinbuf);
         free(fepcb->tjinbuf);
         free(fepcb->eninbuf);
         free(fepcb->abcinbuf);
         free(fepcb->udinbuf);
         ZHedFreeDictionaryName(fepcb);
         if(fepcb->mi.pysysmi[COMM] != NULL) free(fepcb->mi.pysysmi[COMM]);
         if(fepcb->mi.pysysmi[GB] != NULL) free(fepcb->mi.pysysmi[GB]);
         if(fepcb->mi.pysysmi[CNS] != NULL) free(fepcb->mi.pysysmi[CNS]);
         if(fepcb->mi.pysysmi[JK] != NULL) free(fepcb->mi.pysysmi[JK]);
         if(fepcb->mi.pyusrmi != NULL) free(fepcb->mi.pyusrmi);
         if(fepcb->mi.tjsysmi != NULL) free(fepcb->mi.tjsysmi);
         if(fepcb->mi.tjusrmi != NULL) free(fepcb->mi.tjusrmi);
         if(fepcb->mi.lesysmi != NULL) free(fepcb->mi.lesysmi);
         if(fepcb->mi.leusrmi != NULL) free(fepcb->mi.leusrmi);
         if(fepcb->mi.ensysmi != NULL) free(fepcb->mi.ensysmi);
         if(fepcb->mi.enusrmi != NULL) free(fepcb->mi.enusrmi);
         if(fepcb->mi.abcsysmi != NULL) free(fepcb->mi.abcsysmi);
         if(fepcb->mi.abcusrmi != NULL) free(fepcb->mi.abcusrmi);
         if(fepcb->fd.pysysfd[COMM] != NULL) fclose(fepcb->fd.pysysfd[COMM]);
         if(fepcb->fd.pysysfd[GB] != NULL) fclose(fepcb->fd.pysysfd[GB]);
         if(fepcb->fd.pysysfd[CNS] != NULL) fclose(fepcb->fd.pysysfd[CNS]);
         if(fepcb->fd.pysysfd[JK] != NULL) fclose(fepcb->fd.pysysfd[JK]);
         if(fepcb->fd.tjsysfd != NULL) fclose(fepcb->fd.tjsysfd);
         if(fepcb->fd.lesysfd != NULL) fclose(fepcb->fd.lesysfd);
         if(fepcb->fd.ensysfd != NULL) fclose(fepcb->fd.ensysfd);
         if(fepcb->fd.abcsysfd[ABCCWD_S] != NULL) fclose(fepcb->fd.abcsysfd[ABCCWD_S]);
         if(fepcb->fd.abcsysfd[ABCCWD_T] != NULL) fclose(fepcb->fd.abcsysfd[ABCCWD_T]);
         if(fepcb->fd.abcsysfd[ABCOVL] != NULL) fclose(fepcb->fd.abcsysfd[ABCOVL]);
         if(fepcb->fd.pyusrfd != NULL) fclose(fepcb->fd.pyusrfd);
         if(fepcb->fd.tjusrfd != NULL) fclose(fepcb->fd.tjusrfd);
         if(fepcb->fd.leusrfd != NULL) fclose(fepcb->fd.leusrfd);
         if(fepcb->fd.enusrfd != NULL) fclose(fepcb->fd.enusrfd);
         if(fepcb->fd.abcusrfd[USRREM] != -1) close(fepcb->fd.abcusrfd[USRREM]);
         if(fepcb->fd.abcusrfd[USR] != -1) close(fepcb->fd.abcusrfd[USR]);

/**************************************************************************/
/* The User defined Input Method Module don't provide temporarily         */
/**************************************************************************/
         if(udimcomm->ret == UD_FOUND)
           (*udimcomm->UserDefinedClose)();

/* Free Candidates structure, and free control block    */
         if(fepcb->pystruct.cand != NULL) free(fepcb->pystruct.cand);
         if(fepcb->pystruct.curptr != NULL) free(fepcb->pystruct.curptr);

         if(fepcb->tjstruct.cand != NULL) free(fepcb->tjstruct.cand);
         if(fepcb->tjstruct.curptr != NULL) free(fepcb->tjstruct.curptr);

         if(fepcb->lestruct.cand != NULL) free(fepcb->lestruct.cand);
         if(fepcb->lestruct.curptr!= NULL) free(fepcb->lestruct.curptr);

         if(fepcb->enstruct.cand != NULL) free(fepcb->enstruct.cand);
         if(fepcb->enstruct.curptr != NULL) free(fepcb->enstruct.curptr);

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

