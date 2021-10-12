static char sccsid[] = "@(#)12	1.11.2.14  src/bos/diag/da/sys/dsys.c, dasys, bos41J, 9511A_all 3/14/95 06:08:20";
/*
 * COMPONENT_NAME: DASYS  
 *
 * FUNCTIONS: check_rc, clean_up etc.
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <nl_types.h>
#include <limits.h>
#include <math.h>
#include <locale.h>
#include <sys/ioctl.h>
#include <sys/devinfo.h>
#include <sys/iplcb.h>
#include <sys/mdio.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/errids.h>
#include <signal.h>
#include <string.h>

#include "diag/class_def.h"	/* Includes odm definitions */
#include "diag/da.h"		/* The following header files contain */
#include "diag/diag_exit.h"	/* information that is specific to this */
#include "diag/tmdefs.h"	/* application. */
#include "diag/tm_input.h"
#include "diag/diago.h"
#include "diag/diag.h"
#include "diag/dcda_msg.h"
#include "diag/modid.h"
#include "sys_msg.h"

extern	getdainput();			/* diag. ctrl. info. to da */
int	get_eesr();
int	get_psr();

struct	tm_input 	tm_input;	/* tm_input holds input for this da */
struct  fru_bucket 	temp_frub;	/* used for substitution */
extern  nl_catd diag_catopen(char *, int);
nl_catd	fdes;				/* file descriptor for catalog file */
int     rc;

struct CuDv		*T_CuDv;
struct objlistinfo	c_info;
struct errdata          err_data;
int rc;
int fd;
int i;
int get_enddate(char *, char *, int);
int find_cpucard(int, fru_t *);
int int_proc_test();
uint mear;
uint mesr;
uint byte12;
uint bit4, bit8, bit9, bit10, bit11, bit12, bit13, bit14;
uint mear_mask_09, mear_mask_1;
uint beg_addr[12], end_addr[12];
char slot10[16][2], slot09[16];
char sal_simm1[] = { 'A','C','E','G' };
char sal_simm2[] = { 'B','D','F','H' };
char chk_simm1[] = { '1','3','2','4' };
char chk_simm2[] = { '5','7','6','8' };
char  *crit;
char mach_msg[512];
char give_goal[512];

#define DATE_SIZE 11
#define MEG_8	"8mb"
#define MEG_16	"16mb"
#define MEG_32	"32mb"
#define MEG_64	"64mb"
#define CARD_SIZE(a) ((((a & 0x0000ffff) ^ 0x0000ffff) +1) / 16)
#define SIMM_SIZE(a) (CARD_SIZE(a) / 8)
#define DEBUG    0
#define EXTCHK   0
#define EPOW   0

#define	MAXCPU	8

#define CPU_E1M 1	/*Pegasus cpucard type */
#define CPU_E1D 2
#define CPU_C1D 3
#define CPU_C4D 4


int	processor_present[MAXCPU];


struct fru_bucket l2error[] = {

    { "", FRUB1, 0x811, 0x102, R_L2,                    {
				 { 100, "L2 CACHE", " ", F_BLANK, NOT_IN_DB, EXEMPT },
							      },
    },
};

struct fru_bucket newsimm[] = {

    { "", FRUB1, 0x940, 0x100, R_BLANK, {
				 { 100, "SIMM", "", F_BLANK, NOT_IN_DB, EXEMPT },
		         	      },
    },
};

struct fru_bucket newcard_2simms[]=
{
  	{"", FRUB1, 0x818, 0x300, R_BLANK,
	 	{
		      {99, "SIMM", "", F_SIMM, NOT_IN_DB, EXEMPT},
		      {99, "SIMM", "", F_SIMM, NOT_IN_DB, EXEMPT},
		      {1, "MEMORY CARD", "", F_MEM, NOT_IN_DB, EXEMPT},
		},
	},
};

struct fru_bucket card_and_cpu[]=
{
  	{"", FRUB1, 0x818, 0x400, R_BLANK,
	 	{
		      {99, "MEMORY CARD", "", F_MEM, NOT_IN_DB, EXEMPT},
                      {1,  "", "", 0, DA_NAME, NONEXEMPT},
		},
	},
};

struct fru_bucket frub[] =
{
    {"", FRUB1, 0x811, 0x999, R_IO,  {
                                 {100, "ioplanar0", "", 0, DA_NAME, NONEXEMPT},
                                 },
    },
  
    {"", FRUB1, 0x811, 0x888, R_NIO, {
                                 {100, "niocard0", "", 0, DA_NAME, NONEXEMPT},
                                 },
    },
};

struct fru_bucket sal_frub[] =
{
    {"", FRUB1, 0x812, 0x900, R_CHECKSTOP,  {
                                 {100, "SIMM", "", F_BLANK, NOT_IN_DB, EXEMPT},
                                 {100, "SIMM", "", F_BLANK, NOT_IN_DB, EXEMPT},
                                 },
    },
};
  
struct fru_bucket tcw_frub[] =
{
    {"", FRUB1, 0x818, 0x990, R_TCW,  {
                                 {90, "TCW simm","00-0K",F_TCW,NOT_IN_DB,EXEMPT},
                                 {10, "", "", 0, DA_NAME, NONEXEMPT},
                                 },
    },

    {"", FRUB1, 0x818, 0x991, R_IO,  {
                                 {90, "ioplanar1","",0,CHILD_NAME,NONEXEMPT},
                                 {10, "", "", 0, DA_NAME, NONEXEMPT},
                                 },
    },
};
  
struct fru_bucket stopmach_frub[] =
{
    {"", FRUB1, 0x818, 0x101, R_CHECKSTOP,  {
                                 {100, "", "", 0, DA_NAME, NONEXEMPT},
                                 },
    },
  
    {"", FRUB1, 0x818, 0x100, R_18, {
                                 {100, "mem", "", F_MEM, NOT_IN_DB, EXEMPT},
                                 {100, "simms", "", F_MSIMMS, NOT_IN_DB, EXEMPT},
                                 },
    },

    {"", FRUB1, 0x818, 0x200, R_216, {
                                 {100, "mem", "", F_MEM, NOT_IN_DB, EXEMPT},
                                 {100, "simms", "", F_MSIMMS, NOT_IN_DB, EXEMPT},
                                 {100, "mem", "", F_MEM, NOT_IN_DB, EXEMPT},
                                 {100, "simms", "", F_MSIMMS, NOT_IN_DB, EXEMPT},
                                 },
    },
};

struct fru_bucket epow_frub[] =
{
    {"", FRUB1, 0x811, 0x991, R_EPOW,{
                   {100, "power supply", "00-00", F_HIGH, NOT_IN_DB, EXEMPT},
                                 },
    },
  
    {"", FRUB1, 0x811, 0x992, R_EPOW,{
                   {100, "power supply", "00-00", F_INT, NOT_IN_DB, EXEMPT},
                                 },
    },
  
    {"", FRUB1, 0x811, 0x993, R_EPOW,{
                   {100, "power supply", "00-00", F_OVER, NOT_IN_DB, EXEMPT},
                                 },
    },
  
    {"", FRUB1, 0x811, 0x994, R_EPOW,{
                   {100, "power supply", "00-00", F_PRIM, NOT_IN_DB, EXEMPT},
                                 },
    },
  
    {"", FRUB1, 0x811, 0x995, R_EPOW,{
                   {100, "fan-J49A", "00-00", F_FAN1, NOT_IN_DB, EXEMPT},
                                 },
    },
  
    {"", FRUB1, 0x811, 0x996, R_EPOW,{
                   {100, "fan-J49B", "00-00", F_FAN2, NOT_IN_DB, EXEMPT},
                                 },
    },
  
    {"", FRUB1, 0x811, 0x997, R_EPOW,{
                   {100, "fan-J49C", "00-00", F_FAN3, NOT_IN_DB, EXEMPT},
                                 },
    },
  
};

struct fru_bucket chk_frub[] =
{
    {"", FRUB1, 0x811, 0, 0,      {
                              {0, "", "", 0, DA_NAME, EXEMPT},    /*Memory*/
                              {0, "", "", 0, DA_NAME, NONEXEMPT}, /*CEC*/
                              },
    },

    {"", FRUB1, 0x811, 0, 0,      {
                              {60,"software","00-00",F_SOFT,NOT_IN_DB,EXEMPT},
                              {30, "", "", 0, DA_NAME, NONEXEMPT}, /*CEC*/
                              {10, "", "", 0, DA_NAME, EXEMPT},    /*Memory*/
                              },
    },

    {"", FRUB1, 0x811, 0, 0,      {
                              {50,"software","00-00",F_SOFT,NOT_IN_DB,EXEMPT},
                              {50, "", "", 0, DA_NAME, NONEXEMPT},  /*CEC*/
                              },
    },

    {"", FRUB1, 0x811, 0, 0,      {
                              {100, "", "", 0, DA_NAME, NONEXEMPT}, /*CEC*/
                              },
    },

    {"", FRUB1, 0x811, 0, 0,      {
                              {0, "", "", 0, DA_NAME, EXEMPT},    /*Memory*/
                              {0, "", "", 0, DA_NAME, EXEMPT},    /*Memory*/
                              {0, "", "", 0, DA_NAME, NONEXEMPT}, /*CEC*/
                              },
    },
};

