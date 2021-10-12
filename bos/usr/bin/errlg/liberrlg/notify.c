static char sccsid[] = "@(#)12        1.8  src/bos/usr/bin/errlg/liberrlg/notify.c, cmderrlg, bos411, 9428A410j 5/16/94 14:55:47";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: notifyinit, notifyadd, notifydelete, notifydelete_pers,
 *            notifypause, notify, notifywait3
 *            CuDv_getbyresource, CuDv_dump
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define _ILS_MACROS
#include <signal.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <errnotify.h>
#include <errlg.h>

#define CLASSNULL ((struct Class *)-1)
#define FIRST 1
#define NEXT  0
static struct Class *jodm_get_obj();

/*extern struct Class *odm_open_class();*/
/*extern struct Class *odm_mount_class();*/
static struct ClassElem *nametoelem();

static struct errnotify x_errnotify;
static char *N_classname = "errnotify";
static char *CuDv_classname = "CuDv";
static char *CuVPD_classname = "CuVPD";
static struct Class *Nclp;
static struct Class *CuDvclp;
static struct Class *CuVPDclp;  /* for vpd only */
static char *CuDvbuf;
static char *CuVPDbuf;          /* for vpd only */
static char *Objrepos = "/etc/objrepos";

static Nproc;   /* wait3 for this many processes */

char *CuDv_name;
char *CuDv_rclass;
char *CuDv_vpd;
char *CuVPD_vpd;
char *CuDv_connwhere;
char *CuDv_location;

extern	char Logfile[PATH_MAX];		

/*
 * "name" must be first.
 */
struct clist {
        char  *c_name;
        char  *c_source;
        char  *c_target;
        int    c_size;
} clist[] = {
        { "name", },
        { "location", },
        { "vpd", },
        { "connwhere", },
        { "PdDvLn", },
        { 0 },
};

char vpdbuf[VPDSIZE];

int CuDvbufsize_tmp;
int CuVPDbufsize_tmp;

/* This flag is set to TRUE, when the error has */
/* symptom data, otherwise it is FALSE.         */
Boolean symptomdataflg;

/* This flag is set to TRUE when the errpt command is executed */
/* with the -Y option, otherwise it is set to FALSE.           */
Boolean errpt_symptomflg;

notifyinit()
{
  int rc = 0;
        struct clist *cp;
        struct ClassElem *ep;

        if(odm_initialize() < 0) {
                odm_perror("odm_initialize");
                rc = (-1);
        }

        odm_set_path(Objrepos);
        odm_set_perms(0666);

        if ((Nclp = odm_mount_class(N_classname)) == CLASSNULL) {
                Nclp = 0;
                odm_perror("%s/%s",Objrepos,N_classname);
          rc = (-1);
        }

        if((CuDvclp = odm_mount_class(CuDv_classname)) == CLASSNULL) {
                CuDvclp = 0;
                odm_perror("%s/%s",Objrepos,CuDv_classname);
        } else {
                CuDvbuf = jalloc(CuDvclp->structsize+4);
                CuDvbufsize_tmp = CuDvclp->structsize+4;
                for(cp = clist; cp->c_name; cp++) {
                        if (!strcmp(cp->c_name,"vpd"))
                          continue;
                        if((ep = nametoelem(CuDvclp,cp->c_name)) == 0) {
                                cp->c_target = jalloc(5);
                                continue;
                        }
                        cp->c_source = &CuDvbuf[ep->offset];
                        cp->c_size   = ep->size;
                        cp->c_target = jalloc(ep->size+5);
                        if (streq(cp->c_name,"PdDvLn")) 
                          cp->c_source = &CuDvbuf[ep->offset + 8];
                        
          }
        }

        return(rc);
}

static struct ClassElem *nametoelem(clp,name)
struct Class *clp;
char *name;
{
        int i;

        for(i = 0; i < clp->nelem; i++)
                if(streq(clp->elem[i].elemname,name))
                        return(&clp->elem[i]);
        return(0);
}

extern char *lst_lookup();

#define SIGTOMASK(sig) ( 1 << ((sig) - 1))

