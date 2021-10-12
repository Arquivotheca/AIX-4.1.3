static char sccsid[] = "@(#)42	1.17  src/bos/usr/bin/chhwkbd/chhwkbd.c, cmdinputdd, bos411, 9434B411a 8/25/94 05:11:54";
/*
 * COMPONENT_NAME: (CMDINPUTDD) input device commands
 *
 * FUNCTIONS:  main, update_attr, parse, fmt, bye
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <nl_types.h>
#include <sys/cfgodm.h>
#include <sys/lft_ioctl.h>
#include <sys/inputdd.h>
#include <errno.h>
#include <locale.h>
#include <string.h>
#include <fcntl.h>
#include "chhwkbd_msg.h"
#include "chhwkbd_msgdft.h"

#define PGM            "chhwkbd"       /* name of program                    */
#define CATNAME        "chhwkbd.cat"   /* name of message catalog            */

#define BAD_RC         -1              /* bad return code                    */
                                       /* good return code is 0              */

                                       /* attribute names                    */
#define RATE_ATTR      "typamatic_rate"
#define DELAY_ATTR     "typamatic_delay"
#define VOLUME_ATTR    "volume"
#define CLICKER_ATTR   "click"
#define MAP_ATTR       "special_map"   
#define TYPE_ATTR      "special_type"
#define ODM_NOMAP        "0"           /* normal special_map value           */
#define ODM_KRMAP        "1"           /* Korean special_map value           */
#define ODM_JPMAP        "2"           /* Japanese special_map value         */
#define ODM_TWMAP        "3"           /* Chinese special_map value          */
#define KB106type      "kb106"
#define PSKBDtype      "ps2"

                                       /* command line parms                 */
#define TRATE          'r'             /*   set typamatic rate flag (-r)     */
#define MINRATE        2               /*   minimum typamatic rate           */
#define MAXRATE        30              /*   maximum typamatic rate           */

#define TDELAY         'd'             /*   set typamatic delay flag (-d)    */
#define D250           "250"           /*   typamatic delay of 250 ms        */
#define D500           "500"           /*   typamatic delay of 500 ms        */
#define D750           "750"           /*   typamatic delay of 750 ms        */
#define D1000          "1000"          /*   typamatic delay of 1000 ms       */

#define CVOL           'c'             /*   set clicker volume flag (-c)     */
#define MINCVOL        0               /*   minimum clicker volume           */
#define MAXCVOL        3               /*   maximum clicker volume           */

#define AVOL           'a'             /*   set alarm volume flag (-a)       */
#define MINAVOL        0               /*   minimum alarm volume             */
#define MAXAVOL        3               /*   maximum alarm volume             */

#define SMAP           'm'             /*   set special map flag (-m)        */
#define KOREAN         "KR"            /*   Korean map specification         */
#define JAPANESE       "JP"            /*   Japanese map specification       */
#define CHINESE        "TW"            /*   Chinese map specification        */
#define MINSMAP        0               /*   minimum special map value        */
#define MAXSMAP        3               /*   maximum special map value        */

#define STYPE          't'             /* set special type flag (-t)         */
#define NONUM          "nonum"         /*    kbd without numeric pad         */
#define MINSTYPE       0               /* minimum special type value         */
#define MAXSTYPE       1               /* maximum special type value         */

                                       /* msg catolog string                 */
#define MSGSTR(msgnum, string) catgets(catd, CHHWKBD, msgnum, string)


struct PdDv pdobj;                     /* predefined device object storage   */
struct CuDv cuobj;                     /* customized device object storage   */
struct PdAt pdatt;                     /* predefined attribute object storage*/
struct CuAt cuatt;                     /* customized attribute object storage*/

struct Class *pddv;                    /* PdDv class pointer                 */
struct Class *cudv;                    /* CuDv class pointer                 */
struct Class *pdat;                    /* PdAt class pointer                 */
struct Class *cuat;                    /* CuAt class pointer                 */

