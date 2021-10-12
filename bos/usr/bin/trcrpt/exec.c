static char sccsid[] = "@(#)52        1.15  src/bos/usr/bin/trcrpt/exec.c, cmdtrace, bos411, 9433B411a 8/17/94 03:29:25";

/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 *
 * FUNCTIONS: setpid, exec_init, svc_lookup, svc_install
 *            exec_lookup, exec_lookupbypid, Tdeviceinstall, devicelookup
 *            Tpidinstall, exec_install, loaderlookup, Tloaderinstall
 *            pidlist_chk, pidlist_init, Tlookuppninstall1, Tlookuppninstall2
 *            Tfdinstall, fdlookup, vnodelookup
 *            Tpfsrdwrinstall1, Tpfsrdwrinstall2
 *            sidlookup, vpagelookup, Tvmmbufinstall, Tbufinstall, buflookup
 *            currfile_lookup, Tstarttimer Tendtimer
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *  @IBM_COPYRIGHT@
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *  
 *  LEVEL 1, 5 Years Bull Confidential Information
 *  
 */
/*
 * FUNCTION: manage pid-to-pathname tables.
 */  

#include <stdio.h>
#include <string.h>
#include "rpt.h"

extern Aseconds;
extern Ananoseconds;

#define VCPY(from,to_array) \
	strncpy(to_array,from,sizeof(to_array)); \
	to_array[sizeof(to_array)-1] = '\0';

#define PAGESIZE 0x1000

extern char *basename();	/* raslib.a routine */
extern Pid;
extern Intr_depth;

struct queue_entry {
	struct queue_entry *qe_next;
	struct queue_entry *qe_prev;
};

struct queue {
	struct queue_entry *q_head;
	struct queue_entry *q_tail;
	struct queue_entry *q_free;
};
static struct queue va_queue;
static struct queue b_queue;
static struct queue timer_queue;

#define TAIL(X) \
	(struct X *)(X/**/_queue.q_tail)

static struct queue_entry *qget();

struct timer {
	struct timer *t_next;
	struct timer *t_prev;
	int           t_timerid1;
	int           t_timerid2;
	int           t_seconds;
	int           t_nanoseconds;
};
static struct timer *timer_lookup();
static struct timer *gett();
#define NTIMER 100

struct b {
	struct b *b_next;
	struct b *b_prev;
	int       b_buf;
	int       b_vnode;
	int       b_type;		/* "state" of this bp (VMM,LVM,etc) */
};
static struct b *blookup();
static struct b *binstall();
static struct b *getb();
#define NB 100

struct va {
	struct va *va_next;
	struct va *va_prev;
	int        va_startpage;
	int        va_endpage;
	int        va_sid;
	int        va_vnode;
};

static struct va *va_pagelookup();
static struct va *va_install();
static struct va *getva();
#define NVA 100

struct dev {
	struct dev  *d_next;
	struct dev  *d_prev;
	int          d_rdev;
	char        *d_name;
};
char *devicelookup();
#define DEV_NULL (struct dev *)NULL
static struct dev *dev_anchor = DEV_NULL;

struct v {
	struct v *v_next;
	struct v *v_prev;
	int       v_vnode;
	char     *v_filename;
};

/*
 * connect fd to vnode and file_offset (fd = index into file table)
 */
struct file {
	int f_vnode;
	int f_offset;
};
#define NFILES 32

struct u {
	struct u    *u_next;
	struct u    *u_prev;
	int          u_pid;
	int          u_svc;
	struct file  u_file[NFILES];
	char        *u_currfile;
	int          u_currvnode;
	char        *u_execname;
	int 	     u_tid;
	int 	     u_cpuid;
	int 	     u_pri;
};
#define U_NULL (struct u *)NULL
static struct u *u_anchor = U_NULL;

static struct u *curru = U_NULL;

#define V_NULL (struct v *)NULL
static struct v *v_anchor = V_NULL;

static struct u *getu();
static struct u *u_lookup();
static struct u *u_install();

static struct v *v_lookup();
static struct v *v_install();