#define CCPY(NAME,ELNAME) \
{ \
        char *cp; \
        cp = lst_lookup(ELNAME); \
        strncpy(x_errnotify.NAME,cp ? cp : "",sizeof(x_errnotify.NAME)); \
}

#define ICPY(NAME,ELNAME) \
{ \
        char *cp; \
        cp = lst_lookup(ELNAME); \
        x_errnotify.NAME = cp ? 0 : strtoul(cp,0,16); \
}

notifyadd()
{
        static msgflg;

        if(Nclp == 0)
                return;
        notifydelete();
        memset(&x_errnotify,0,sizeof(x_errnotify));
        x_errnotify.en_pid = getpid();  /* fill in the pid */
        strcpy(x_errnotify.en_name,"concurrent");
        CCPY(en_label,    "el_label");
        ICPY(en_crcid,    "el_crcid");
        CCPY(en_class,    "el_class");
        CCPY(en_type,     "el_type");
        CCPY(en_alertflg, "et_alertflg");
        CCPY(en_resource, "el_resource");
        CCPY(en_rtype,    "el_rtype");
        CCPY(en_rclass,   "el_rclass");
        if (errpt_symptomflg)
           strcpy(x_errnotify.en_symptom, "TRUE");
        sprintf(x_errnotify.en_method,"kill -%d %d",SIGUSR1,getpid());
        sigsetmask(SIGTOMASK(SIGUSR1)); /* block SIGUSR1 until ready */
        if(odm_add_obj(Nclp,&x_errnotify) < 0) {
                if(!msgflg) {
                        msgflg++;
                        odm_perror("%s: odm_add_obj",N_classname);
                }
        }
}

/*
 * NAME:      notifydelete
 * FUNCTION:  Stop errlog notification by deleting the errnotify object.
 *            This routine is called at the end of concurrent error reporting.
 * INPUTS:    NONE
 * RETURNS:   NONE
 */

notifydelete()
{
        char ucritstr[64];

        if(Nclp == 0)
                return;
        sprintf(ucritstr,"%s = %d","en_pid",getpid());
        odm_rm_obj(Nclp,ucritstr);
}

notifydelete_pers()
{

        if(Nclp == 0)
                return;
        odm_rm_obj(Nclp,"en_persistenceflg = 0");
}

static void ignore()
{
}

/*
 * NAME:      notifypause
 * FUNCTION:  Pause for signal SIGUSR1.
 *            This routine is called during concurrent error reporting to
 *              wait for the occurence of errors specified by notifyinit.
 * INPUTS:    NONE
 * RETURNS:   NONE
 */
notifypause()
{

        signal(SIGUSR1,(void(*)(int)) ignore);
        sigpause(0);                    /* allow SIGUSR1 to cause return from pause */
}

CuDv_getbyresource(resource)
char *resource;
{
        char ucritbuf[128];
        int rv;

        sprintf(ucritbuf,"%s = %s",clist[0].c_name,resource);
        rv = CuDv_get(ucritbuf);
        return(rv);
}

/*
  Interface to the odm_get_obj() call, This routine sets the user criteria
  to the the string 'name = resource' for what ever value is in resource.
.*/
CuVPD_getbyresource(resource)
char *resource;
{
  char ucritbuf[128];
  int rv;

  sprintf(ucritbuf,"%s = %s",clist[0].c_name,resource);
  rv = CuVPD_get(ucritbuf);
  return(rv);
} /* end of CuVPD_getbyresource */