int odm_state;                         /* odm data base open/init state      */
nl_catd  catd;                         /* msg catalog handle                 */
int CuAt_changed;                      /* CuAt changed if TRUE               */

/* forward references                                                        */
int update_attr(char *, char *);
int parse(char ***, char **);
char *fmt(char *, char *, int,  int);
void bye(int);

/*****************************************************************************/
/*                                                                           */
/* NAME:        main                                                         */
/*                                                                           */
/* FUNCTION:    Change keyboard device attributes in CuDv. These changes     */
/*              take affect the next time the keyboard is configured. See    */
/*              "INPUTS" for attributes that can be changed.                 */
/*                                                                           */
/* INPUTS:      -r ##  where ## is new typamatic rate (10,20,30)             */
/*              -d ### where ### is new typamatic delay (250, 500,           */
/*                       750, or 1000)                                       */
/*              -c #   where # is new clicker volume (0 = off, 1 = low,      */
/*                       2 = medium, 3 = high)                               */
/*              -a #   where # is new alarm volume (0 = off, 1 = low,        */
/*                       2 = medium, 3 = high)                               */
/*              -m   , -m 0    no special map (standard)                     */
/*              -m KR, -m 1    enable special mapping for Korean kbd         */
/*              -m JP, -m 2    enable special mapping for Japanese kbd       */
/*              -m TW, -m 3    enable special mapping for Chinese kbd        */
/*              -t      , -t 0  with numeric pad                             */
/*              -t nonum, -t 1  no numeric pad                               */
/*                                                                           */
/* OUTPUTS:     0      = success                                             */
/*             -1      = failure                                             */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int main(int argc, char **argv)
{
   int rc, first_next, found, flag, fp, data;
   char ss[128];
   char rate_buff[3], volume_buff[2], clicker_buff[2], map_buff[2];
   char type_buff[2];
   char *delay, *rate, *volume, *clicker, *map, *type, *parm;
   lft_query_t lft_query;              /* struct to hold lft query info      */

   char *nomap = {ODM_NOMAP};          /* not special keyboard               */
   char *krmap = {ODM_KRMAP};          /* Korean keyboard                    */
   char *jpmap = {ODM_JPMAP};          /* Japanese keyboard                  */
   char *twmap = {ODM_TWMAP};          /* Chinese keyboard                   */

/* initialize some variables                                                 */
   odm_state = 0;                      /* odm is not initialized             */
   delay = NULL;                       /* typamatic rate has not been changed*/
   rate = NULL;                        /* typamatic delay has not been chg'ed*/
   volume = NULL;                      /* alarm volume has not been changed  */
   clicker = NULL;                     /* clicker volume has not been changed*/
   map = NULL;                         /* special map has not been changed   */
   type = NULL;                        /* special type has not been changed  */
   CuAt_changed = 0;                   /* CuAt has not been changed          */

/* Open the message facilities catalog.                                      */
   setlocale(LC_ALL,"");
   catd=catopen(CATNAME, NL_CAT_LOCALE);


/* initialize and open odm                                                   */
   if(odm_initialize() == -1) {
     fprintf(stderr, MSGSTR(ODM_INIT, M_ODM_INIT), PGM, odmerrno);
     bye(BAD_RC);
   }
   odm_state++;                        /* odm is initialized                 */
   if ((int)(pddv = odm_open_class(PdDv_CLASS)) == -1) {
     fprintf(stderr, MSGSTR(ODM_FUNC, M_ODM_FUNC), PGM,
          "odm_open_class", odmerrno, "PdDv");
     bye(BAD_RC);
   }
   odm_state++;                        /* PdDv object class is open          */
   if ((int)(cudv = odm_open_class(CuDv_CLASS)) == -1) {
     fprintf(stderr, MSGSTR(ODM_FUNC, M_ODM_FUNC), PGM,
          "odm_open_class", odmerrno, "CuDv");
     bye(BAD_RC);
   }
   odm_state++;                        /* CuDv object class is also open     */
   if ((int)(pdat = odm_open_class(PdAt_CLASS)) == -1) {
     fprintf(stderr, MSGSTR(ODM_FUNC, M_ODM_FUNC), PGM,
          "odm_open_class", odmerrno, "PdAt");
     bye(BAD_RC);
   }
   odm_state++;                        /* PdAt object class is also open     */
   if ((int)(cuat = odm_open_class(CuAt_CLASS)) == -1) {
     fprintf(stderr, MSGSTR(ODM_FUNC, M_ODM_FUNC), PGM,
          "odm_open_class", odmerrno, "CuAt");
     bye(BAD_RC);
   }
   odm_state++;                        /* CuAt object class is also open     */

/* find first available keyboard                                             */
   first_next = ODM_FIRST;
   found = 0;
   while(!found) {
                                       /* get keyboard object from           */
                                       /* predefine data base                */
     rc = (int) odm_get_obj(pddv, "class = keyboard", &pdobj, first_next);
     if (rc == -1) {                   /* odm error                          */
       fprintf(stderr, MSGSTR(ODM_FUNC, M_ODM_FUNC), PGM,
          "odm_get_obj", odmerrno, "PdDv");
       break;
     }
     else {
       if (rc == 0) {                  /* no more keyboard objects           */
         break;                        /* so break out of loop               */
       }
       else {                          /* see if instance of object is       */
                                       /* defined in customized data base    */
                                       /* and status is "AVAILABLE"          */
         sprintf(ss,"PdDvLn = %s AND status = 1", pdobj.uniquetype);
         rc = (int) odm_get_obj(cudv, ss, &cuobj, ODM_FIRST);
         if (rc == -1) {               /* odm error                          */
           fprintf(stderr, MSGSTR(ODM_FUNC, M_ODM_FUNC), PGM,
              "odm_get_obj", odmerrno, "CuDv");
           break;
         }
         else {
           if (rc != 0) {              /* available keyboard found           */
              found = 1;
           }
         }
       }
     }
     first_next = ODM_NEXT;            /* get next keyboard object           */
   }
                                       /* abort command if no keyboard       */
   if (!found) {                       /* is available                       */
     if (rc != -1) {                   /* output err msg if not already done */
       fprintf(stderr, MSGSTR(ODM_NOKBD, M_ODM_NOKBD), PGM);
     }
     bye(BAD_RC);
   }

/* process command line options                                              */
   rc = 0;
   while (!rc && (flag = parse(&argv,&parm))) {
     switch(flag) {

       case TDELAY:                    /* typamatic delay specification      */
         if ((!strcmp(parm,D250)) ||  /* ... validate                       */
             (!strcmp(parm,D500)) ||
             (!strcmp(parm,D750)) ||
             (!strcmp(parm,D1000))) {
           delay = parm;               /* ... save ptr to string if ok       */
         }
         else {                        /* ... output msg if error            */
           fprintf(stderr, MSGSTR(BADDELAY, M_BADDELAY), PGM);
           rc = 1;
         }
         break;

       case TRATE:                     /* typamatic rate specification       */
         if (!(rate = fmt(parm,rate_buff,MINRATE,MAXRATE))) {
           fprintf(stderr, MSGSTR(BADRATE, M_BADRATE), PGM);
           rc = 1;
         }
         break;

       case CVOL:                      /* clicker volume specification       */
         if (!(clicker = fmt(parm,clicker_buff,MINCVOL,MAXCVOL))) {
           fprintf(stderr, MSGSTR(BADCLICK, M_BADCLICK), PGM);
           rc = 1;
         }
         break;

       case AVOL:                      /* alarm volume specification         */
         if (!(volume = fmt(parm,volume_buff,MINAVOL,MAXAVOL))) {
           fprintf(stderr, MSGSTR(BADVOL, M_BADVOL), PGM);
           rc = 1;
         }
         break;

       case SMAP:                      /* special translation specification  */
                                       /* ... valid only on 106 key kbd and  */
                                       /*     PS/2 keyboards                 */
         if (strcmp(pdobj.type, KB106type) &&
             strcmp(pdobj.type, PSKBDtype)) {
           fprintf(stderr, MSGSTR(MFLAG, M_MFLAG), PGM);
           rc = 1;
         }
         else {
           if(*parm == '\0') {         /* ... no parm so turn off option     */
             map = nomap;
           }
           else {

             if (!strcmp(parm,KOREAN)) {
               map = krmap;            /* ... set map if parm = 'KR'         */
             }
             else if (!strcmp(parm, JAPANESE)) {
               map = jpmap;            /* ... set map if parm = 'JP'         */
             }
             else if (!strcmp(parm, CHINESE)) {
               map = twmap;            /* ... set map if parm = 'TW'         */
             }
             else {                    /* ... handle 0,1 input from smit     */
               if (!(map = fmt(parm,map_buff,MINSMAP,MAXSMAP))) {
                                       /* ... output invalid map value       */
                 fprintf(stderr, MSGSTR(BADMAP, M_BADMAP), PGM);
                 rc = 1;
               }
             }
           }
         }
         break;

       case STYPE:                     /* special handling for space saver   */
                                       /* keyboard                           */
                                       /* ... valid only on PS/2 keyboards   */
         if (strcmp(pdobj.type, PSKBDtype)) {
           fprintf(stderr, MSGSTR(TFLAG, M_TFLAG), PGM);
           rc = 1;
         }
         else {
           if(*parm == '\0') {         /* ... no parm so turn off option     */
             type = "0";
           }
           else {
                                       /* ... turn on if parm = 'nonum'      */
             if (!strcmp(parm,NONUM)) {
               type = "1";
             }
             else {                     /* ... handle 0,1 input from smit     */
               if (!(type = fmt(parm,type_buff,MINSTYPE,MAXSTYPE))) {
                                       /* ... output invalid map value       */
                 fprintf(stderr, MSGSTR(BADTYPE, M_BADTYPE), PGM);
                 rc = 1;
               }
             }
           }
         }
         break;

       case BAD_RC:                    /* command line parsing error         */
         fprintf(stderr, MSGSTR(PARSE, M_PARSE), PGM);
         fprintf(stderr, MSGSTR(CHHWKBD_USAGE, M_CHHWKBD_USAGE));
         rc =  1;
         break;

       default:                        /* unsupported flag                   */
         fprintf(stderr, MSGSTR(BADFLAG, M_BADFLAG), PGM, flag);
         fprintf(stderr, MSGSTR(CHHWKBD_USAGE, M_CHHWKBD_USAGE));
         rc =  1;
         break;
     }
   }

   if (rc) bye(BAD_RC);                /* exit if error                      */
                                       /* exit if nothing to do              */
   if (!rate && !delay && !volume && !clicker && !map && !type) bye(0);

/* update keyboad configuration in ODM                                       */

                                       /* typamatic rate                     */
   if (update_attr(RATE_ATTR, rate) == -1) bye(BAD_RC);
                                       /* typamatic delay                    */
   if (update_attr(DELAY_ATTR, delay) == -1) bye(BAD_RC);
                                       /* alarm volume                       */
   if (update_attr(VOLUME_ATTR, volume) == -1) bye(BAD_RC);
                                       /* clicker volume                     */
   if (update_attr(CLICKER_ATTR, clicker) == -1) bye(BAD_RC);
                                       /* keyboard special map               */
   if (update_attr(MAP_ATTR, map) == -1) bye(BAD_RC);
                                       /* keyboard special type              */
   if (update_attr(TYPE_ATTR, type) == -1) bye(BAD_RC);

   if(CuAt_changed) {                  /* if CuAt changed ...                */
                                       /* check for runtime attr             */
     rc = (int) odm_get_obj(pdat, PHASE1_DISALLOWED, &pdatt, ODM_FIRST);
     if (rc == -1) {                   /* odm error                          */
       fprintf(stderr, MSGSTR(ODM_FUNC, M_ODM_FUNC), PGM,
          "odm_get_obj", odmerrno, "PdAt");
       bye(BAD_RC);
     }
     else {
       if (rc) {                       /* runtime attr exists so need to     */
                                       /* run "savebase"                     */
         if(odm_run_method( "/usr/sbin/savebase", "", NULL, NULL)) {
           fprintf(stderr, MSGSTR(SAVEBASE_ERR, M_SAVEBASE_ERR), PGM);
           bye(BAD_RC);
         }
       }
     }
   }

/* update keyboard device                                                    */

   if (!map && !type) {                /* if -m or -t flag is not specified  */
                                       /* if cmd run from lft                */
     if ( ioctl(0, LFT_QUERY_LFT, &lft_query) != -1) {
                                       /* if keyboard can be opened          */
       sprintf(ss, "/dev/%s", cuobj.name);
       if ((fp=open(ss, O_RDWR, 0)) != -1) {
          rc = 0;
          if (rate) {                  /* update typamatic rate if changed   */
             data = atoi(rate);
             if (ioctl(fp,KSTRATE,&data) == -1) rc = BAD_RC;
          }
          if (delay && !rc) {          /* update typamatic delay if changed  */
             data = KSTDLY250;
             if (!strcmp(delay,D500)) data = KSTDLY500;
             if (!strcmp(delay,D750)) data = KSTDLY750;
             if (!strcmp(delay,D1000)) data = KSTDLY1000;
             if (ioctl(fp,KSTDELAY,&data) == -1) rc = BAD_RC;
          }
          if (volume && !rc) {         /* update alarm volume if changed     */
             data = atoi(volume);
             if (ioctl(fp,KSVOLUME,&data) == -1) rc = BAD_RC;
          }
          if (clicker && !rc) {        /* update clicker volume if changed   */
             data = atoi(clicker);
             if (ioctl(fp,KSCFGCLICK,&data) == -1) rc = BAD_RC;
          }

          close(fp);                   /* close keyboard file                */

          if (rc) {
            fprintf(stderr, MSGSTR(KBDFAIL, M_KBDFAIL), PGM, errno);
          }
          bye(rc);                     /* exit                               */
       }
     }
   }
                                       /* if cmd not run from lft, or kbd    */
                                       /* could not be open'ed or if -m, -t  */
                                       /* flag specified then output notice  */
                                       /* that keyboard has to be reconfig'ed*/
                                       /* before changes take effect         */
   if(CuAt_changed) {     
      printf(MSGSTR(CFG_MSG, M_CFG_MSG), PGM);
   }

   bye(0);                             /* exit                               */
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        update_attr                                                  */
/*                                                                           */
/* FUNCTION:    Update and/or create attribute in CuAt                       */
/*                                                                           */
/* INPUTS:      attr = pointer to string containing name of attribute        */
/*              value = pointer to attribute value                           */
/*                                                                           */
/* OUTPUTS:     -1 = failed                                                  */
/*              anything else means success                                  */
/*                                                                           */
/* NOTES:       odm data bases assumed to initialized and opened             */
/*              cuobj holds customized instance of available keyboard        */
/*              pdobj holds predefined instance of available keyboard        */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

int update_attr(char *attr, char *value)
{

   int rc = 0;
   char ss[128];


   if (value) {                        /* if attribute is to be updated      */

                                       /* get attribute from customized      */
                                       /* data base                          */
     sprintf(ss,"attribute = %s AND name = %s", attr, cuobj.name);
     rc = (int) odm_get_obj(cuat, ss, &cuatt, ODM_FIRST);
     if (rc == -1) {                   /* odm error                          */
       fprintf(stderr, MSGSTR(ODM_FUNC, M_ODM_FUNC), PGM,
          "odm_get_obj", odmerrno, "CuAt");
     }
     else {                            /* found instance of customized       */
                                       /* attribute and it is different      */
                                       /* than new value so change it        */
       if (rc != 0) {
        if (strcmp(cuatt.value, value)) {
         strcpy(cuatt.value, value);
         if ((rc = odm_change_obj(cuat, &cuatt)) == -1) {
                                       /* odm error                          */
           fprintf(stderr, MSGSTR(ODM_FUNC, M_ODM_FUNC), PGM,
              "odm_change_obj", odmerrno, "CuAt");
         }
         else {
           CuAt_changed = 1;           /* CuAt changed                       */
         }
        }
       }
       else {                          /* attribute not found in customized  */
                                       /* data based so get definition from  */
                                       /* predefine data base                */
         sprintf(ss,"attribute = %s AND uniquetype = %s",
             attr, pdobj.uniquetype);
         rc = (int) odm_get_obj(pdat, ss, &pdatt, ODM_FIRST);
         if (rc == -1) {               /* odm error                          */
           fprintf(stderr, MSGSTR(ODM_FUNC, M_ODM_FUNC), PGM,
              "odm_get_obj", odmerrno, "PdAt");
         }
         else {
           if (rc == 0) {              /* yikes,  not in predefine either    */
                                       /* so output error                    */
             fprintf(stderr, MSGSTR(ODM_NOATTR, M_ODM_NOATTR), PGM, attr);
             rc = -1;
           }
           else {
             if (strcmp(pdatt.deflt, value)) {
                                       /* load customized attribute struct   */
               bzero(&cuatt,sizeof(struct CuAt));
               strcpy(cuatt.name, cuobj.name);
               strcpy(cuatt.attribute, pdatt.attribute);
               strcpy(cuatt.value, value);
               strcpy(cuatt.type, pdatt.type);
               strcpy(cuatt.generic, pdatt.generic);
               strcpy(cuatt.rep, pdatt.rep);
               cuatt.nls_index = pdatt.nls_index;
                                       /* create customized attribute        */
               if ((rc = odm_add_obj(cuat, &cuatt)) == -1) {
                                       /* odm error                          */
                 fprintf(stderr, MSGSTR(ODM_FUNC, M_ODM_FUNC), PGM,
                    "odm_add_obj", odmerrno, "CuAt");
               }
               else {
                 CuAt_changed = 1;     /* CuAt changed                       */
               }
             }
           }
         }
       }
     }
   }
   return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        parse                                                        */
/*                                                                           */
/* FUNCTION:    Parse command line options                                   */
/*                                                                           */
/* INPUTS:      argptr = pointer to argv (from main)                         */
/*              flagparm = area to return pointer to start of flag           */
/*                         parameters                                        */
/*                                                                           */
/* OUTPUTS:     0  = end of command line options                             */
/*              BAD_RC = parsing error                                       */
/*                                                                           */
/*              anything else is option flag                                 */
/*                                                                           */
/* NOTES:       sort of like getopt() but works better for this application  */
/*                                                                           */
/*****************************************************************************/

int parse(char ***argptr, char **flagparm)
{
   int flag;
   static char no_parm = '\0';         /* empty string returned when there   */
                                       /* is no parameter following flag     */

   (*argptr)++;                        /* point to next command line option  */
   if (**argptr == NULL) {             /* if end of options then             */
     flag = 0;                         /* return 0                           */
   }
   else {
     if (***argptr != '-') {           /* if option does not start with '-'  */
        flag = BAD_RC;                 /* then return -1 (parsing error)     */
     }
     else {
       if (*((**argptr)+1) == '\0') {  /* if nothing follows '-' then        */
         flag = BAD_RC;                /* return -1 (parsing error)          */
       }
       else {                          /* flag is character following '-'    */
         flag = (int) *((*(*argptr))+1);
                                       /* if additional characters follow    */
                                       /* flag character then these must     */
                                       /* be parameters so point flagparm    */
                                       /* to this string                     */
         if (*((**argptr)+2) != '\0') {
           *flagparm = (**argptr)+2;
         }
         else {                        /* if another command line option     */
                                       /* follows and it does not start with */
                                       /* '-' then this option must be       */
                                       /* parameters of the current flag so  */
                                       /* point flagparm to this string and  */
                                       /* increment argptr so next command   */
                                       /* line option is not parsed          */
           if ((*((*argptr)+1) == NULL) ||
               (**((*argptr)+1) == '-'))
             *flagparm = &no_parm;
           else {
             (*argptr)++;
             *flagparm = **argptr;
           }
         }
       }
     }
   }

   return(flag);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        fmt                                                          */
/*                                                                           */
/* FUNCTION:    Format and range check numerical string                      */
/*                                                                           */
/* INPUTS:      parm = string containing numeric value                       */
/*              fptr = location to put formated value                        */
/*              low  = lowest valid value                                    */
/*              high = highest valid value (must be lower than 999)          */
/*                                                                           */
/* OUTPUTS:     NULL if error                                                */
/*              pointer to fmt_buff if ok                                    */
/*                                                                           */
/* NOTES:       one might ask why do all of this. Well the answer is to      */
/*              correctly handle things like -r 00004  or -r "   20 "        */
/*                                                                           */
/*****************************************************************************/

char *fmt(char *parm, char *fptr, int low, int high)

{
   int data, state;
   char *rc;

   data = 0;
   state = 0;
                                       /* scan data till error or null       */
   while(fptr && (*parm != '\0')) {
     switch (state) {
        case 0:                        /* strip leading blanks               */
          if (*parm == ' ') break;
          state++;
                                       /* convert string into binary         */
        case 1:                        /* error if not blank, '0' - '9'      */
          if (*parm == ' ') state++;
          else {
            if ((*parm < '0') || (*parm > '9')) fptr = NULL;
            else {
              data = data * 10 + (int)(*parm & 0x0f);
            }
          }
          break;
                                       /* strip trailing blanks              */
        default:                       /* error if any non blank characters  */
          if (*parm != ' ') fptr = NULL;
     }
     parm++;
   }

   if (fptr) {
     if (!state) fptr = NULL;          /* error if no digits specified       */
     else {                            /* error if number out of range       */
       if ((data > high) || (data < low)) fptr = NULL;
       else {                          /* make room to format string         */
         fptr++;
         if (data>=10) fptr++;
         if (data>=100) fptr++;
         *fptr = '\0';                 /* null terminate fmt'ed string       */
         do {                          /* convert binary to string           */
           fptr--;
           *fptr = ((char)(data % 10)) | 0x30;
           data = data/10;
         } while (data);
       }
     }
   }

   return(fptr);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        bye                                                          */
/*                                                                           */
/* FUNCTION:    Exit program                                                 */
/*                                                                           */
/* INPUTS:      rc = return code                                             */
/*                                                                           */
/* OUTPUTS:     none                                                         */
/*                                                                           */
/* NOTES:       This routine terminates the process                          */
/*                                                                           */
/*****************************************************************************/

void bye(int rc)

{

   switch(odm_state) {                 /* close and terminate odm access     */

     case 5:
        if (odm_close_class(cuat) == -1) {
          fprintf(stderr, MSGSTR(ODM_FUNC, M_ODM_FUNC), PGM,
             "odm_close_class", odmerrno, "CuAt");
          rc = BAD_RC;
        }

     case 4:
        if (odm_close_class(pdat) == -1) {
          fprintf(stderr, MSGSTR(ODM_FUNC, M_ODM_FUNC), PGM,
             "odm_close_class", odmerrno, "PdAt");
          rc = BAD_RC;
        }

     case 3:
        if (odm_close_class(cudv) == -1) {
          fprintf(stderr, MSGSTR(ODM_FUNC, M_ODM_FUNC), PGM,
             "odm_close_class", odmerrno, "CuDv");
          rc = BAD_RC;
        }

     case 2:
        if (odm_close_class(pddv) == -1) {
          fprintf(stderr, MSGSTR(ODM_FUNC, M_ODM_FUNC), PGM,
             "odm_close_class", odmerrno, "PdDv");
          rc = BAD_RC;
        }

     case 1:
        odm_terminate();

     default:
        break;
   }

   catclose(catd);                     /* close message catalog              */

   exit(rc);                           /* terminate                          */

}