#define SWTCH() { \
		if(Opts.opt_execflg) pr_exec(); \
		if(Opts.opt_svcflg)  pr_svc(); \
		if(Opts.opt_fileflg) pr_file();\
		}

#define FSWTCH() \
	if(Opts.opt_fileflg) pr_file();

#define PSWTCH() \
	if(Opts.opt_execflg) pr_exec();


#ifdef TRCRPT

setpid()
{
	struct u *up;

	if((up = u_lookup(Pid)) == 0 && (up = u_install(Pid,Pid)) == 0)
		return;
	curru = up;
	if(!Threadflg)
		SWTCH();
}


void settid(tid,pid)
{
	struct u *up;

	if((up = u_lookup(tid)) == 0 && (up = u_install(tid,pid)) == 0)
		return;
	curru = up;
	if(!Threadflg) 
		SWTCH();
}

#endif

static struct u *last_u = U_NULL;

static struct u *u_lookup(tid)
{
	if(last_u && last_u->u_tid == tid)
		return(last_u);
	for(last_u=u_anchor;
	    last_u && last_u->u_tid!=tid;
	    last_u=last_u->u_next);

	Debug("ulookup: return %x\n",last_u);
	return(last_u);
}

static struct u *u_lookup_pid(pid)
{
	if(last_u && last_u->u_pid == pid)
		return(last_u);
	for(last_u=u_anchor;
	    last_u && last_u->u_pid!=pid;
	    last_u=last_u->u_next);

	Debug("u_lookup_pid: return %x\n",last_u);
	return(last_u);
}

static struct u *u_lookup_cpuid(cpuid)
{
	if(last_u && last_u->u_cpuid == cpuid)
		return(last_u);
	for(last_u=u_anchor;
	    last_u && last_u->u_cpuid!=cpuid;
	    last_u=last_u->u_next);

	Debug("u_lookup_cpuid: return %x\n",last_u);
	return(last_u);
}

static struct u *u_install(tid,pid)
{
	register struct u *up;

	/* Allocate a structure. */
	if((up = getu()) == U_NULL)
		return(U_NULL);
	Debug("u_install up=%x\n",up);
	
	/* Put it on the chain. */
	up->u_next = u_anchor;
	if (u_anchor != U_NULL)
		u_anchor->u_prev = up;
	up->u_prev = U_NULL;
	u_anchor = up;

	up->u_tid = tid;
	up->u_pid = pid;
	up->u_cpuid = -1;
	up->u_pri = -1;
	return(up);
}


int pid_lookup()
{

	return(curru->u_pid);
}

int cpuid_lookup()
{

	return(curru->u_cpuid);
}

int pri_lookup()
{

	return(curru->u_pri);
}

void pid_install()
{

	Debug("pid_install(%x) Tid = %x \n",Pid,Tid);
	curru->u_pid = Pid;
}

void cpuid_install()
{

	Debug("cpupid_install(%x) Tid = %x \n",Cpuid,Tid);
	curru->u_cpuid = Cpuid;
}

void pri_install()
{

	Debug("pri_install(%x) Tid = %x \n",Pri,Tid);
	curru->u_pri = Pri;
}
void cpuid_remove(cpuid)
{

	struct u *up;

	if((up = u_lookup_cpuid(cpuid)) != 0) {
		up->u_cpuid = -1;
		Debug("cpuid_remove(%x)  tid = %x \n",Cpuid, up->u_tid);
	}	
}

#ifdef TRCRPT

exec_init()
{
	struct b *bp;
	struct va *vap;
	struct timer *tp;
	int i;

	vap = MALLOC(NVA,struct va);
	for(i = 0; i < NVA; i++)
		qfree(&va_queue,&vap[i]);
	bp = MALLOC(NB,struct b);
	for(i = 0; i < NVA; i++)
		qfree(&b_queue,&bp[i]);
	tp = MALLOC(NTIMER,struct timer);
	for(i = 0; i < NTIMER; i++)
		qfree(&timer_queue,&tp[i]);
}