static CuDv_get(ucritstr)
char *ucritstr;
{
        struct clist *cp;
        static msgflg;
        int firstflg;
        struct Class *clp;
        int rc = 0;
        int i;

        /* Link the value pointers to the targets they represent. */
        CuDv_location = clist[1].c_target;
        CuDv_connwhere = clist[3].c_target;
        CuDv_rclass = clist[4].c_target;

        /* Now get the data out of the CuDv class, one buffer at a time. */
        if(CuDvclp == 0) 
                rc = 0;
        
        else {
          firstflg = ucritstr ? 1 : 0;
          /* This is a totally unsupported method of calling odm. */
          clp = jodm_get_obj(CuDvclp,ucritstr,CuDvbuf,firstflg);
          if(clp == CLASSNULL || clp == 0) {
                if(clp == CLASSNULL && !msgflg) {
                        msgflg++;
                        odm_perror("odm_get_obj(CuDv,%s)",ucritstr);
                }
                for(cp = clist; cp->c_name; cp++) {
                        if (!strcmp(cp->c_name,"vpd"))
                          continue;
                        strcpy(cp->c_target,"NONE");
                }
                rc = 0; 
          }
          else
            for(cp = clist; cp->c_name; cp++) {
             if (!strcmp(cp->c_name,"vpd"))
               continue;
              strcpy(cp->c_target,cp->c_source?cp->c_source:"NONE");
            }
        }

        return(rc);

} /* end of CuDvget */

int CuVPD_get(ucritstr)
char *ucritstr;
{ 
  struct CuVPD CuVPDbuf;
  struct CuVPD *clp;
  static msgflg;
  int firstflg;
  int rc = 0;

  /* Link the global pointer to the data buffer vpdbuf[]. */
  CuVPD_vpd = vpdbuf;

  firstflg = *ucritstr ? 1 : 0;
  /* Now get the data out of the CuVPD class, one buffer at a time. */
  if ((clp = odm_get_obj(CuVPD_CLASS,ucritstr,&CuVPDbuf,firstflg)) < 0) {
    if (!msgflg) { /* Problem with the odm_get_obj() call. */
      msgflg++;
      odm_perror("odm_get_obj(CuVPD,%s)",ucritstr);
      rc = 1;
    }
  }
  else if (clp == NULL) {   /* No objects were found in CuVPD. */
    strcpy(vpdbuf,"NONE"); 
    rc = 0;
  }
  else {  /* Copy the data from the vpd[] array to the vpdbuf[] array. */

    /* Note that VPDSIZE=512;VPD data may contain embedded nulls. */
    bcopy(CuVPDbuf.vpd,vpdbuf,VPDSIZE);
    rc = 0;
  }

  return(rc);

} /* end of CuVPDget */

CuDv_dump(firstflg)
{

        return(CuDv_get(firstflg ? "" : 0));
}

#define ALERTSTR(alert) (alert ? "TRUE" : "FALSE")
#define S(str)          (str[0] && !isspace(str[0]) ? str : "-")
#define SYMPTSTR(sympt) (sympt ? "TRUE" : "FALSE")

/*
 * NAME:     notify
 * FUNCTION: Notify waiting processes (thru ODM object class) of errlog entry
 * INPUTS:   none
 * RETURNS:  none
 *
 * Notify waiting processes of this new entry
 * The notifymatch routine checks for a match between
 *  the selected error and the contents in an errnotify object.
 * The notifymethod routine expands the en_method field in the
 *  errnotify object with the sequence number, crcid, etc.
 * If there is no errnotify class, just return.
 *
 * The notifywait3() routine is to wait3 for exited fork/exec-ed method
 *  processes. The parent keeps a count 'Nproc' of the number of
 *  forked children and does wait3's for that many.
 */
notify()
{
        int firstflg;
        static msgflg;
        struct Class *clp;
        char *x;
        int i;

        if(Nclp == 0)
                return;
        x = (char *) &x_errnotify;
        for(firstflg = 1; ; firstflg = 0) { /* read each errnotify object */
                clp = jodm_get_obj(Nclp,"",&x_errnotify,firstflg);
                if(clp == CLASSNULL || clp == 0) {
                        if(clp == CLASSNULL && !msgflg) {
                                msgflg++;
                                odm_perror("odm_get_obj(errnotify)");
                        }
                        return;
                }
                notifywait3();
                if(notifymatch()) {
                        notifymethod();
                        Nproc++;
                }
        }
}

/*
 * Do 'Nproc' wait3's get get rid of exited processes.
 * (At the time this was written, using signal(SIGCLD,SIG_IGN) to cause
 * exiting processes to not become zombies did not work right)
 */
