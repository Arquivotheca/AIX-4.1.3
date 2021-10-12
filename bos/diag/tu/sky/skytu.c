static char sccsid[] = "@(#)16	1.3  src/bos/diag/tu/sky/skytu.c, tu_sky, bos411, 9428A410j 1/21/94 11:32:50";
/*
 *   COMPONENT_NAME : (tu_sky) Color Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: INT_TO_ARRAY
 *		skytu
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/termio.h>
#include <sys/rcm_win.h> /*must go before include of aixgsc.h*/
#include <sys/aixgsc.h>
#include <sys/rcmioctl.h>
#include <sys/errno.h>
#include <entdisp.h>

extern   errno;

int      rcm_fdes;
gsc_handle  our_gsc_handle;
/* struct our_gsc_handle{
 *        char devname[20];
 *        long handle;  }
 */ 
make_gp  makemap;

#include "skytu.h"

#define ERROR      -1

#define INT_TO_ARRAY(a,i)   *((char *) (a))     = (i) >> 24;\
	 		    *((char *) (a) + 1) = (i) >> 16;\
		 	    *((char *) (a) + 2) = (i) >>  8;\
	                    *((char *) (a) + 3) = (i);

lword segreg = -1;  /*segment register nibble */
struct sky_map skydat;
extern struct cntlparm cptr;


skytu(tu_name, skyptr)
word tu_name[];
struct tu_info *skyptr;
{
  short int  rc;
  lword loops;
  char  *ldn;

  strcpy(tinfo.info.err_buff,"Error Trace");
  strcpy(LASTERR,"Error Trace");
 
  if (skyptr == NULL)
  {
    skydflts();
    if (tu_name == GET_DEFAULT_PTRX)
    {
     return((lword)(&tinfo));
    }
  }
  else
  { if (tu_name == GET_DEFAULT_PTRX)
    { sprintf(LASTMSG, "\nThe combination of setting a structure pointer and  requesting GET_DEFAULT_PTR is Illegal");
      sprintf(LASTERR, ":%s%X",LASTERR, (SKYTU | CPF_INVDAT));
      return(SKYTU | CPF_INVDAT);
    } /* endif */
  }

  /* --------------------------------------------------------------------*/
  /* -----  START OF TU_OPEN --------------------------------------------*/
  /* --------------------------------------------------------------------*/

  ldn=tinfo.skyway_ldn;
  if (segreg == -1)     /* ck if segreg initialized */
  {
     /*check if TU_OPEN (90ff) is being called first */
     if (tu_name != TU_OPEN)
     { sprintf(tinfo.info.msg_buff,"TU_OPEN NOT CALLED FIRST ERROR");
       CatErr(SKYTU | LFTNOTOPEN);
       return(SKYTU | LFTNOTOPEN);
     } /* end if tu_name */
 
    /* open rcm  */
    if ((rcm_fdes = open("/dev/rcm0",O_RDWR)) < 0 ) 
     { sprintf(tinfo.info.msg_buff,"/dev/rcm open ERROR");
       CatErr(SKYTU | LFTRCMOPEN);
       return(SKYTU | LFTRCMOPEN);
     } /* end if rcm_fdes */

    tinfo.skyway_rcm_fdes = rcm_fdes;   
    strcpy(our_gsc_handle.devname,ldn);

    /* get handle */
    rc = ioctl(rcm_fdes,GSC_HANDLE, &our_gsc_handle);
    if (rc)  /* GSC_HANDLE return code */
    {
       if (errno == EBUSY)
       { sprintf(tinfo.info.msg_buff,"display device %s is busy",ldn);
         CatErr(SKYTU | LFTDEVBUSY);
         return(SKYTU | LFTDEVBUSY);
        } /* end if errno */
        else {
         sprintf(tinfo.info.msg_buff,"display device %s error",ldn);
         CatErr(SKYTU | SYS_FAILUR);
         return(SKYTU | SYS_FAILUR);
        } /* end if errno */
    } /* end if rc GSC_HANDLE */

        makemap.pData = (genericPtr) &skydat;
        makemap.length = sizeof(skydat);
        if (aixgsc(our_gsc_handle.handle, MAKE_GP, &makemap))
        { sprintf(tinfo.info.msg_buff,"MAKE_GP  aixgsc Failure in skytu()");
          CatErr(SKYTU | LFTMAKEGP);
          return(SKYTU | LFTMAKEGP);
        }
        segreg = (ulong) makemap.segment;
        skydflts();
        return(SKYTU | GOOD_OP);
   } /* end if segreg */
  /* --------------------------------------------------------------------*/
  /* -----  END OF TU_OPEN ----------------------------------------------*/
  /* --------------------------------------------------------------------*/
  /* --------------------------------------------------------------------*/
  /* ----  START OF TU_CLOSE---------------------------------------------*/
  /* --------------------------------------------------------------------*/
     if (tu_name == TU_CLOSE)
     {
      if (aixgsc(our_gsc_handle.handle, UNMAKE_GP, &makemap))
      { sprintf(tinfo.info.msg_buff,"UNMAKE_GP  aixgsc Failure in TU_CLOSE()");
        CatErr(SKYTU | LFTMAKEGP);
        return(SKYTU | LFTMAKEGP);
      }
       close(tinfo.skyway_rcm_fdes);
       segreg = -1;
       return(SKYTU | GOOD_OP);
     }
  /* --------------------------------------------------------------------*/
  /* ----  END OF TU_CLOSE---------------------------------------------*/
  /* --------------------------------------------------------------------*/

  /* Run the TU */
  strcpy(LASTMSG,"");
  strcpy(LASTERR,"");
  strcpy(tinfo.info.msg_buff,"");
  strcpy(tinfo.info.err_buff,"");
  for (loops=0;loops<cptr.max_count; loops++)
  { 
       rc = run_tu(tu_name);   

     /* NOW TEST THE RESULTS OF THE TU */
     if (((word)rc) > ERR_LEVEL)
     { sprintf(LASTMSG, "%s \n%s%x\n%s",
         LASTMSG, "===>On Loop 0x", loops+1, tinfo.info.msg_buff);
       sprintf(LASTERR, "%s \n%s%x\n%s",
         LASTERR, "===>On Loop 0x", loops+1, tinfo.info.err_buff);
         cptr.err_count++; 
       if (cptr.break_on_err) {loops++; break;}
     } /* if rc > err_level) */
   } /* for count to loops */
   if (((word)rc) <= ERR_LEVEL)
   { sprintf(LASTMSG, "%s", tinfo.info.msg_buff);
     sprintf(LASTERR, "%s", tinfo.info.err_buff);
   } /* if rc <= err_level) */
   if (cptr.max_count > 1)
   { sprintf(LASTMSG, "%s %s%x %s%x %s%x %s%x%s",
 	LASTMSG,
 	"\n===>Finished 0x",loops,
 	"of 0x",cptr.max_count,
 	"iterations of ",tu_name,
 	"with 0x", cptr.err_count," errors");
   }
   if (cptr.err_count > 0) rc = 0xA0F0;
   strcpy(tinfo.info.err_buff, LASTERR);
   strcpy(tinfo.info.msg_buff, LASTMSG);

  return(SKYTU | rc);
}