int svc_lookup()
{

	Debug("svc_lookup \n");
	return(curru->u_svc);
}

int svc_install(svc)
{

	Debug2("svc_install(%x)\n",svc);
	curru->u_svc = svc;
}

#endif


char *exec_lookup()
{
	static char buf[16];

	switch(curru->u_pid) {
        case 0x00000000: return("scheduler");
	}
	if(curru->u_execname)
		return(curru->u_execname);
	sprintf(buf,"-%d-",curru->u_pid);
	return(buf);
}


#ifdef TRCRPT

char *exec_lookupbypid(pid)
{
	struct u *up;
	static char buf[16];

	if (Threadflg) {
		switch(curru->u_pid) {
        	case 0x00000000: return("scheduler");
        	}
	}
	else {
		switch(pid) {
        	case 0x00000000: return("scheduler");
        	}
	}
	up = u_lookup_pid(pid);
	if(up)
		return(up->u_execname);
	sprintf(buf,"-%d-",pid);
	return(buf);
}

#endif


/*
 * template function. for fork and trace process table (ps) initialization
 */
Tdeviceinstall(rdev,name)
char *name;
{
	struct dev *dp;

	Debug("deviceinstall(%x,%s)\n",rdev,name);
	/* Allocate a new structure. */
	if ((dp=(struct dev *)malloc(sizeof(struct dev)))==DEV_NULL) {
		Debug("Couldn't allocate device structure.\n");
		genexit(1);
	}

	/* Put at head of chain. */
	dp->d_next = dev_anchor;
	if (dev_anchor != DEV_NULL)
		dev_anchor->d_prev = dp;
	dp->d_prev = DEV_NULL;
	dev_anchor = dp;

	dp->d_rdev = rdev;
	dp->d_name = stracpy(name);
}

char *devicelookup(rdev)
{
	struct dev *dp;

	for(dp = dev_anchor; dp; dp = dp->d_next)
		if(dp->d_rdev == rdev)
			return(dp->d_name);

	/* None found. */
	return((char *)NULL);
}

/*
 * template function. for fork and trace process table (ps) initialization
 */

Tpidinstall(pid,ppid,pathname,tid,cpuid,pri)
char *pathname;
{
	char *name;

	if (!Threadflg)
		tid = pid;
	Debug("pidinstall(%x,%x,%s,%x,%x,%x)\n",pid,ppid,pathname,tid,cpuid,pri);
	if(((curru=u_lookup(tid))==U_NULL) && 
	   ((curru=u_install(tid,pid))==U_NULL))
		return;

	curru->u_cpuid = cpuid;
	curru->u_pri = pri;
	name = basename(pathname);
	curru->u_execname = stracpy(name);
	exec_install(name);
	PSWTCH();
}


exec_install(pathname)
char *pathname;
{
	char *name;
	Debug("exec_install pid=%x '%s' tid=%x \n",curru->u_pid,pathname,
		curru->u_tid);
	name = basename(pathname);
	curru->u_execname = stracpy(name);
}

static struct v *last_v = V_NULL;

static struct v *v_lookup(vnode)
{

	Debug("vlookup(%x)\n",vnode);
	if(vnode == 0) {
		/*curru->u_currfile = 0;*/
		FSWTCH();
		return(V_NULL);
	}
	if(last_v && last_v->v_vnode == vnode) {
		/*curru->u_currfile = last_v->v_filename;*/
		FSWTCH();
		return(last_v);
	}
	for(last_v=v_anchor;
	    last_v && last_v->v_vnode!=vnode;
	    last_v=last_v->v_next);

	FSWTCH();
	return(last_v);
}

static struct v *v_install(vnode,name)
char *name;
{
	register struct v *vp;

	if(vnode == 0)
		return(V_NULL);
	curru->u_currfile = 0;
	if((vp = v_lookup(vnode)) == V_NULL) {
		if((vp=(struct v *)malloc(sizeof(struct v))) == V_NULL) {
			Debug("Couldn't allocate vnode structure.\n");
			genexit(1);
		}
		Debug("v_install vp=%x\n",vp);
		vp->v_next = v_anchor;
		if (v_anchor != V_NULL)
			v_anchor->v_prev = vp;
		vp->v_prev = V_NULL;
		v_anchor = vp;
	}
	curru->u_currfile = name;
	vp->v_filename = name;
	vp->v_vnode    = vnode;
	FSWTCH();
	return(vp);
}