struct fru_bucket iod_frub[] =
{
    {"", FRUB1, 0x716, 0x001, R_IOD,	{
                              {80, "ioplanar0", "", 0, CHILD_NAME, EXEMPT},
                              {20, "", "", 0, DA_NAME, EXEMPT},
                              },
    },
};


  
struct fru_bucket cpuipl_frub[] =
{
    {"", FRUB1, 0x716, 0x011, R_CPU,  {

                                 {80, "","",0,CHILD_NAME,EXEMPT},
                                 {20, "", "", 0, DA_NAME, EXEMPT},
                                 },
    },
};

struct fru_bucket smp_frub[] =
{
    {"", FRUB1, 0x820, 0x101, R_CPUT,  {

                                 {60, "","",0,DA_NAME,EXEMPT},
                                 {20, "", "", 0, CHILD_NAME, EXEMPT},
                                 {20, "", "", 0, CHILD_NAME, EXEMPT},
                                 },
    },
};

struct  CuAt            *cuat_type;
struct  CuAt            *cuat_cardec;

int	fd;			/* file descriptor for ioctl */
int	psr_content;
int	psr[32];
int	eesr_content;
int	eesr[32];
int     failure;
int	x;
int	validbit;
char startdate[DATE_SIZE];
char enddate[DATE_SIZE];
char	slot;
char	mslot, mslot0, mslot1;
char	*buffer;

IPL_DIRECTORY iplcb_dir;
IPL_INFO      iplcb_info;
NIO_POST_RESULTS      iplcb_post;
RAM_DATA	iplcb_ram_data;
RAM46_DATA	iplcb_ram46_data;
IOCC_POST_RESULTS iplcb_iocc;
SYSTEM_INFO	iplcb_sys;
PROCESSOR_DATA	iplcb_proc;