static notifywait3()
{
        int n;
        int i;
        int status;

        n = Nproc;
        for(i = 0; i < n; i++)
                if(wait3(&status,WNOHANG,0) >= 0 && Nproc > 0)
                        Nproc--;
}

/*
 * expand the en_method string from the errnotify class into
 * a command line by replacing:
 * $1   with el_sequence (decimal)
 * $2   with el_crcid    (hex)
 * $3   with el_class    (alpha)
 * $4   with el_type     (alpha)
 * $5   with et_alertflg (TRUE/FALSE)
 * $6   with el_resource
 * $7   with el_rtype
 * $8   with el_rclass
 * $9   with en_label
 */
static notifymethod()
{
        char *tcp,*fcp;
        int c,cc;
        int prevc;
        char buf[1024];

        fcp = x_errnotify.en_method;
        tcp = buf;
        prevc = 0;
        for(c = *fcp++; c; prevc = c, c = *fcp++) {     /* loop till '\0' */
                if(c != '$') {
                        *tcp++ = c;
                        continue;
                }
                if(!((prevc == ' ' || prevc == '\t') && isdigit(*fcp))) {
                        *tcp++ = c;
                        continue;
                }
                cc = *fcp++;
                switch(cc) {
                case '1':  {
					if (*fcp == '0') {
						fcp++;
						strcpy(tcp,Logfile);
					}
					else
						sprintf(tcp,"%d",T_errlog.el_sequence); 
					break;
		        }
                case '2': sprintf(tcp,"0x%08x",T_errtmplt.et_crcid);      break;
                case '3': strcpy(tcp,T_errtmplt.et_class);                break;
                case '4': strcpy(tcp,T_errtmplt.et_type);                 break;
                case '5': strcpy(tcp,ALERTSTR(T_errtmplt.et_alertflg)); break;
                case '6': strcpy(tcp,T_errlog.el_resource);             break;
                case '7': strcpy(tcp,T_errlog.el_rtype);                break;
                case '8': strcpy(tcp,T_errlog.el_rclass);               break;
                case '9': strcpy(tcp,T_errtmplt.et_label);             break;
                default:
                        cc = 0;
                        break;
                }
                if(cc)
                        tcp += strlen(tcp);
        }
        *tcp = '\0';
        if(fork() == 0) {
                int i;

                signal(SIGCLD,SIG_DFL);
                for(i = 3; i < 20; i++)
                        close(i);
                execl("/bin/sh","sh","-c",buf,0);
                exit(1);
        }
}

#define N_STREQ(s1,s2) \
(s1 == 0 || s1[0] == '\0' || STREQ(s1,s2) || (s1[0] == '-' && s1[1] == '\0'))

#define NICHK(s1,s2) \
if(s1 && (ulong)s1 != s2) \
        return(0);

#define NCCHK(s1,s2) \
if(!N_STREQ(s1,s2)) \
        return(0);

/*
 * compare each "matchable" field in the errnotify class
 * with its corresponding field in the errlog class.
 * A match is an exact string equality or a "-" in the errnotify field.
 */
static notifymatch()
{

        NICHK(x_errnotify.en_crcid,T_errtmplt.et_crcid);
        NCCHK(x_errnotify.en_label,T_errtmplt.et_label);
        NCCHK(x_errnotify.en_class,T_errtmplt.et_class);
        NCCHK(x_errnotify.en_type,T_errtmplt.et_type);
        NCCHK(x_errnotify.en_alertflg,ALERTSTR(T_errtmplt.et_alertflg));
        NCCHK(x_errnotify.en_resource,T_errlog.el_resource);
        NCCHK(x_errnotify.en_rtype,T_errlog.el_rtype);
        NCCHK(x_errnotify.en_rclass,T_errlog.el_rclass);
        NCCHK(x_errnotify.en_symptom,SYMPTSTR(symptomdataflg));
        return(1);
}

static struct Class *jodm_get_obj(clp,ucritstr,buf,flg)
struct Class *clp;
char *ucritstr;
char *buf;
{

        return((struct Class *)odm_get_obj(clp,ucritstr,buf,flg));
}