/*
 * Allocate a "u" structure and zero it.
 */
static struct u *getu()
{
	struct u *up;

	if ((up=(struct u *)malloc(sizeof(struct u)))==U_NULL) {
		Debug("Couldn't allocate user structure.\n");
		genexit(1);
	}

	memset(up,0,sizeof(struct u));
	return(up);
}

struct sr {
	struct sr *sr_next;
	unsigned   sr_start;
	int        sr_range;
	char      *sr_name;
};

static struct sr *getsr()
{

	return(MALLOC(1,struct sr));
}

static struct sr *Sr;

/*
 * Try to find value in the list of loaderinstall-ed names
 */
char *loaderlookup(value)
unsigned value;
{
	register struct sr *srp;

	for(srp = Sr; srp; srp = srp->sr_next)
		if(srp->sr_start <= value && value < srp->sr_start + srp->sr_range)
			return(srp->sr_name);
	return(0);
}

/*
 * template function
 */
Tloaderinstall(start,range,name)
char *name;
{
	struct sr *srp;

	Debug("loaderinstall(%x,%x,%s)\n",start,range,name);
	srp = getsr();
	srp->sr_next = Sr;
	Sr = srp;
	srp->sr_start = start;
	srp->sr_range = range;
	srp->sr_name  = stracpy(name);
}

/*
 * Support -p option (pid filtering)
 */

struct pidlist {
	struct pidlist *pl_next;
	int             pl_pid;
	int             pl_negfiltflg;
	char           *pl_pidname;
};

static Kpidflg;
static Pidflg;					/* true if pidlist checking is on */
static struct pidlist *Pidlist;	/* head of list of pidlist structures */

static struct pidlist *getpl();	/* allocate a pidlist structure */

pidlist_chk()
{
	register struct pidlist *pp;
	char *execname;

	if(Pidflg == 0)
		return(1);
	execname = exec_lookup();
	if (Threadflg) {
	    for(pp = Pidlist; pp; pp = pp->pl_next) {
		if((pp->pl_pidname == 0 && pp->pl_pid == Pid) ||
		   (pp->pl_pidname && STREQ(pp->pl_pidname,execname)))
			break;
	    }
	}
	else {
	    for(pp = Pidlist; pp; pp = pp->pl_next) {
		if((pp->pl_pidname == 0 && pp->pl_pid == Pid) ||
		   (pp->pl_pidname && STREQ(pp->pl_pidname,execname)))
			break;
	    }
	}
	if(pp)
		return(pp->pl_negfiltflg ? 0 : 1);
	return(Kpidflg ? 1 : 0);
}


#ifdef TRCRPT

static char *ntostr(n)
{
	static char buf[10];

	sprintf(buf,"%08x\n",n);
	return(buf);
}

static struct pidlist *getpl()
{

	return(MALLOC(1,struct pidlist));		/* zeroes the structure */
}

pidlist_init(list)
char *list;
{
	char *cp;
	struct pidlist *pp;
	int negfiltflg;

	if(list == 0)
		return;
	Pidflg++;
	for(cp = strtok(list,", \t\n"); cp; cp = strtok(0,", \t\n")) {
		if(*cp == '!') {
			Kpidflg = 1;
			negfiltflg = 1;
			cp++;
		} else {
			negfiltflg = 0;
		}
		pp = getpl();
		pp->pl_next = Pidlist;
		Pidlist = pp;
		pp->pl_negfiltflg = negfiltflg;
		if(streq(cp,"INTR"))
			pp->pl_pid = -1;
		else if(numchk(cp,0) <= 0)
			pp->pl_pidname = cp;
		else
			pp->pl_pid = strtol(cp,0,0);
		Debug("pidlist_init: install %s\n",
			pp->pl_pidname ? pp->pl_pidname : ntostr(pp->pl_pid));
	}
	for(pp = Pidlist; pp; pp = pp->pl_next)
		Debug("pidlist_init: %s\n",
			pp->pl_pidname ? pp->pl_pidname : ntostr(pp->pl_pid));
}