int	cpu_model;
main()
{

    int	    rc;			/* user's input */
    int	    i;
    int	    flag;		/* indicates when to break out the for loop */
    int	    model;		/* model = 0 for 0.9 machine, = 1 for 1.0 */
    int	    num;		/* integer value of short_timestamp */
    /*int	    salmon=FALSE;*/
    char    short_timestamp[6]; /* timestamp that has year,month and day only */

	struct CuDv *cudv;
	struct listinfo cudv_info;
	char criteria[256];

    MACH_DD_IO	l;
    MACH_DD_IO	m;
    MACH_DD_IO	n;
    MACH_DD_IO	o;
    MACH_DD_IO	p;
    MACH_DD_IO	s;
    MACH_DD_IO	pc;
    MACH_DD_IO	pr;

    setlocale(LC_ALL,"");

    DA_SETRC_STATUS(DA_STATUS_GOOD);
    DA_SETRC_ERROR(DA_ERROR_NONE);
    DA_SETRC_USER(DA_USER_NOKEY);
    DA_SETRC_TESTS(DA_TEST_SHR);
    DA_SETRC_MORE(DA_MORE_NOCONT);


    if ((fd = open("/dev/nvram", 0)) < 0) {
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        DA_EXIT();
    }

    l.md_incr = MV_BYTE;
    l.md_addr = 128;
    l.md_data = (char *) &iplcb_dir;
    l.md_size = sizeof(iplcb_dir);
    if (ioctl(fd, MIOIPLCB, &l)) {
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        DA_EXIT();
    }

/* ? - power pc information */
    pc.md_incr = MV_BYTE;
    pc.md_addr = iplcb_dir.ram_post_results_offset;
    pc.md_data = (char *) &iplcb_ram46_data;
    pc.md_size = sizeof(iplcb_ram46_data);
    if (ioctl(fd, MIOIPLCB, &pc)) {
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        DA_EXIT();
    }

/* s13 - salmon information */
    o.md_incr = MV_BYTE;
    o.md_addr = iplcb_dir.ram_post_results_offset;
    o.md_data = (char *) &iplcb_ram_data;
    o.md_size = sizeof(iplcb_ram_data);
    if (ioctl(fd, MIOIPLCB, &o)) {
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        DA_EXIT();
    }

/* s? - sacasil/TCW information */
    p.md_incr = MV_BYTE;
    p.md_addr = iplcb_dir.iocc_post_results_offset;
    p.md_data = (char *) &iplcb_iocc;
    p.md_size = sizeof(iplcb_iocc);
    if (ioctl(fd, MIOIPLCB, &p)) {
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        DA_EXIT();
    }

/* s1 */
    m.md_incr = MV_BYTE;
    m.md_addr = iplcb_dir.ipl_info_offset;
    m.md_data = (char *) &iplcb_info;
    m.md_size = sizeof(iplcb_info);
    if (ioctl(fd, MIOIPLCB, &m)) {
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        DA_EXIT();
    }

/* s3 */
    n.md_incr = MV_BYTE;
    n.md_addr = iplcb_dir.nio_dskt_post_results_offset;
    n.md_data = (char *) &iplcb_post;
    n.md_size = sizeof(iplcb_post);
    if (ioctl(fd, MIOIPLCB, &n)) {
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        DA_EXIT();
    }

/* system */
    s.md_incr = MV_BYTE;
    s.md_addr = iplcb_dir.system_info_offset;
    s.md_data = (char *) &iplcb_sys;
    s.md_size = sizeof(iplcb_sys);
    if (ioctl(fd, MIOIPLCB, &s)) {
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        DA_EXIT();
    }


    if (init_dgodm() != 0) {
        term_dgodm();
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        DA_EXIT();
    }
    rc = getdainput(&tm_input);
    if (rc != 0) {
        term_dgodm();
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        DA_EXIT();
    }
    fdes = diag_catopen(MF_SYS,0);
    if (tm_input.console == CONSOLE_TRUE)
        diag_asl_init(ASL_INIT_DEFAULT);

    /* diag_asl_msg("cache_line size = %d\n",iplcb_info.cache_line_size); */
    if (iplcb_info.cache_line_size == 64) /* 0.9 machine */
        model = 0;
    else  /* anything other than a 64 could be 128 or 256, which could be 1.0, 1.9 or 2.0.
	     But in any case, it would be a deskside or rack, so assigning model=1 is ok */
        model = 1;

    /* diag_asl_msg("model = %d\n",model); */

    /* Display "stand by" screen and do analysis */
    if (tm_input.console == CONSOLE_TRUE) {
        switch (tm_input.advanced) {
        case ADVANCED_TRUE:
            if (tm_input.loopmode != LOOPMODE_NOTLM)
                rc = diag_msg_nw(0x811004,fdes,ALOOP,ALTITLE,tm_input.lcount,
                                 tm_input.lerrors);
            else
                rc = diag_msg_nw(0x811003, fdes, ADVANCED, ATITLE);
            break;
        case ADVANCED_FALSE:
            if (tm_input.loopmode != LOOPMODE_NOTLM)
                rc = diag_msg_nw(0x811002, fdes, LOOP, LTITLE, tm_input.lcount,
                                 tm_input.lerrors);
            else
                rc = diag_msg_nw(0x811001, fdes, REGULAR, RTITLE);
            break;
        default:
            break;
        }
        sleep(2);
	rc = diag_asl_read(ASL_DIAG_KEYS_ENTER_SC, FALSE, NULL);
        check_rc(rc);
    }

    failure = 0;

    for (i=0; i<6; ++i)
        short_timestamp[i] = iplcb_info.ipl_ros_timestamp[i];
    num = atoi(short_timestamp);

    cpu_model = get_cpu_model(&i);

rc = salmon();

if (rc == 0) { /* non-RSC and non-POWER PC */
    /* IOCC POST RESULTS ANALYSIS - analyze only ros is after July 28, 1989 */
    if (num >= 890728) {
        if (iplcb_info.CRR_results != 0x00aa55ff) {
            strncpy(frub[0].dname,tm_input.dname,sizeof(frub[0].dname));
            addfrub(&frub[0]);
            failure = 1;
        }
    }

    /* NIO POST RESULTS ANALYSIS - analyze only ros is after Dec 22, 1989 */
    if (num >= 891222) {
        if (iplcb_post.g_nio_adapter_present != 0) {    /* adapter present */
            if (iplcb_post.g_nio_post_failed != 0) {    /* adapter fails */
                strncpy(frub[1].dname,tm_input.dname,sizeof(frub[1].dname));
                addfrub(&frub[1]);
                failure = 1;
            }
        }
    }
}

/******
diag_asl_msg("length = %d\n",iplcb_iocc.length);
diag_asl_msg("buid_21 = %d\n",iplcb_iocc.IOCC_POST.RESULTS.buid_21);
iplcb_iocc.IOCC_POST.RESULTS.buid_21 = 0x0123487f7;
******/



    if(IsPowerPC_SMP(cpu_model)) {

	/* iod post analyse */
	if(iplcb_iocc.length == 2 && iplcb_iocc.IOCC_POST.RESULTS.buid_21) {
		/* IOD_FAILED */
	    strncpy(iod_frub[0].dname,tm_input.dname,sizeof(iod_frub[0].dname));
	    strncpy(iod_frub[0].frus[1].fname,tm_input.dname,sizeof(iod_frub[0].frus[1].fname));
            addfrub(&iod_frub[0]);
	    failure = 1;
	}
    }
    else if (iplcb_iocc.length == 2 ) {
	switch (iplcb_iocc.IOCC_POST.RESULTS.buid_21&0x0000000f) {
	    case 0x0 : break;
	    case 0x4 :
	    case 0x5 :
	    case 0x7 : strncpy(tcw_frub[0].dname,tm_input.dname,sizeof(tcw_frub[0].dname));
		       insert_frub(&tm_input, &tcw_frub[0]);
		       tcw_frub[0].sn=0x818;
                       addfrub(&tcw_frub[0]);
		       failure = 1;
		       break;
	    default  : strncpy(tcw_frub[1].dname,tm_input.dname,sizeof(tcw_frub[1].dname));
		       insert_frub(&tm_input, &tcw_frub[1]);
		       tcw_frub[1].sn=0x818;
                       addfrub(&tcw_frub[1]);
		       failure = 1;
		       break;
	}
    }

   if(IsPowerPC_SMP(cpu_model)) {
	/* post processor analyse */
	int p;
	for(p=0;p<iplcb_sys.num_of_procs;p++)
	{	
		pr.md_incr = MV_BYTE;
		pr.md_addr = iplcb_dir.processor_info_offset +
				( p * sizeof(iplcb_proc ));
		pr.md_data = (char *) &iplcb_proc;
		pr.md_size = sizeof(iplcb_proc);
		if (ioctl(fd, MIOIPLCB, &pr)) {
			DA_SETRC_ERROR(DA_ERROR_OTHER);
			DA_EXIT();
		}
		processor_present[p]=iplcb_proc.processor_present;
		if(iplcb_proc.processor_present == -1) {
			/* Processor failed */
		int nb;
			temp_frub = cpuipl_frub[0];
			strncpy(temp_frub.dname,tm_input.dname,
				sizeof(temp_frub.dname));

			sprintf(criteria,"name = proc%d",p);
			cudv = odm_get_list(CuDv_CLASS,criteria,&cudv_info,3,1);
			if(cudv ==(struct CuDv *) -1 || cudv_info.num < 1 ) {
				continue;	/* cpu not available */
			}
			sprintf(temp_frub.frus[0].fname,"%s",cudv->parent);
			insert_frub(&tm_input, &temp_frub);

			nb = get_cpucard_vpd(cudv->parent);
			odm_free_list(cudv,&cudv_info);
			/* Initialize fru_bucket structure */
			switch(nb) {
			case CPU_E1M:
				temp_frub.sn= 0x716;
				temp_frub.rcode = 0x011;
				break;
			case CPU_C1D:
				temp_frub.sn= 0x716;
				temp_frub.rcode = 0x012;
				break;
			case CPU_E1D:
				temp_frub.sn = 0x810;
				temp_frub.rcode = 0x511;
				break;
			case CPU_C4D:
				temp_frub.sn = 0x810;
				temp_frub.rcode = 0x512;
				break;
			default :
				/* Unknown cpucard type */
				continue;
			}
			
			addfrub(&temp_frub);
			failure = 1;
		}
	}
	int_proc_test();
   }


#if EPOW
    /* EPOW FAILURE ANALYSIS - only in PD mode */
    if (tm_input.dmode == DMODE_PD) {
        psr_content = get_psr();
        for (i=0; i<32; ++i)
            psr[31-i] = (((psr_content >> i) & 1) ? 1 : 0);
        if ((psr[28] == 0) && (psr[29] == 1)) {  /* super-mini */
            switch(psr_content&0xf0000000) {
	    /*
                case 0x50000000 : 
                                  strncpy(epow_frub[0].dname,tm_input.dname,
                                          sizeof(epow_frub[0].dname));
                                  addfrub(&epow_frub[0]);
                                  failure = 1;
                                  break;
	    */
                case 0x60000000 : 
                                  strncpy(epow_frub[1].dname,tm_input.dname,
                                          sizeof(epow_frub[1].dname));
                                  addfrub(&epow_frub[1]);
                                  failure = 1;
                                  break;
	    /*
                case 0x70000000 : 
                                  strncpy(epow_frub[2].dname,tm_input.dname,
                                          sizeof(epow_frub[2].dname));
                                  addfrub(&epow_frub[2]);
                                  failure = 1;
                                  break;
                case 0x80000000 : 
                                  strncpy(epow_frub[3].dname,tm_input.dname,
                                          sizeof(epow_frub[3].dname));
                                  addfrub(&epow_frub[3]);
                                  failure = 1;
                                  break;
	    */
                case 0x90000000 : 
                                  strncpy(epow_frub[4].dname,tm_input.dname,
                                          sizeof(epow_frub[4].dname));
                                  addfrub(&epow_frub[4]);
                                  failure = 1;
                                  break;
                case 0xa0000000 : 
                                  strncpy(epow_frub[5].dname,tm_input.dname,
                                          sizeof(epow_frub[5].dname));
                                  addfrub(&epow_frub[5]);
                                  failure = 1;
                                  break;
                case 0xb0000000 : 
                                  strncpy(epow_frub[6].dname,tm_input.dname,
                                          sizeof(epow_frub[6].dname));
                                  addfrub(&epow_frub[6]);
                                  failure = 1;
                                  break;
                default         : break;
            }
        }
    }
#endif
	
    /* For the past 4 days, if there is no MACHINECHECK, then see if there is a
       CHECKSTOP - only in PD mode */
       /* For 1.0 machine */
	slot10[0][0] = 'D';
	slot10[1][0] = 'D';
	slot10[2][0] = 'D';
	slot10[3][0] = 'D';
	slot10[4][0] = 'B';
	slot10[5][0] = 'B';
	slot10[6][0] = 'B';
	slot10[7][0] = 'B';
	slot10[8][0] = 'C';
	slot10[9][0] = 'C';
	slot10[10][0] = 'C';
	slot10[11][0] = 'C';
	slot10[12][0] = 'A';
	slot10[13][0] = 'A';
	slot10[14][0] = 'A';
	slot10[15][0] = 'A';

	slot10[0][1] = 'H';
	slot10[1][1] = 'H';
	slot10[2][1] = 'H';
	slot10[3][1] = 'H';
	slot10[4][1] = 'F';
	slot10[5][1] = 'F';
	slot10[6][1] = 'F';
	slot10[7][1] = 'F';
	slot10[8][1] = 'G';
	slot10[9][1] = 'G';
	slot10[10][1] = 'G';
	slot10[11][1] = 'G';
	slot10[12][1] = 'E';
	slot10[13][1] = 'E';
	slot10[14][1] = 'E';
	slot10[15][1] = 'E';

	/* For 0.9 machine */
	slot09[0] = 'H';
	slot09[1] = 'H';
	slot09[2] = 'D';
	slot09[3] = 'D';
	slot09[4] = 'F';
	slot09[5] = 'F';
	slot09[6] = 'B';
	slot09[7] = 'B';
	slot09[8] = 'G';
	slot09[9] = 'G';
	slot09[10] = 'C';
	slot09[11] = 'C';
	slot09[12] = 'E';
	slot09[13] = 'E';
	slot09[14] = 'A';
	slot09[15] = 'A';

	/* For Llano, slot D is marked B, slot H is marked C */
	if ((iplcb_info.IO_planar_level_reg & 0x80000000) == 0x80000000) {
		slot09[0] = 'C';
		slot09[1] = 'C';
		slot09[2] = 'B';
		slot09[3] = 'B';
        }

    flag = 0;
    if (tm_input.dmode == DMODE_PD) {
        buffer = (char *) malloc (512);
	crit = (char *) malloc (1024);
	get_enddate(startdate, enddate, DATE_SIZE);
	sprintf(buffer, "-s %s -e %s", startdate, enddate);
	sprintf(crit, "-j %x %s",ERRID_MACHINECHECK,buffer);
	rc = error_log_get (INIT, crit, &err_data);
	if (rc == -1) {
            DA_SETRC_ERROR(DA_ERROR_OTHER);
	    clean_up();
	}
	else if (rc > 0) {
	    byte12 = (int) (err_data.detail_data[12]);
	    bit4 = (byte12 & 0x00000008)>>3;
	    if (bit4 == 1) {
	       strncpy(l2error[0].dname,tm_input.dname,sizeof(l2error[0].dname));
	       insert_frub(&tm_input, &l2error[0]);
	       l2error[0].sn=0x811;
               addfrub(&l2error[0]);
               DA_SETRC_STATUS(DA_STATUS_BAD);
	       clean_up();
	    }
	    else if ((salmon()==1) || (salmon()==2)){ /* salmon type and power machine */
		validbit = (int) (err_data.detail_data[0]);
		validbit &= 0x00000001; /* look at first bit of the first byte 
					   - the valid bit, ==1 means invalid */
		if (validbit == 0x00000000) { /* ==0 means entry is valid */
    		    mear = (int) (err_data.detail_data[1]);
    		    if ((mear >= 0x30) && (mear <= 0x33)) {
    		        temp_frub = sal_frub[0];
        	        sprintf(temp_frub.frus[0].floc,"00-0%c",sal_simm1[(mear-0x30)]);
        		sprintf(temp_frub.frus[1].floc,"00-0%c",sal_simm2[(mear-0x30)]);
			if (salmon()==1)
    		            temp_frub.rcode += iplcb_ram_data.simm_size[((mear-0x30)*2)];
			else if (salmon()==2)
    		            temp_frub.rcode += iplcb_ram46_data.simm_size[((mear-0x30)*2)];
        		strncpy(temp_frub.dname,tm_input.dname, sizeof(temp_frub.dname));
        		addfrub(&temp_frub);
                	DA_SETRC_STATUS(DA_STATUS_BAD);
        		clean_up();
    		    }
		}
	    }
	    else {
		mesr  = (int) (err_data.detail_data[13]); /*only need to look at bits 8-14*/
		bit8 = (mesr & 0x00000080)>>7;
		bit9 = (mesr & 0x00000040)>>6;
		bit10 = (mesr & 0x00000020)>>5;
		bit11 = (mesr & 0x00000010)>>4;
		bit12 = (mesr & 0x00000008)>>3;
		bit13 = (mesr & 0x00000004)>>2;
		bit14 = (mesr & 0x00000002)>>1;
    	        mear  = (int) (err_data.detail_data[16]<<24) +
    		        (int) (err_data.detail_data[17]<<16) +
    		        (int) (err_data.detail_data[18]<<8) +
    		        (int) (err_data.detail_data[19]) ;
                rc = error_log_get (TERMI, crit, &err_data);
                if (rc != 0) {
                    DA_SETRC_ERROR(DA_ERROR_OTHER);
                    clean_up();
                }
		mear_mask_1 = (mear & 0x00000030) >> 4;
		mear_mask_09 = (mear & 0x00000018) >> 3;
    	        /* add frus */
                for (i=0; i<16; ++i) {
    	        beg_addr[i] = iplcb_info.cre[i] & 0xffff0000;
    		end_addr[i] = beg_addr[i] + ((((iplcb_info.cre[i]&0x0000ffff)
    					        ^ 0x0000ffff) + 1) << 16);
    		if ((mear >= beg_addr[i]) && (mear <= end_addr[i])) {
    		    if (model == 0) {
    			mslot = slot09[i];
    			if (iplcb_info.cre[i-(i%2)] != (uint) 0xf0000000)
    			    x = SIMM_SIZE(iplcb_info.cre[i-(i%2)]) ;
    			if (iplcb_info.cre[(i-(i%2))+1] != (uint) 0xf0000000)
    		            x += SIMM_SIZE(iplcb_info.cre[(i-(i%2))+1]);

			if ((bit8==1) || (bit9==1) || (bit10==1))
			{
			    temp_frub = card_and_cpu[0];
    			    sprintf(temp_frub.frus[0].floc,"00-0%c",mslot);
			}
			else if ((bit11==1) || (bit12==1))
			{
    		            if (tm_input.console==CONSOLE_TRUE) {
    		                sprintf (give_goal, (char *) diag_cat_gets (fdes, MACHCHECK,
    		                MACHCHK_MSG, NULL));
			        menugoal(give_goal);
            	                DA_SETRC_STATUS(DA_STATUS_BAD);
			    }
			    clean_up();
		 	}
			else if (bit13==1)
			{
			    temp_frub = newcard_2simms[0];
    			    sprintf(temp_frub.frus[0].floc,"00-0%c-00-0%c",mslot,
				    chk_simm1[mear_mask_09]);
    			    sprintf(temp_frub.frus[1].floc,"00-0%c-00-0%c",mslot,
				    chk_simm2[mear_mask_09]);
    			    sprintf(temp_frub.frus[2].floc,"00-0%c",mslot);
			}
			else if (bit14==1)
			{
			    temp_frub = newsimm[0];
                	    temp_frub.rcode += 16 * (log(x)/0.69314718 +1);
    		            temp_frub.rcode += get_type(mslot);
    			    sprintf(temp_frub.frus[0].floc,"00-0%c-00-0%c",mslot,
				    chk_simm1[mear_mask_09]);
    		            strncpy(temp_frub.dname,tm_input.dname, sizeof(temp_frub.dname));
    		            addfrub(&temp_frub);
			    temp_frub = newsimm[0];
    			    sprintf(temp_frub.frus[0].floc,"00-0%c-00-0%c",mslot,
				    chk_simm2[mear_mask_09]);
			}

    		        /* For 1 meg simm will have a value of 16 or 10(in hex) 
    			   to be added to the rcode. 2 meg will be 20 (hex) and so on */
                	temp_frub.rcode += 16 * (log(x)/0.69314718 +1);
    		        temp_frub.rcode += get_type(mslot);

    		    }
    		    else {
    			mslot0 = slot10[i][0];
    			mslot1 = slot10[i][1];
    			if (iplcb_info.cre[i-(i%4)] != (uint) 0xf0000000)
    			    x = SIMM_SIZE(iplcb_info.cre[i-(i%4)]) ;
    			if (iplcb_info.cre[(i-(i%4))+1] != (uint) 0xf0000000)
    		            x += SIMM_SIZE(iplcb_info.cre[(i-(i%4))+1]);
    			if (iplcb_info.cre[(i-(i%4))+1] != (uint) 0xf0000000)
    		            x += SIMM_SIZE(iplcb_info.cre[(i-(i%4))+2]);
    			if (iplcb_info.cre[(i-(i%4))+1] != (uint) 0xf0000000)
    		            x += SIMM_SIZE(iplcb_info.cre[(i-(i%4))+3]);
    			x = x/2; /* x was the size of the entire extent, now is for 1 card */

			if ((bit8==1) || (bit9==1) || (bit10==1))
			{
			    temp_frub = card_and_cpu[0];
                	    temp_frub.rcode += 16 * (log(x)/0.69314718 +1);
    		            temp_frub.rcode += get_type(mslot0);
    			    sprintf(temp_frub.frus[0].floc,"00-0%c",mslot0);
    		            strncpy(temp_frub.dname,tm_input.dname, sizeof(temp_frub.dname));
    		            addfrub(&temp_frub);
			    temp_frub = card_and_cpu[0];
    			    sprintf(temp_frub.frus[0].floc,"00-0%c",mslot1);
			}
			else if ((bit11==1) || (bit12==1))
			{
    		            if (tm_input.console==CONSOLE_TRUE) {
    		                sprintf (give_goal, (char *) diag_cat_gets (fdes, MACHCHECK,
    		                MACHCHK_MSG, NULL));
			        menugoal(give_goal);
            	                DA_SETRC_STATUS(DA_STATUS_BAD);
			    }
			    clean_up();
		 	}
			else if (bit13==1)
			{
			    temp_frub = newcard_2simms[0];
                	    temp_frub.rcode += 16 * (log(x)/0.69314718 +1);
    		            temp_frub.rcode += get_type(mslot0);
    			    sprintf(temp_frub.frus[0].floc,"00-0%c-00-0%c",mslot0,
				    chk_simm1[mear_mask_1]);
    			    sprintf(temp_frub.frus[1].floc,"00-0%c-00-0%c",mslot0,
				    chk_simm2[mear_mask_1]);
    			    sprintf(temp_frub.frus[2].floc,"00-0%c",mslot0);
    		            strncpy(temp_frub.dname,tm_input.dname, sizeof(temp_frub.dname));
    		            addfrub(&temp_frub);
			    temp_frub = newcard_2simms[0];
    			    sprintf(temp_frub.frus[0].floc,"00-0%c-00-0%c",mslot1,
				    chk_simm1[mear_mask_1]);
    			    sprintf(temp_frub.frus[1].floc,"00-0%c-00-0%c",mslot1,
				    chk_simm2[mear_mask_1]);
    			    sprintf(temp_frub.frus[2].floc,"00-0%c",mslot1);
			}
			else if (bit14==1)
			{
			    temp_frub = newsimm[0];
                	    temp_frub.rcode += 16 * (log(x)/0.69314718 +1);
    		            temp_frub.rcode += get_type(mslot0);
    			    sprintf(temp_frub.frus[0].floc,"00-0%c-00-0%c",mslot0,
				    chk_simm1[mear_mask_1]);
    		            strncpy(temp_frub.dname,tm_input.dname, sizeof(temp_frub.dname));
    		            addfrub(&temp_frub);

			    temp_frub = newsimm[0];
                	    temp_frub.rcode += 16 * (log(x)/0.69314718 +1);
    		            temp_frub.rcode += get_type(mslot0);
    			    sprintf(temp_frub.frus[0].floc,"00-0%c-00-0%c",mslot0,
				    chk_simm2[mear_mask_1]);
    		            strncpy(temp_frub.dname,tm_input.dname, sizeof(temp_frub.dname));
    		            addfrub(&temp_frub);

			    temp_frub = newsimm[0];
                	    temp_frub.rcode += 16 * (log(x)/0.69314718 +1);
    		            temp_frub.rcode += get_type(mslot1);
    			    sprintf(temp_frub.frus[0].floc,"00-0%c-00-0%c",mslot1,
				    chk_simm1[mear_mask_1]);
    		            strncpy(temp_frub.dname,tm_input.dname, sizeof(temp_frub.dname));
    		            addfrub(&temp_frub);

			    temp_frub = newsimm[0];
    			    sprintf(temp_frub.frus[0].floc,"00-0%c-00-0%c",mslot1,
				    chk_simm2[mear_mask_1]);
			}

    		        /* For 1 meg simm will have a value of 16 or 10(in hex) 
    			   to be added to the rcode. 2 meg will be 20 (hex) and so on */
                	temp_frub.rcode += 16 * (log(x)/0.69314718 +1);
    		        temp_frub.rcode += get_type(mslot1);
    		    }
    		    strncpy(temp_frub.dname,tm_input.dname, sizeof(temp_frub.dname));
    		    addfrub(&temp_frub);
		    /*
    		    if ((tm_input.console==CONSOLE_TRUE)&&
    			(tm_input.advanced==ADVANCED_TRUE)) {
    		        sprintf (mach_msg, (char *) diag_cat_gets (fdes, MACHSTOP,
    		        MACH_MSG, NULL),temp_frub.rcode);
    		        menugoal(mach_msg);
    		    }
		    */
            	    DA_SETRC_STATUS(DA_STATUS_BAD);
    		    clean_up();
    		    }
                } /* end for loop */
            }
	}
	else { /* check stop analysis */
	    sprintf(buffer, "-s %s -e %s", startdate, enddate);
	    sprintf(crit, "-j %x %s",ERRID_CHECKSTOP,buffer);
	    rc = error_log_get (INIT, crit, &err_data);
	    if (rc == -1) {
                DA_SETRC_ERROR(DA_ERROR_OTHER);
	        clean_up();
	    }
	    else if (rc > 0) { /* fru callout */
		insert_frub(&tm_input,&stopmach_frub[0]);
		strncpy(stopmach_frub[0].dname,tm_input.dname,
			sizeof(stopmach_frub[0].dname));
		addfrub(&stopmach_frub[0]);
        	failure = 1;
	    }
            rc = error_log_get (TERMI, crit, &err_data);
            if (rc != 0) {
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                clean_up();
            }
	}
    }

#if EXTCHK
    /* EXTERNAL CHECK ANALYSIS - only in PD mode */
    flag = 0;
    if (tm_input.dmode == DMODE_PD) {
        eesr_content = get_eesr(model);
        for (i=0; i<32; ++i)
            eesr[31-i] = (((eesr_content >> i) & 1) ? 1 : 0);

        for (i=8; i<32; ++i) {
            if (eesr[i] == 1) {
                switch (i) {
		    case 8:
		    case 9:  if (model == 0)
                                 fill_frub(66,34,0x600,R_ECHK);
                             else
                                 fill_frub1(33,34,0x605,R_ECHK);
                             flag = 1;
                             break;
                    case 10:
		    case 11:
                    case 17: if (model == 0)
                                 fill_frub(90,10,0x610,R_ECHK);
                             else
                                 fill_frub1(45,10,0x615,R_ECHK);
                             flag = 1;
                             break;
                    case 22: temp_frub = chk_frub[3];
                             temp_frub.rcode = 0x620;
                             temp_frub.rmsg = R_ECHK;
                             strncpy(temp_frub.frus[0].fname,tm_input.dname,
                                     sizeof(temp_frub.frus[0].fname));
                             strncpy(temp_frub.dname,tm_input.dname,
                                     sizeof(temp_frub.dname));
                             addfrub(&temp_frub);
                             failure = 1;
                             flag = 1;
                             break;
                    case 23: temp_frub = chk_frub[2];
                             temp_frub.rcode = 0x630;
                             temp_frub.rmsg = R_ECHK;
                             strncpy(temp_frub.frus[1].fname,tm_input.dname,
                                     sizeof(temp_frub.frus[1].fname));
                             strncpy(temp_frub.dname,tm_input.dname,
                                     sizeof(temp_frub.dname));
                             addfrub(&temp_frub);
                             failure = 1;
                             flag = 1;
                             break;
                    case 24: if (model == 0)
                                 fill_frub(80,20,0x640,R_ECHK);
                             else
                                 fill_frub1(40,20,0x645,R_ECHK);
                             flag = 1;
                             break;
                    case 27: temp_frub = chk_frub[1];
                             temp_frub.frus[0].conf = 70;
                             temp_frub.frus[1].conf = 25;
                             temp_frub.frus[2].conf = 5;
                             temp_frub.rcode = 0x650;
                             temp_frub.rmsg = R_ECHK;
                             flag = 1;
                             if (slot == 'X') {
                                 temp_frub.rcode -= 0x400;
                                 strncpy(temp_frub.frus[2].fname,"memory card",
                                         sizeof(temp_frub.frus[2].fname));
                                 temp_frub.frus[2].fmsg = F_UNMEM;
                                 temp_frub.frus[2].fru_flag = NOT_IN_DB;
                             } else {
                                 sprintf(buffer,"parent=%s and connwhere=%c",
                                         tm_input.dname,slot);
                                 T_CuDv = get_CuDv_list(CuDv_CLASS,buffer,
                                                        &c_info,1,2);
                                 if (strcmp(T_CuDv->PdDvLn->type,MEG_8)==0) 
                                     temp_frub.rcode -= 0x200;
                                 if (strcmp(T_CuDv->PdDvLn->type,MEG_32)==0) 
                                     temp_frub.rcode += 0x200;
                                 strncpy(temp_frub.frus[2].fname,T_CuDv[0].name,
                                         sizeof(temp_frub.frus[2].fname));
                             } /* end if - else */
                             strncpy(temp_frub.frus[1].fname,tm_input.dname,
                                     sizeof(temp_frub.frus[1].fname));
                             strncpy(temp_frub.dname,tm_input.dname,
                                     sizeof(temp_frub.dname));
                             addfrub(&temp_frub);
                             failure = 1;
                             break;
		} /* end switch */
	    } /* end if */
            if (flag == 1) break;   /* break out of for loop */
        } /* end for loop */
    }
#endif
	
    if (failure == 1)
        DA_SETRC_STATUS(DA_STATUS_BAD);
    else
        DA_SETRC_STATUS(DA_STATUS_GOOD);

    clean_up();

}  /* main end */