#endif


/*
 * Support -T option (thread filtering)
 */

struct tidlist {
	struct tidlist *pl_next;
	int                  pl_tid;
	int                  pl_negfiltflg;
};

static Ktidflg;
static Tidflg;					/* true if pidlist checking is on */
static struct tidlist *Tidlist;	/* head of list of tidlist structures */

static struct tidlist *getpltid();	/* allocate a tidlist structure */

int tidlist_chk()
{
	register struct tidlist *pp;

	if(Tidflg == 0)
		return(1);
	if (Threadflg) {
		for(pp = Tidlist; pp; pp = pp->pl_next) {
			if(pp->pl_tid == Tid) 
				break;
		}
	}
	else {
		for(pp = Tidlist; pp; pp = pp->pl_next) {
                        if(pp->pl_tid == Tid)
                                break;
                }
	}
	if(pp)
		return(pp->pl_negfiltflg ? 0 : 1);
	return(Ktidflg ? 1 : 0);
}


static struct tidlist *getpltid()
{

	return(MALLOC(1,struct tidlist));	/* zeroes the structure */
}

void tidlist_init(list)
char *list;
{
	char *cp;
	struct tidlist *pp;
	int negfiltflg;

	if(list == 0)
		return;
	Tidflg++;
	for(cp = strtok(list,", \t\n"); cp; cp = strtok(0,", \t\n")) {
		if(*cp == '!') {
			Ktidflg = 1;
			negfiltflg = 1;
			cp++;
		} else {
			negfiltflg = 0;
		}
		pp = getpltid();
		pp->pl_next = Tidlist;
		Tidlist = pp;
		pp->pl_negfiltflg = negfiltflg;
		pp->pl_tid = strtol(cp,0,0);
		Debug("tidlist_init: install %d \n",pp->pl_tid);
	}
	for(pp = Tidlist; pp; pp = pp->pl_next)
		Debug("tidlist_init: %d \n",pp->pl_tid);
}


Tlookuppninstall1(name)
char *name;
{

	Debug("lookuppninstall1(%s)\n",name);
	curru->u_currfile = stracpy(name);
	FSWTCH();
}

Tlookuppninstall2(vnode)
int vnode;
{

	if(v_install(vnode,curru->u_currfile) == 0)
		return;
	curru->u_currvnode = vnode;
	Debug("lookuppninstall2(%x) file='%s'\n",vnode,curru->u_currfile);
}

Tfdinstall(fd)
unsigned fd;
{
	struct u *up;

	Debug("Tfdinstall(%d)\n",fd);
	if(fd >= NFILES)
		return;
	curru->u_file[fd].f_vnode  = curru->u_currvnode;
	curru->u_file[fd].f_offset = 0;
}

char *fdlookup(fd)
unsigned fd;
{
	struct v *vp;

	if(fd >= NFILES)
		return(0);
	if((vp = v_lookup(curru->u_file[fd].f_vnode)) == 0) {
		Debug("fdlookup: no vp for vnode %x\n",curru->u_file[fd].f_vnode);
		return(0);
	}
	return(vp->v_filename);
}

char *vnodelookup(vnode)
{
	struct v *vp;

	if((vp = v_lookup(vnode)) == 0)
		return(0);
	return(vp->v_filename);
}

/*
 * template function
 */
Tpfsrdwrinstall1(vnode,inode)
{

	Debug("pfwrdwr1(%x,%x)\n",vnode,inode);
	curru->u_currvnode = vnode;
}

Tpfsrdwrinstall2(vaddr,vcount,sid,inode)
{

	va_install(vaddr,vcount,sid,curru->u_currvnode);
}