/*
 * NAME: check_rc
 *                                                                    
 * FUNCTION: Check if the user has entered the Esc or Cancel key.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *    Called by the main program.
 *                                                                   
 * RETURNS: returns the input parameter, rc - the user's input to screen.
 */

int 
check_rc(rc)
    int	rc;			/* user's input from screen */
{
    if (rc == ASL_CANCEL) {
        term_dgodm();
        if (tm_input.console == CONSOLE_TRUE)
            diag_asl_quit();
        DA_SETRC_USER(DA_USER_QUIT);
        DA_EXIT();
    }

    if (rc == ASL_EXIT) {
        term_dgodm();
        if (tm_input.console == CONSOLE_TRUE)
            diag_asl_quit();
        DA_SETRC_USER(DA_USER_EXIT);
        DA_EXIT();
    }
    return (rc);
}

/*
 * NAME: clean_up
 *                                                                    
 * FUNCTION: Closing file descriptors that are opened before and return to 
 *           caller.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *    Called by the main program.
 *                                                                   
 * RETURNS: What this code returns (NONE, if it has no return value)
 */

clean_up()
{
    free(crit);
    free(buffer);
    catclose(fdes);
    if (fd > 0)
        close(fd);
    term_dgodm();
    if (tm_input.console == CONSOLE_TRUE)
        diag_asl_quit();
    DA_EXIT();
}