static struct va *getva()
{
	struct va *vap;

	return((struct va *)qget(&va_queue));
}

static struct va *va_install(vaddr,vcount,sid,vnode)
{
	register struct va *vap;

	if(vcount == 0)
		return(0);
	vap = getva();
	qinsert(&va_queue,vap);
	Debug("va_install vap=%x vaddr=%x vcount=%x sid=%x vnode=%x\n",
		vap,vaddr,vcount,sid,vnode);
	vap->va_startpage = vaddr/PAGESIZE;
	vap->va_endpage   = (vaddr + vcount - 1)/PAGESIZE;
	vap->va_sid       = sid;
	vap->va_vnode     = vnode;
	Debug("[%x,%x] %x\n",vap->va_startpage,vap->va_endpage);
	return(vap);
}

#define ISBETWEEN(start,a,end) \
	((start) <= (a) && (a) <= (end))

static struct va *va_pagelookup(vpage,sid)
{
	register struct va *vap;

	Debug("va_pagelookup(%x,%x)\n",vpage,sid);
	for(vap = TAIL(va); vap; vap = vap->va_prev) {
		Debug("[%x,%x] %x\n",vap->va_startpage,vap->va_endpage,vap->va_sid);
		if(vap->va_sid == sid &&
		   ISBETWEEN(vap->va_startpage,vpage,vap->va_endpage))
			return(vap);
	}
	return(0);
}

char *sidlookup(sid)
{
	struct va *vap;
	struct v *vp;

	for(vap = TAIL(va); vap; vap = vap->va_prev)
		if(vap->va_sid == sid)
			break;
	if(vap && (vp = v_lookup(vap->va_vnode)))
		return(vp->v_filename);
	return(0);
}

char *vpagelookup(vpage,sid)
{
	struct va *vap;
	struct v *vp;

	if((vap = va_pagelookup(vpage,sid)) && (vp = v_lookup(vap->va_vnode)))
		return(vp->v_filename);
	return(0);
}

#define BUFT_VMM 1
#define BUFT_LVM 2
#define BUFT_DD  3

Tvmmbufinstall(vpage,sid,buf)
{
	struct va *vap;
	struct b *bp;

	Debug("vmmbufinstall(%x,%x,%x)\n",vpage,sid,buf);
	if((vap = va_pagelookup(vpage,sid)) == 0)
		return;
	Debug("va_pagelookup returns %x. vnode=%x\n",vap,vap->va_vnode);
	if(vap->va_vnode == 0)
		return;
	if(bp = blookup(buf)) {
		bp->b_vnode = vap->va_vnode;
		bp->b_type  = BUFT_VMM;
	} else {
		binstall(buf,vap->va_vnode,BUFT_VMM);
	}
}

Tbufinstall(buf,newbuf)
{
	struct b *bp;

	if(bp = blookup(buf))
		binstall(newbuf,bp->b_vnode,BUFT_LVM);
	return;
}

char *buflookup(buf)
{
	struct b *bp;
	struct v *vp;

	Debug("buflookup(%x)\n",buf);
	if((bp = blookup(buf)) && (vp = v_lookup(bp->b_vnode)))
		return(vp->v_filename);
	if(bp)
		Debug("buflookup: return 0 (%x)\n",bp->b_vnode);
	else
		Debug("buflookup: return 0\n");
	return(0);
}

static struct b *blookup(buf)
{
	struct b *bp;

	for(bp = TAIL(b); bp; bp = bp->b_prev)
		if(bp->b_buf == buf)
			return(bp);
	return(0);
}

static struct b *binstall(buf,vnode,type)
{
	struct b *bp;

	if(vnode == 0)
		return;
	bp = getb();
	qinsert(&b_queue,bp);
	bp->b_buf   = buf;
	bp->b_vnode = vnode;
	bp->b_type  = type;
	return(bp);
}

static struct b *getb()
{

	return((struct b *)qget(&b_queue));
}

/*
 * unlink from list and place on free list
 */
static qfree(qp,p)
struct queue *qp;
struct queue_entry *p;
{