/*
 * NAME: get_psr
 *                                                                    
 * FUNCTION: Retrieve psr content from error log entry.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *    Called by the main program.
 *                                                                   
 * RETURNS:  contents of psr.
 */

int
get_psr()
{
    int i;
    int *e_psr;

    i = 0;
    e_psr = &i;
    crit = (char *) malloc(256);
    sprintf(crit, "-j 74533d1a %s", tm_input.date);
    rc = error_log_get (INIT, crit, &err_data);
    /* rc = error_log_get (INIT, "-N 'EPOW_SUS'", &err_data); */
    if (rc == -1) {
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        /* diag_asl_msg ("error_log_get INIT failed.\n"); */
        clean_up();
    }
    else if (rc > 0) {
	e_psr = (int *) &(err_data.detail_data[8]);
        /* diag_asl_msg("psr = %x\n",*e_psr); */
    }
    /* rc = error_log_get (TERMI, "-N 'EPOW_SUS'", &err_data); */
    rc = error_log_get (TERMI, crit, &err_data);
    if (rc != 0) {
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        /* diag_asl_msg ("error_log_get TERMI failed.\n"); */
        clean_up();
    }
    return (*e_psr);
} /* end get_psr */


/*
 * NAME: get_eesr
 *                                                                    
 * FUNCTION: Retrieve eesr content and determine failing memory slot by
 *           reading eear and configuration registers.         
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *    Called by the main program.
 *                                                                   
 * RETURNS: content of eesr.
 */

int
get_eesr(mmodel)
    int mmodel;
{
    unsigned int temp;
    int i;
    unsigned int starta0[8];
    unsigned int enda0[8];
    unsigned int starta1[8];
    unsigned int enda1[8];
    unsigned int creg[8];
    char mem_slot[8];

    int *e_eesr;
    int *e_eear;
    int *tp;

    i = 0;
    e_eesr = &i;
    e_eear = &i;
    slot = 'X';
    crit = (char *) malloc(256);
    sprintf(crit, "-j f7e70b81 %s", tm_input.date);
    rc = error_log_get (INIT, crit, &err_data);
    /* rc = error_log_get (INIT, "-N 'xxxxxx'", &err_data); */
    if (rc == -1) {
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        /* diag_asl_msg ("error_log_get INIT failed.\n"); */
        clean_up();
    }
    else if (rc > 0) {
	e_eesr = (int *) &(err_data.detail_data[0]);
	e_eear = (int *) &(err_data.detail_data[4]);
        /* diag_asl_msg("eesr = %x\n",*e_eesr);
           diag_asl_msg("eear = %x\n",*e_eear); */

        /* Assigning slots to mem_slot */
        mem_slot[0] = 'H';
        mem_slot[1] = 'D';
        mem_slot[2] = 'F';
        mem_slot[3] = 'B';
        mem_slot[4] = 'G';
        mem_slot[5] = 'C';
        mem_slot[6] = 'E';
        mem_slot[7] = 'A';
    
        for (i=0; i<8; ++i) {
            tp = (int *) &(err_data.detail_data[(i+3)*4]);
            creg[i] = *tp;
        } 
        
        if (mmodel == 0) {
            for (i=0; i<8; ++i) {
                temp = creg[i] & 0x0000ffff;         /* get size of extent */
                starta0[i] = creg[i] & (temp << 16); /* base address-too big */
                starta0[i] = starta0[i] >> 16;	     /* adjust base address */
                enda0[i] = starta0[i] + temp;
                if ((*e_eear >= starta0[i]) && (*e_eear <= enda0[i]))
                    slot = mem_slot[i];
            }
        }
        else if (mmodel == 1) {
            for (i=0; i<8; i+=2) {
                temp = creg[i] & 0x0000ffff;
                starta1[i] = creg[i] & (temp << 16);
                starta1[i] = starta1[i] >> 16;
                enda1[i] = starta1[i] + temp;
                if ((*e_eear >= starta1[i]) && (*e_eear <= enda0[i]))
                    slot = mem_slot[i];
            }
        } /* end if_else-if */

        /* If Llano, slots are different - D is marked B, H is marked C */
        if ((iplcb_info.IO_planar_level_reg&0xf0000000)==0x80000000) {
            if (slot == 'D')
                slot = 'B';
            else if (slot == 'H')
                slot = 'C';
            else
                slot = 'X';
        } /* end check Llano */
    } /* end ELA for machine check */

    /* rc = error_log_get (TERMI, "-N 'xxxxxx'", &err_data); */
    rc = error_log_get (TERMI, crit, &err_data);
    if (rc != 0) {
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        /* diag_asl_msg ("error_log_get TERMI failed.\n"); */
        clean_up();
    }

    return (*e_eesr);
} /* end get_eesr */

/*
 * NAME: fill_frub
 *                                                                    
 * FUNCTION: filling in the fields of the fru bucket and call addfrub to 
 *           report the frub. This routine is for 0.9 machine.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *    Called by the main program.
 *                                                                   
 * RETURNS: NONE
 */

fill_frub(conf1,conf2,rrcode,rrmsg)
int 	conf1;
int 	conf2;
int	rrcode;
int	rrmsg;
{
    failure = 1;
    temp_frub = chk_frub[0];
    temp_frub.rmsg = rrmsg;
    temp_frub.frus[0].conf = conf1;
    temp_frub.frus[1].conf = conf2;
    temp_frub.rcode = rrcode;
    if (slot == 'X') {
        temp_frub.rcode -= 0x400;
        strncpy(temp_frub.frus[0].fname,"memory card",
                sizeof(temp_frub.frus[0].fname));
        temp_frub.frus[0].fmsg = F_UNMEM;
        temp_frub.frus[0].fru_flag = NOT_IN_DB;
    } else {
        sprintf(buffer,"parent = %s and connwhere = %c",tm_input.dname,slot);
        T_CuDv = get_CuDv_list(CuDv_CLASS,buffer,&c_info,1,2);
        if (strcmp(T_CuDv->PdDvLn->type,MEG_8)==0) 
            temp_frub.rcode -= 0x200;
        if (strcmp(T_CuDv->PdDvLn->type,MEG_32)==0) 
            temp_frub.rcode += 0x200;
        strncpy(temp_frub.frus[0].fname,T_CuDv[0].name,
                sizeof(temp_frub.frus[0].fname));
    } /* end if - else */
    strncpy(temp_frub.frus[1].fname,tm_input.dname,
            sizeof(temp_frub.frus[1].fname));
    strncpy(temp_frub.dname,tm_input.dname, sizeof(temp_frub.dname));
    addfrub(&temp_frub);
} /* end fill_frub */
    
/*
 * NAME: fill_frub1
 *                                                                    
 * FUNCTION: filling in the fields of the fru bucket and call addfrub to 
 *           report the frub. This routine is for 1.0 machine.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *    Called by the main program.
 *                                                                   
 * RETURNS: NONE
 */

fill_frub1(conf1,conf2,rrcode,rrmsg)
int 	conf1;
int 	conf2;
int	rrcode;
int	rrmsg;
{
char    sslot;
 
    failure = 1;
    if ((slot=='X')||!((slot=='H')||(slot=='F')||(slot=='G')||(slot=='E'))) {
        temp_frub = chk_frub[0];
        temp_frub.rmsg = rrmsg;
        temp_frub.frus[0].conf = 2*conf1;
        temp_frub.frus[1].conf = conf2;
        temp_frub.rcode = rrcode - 0x405; /* -0x405 to give same srn as 0.9's */
        strncpy(temp_frub.frus[0].fname,"memory card",
                sizeof(temp_frub.frus[0].fname));
        temp_frub.frus[0].fmsg = F_UNMEM;
        temp_frub.frus[0].fru_flag = NOT_IN_DB;
        strncpy(temp_frub.frus[1].fname,tm_input.dname,
                sizeof(temp_frub.frus[1].fname));
    } else {
        temp_frub = chk_frub[4];
        temp_frub.rmsg = rrmsg;
        temp_frub.frus[0].conf = conf1;
        temp_frub.frus[1].conf = conf1;
        temp_frub.frus[2].conf = conf2;
        temp_frub.rcode = rrcode;
        sprintf(buffer,"parent = %s and connwhere = %c",tm_input.dname,slot);
        T_CuDv = get_CuDv_list(CuDv_CLASS,buffer,&c_info,1,2);
        if (strcmp(T_CuDv->PdDvLn->type,MEG_8)==0) 
            temp_frub.rcode -= 0x200;
        if (strcmp(T_CuDv->PdDvLn->type,MEG_32)==0) 
            temp_frub.rcode += 0x200;
        strncpy(temp_frub.frus[0].fname,T_CuDv[0].name,
                sizeof(temp_frub.frus[0].fname));
    
        switch (slot) {
            case 'H' : sslot = 'D';
                       break;
            case 'F' : sslot = 'B';
                       break;
            case 'G' : sslot = 'C';
                       break;
            case 'E' : sslot = 'A';
                       break;
            default  : break;
        }
    
        sprintf(buffer,"parent = %s and connwhere = %c",tm_input.dname,
                sslot);
        T_CuDv = get_CuDv_list(CuDv_CLASS,buffer,&c_info,1,2);
        strncpy(temp_frub.frus[1].fname,T_CuDv[0].name,
                sizeof(temp_frub.frus[1].fname));

        strncpy(temp_frub.frus[2].fname,tm_input.dname,
                sizeof(temp_frub.frus[2].fname));
    }
    strncpy(temp_frub.dname,tm_input.dname, sizeof(temp_frub.dname));
    addfrub(&temp_frub);
} /* end fill_frub1 */

int
salmon()
{
int     cuat_mod;
int     ipl_mod;
#if DEBUG
int     x;
char    *tbuf;
#endif

ipl_mod = get_cpu_model(&cuat_mod);

ipl_mod &= 0xff000000;

#if DEBUG
tbuf = (char *) malloc(1024);
sprintf(tbuf,"ipl_mod = %x\n",ipl_mod);
diag_asl_msg(tbuf);
sprintf(tbuf,"cuat_mod = %x\n",cuat_mod);
diag_asl_msg(tbuf);
x = IS_RSC(cuat_mod);
sprintf(tbuf,"x = %d\n",x);
diag_asl_msg(tbuf);
free (tbuf);
#endif

/* 0x02010041 and 0x02010045 - salmom (45 is in the field, 41 early internals)
   0x02010043        - cabeza
   0x08010046        - rainbow 3 
   0x02010047        - chaparral 
   or in general 0x02xxxxxx is RSC and 0x08xxxxxx is POWER PC */

if (ipl_mod == 0x02000000) 
    return (1);
else if (ipl_mod == 0x08000000)
    return(2);
else
    return(0);
}

/*
* NAME: get_type
*                                                                    
* FUNCTION: Checks the type (e.g. S1, S1.5, U1 etc) of a simm (memory card)
*                                                                    
* EXECUTION ENVIRONMENT:
*	This routine is called by the main program.
*
* RETURNS: a decimal number that indicates the type of memory
*
*	U1	0
*	S1	1
*	S1.5	2
*	S2.5	3
*	S3	4
*	S4	5
*	S5	6
*/