	if(p->qe_prev)
		p->qe_prev->qe_next = p->qe_next;
	if(p->qe_next)
		p->qe_next->qe_prev = p->qe_prev;
	if(qp->q_head == p)
		qp->q_head = p->qe_next;
	if(qp->q_tail == p)
		qp->q_tail = p->qe_prev;
	p->qe_next = qp->q_free;
	qp->q_free = p;
}

static struct queue_entry *qget(qp)
struct queue *qp;
{
	struct queue_entry *p;

	if(qp->q_free == 0) {		/* remove from end of queue */
		Debug("out of qentries\n");
		p = qp->q_tail;
		qp->q_tail = qp->q_tail->qe_prev;
		qp->q_tail->qe_next = 0;
		return(p);
	}
	p = qp->q_free;
	qp->q_free = qp->q_free->qe_next;
	return(p);
}

static qdump(qp)
struct queue *qp;
{
	struct queue_entry *p;

	if(!Debugflg)
		return;
	for(p = qp->q_tail; p; p = p->qe_prev)
		Debug("p is %x\n",p);
}

static qinsert(qp,p)
struct queue *qp;
struct queue_entry *p;
{

	Debug("qinsert %x head=%x tail=%x\n",p,qp->q_head,qp->q_tail);
	qdump(qp);
	if(qp->q_head) {
		qp->q_head->qe_prev = p;
		p->qe_next = qp->q_head;
	} else {
		qp->q_tail = p;
		p->qe_next = 0;
	}
	qp->q_head = p;
	p->qe_prev = 0;

}

char *currfile_lookup()
{
	struct u *up;

	if(curru->u_currfile)
		return(curru->u_currfile);
	return("-");
}

Tstarttimer(timerid1,timerid2)
{
	struct timer *tp;

	if((tp = timer_lookup(timerid1,timerid2)) == 0) {
		tp = gett();
		qinsert(&timer_queue,tp);
		Debug("timerinstall: %x id=%08x|%08x time=(%02d.%06d)\n",
			tp,tp->t_timerid1,tp->t_timerid2,tp->t_seconds,tp->t_nanoseconds);
	}
	tp->t_timerid1    = timerid1;
	tp->t_timerid2    = timerid2;
	tp->t_seconds     = Aseconds;
	tp->t_nanoseconds = Ananoseconds;
}

#define BILLION 1000000000

Tendtimer(timerid1,timerid2)
{
	struct timer *tp;
	int seconds,microseconds,nanoseconds;
	char buf[32];

	if((tp = timer_lookup(timerid1,timerid2)) == 0)
		return;
	seconds      = Aseconds - tp->t_seconds;
	nanoseconds  = Ananoseconds - tp->t_nanoseconds;
	if(nanoseconds < 0) {
		nanoseconds += BILLION;
		seconds--;
	}
	microseconds = nanoseconds / 1000;
	if(seconds > 0)
		sprintf(buf,"[%d.%06d sec]",seconds,microseconds);
	else
		sprintf(buf,"[%d usec]",microseconds);
	pr_string(buf);
	qfree(&timer_queue,tp);
}

static struct timer *timer_lookup(timerid1,timerid2)
{
	struct timer *tp;

	timerdump("timerlookup %x %x",timerid1,timerid2);
	for(tp = TAIL(timer); tp; tp = tp->t_prev)
		if(tp->t_timerid1 == timerid1 && tp->t_timerid2 == timerid2)
			return(tp);
	return(0);
}

static timerdump(fmt,a,b,c)
char *fmt;
{
	struct timer *tp;

	if(!Debugflg)
		return;
	if(fmt)
		Debug(fmt,a,b,c);
	for(tp = TAIL(timer); tp; tp = tp->t_prev)
		Debug("%x id=%08x|%08x time=(%02d.%06d)\n",
			tp,tp->t_timerid1,tp->t_timerid2,tp->t_seconds,tp->t_nanoseconds);
}

static struct timer *gett()
{

	return((struct timer *)qget(&timer_queue));
}