get_type(mem_slot)
char mem_slot;
{
    int  how_many;

    if (tm_input.dmode == DMODE_MS1)
        sprintf(buffer,"parent=%s and connwhere=%c and chgstatus=3", 
                tm_input.dname, mem_slot);
    else
        sprintf(buffer,"parent=%s and connwhere=%c and chgstatus != 3", 
                tm_input.dname, mem_slot);
    T_CuDv = get_CuDv_list(CuDv_CLASS,buffer,&c_info,1,1);
    cuat_type = (struct CuAt *)getattr(T_CuDv->name, "type", FALSE, &how_many);
    cuat_cardec = (struct CuAt *)getattr(T_CuDv->name, "cardec", FALSE, &how_many);

    if (strcmp(cuat_cardec->value,"0x0")==0)
	return(1); /* S1 */

    if  ( (strcmp(cuat_cardec->value,"0x20")==0)
        ||(strcmp(cuat_cardec->value,"0x21")==0)
        ||(strcmp(cuat_cardec->value,"0x22")==0)
        ||(strcmp(cuat_cardec->value,"0x23")==0)
        ||(strcmp(cuat_cardec->value,"0x24")==0)
        ||(strcmp(cuat_cardec->value,"0x25")==0)
        ||(strcmp(cuat_cardec->value,"0x26")==0)
        ||(strcmp(cuat_cardec->value,"0x27")==0)
        ||(strcmp(cuat_cardec->value,"0x28")==0)
        ||(strcmp(cuat_cardec->value,"0x29")==0)
        ||(strcmp(cuat_cardec->value,"0x2a")==0)
        ||(strcmp(cuat_cardec->value,"0x2b")==0)
        ||(strcmp(cuat_cardec->value,"0x2c")==0)
        ||(strcmp(cuat_cardec->value,"0x2d")==0)
        ||(strcmp(cuat_cardec->value,"0x2e")==0)
        ||(strcmp(cuat_cardec->value,"0x2f")==0) )
	    return(4); /* S3 */
    else if  ( (strcmp(cuat_cardec->value,"0x10")==0)
        ||(strcmp(cuat_cardec->value,"0x11")==0)
        ||(strcmp(cuat_cardec->value,"0x12")==0)
        ||(strcmp(cuat_cardec->value,"0x13")==0)
        ||(strcmp(cuat_cardec->value,"0x14")==0)
        ||(strcmp(cuat_cardec->value,"0x15")==0)
        ||(strcmp(cuat_cardec->value,"0x16")==0)
        ||(strcmp(cuat_cardec->value,"0x17")==0)
        ||(strcmp(cuat_cardec->value,"0x18")==0)
        ||(strcmp(cuat_cardec->value,"0x19")==0)
        ||(strcmp(cuat_cardec->value,"0x1a")==0)
        ||(strcmp(cuat_cardec->value,"0x1b")==0)
        ||(strcmp(cuat_cardec->value,"0x1c")==0)
        ||(strcmp(cuat_cardec->value,"0x1d")==0)
        ||(strcmp(cuat_cardec->value,"0x1e")==0)
        ||(strcmp(cuat_cardec->value,"0x1f")==0) )
	    return(3); /* S2.5 */
    else if (((strcmp(cuat_cardec->value,"0x0")==0)
            ||(strcmp(cuat_cardec->value,"0x1")==0)
            ||(strcmp(cuat_cardec->value,"0x2")==0)
            ||(strcmp(cuat_cardec->value,"0x3")==0)
            ||(strcmp(cuat_cardec->value,"0x4")==0)
            ||(strcmp(cuat_cardec->value,"0x5")==0)
            ||(strcmp(cuat_cardec->value,"0x6")==0)
            ||(strcmp(cuat_cardec->value,"0x7")==0)
            ||(strcmp(cuat_cardec->value,"0x8")==0)
            ||(strcmp(cuat_cardec->value,"0x9")==0)
            ||(strcmp(cuat_cardec->value,"0xa")==0)
            ||(strcmp(cuat_cardec->value,"0xb")==0)
            ||(strcmp(cuat_cardec->value,"0xc")==0)
            ||(strcmp(cuat_cardec->value,"0xd")==0)
            ||(strcmp(cuat_cardec->value,"0xe")==0)
            ||(strcmp(cuat_cardec->value,"0xf")==0))
	    &&(strcmp(cuat_type->value,"0x1c")==0) )
	        return(0);  /* U1 */
    else if (((strcmp(cuat_cardec->value,"0x0")==0)
            ||(strcmp(cuat_cardec->value,"0x1")==0)
            ||(strcmp(cuat_cardec->value,"0x2")==0)
            ||(strcmp(cuat_cardec->value,"0x3")==0)
            ||(strcmp(cuat_cardec->value,"0x4")==0)
            ||(strcmp(cuat_cardec->value,"0x5")==0)
            ||(strcmp(cuat_cardec->value,"0x6")==0)
            ||(strcmp(cuat_cardec->value,"0x7")==0)
            ||(strcmp(cuat_cardec->value,"0x8")==0)
            ||(strcmp(cuat_cardec->value,"0x9")==0)
            ||(strcmp(cuat_cardec->value,"0xa")==0)
            ||(strcmp(cuat_cardec->value,"0xb")==0)
            ||(strcmp(cuat_cardec->value,"0xc")==0)
            ||(strcmp(cuat_cardec->value,"0xd")==0)
            ||(strcmp(cuat_cardec->value,"0xe")==0)
            ||(strcmp(cuat_cardec->value,"0xf")==0)))
		return(2); /* S1.5 */
    else if (((strcmp(cuat_cardec->value,"0x30")==0)
            ||(strcmp(cuat_cardec->value,"0x31")==0)
            ||(strcmp(cuat_cardec->value,"0x32")==0)
            ||(strcmp(cuat_cardec->value,"0x33")==0)
            ||(strcmp(cuat_cardec->value,"0x34")==0)
            ||(strcmp(cuat_cardec->value,"0x35")==0)
            ||(strcmp(cuat_cardec->value,"0x36")==0)
            ||(strcmp(cuat_cardec->value,"0x37")==0)
            ||(strcmp(cuat_cardec->value,"0x38")==0)
            ||(strcmp(cuat_cardec->value,"0x39")==0)
            ||(strcmp(cuat_cardec->value,"0x3a")==0)
            ||(strcmp(cuat_cardec->value,"0x3b")==0)
            ||(strcmp(cuat_cardec->value,"0x3c")==0)
            ||(strcmp(cuat_cardec->value,"0x3d")==0)
            ||(strcmp(cuat_cardec->value,"0x3e")==0)
            ||(strcmp(cuat_cardec->value,"0x3f")==0)))
		return(5); /* S4 */
    else
	        return(6); /* S5 */
    
}

/*
* NAME: get_enddate
*
* FUNCTION:    This function obtains the current date and time.
*              It is returned in the buffer given as input.
*
* EXECUTION ENVIRONMENT:
*
*      This should describe the execution environment for this
*      procedure. For example, does it execute under a process,
*      interrupt handler, or both. Can it page fault. How is
*      it serialized.
*
* RETURNS: 0
*/

int get_enddate(
	char *start,
	char *end,
	int len)
{
	long    end_time_loc;
	long    start_time_loc;
	char    min[3];
	char    year[3];
	struct  tm *date;

	/* NOTE: the start and end strings are generated in a   */
	/* cumbersome manner because the SCCS software gets     */
	/* confused when certain symbols, e.g. 2/20/92, are present     */
	/* in the source code.  By splitting up the date conv-  */
	/* ersions, we avoid this confusion.                    */

	/* get todays date and time. Make sure to include */
	/* errors logged within the past minute.          */
	end_time_loc = time((time_t *)0) + 60;
	date = localtime(&end_time_loc);
	strftime(end, len, "%m%d%H", date);
	strftime(min, 3, "%M", date);
	strftime(year, 3, "%y", date);
	strcat(end, min);
	strcat(end, year);

	/* get yesterdays date and time.  Subtract 4 days */
	/*  4*24-hour (=4*86400 seconds) period.             */
	start_time_loc = end_time_loc - (4*86400);
	date = localtime(&start_time_loc);
	strftime(start, len, "%m%d%H", date);
	strftime(min, 3, "%M", date);
	strftime(year, 3, "%y", date);
	strcat(start, min);
	strcat(start, year);

	return(0);
	}

/*
*
*	Interprocessor Test
*
*/


/*
* NAME: fill_frucpu
*
* FUNCTION:
*
* EXECUTION ENVIRONMENT:
*
* RETURNS: -1 on error
*/
int fill_frucpu(int cpu1,int cpu2)

{
struct CuDv *cudv;
struct listinfo cudv_info;
char criteria[256];
char parent[16];
int type[2];

	if(!IsPowerPC_SMP(cpu_model)) return(-1);
	if(cpu2<0)	cpu2=cpu1;
        temp_frub = smp_frub[0];

	sprintf(criteria,"name = proc%d",cpu1);
	cudv = odm_get_list(CuDv_CLASS,criteria,&cudv_info,3,1);
	if(cudv ==(struct CuDv *) -1 || cudv_info.num <= 0 ) return(-1);

	/* Search for cpucard type */
	if((type[0] = get_cpucard_vpd(cudv->parent)) == 0 ) {
		odm_free_list(cudv,&cudv_info);
		return(-1);
	}
	sprintf(criteria,"parent = %s",cudv->parent);
	odm_free_list(cudv,&cudv_info);
	cudv = odm_get_list(CuDv_CLASS,criteria,&cudv_info,2,1);
	if(cudv ==(struct CuDv *) -1 || cudv_info.num <= 0) return(-1);
	strcpy(parent,cudv->parent);
	strcpy(temp_frub.frus[0].fname,tm_input.dname);
	strcpy(temp_frub.frus[1].fname,cudv->parent);
	odm_free_list(cudv,&cudv_info);

	sprintf(criteria,"name = proc%d",cpu2);
	cudv = odm_get_list(CuDv_CLASS,criteria,&cudv_info,3,1);
	if(cudv ==(struct CuDv *) -1 || cudv_info.num <= 0 ) return(-1);

	/* Search for cpucard type */
	if((type[1] = get_cpucard_vpd(cudv->parent)) == 0 ) {
		odm_free_list(cudv,&cudv_info);
		return(-1);
	}
	sprintf(criteria,"parent = %s",cudv->parent);
	odm_free_list(cudv,&cudv_info);
	cudv = odm_get_list(CuDv_CLASS,criteria,&cudv_info,2,1);
	if(cudv ==(struct CuDv *) -1 || cudv_info.num <= 0) return(-1);

	strcpy(temp_frub.frus[2].fname,cudv->parent);
	
	/*	SRN							*/
	/*	type[0]==E1M && type[1]==E1D	810-521			*/
	/*	type[0]==C1D && type[1]==E1D	810-522			*/
	/*	type[0]==E1D && type[1]==E1D	810-523			*/
	/*	type[0]==C4D && type[1]==C4D	810-524			*/
	/*	type[0]==E1D && type[1]==E1D	810-525	same cpucard	*/
	/*	type[0]==C4D && type[1]==C4D	810-526	same cpucard	*/
	/*	type[0]==E1M && type[1]==E1M	820-101			*/
	/*	type[0]==E1M && type[1]==C1D	820-102			*/
	/*	type[0]==C1D && type[1]==C1D	820-103			*/
	/*	type[0]==C1D && type[1]==C1D	820-104	same cpucard	*/

	if(!strcmp(parent,cudv->parent)) {
		/* Same parent	*/
		temp_frub.frus[1].conf=40;
		temp_frub.frus[2].conf=0;
	} else {
		temp_frub.frus[1].conf=20;
		temp_frub.frus[2].conf=20;
		type[0] = (type[0] * 100) + type[1];
	}

	odm_free_list(cudv,&cudv_info);

	switch( type[0]) {
		case CPU_E1D:
			temp_frub.sn= 0x810;
			temp_frub.rcode= 0x525;
			break;
		case CPU_C1D:
			temp_frub.sn= 0x820;
			temp_frub.rcode= 0x104;
			break;
		case CPU_C4D:
			temp_frub.sn= 0x810;
			temp_frub.rcode= 0x526;
			break;
		case (CPU_E1M * 100) + CPU_E1D:
		case (CPU_E1D * 100) + CPU_E1M:
			temp_frub.sn= 0x810;
			temp_frub.rcode= 0x521;
			break;
		case (CPU_C1D * 100) + CPU_E1D:
		case (CPU_E1D * 100) + CPU_C1D:
			temp_frub.sn= 0x810;
			temp_frub.rcode= 0x522;
			break;
		case (CPU_E1D * 100) + CPU_E1D:
			temp_frub.sn= 0x810;
			temp_frub.rcode= 0x523;
			break;
		case (CPU_C4D * 100) + CPU_C4D:
			temp_frub.sn= 0x810;
			temp_frub.rcode= 0x524;
			break;
		case (CPU_E1M * 100) + CPU_E1M:
			temp_frub.sn= 0x820;
			temp_frub.rcode= 0x101;
			break;
		case (CPU_E1M * 100) + CPU_C1D:
		case (CPU_C1D * 100) + CPU_E1M:
			temp_frub.sn= 0x820;
			temp_frub.rcode= 0x102;
			break;
		case (CPU_C1D * 100) + CPU_C1D:
			temp_frub.sn= 0x820;
			temp_frub.rcode= 0x103;
			break;
		default:
			return(-1);
	}

	failure = 1;
	return(0);
}

/*
* NAME: int_proc_test
*
* FUNCTION:	Interprocessor Test
*
* EXECUTION ENVIRONMENT:
*
* RETURNS: NONE
*/
int int_proc_test()
{
int i,m,p,status,rc;
int diag_ipl;
char command[80],opt1[80];
char *dadir;
char *diskenv;
char    *options[3];
pid_t pid,pid2;
	if(!IsPowerPC_SMP(cpu_model)) return(0);

	if((diskenv = (char *)getenv("DIAG_IPL_SOURCE")) == (char *)NULL)
		diag_ipl=0;
	else	diag_ipl=atoi(diskenv);
	if(diag_ipl == 1 || diag_ipl == 2)	return(0);

	p=0;
	for(i=0,m=1;i<MAXCPU;i++)
	{
		if(processor_present[i]==1) p|=m;
		m = m << 1;
	}
	
	if ((dadir = (char *)getenv("DADIR")) == NULL)
		dadir = DEFAULT_DADIR;
	sprintf(command,"%s/dmptu",dadir);
	sprintf(opt1,"-p%d",p);
	options[0]="dmptu";
	options[1]=opt1;
	options[2]=(char *)NULL;
	if((pid=fork())==0) { /* child process */
		execv(command, options);
		exit(0);	/* errors disregarded at this point	*/ 
	}
	if(pid<0) {		/* fork failed	*/
        	DA_SETRC_ERROR(DA_ERROR_OTHER);
        	DA_EXIT();
	}
	for(pid2=0;pid2<=0;)
	{
		pid2=waitpid(pid, &status,WNOHANG);
		if (tm_input.console == CONSOLE_TRUE)
		{
			rc = diag_asl_read(ASL_DIAG_KEYS_ENTER_SC, FALSE, NULL);
    			if (rc == ASL_CANCEL || rc == ASL_EXIT) {
        			term_dgodm();
            			diag_asl_quit();
        			DA_SETRC_USER(rc==ASL_CANCEL?DA_USER_QUIT:DA_USER_EXIT);
				if(pid2!=pid) {
					kill(pid,SIGTERM);
					pid2=waitpid(pid, &status,WNOHANG);
				}
       				DA_EXIT();
			}
    		}
		usleep(1000);
	}

	if(status & 0xFF) {
        	DA_SETRC_ERROR(DA_ERROR_OTHER);
        	DA_EXIT();
	}
	else if(status & 0xFF00) {
		int proc_def[2],k;
		proc_def[0]= -1;
		proc_def[1]= -1;
#if 0 /* this sequence does not work due to a compiler bug */
		for(i=0,m=0x100,k=0;i<8 && k<2;i++)
		{
			if((m & status) && (processor_present[i])) {
				proc_def[k]=i; /* error processor i */
				k++;
			}
			m= m << 1;
		}
#else /* this sequence does the same but works */
		m=0x100;
		k=0;
		for(i=0 ; i<8 ; i++)
		{
			if((m & status) && (processor_present[i])) {
				proc_def[k]=i; /* error processor i */
				k++;
				if(k >= 2) break;
			}
			m= m << 1;
		}
#endif /* compiler bug */
		/*	ERRORS			*/
		/*	60% MPB			*/
		/*	20% CPU proc_def[0]	*/
		/*	20% CPU proc_def[1]	*/
		if((fill_frucpu(proc_def[0],proc_def[1])) == 0) {
    			strncpy(temp_frub.dname,tm_input.dname,
				sizeof(temp_frub.dname));
			addfrub(&temp_frub);
		}
	}
	return(0);
}

/*
 * NAME: get_cpucard_vpd
 *                                                                    
 * FUNCTION:  Search in VPD for cpucard type
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS:	cpucard type
		0 if error
 */

int get_cpucard_vpd(char *cuvpd_name) {

char	criteria[256];
struct  listinfo	vpd_info;
struct	CuVPD	*vpd;
char	*pt;
int	cpu;


	cpu = 0;
	sprintf(criteria,"name=%s",cuvpd_name);
	vpd = odm_get_list( CuVPD_CLASS,criteria,&vpd_info,1,2);
	if( vpd != (struct CuVPD *) -1 && vpd_info.num >= 1) {
		if((pt=strstr(vpd->vpd,"*FN")) != 0) {
			pt+=4;
			if     (strncmp(pt,"E1M",3) == 0) cpu = CPU_E1M;
			else if(strncmp(pt,"E1D",3) == 0) cpu = CPU_E1D;
			else if(strncmp(pt,"C1D",3) == 0) cpu = CPU_C1D;
			else if(strncmp(pt,"C4D",3) == 0) cpu = CPU_C4D;
		}
	}
	odm_free_list(vpd,&vpd_info);
	return(cpu);
}

