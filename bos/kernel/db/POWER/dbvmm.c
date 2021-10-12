static char sccsid[] = "@(#)21  1.44  src/bos/kernel/db/POWER/dbvmm.c, sysdb, bos41J, 9511A_all 3/14/95 17:20:38";
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: 	pr_vmm, check_address
 *
 * ORIGINS: 27 83
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
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#include <sys/proc.h>
#include <sys/user.h>
#include <sys/systemcfg.h>
#include <sys/errids.h>
#include "vmm/vmsys.h"
#include "vmm/vmdefs.h"
#include "vmm/vm_mmap.h"
#include "vmm/vm_map.h"
#include "parse.h"                      /* Parser structure.            */
#ifdef _POWER_MP
#include <sys/ppda.h>
#endif /* _POWER_MP */

/*
 * Globals
 */
extern uint g_ksrval;                   /* kernel segment register value */
extern char *getterm();                 /* get keystroke from terminal   */

/*
 * Internal routines
 */
int pr_vmm();
static void pr_main();
static void pr_vmker();
static void pr_pfhdata();
static void pr_vmstat();
static void pr_pdtentry();
static void pr_scb();
static int pr_scbidx();
static void pr_pft();
static void pr_pte();
static void pr_apt();
static void pr_aptidx();
static void pr_pteidx();
static void pr_ptenfr();
static void pr_pftnfr();
static void pr_vmwait();
static void pr_vmaddr();
static void pr_ames();
static void pr_map();
static int pr_ame();
static int getdec();
static int gethex();
int check_address();
static void pr_vmchk();
static void gethwinfo();
static void gethwnfr();
static void pr_rmap();
static int nextlru();
static void pr_elog();

/*
 * Structure to return hardware page table information.
 */
struct hwinfo {
	uint rpn;
	uint sid;
	uint pno;
	uint key;
	uint ref;
	uint mod;
	uint swref;
	uint swmod;
	uint val;
	uint avpi;
	uint hsel;
	uint wimg;
} ;

/*
 * NAME: pr_vmm
 *
 * FUNCTION:    Provide a formatted display of VMM data structures.
 *
 * RETURN VALUE:  zero.
 */
int
pr_vmm(ps)
struct parse_out *ps;
{
  int save_ksr, save_vmmsr, save_ptasr, save_tempsr, save_kexsr;

  /*
   * FOR NOW, ONLY PINNED DATA STRUCTURES ARE DISPLAYED SO THE NORMAL
   * DEBUGGER METHOD FOR DISPLAYING MEMORY IS BYPASSED (i.e. the routines
   * don't use the Get_From_Memory routine to display the data, instead
   * the appropriate segments are made addressable and the data structures
   * are referenced as they normally are in the VMM).
   */

  /*
   * Set up to address kernel and VMM data
   */
  save_ksr    = mfsr(0);
  save_vmmsr  = mfsr(VMMSR);
  save_ptasr  = mfsr(PTASR);
  save_tempsr = mfsr(TEMPSR);
  save_kexsr  = mfsr(KERNXSR);
  mtsr(0, g_ksrval);
  mtsr(VMMSR, vmker.vmmsrval);
  mtsr(PTASR, vmker.ptasrval);
  mtsr(TEMPSR, vmker.dmapsrval);
  mtsr(KERNXSR, vmker.kexsrval);

  /*
   * Display selected VMM data structures
   */
  pr_main();

  /*
   * Restore segment registers to original values
   */
  mtsr(0, save_ksr);
  mtsr(VMMSR, save_vmmsr);
  mtsr(PTASR, save_ptasr);
  mtsr(TEMPSR, save_tempsr);
  mtsr(KERNXSR, save_kexsr);

  /*
   * Must return 0 so we don't exit the debugger.
   */
  return(0);
}

/*
 * NAME: pr_main
 *
 * FUNCTION:    VMM main display menu
 *
 * RETURN VALUE:  none.
 */
static
void
pr_main()
{
  char choice;

  for (;;)
  {
    printf("\nVMM\n\n");

    printf(" 1) Kernel Data\n");
    printf(" 2) Control Variables\n");
    printf(" 3) Statistics\n");
    printf(" 4) PDT\n");
    printf(" 5) SCBs\n");
    printf(" 6) PFT\n");
    printf(" 7) PTE\n");
    printf(" 8) APT\n");
    printf(" 9) VMM Wait Info\n");
    printf(" a) VMM Addresses\n");
    printf(" b) AMEs\n");
    printf(" c) VMM consistency check\n");
    printf(" d) RMAP\n");
    printf(" e) VMM error log\n");

    printf("\nEnter your choice or x to exit: ");

    if ((choice = *getterm()) == 'x')
      break;

    switch(choice)
    {
      case '1':
        pr_vmker();
        break;

      case '2':
        pr_pfhdata();
        break;

      case '3':
        pr_vmstat();
        break;

      case '4':
        pr_pdtentry();
        break;

      case '5':
        pr_scb();
        break;

      case '6':
        pr_pft();
        break;

      case '7':
        pr_pte();
        break;

      case '8':
        pr_apt();
        break;

      case '9':
        pr_vmwait();
        break;

      case 'a':
        pr_vmaddr();
        break;

      case 'b':
        pr_ames();
        break;

      case 'c':
        pr_vmchk();
        break;

      case 'd':
        pr_rmap();
        break;

      case 'e':
        pr_elog();
        break;

      default:
        break;
    }
  }

  printf("VMM data structure display exited.\n");
}

/*
 * NAME: pr_vmker
 *
 * FUNCTION:    Display VMM data kept in kernel segment
 *              (structure vmkerdata).
 *
 * RETURN VALUE:  none.
 */
static
void
pr_vmker()
{
  printf("\nVMM Kernel Data:\n\n");

  printf("total page frames : %08x\n", vmker.nrpages    );
  printf("free page frames  : %08x\n", vmker.numfrb     );
  printf("bad page frames   : %08x\n", vmker.badpages   );
  printf("rsvd page frames  : %08x\n", vmker.pfrsvdblks );
  printf("total pgsp blks   : %08x\n", vmker.numpsblks  );
  printf("free pgsp blks    : %08x\n", vmker.psfreeblks );
  printf("rsvd pgsp blks    : %08x\n", vmker.psrsvdblks );
  printf("kernel srval      : %08x\n", vmker.kernsrval  );
  printf("vmm srval         : %08x\n", vmker.vmmsrval   );
  printf("pta srval         : %08x\n", vmker.ptasrval   );
  printf("pgsp map srval    : %08x\n", vmker.dmapsrval  );
  printf("kernel ext srval  : %08x\n", vmker.kexsrval   );
  printf("ram disk srval    : %08x\n", vmker.ramdsrval  );
  printf("shadow srval      : %08x\n", vmker.ukernsrval );
  printf("fetch protect     : %08x\n", vmker.nofetchprot);
  printf("iplcb vaddr       : %08x\n", vmker.iplcbptr   );
  printf("extnd iplcb vaddr : %08x\n", vmker.iplcbxptr   );
  printf("hashbits          : %08x\n", vmker.hashbits   );
  printf("hashmask          : %08x\n", vmker.hashmask	);
  printf("swhashmask        : %08x\n", vmker.swhashmask	);
  printf("ahashmask         : %08x\n", vmker.ahashmask	);

  if (debpg() == FALSE)
    return;

  printf("\nVMM Kernel Data (continued):\n\n");

  printf("stoibits          : %08x\n", vmker.stoibits   );
  printf("stoi mask         : %08x\n", vmker.stoimask   );
  printf("stoinio mask      : %08x\n", vmker.stoinio   );
  printf("base conf srval   : %08x\n", vmker.bconfsrval );
  printf("num perm frames   : %08x\n", vmker.numperm    );
  printf("max perm frames   : %08x\n", vmker.maxperm    );
  printf("repage table size : %08x\n", vmker.rptsize    );
  printf("next free in rpt  : %08x\n", vmker.rptfree    );
  printf("repage decay rate : %08x\n", vmker.rpdecay    );
  printf("global repage cnt : %08x\n", vmker.sysrepage  );
  printf("num client frames : %08x\n", vmker.numclient  );
  printf("max client frames : %08x\n", vmker.maxclient  );
  printf("max file pageout  : %08x\n", vmker.maxpout   	);
  printf("min file pageout  : %08x\n", vmker.minpout	);
  printf("cache alignment   : %08x\n", vmker.cachealign	);
  printf("overflow count    : %08x\n", vmker.overflows 	);
  printf("reload count      : %08x\n", vmker.reloads   	);
  printf("compress frames   : %08x\n", vmker.numcompress);
  printf("compress noflush  : %08x\n", vmker.noflush);

  printf("\n\nPress ENTER to continue: ");
  getterm();
}

/*
 * NAME: pr_pfhdata
 *
 * FUNCTION:    Display VMM control variables
 *              (structure pfhdata).
 *
 * RETURN VALUE:  none.
 */
static
void
pr_pfhdata()
{
  int i;

  printf("\nVMM Control Variables:\n\n");

  printf("1st non-pinned page  : %08x\n", pf_firstnf       );
  printf("1st free sid entry   : %08x\n", pf_sidfree       );
  printf("1st delete pending   : %08x\n", pf_sidxmem       );
  printf("highest sid entry    : %08x\n", pf_hisid         );
  printf("1st free ame entry   : %08x\n", pf_amefree       );
  printf("1st del pending ame  : %08x\n", pf_amexmem       );
  printf("highest ame entry    : %08x\n", pf_hiame         );
  printf("fblru page-outs      : %08x\n", pf_numpout       );
  printf("fblru remote pg-outs : %08x\n", pf_numremote     );
  printf("non-ws page-outs     : %08x\n", pf_numpermio     );
  printf("frames not pinned    : %08x\n", pf_pfavail       );
  printf("next lru candidate   : %08x\n", pf_lruptr        );
  printf("v_sync cursor        : %08x\n", pf_syncptr       );
  printf("last pdt on i/o list : %08x\n", pf_iotail        );
  printf("num of paging spaces : %08x\n", pf_npgspaces     );
  printf("PDT last alloc from  : %08x\n", pf_pdtlast       );
  printf("PDT index of server  : %08x\n", pf_pdtserver     );
  printf("max pgsp PDT index   : %08x\n", pf_pdtmaxpg      );
  printf("sysbr protect key    : %08x\n", pf_kerkey        );
  printf("\n");

  if (debpg() == FALSE)
    return;

  printf("\nVMM Control Variables (continued):\n\n");

  printf("fblru minfree       : %08x\n", pf_minfree          );
  printf("fblru maxfree       : %08x\n", pf_maxfree          );
  printf("minperm             : %08x\n", pf_minperm          );
  printf("scb serial num      : %08x\n", pf_nxtscbnum        );
  printf("comp repage cnt     : %08x\n", pf_rpgcnt[RPCOMP]   );
  printf("file repage cnt     : %08x\n", pf_rpgcnt[RPFILE]   );
  printf("num of comp replaces: %08x\n", pf_nreplaced[RPCOMP]);
  printf("num of file replaces: %08x\n", pf_nreplaced[RPFILE]);
  printf("num of comp repages : %08x\n", pf_nrepaged[RPCOMP] );
  printf("num of file repages : %08x\n", pf_nrepaged[RPFILE] );
  printf("min page-ahead      : %08x\n", pf_minpgahead       );
  printf("max page-ahead      : %08x\n", pf_maxpgahead       );
  printf("SIGDANGER level     : %08x\n", pf_npswarn          );
  printf("SIGKILL level       : %08x\n", pf_npskill          );
  printf("next warn level     : %08x\n", pf_nextwarn         );
  printf("next kill level     : %08x\n", pf_nextkill         );
  printf("adj warn level      : %08x\n", pf_adjwarn          );
  printf("adj kill level      : %08x\n", pf_adjkill          );
  printf("\n");

  if (debpg() == FALSE)
    return;

  printf("\nVMM Control Variables (continued):\n\n");

  printf("cur pdt alloc    : %08x\n", pf_npdtblks      );
  printf("max pdt alloc    : %08x\n", pf_maxpdtblks    );
  printf("num i/o sched    : %08x\n", pf_numsched      );
  printf("freewake         : %08x\n", pf_freewake      );
  printf("lru index        : %08x\n", pf_lruidx        );
  printf("lru skip         : %08x\n", pf_skiplru       );
  printf("free frame wait  : %08x\n", pf_freewait      );
  printf("device i/o wait  : %08x\n", pf_devwait       );
  printf("extend XPT wait  : %08x\n", pf_extendwait    );
  printf("buf struct wait  : %08x\n", pf_bufwait       );
  printf("inh/delete wait  : %08x\n", pf_deletewait    );
  printf("disk quota wait  : %08x\n", pf_dqwait        );

  printf("\nPress ENTER to continue: ");
  getterm();
}

/*
 * NAME: pr_vmstat
 *
 * FUNCTION:    Display VMM statistics
 *
 * RETURN VALUE:  none.
 */
static
void
pr_vmstat()
{
  printf("\nVMM Statistics:\n\n");

  printf("page faults              : %08x\n", vmpf_pgexct      );
  printf("page reclaims            : %08x\n", vmpf_pgrclm      );
  printf("pages paged in           : %08x\n", vmpf_pageins     );
  printf("pages paged out          : %08x\n", vmpf_pageouts    );
  printf("paging space page ins    : %08x\n", vmpf_pgspgins    );
  printf("paging space page outs   : %08x\n", vmpf_pgspgouts   );
  printf("backtracks               : %08x\n", vmpf_backtrks    );
  printf("lockmisses               : %08x\n", vmpf_lockexct    );
  printf("zero filled pages        : %08x\n", vmpf_zerofills   );
  printf("executable filled pages  : %08x\n", vmpf_exfills     );
  printf("pages examined by clock  : %08x\n", vmpf_scans       );
  printf("clock hand cycles        : %08x\n", vmpf_cycles      );
  printf("page steals              : %08x\n", vmpf_pgsteals    );
  printf("free frame waits         : %08x\n", vmpf_freewts     );
  printf("extend XPT waits         : %08x\n", vmpf_extendwts   );
  printf("pending I/O waits        : %08x\n", vmpf_pendiowts   );
  printf("start I/Os               : %08x\n", vmpf_numsios     );
  printf("iodones                  : %08x\n", vmpf_numiodone   );

  printf("\nPress ENTER to continue: ");
  getterm();
}

/*
 * NAME: pr_pdtentry
 *
 * FUNCTION:    Display paging device table
 *              (structure pdtentry).
 *
 * RETURN VALUE:  none.
 */
static
void
pr_pdtentry()
{
  int i;
  int choice;

  printf("\nVMM PDT\n\n");

  printf("Enter the PDT index (0-%x): ",PDTSIZE-1);

  choice = gethex();

  for (i=choice; i>=0 && i<PDTSIZE; i++)
  {
    printf("\nPDT entry %02x of %02x\n\n", i, PDTSIZE-1);

    printf("type of device: ");
    switch(pdt_type(i))
    {
      case 0:
        printf("FREE\n");
        break;

      case D_PAGING:
        printf("PAGING\n");
        break;

      case D_FILESYSTEM:
        printf("FILESYSTEM\n");
        break;

      case D_REMOTE:
        printf("REMOTE\n");
        break;

      case D_LOGDEV:
        printf("LOG\n");
        break;

      default:
        printf("%08x\n", pdt_type(i));
        break;
    }

    printf("next pdt on i/o list  : %08x\n", pdt_nextio(i)  );
    printf("dev_t or strategy ptr : %08x\n", pdt_device(i)  );
    printf("last frame w/pend I/O : %08x\n", pdt_iotail(i)  );
    printf("free buf_struct list  : %08x\n", pdt_bufstr(i)  );
    printf("total buf structs     : %04x\n", pdt_nbufs(i)   );
    printf("available (PAGING)    : %04x\n", pdt_avail(i)   );
    printf("disk map srval        : %08x\n", pdt_dmsrval(i) );
    printf("i/o's not finished    : %08x\n", pdt_iocnt(i)   );

    printf("\nPress ENTER to display next entry, x to exit: ");

    if (*getterm()=='x')
      return;
  }
}

/*
 * NAME: pr_scb
 *
 * FUNCTION:    Display VMM segment control blocks
 *              (structure scb).
 *
 * RETURN VALUE:  none.
 */
static
void
pr_scb()

{
  int sidx, x, key, cnt;
  char choice;

  for (;;)
  {
    printf("\nVMM SCBs\n\n");

    printf("Select the scb to display by:\n\n");

    printf(" 1) index \n");
    printf(" 2) sid   \n");
    printf(" 3) srval \n");
    printf(" 4) search on sibits \n");
    printf(" 5) search on npsblks \n");
    printf(" 6) search on npages \n");
    printf(" 7) search on npseablks\n\n");

    printf("Enter your choice or x to exit: ");

    if ((choice = *getterm()) == 'x')
      break;

    switch(choice)
    {
      case '1':
        do
        {
          printf("\nEnter the index (in hex): ");
          sidx = gethex();
          if (sidx >=0 && sidx < pf_hisid)
	  {
	    for(x=sidx; x<pf_hisid; x++)
	    {
              if (pr_scbidx(x))
		break;
	    }
	  }
          else
            printf("\nIndex must be >= 0 and < %08x (pf_hisid)\n", pf_hisid);
        } while (sidx < 0 || sidx >= pf_hisid);
        break;

      case '2':
        do
        {
          printf("\nEnter the sid (in hex): ");
          x = gethex();
          sidx = STOI(x);
          if (sidx >=0 && sidx < pf_hisid)
	  {
	    for(x=sidx; x<pf_hisid; x++)
	    {
              if (pr_scbidx(x))
		break;
	    }
	  }
          else
          {
            printf("\nThat sid maps to index %08x\n", sidx);
            printf("\nThe index must be >= 0 and < %08x (pf_hisid)\n",
		pf_hisid);
          }
        } while (sidx < 0 || sidx >= pf_hisid);
        break;

      case '3':
        do
        {
          printf("\nEnter the srval (in hex): ");
          x = gethex();
          sidx = STOI(SRTOSID(x));
          if (sidx >=0 && sidx < pf_hisid)
	  {
	    for(x=sidx; x<pf_hisid; x++)
	    {
              if (pr_scbidx(x))
		break;
	    }
	  }
          else
          {
            printf("\nThat srval maps to index %08x\n", sidx);
            printf("\nThe index must be >= 0 and < %08x (pf_hisid)\n",
		pf_hisid);
          }
        } while (sidx < 0 || sidx >= pf_hisid);
        break;

      case '4':
        printf("\nFind all scbs whose sibits match (in hex): ");
        key = gethex();
	cnt = 0;
	for(x=0; x<pf_hisid; x++)
	{
	  if ((scb_sibits(x) & key) == key)
	  {
	    cnt++;
            if (pr_scbidx(x))
	      break;
	  }
	}
	printf("\n%x (hex) matches found with sibits %08x.\n", cnt, key);
	printf("\nPress ENTER to continue: ");
	getterm();
        break;

      case '5':
        printf("\nFind all scbs whose npsblks is greater than (in hex): ");
        key = gethex();
	cnt = 0;
	for(x=0; x<pf_hisid; x++)
	{
	  if (scb_wseg(x) && !scb_delpend(x) && scb_npsblks(x) > key)
	  {
	    cnt++;
            if (pr_scbidx(x))
	      break;
	  }
	}
	printf("\n%x (hex) matches found with npsblks > %08x.\n", cnt, key);
	printf("\nPress ENTER to continue: ");
	getterm();
        break;

      case '6':
        printf("\nFind all scbs whose npages is greater than (in hex):");
        key = gethex();
	cnt = 0;
	for(x=0; x<pf_hisid; x++)
	{
	  if (scb_npages(x) > key)
	  {
	    cnt++;
            if (pr_scbidx(x))
	      break;
	  }
	}
	printf("\n%x (hex) matches found with npages > %08x.\n", cnt, key);
	printf("\nPress ENTER to continue: ");
	getterm();
	break;

      case '7':
        printf("\nFind all scbs whose npseablks is greater than (in hex):");
        key = gethex();
	cnt = 0;
	for(x=0; x<pf_hisid; x++)
	{
	  if (scb_wseg(x) && ( scb_npseablks(x) > key))
	  {
	    cnt++;
            if (pr_scbidx(x))
	      break;
	  }
	}
	printf("\n%x (hex) matches found with npseablks > %08x.\n", cnt, key);
	printf("\nPress ENTER to continue: ");
	getterm();
	break;

      default:
        break;
    }
  }
}

/*
 * NAME: pr_scbidx
 *
 * FUNCTION:    Display a VMM segment control block
 *              (structure scb).
 *
 * RETURN VALUE:  none.
 */
static
int
pr_scbidx(idx)

int idx;
{

    printf("\nVMM SCB Index %08x of %08x  Segment ID: %08x\n", idx,
	   pf_hisid-1, ITOS(idx,0));

    if (scb_wseg(idx))
    {
      printf("WORKING STORAGE SEGMENT\n");
      printf("parent sid             : %08x\n", scb_parent(idx) );
      printf("left child sid         : %08x\n", scb_left(idx)   );
      printf("right child sid        : %08x\n", scb_right(idx)  );
      printf("extent of growing down : %08x\n", scb_minvpn(idx) );
      printf("last page user region  : %08x\n", scb_sysbr(idx)  );
      printf("up limit               : %08x\n", scb_uplim(idx)  );
      printf("down limit             : %08x\n", scb_downlim(idx));
      printf("number of pgsp blocks  : %08x\n", scb_npsblks(idx));
      printf("number of epsa blocks  : %08x\n", scb_npseablks(idx));
    }
    else if (scb_logseg(idx))
    {
      printf("LOG SEGMENT\n");
      printf("index in pdt           : %08x\n", scb_devid(idx)  );
      printf("I/O level              : %08x\n", scb_iolev(idx)  );
      printf("I/O lvl wait list anch : %08x\n", scb_iowait(idx) );
      printf("size in bytes of log   : %08x\n", scb_logsize(idx));
      printf("smallest log diff      : %08x\n", scb_logcur(idx) );
      printf("oldest logval          : %08x\n", scb_logsync(idx));
      printf("last logval v_setlog   : %08x\n", scb_loglast(idx));
    }
    else if (scb_pseg(idx))
    {
      printf("PERSISTENT STORAGE SEGMENT\n");
      printf("index in pdt           : %08x\n", scb_devid(idx)  );
      printf("I/O level              : %08x\n", scb_iolev(idx)  );
      printf("I/O lvl wait list anch : %08x\n", scb_iowait(idx) );
      printf("if not log, gnode ptr  : %08x\n", scb_gnptr(idx)  );
      printf("# of new disk blocks   : %08x\n", scb_newdisk(idx));
      printf("if not log, log's sidx : %08x\n", scb_logsidx(idx));
      printf("allocation group size  : %08x\n", scb_agsize(idx));
      printf("# of pages to prepage  : %02x\n", scb_npgahead(idx));
      printf("last hidden or faulted : %06x\n", scb_lstpagex(idx));
    }
    else if (scb_clseg(idx))
    {
      printf("CLIENT SEGMENT\n");
      printf("index in pdt           : %08x\n", scb_devid(idx)  );
      printf("I/O level              : %08x\n", scb_iolev(idx)  );
      printf("I/O lvl wait list anch : %08x\n", scb_iowait(idx) );
      printf("if not log, gnode ptr  : %08x\n", scb_gnptr(idx)  );
      printf("I/O delete waitor      : %08x\n", scb_delwait(idx) );
      printf("wait for change key    : %08x\n", scb_waitlist(idx));
      printf("# of pages to prepage  : %02x\n", scb_npgahead(idx));
      printf("last hidden or faulted : %06x\n", scb_lstpagex(idx));
    }
    else if (scb_mseg(idx))
    {
      printf("MAPPING SEGMENT\n");
      printf("ame hint               : %08x\n", scb_ame(idx));
      printf("ame start address      : %08x\n", scb_start(idx));
    }
    else
    {
      printf("UNDEFINED SEGMENT TYPE\n");
    }

    printf("default storage key : %01x\n", scb_defkey(idx));
    if (scb_valid(idx))    printf("> valid\n");
    if (scb_cow(idx))      printf("> copy-on-write\n");
    if (scb_jseg(idx))     printf("> journaled\n");
    if (scb_defseg(idx))   printf("> deferred update\n");
    if (scb_system(idx))   printf("> system segment\n");
    if (scb_privseg(idx))  printf("> process private segment\n");
    if (scb_ptaseg(idx))   printf("> PTA segment\n");
    if (scb_inoseg(idx))   printf("> .inodes segment\n");
    if (scb_intrseg(idx))  printf("> interruptible segment\n");
    if (scb_delpend(idx))  printf("> delete pending\n");
    if (scb_delete(idx))   printf("> delete in progress\n");
    if (scb_iodelete(idx)) printf("> delete when i/o done\n");
    if (scb_combit(idx))   printf("> commit in progress\n");
    if (scb_chgbit(idx))   printf("> segment modified\n");
    if (scb_compseg(idx))  printf("> computational segment\n");
    if (scb_eio(idx))      printf("> I/O write error occurred\n");
    if (scb_psearlyalloc(idx))  printf("> early ps alloc segment\n");
    if (scb_sparse(idx))  printf("> sparse segment\n");
    if (scb_shrlib(idx))  printf("> global shared library segment\n");

    printf("next free list/mmap cnt  : %08x\n", scb_free(idx)     );
    printf("xmem attach count        : %04x\n", scb_xmemcnt(idx)  );
    printf("non-fblu pageout count   : %04x\n", scb_npopages(idx) );
    printf("address of XPT root      : %08x\n", scb_vxpto(idx)    );
    printf("pages in real memory     : %08x\n", scb_npages(idx)   );
    printf("page frame at head       : %08x\n", scb_sidlist(idx)  );
    printf("max assigned page number : %08x\n", scb_maxvpn(idx)   );

    printf("\nPress ENTER to display next scb, x to exit: ");
    if (*getterm()=='x')
      return(1);
    else	
      return(0);
}

/*
 * NAME: pr_pft
 *
 * FUNCTION:    Display VMM PFT entries
 *
 * RETURN VALUE:  none.
 */
static
void
pr_pft()

{
  char choice;
  int nfr, sid, pno, hash, lru;
  int x, key, cnt, lcnt, first, last;

  for (;;)
  {
    printf("\nVMM PFT\n\n");

    printf("Select the PFT entry to display by:\n\n");

    printf(" 1) page frame #\n");
    printf(" 2) s/w hash (sid,pno)\n");
    printf(" 3) h/w hash (sid,pno)\n");
    printf(" 4) search on swbits \n");
    printf(" 5) search on pincount \n");
    printf(" 6) search on xmemcnt \n\n");

    printf("Enter your choice or x to exit: ");

    if ((choice = *getterm()) == 'x')
      break;

    switch(choice)
    {
      case '1':
	do
	{
          printf("\n\nEnter the page frame number (in hex): ");
          nfr = gethex();
	  lru = nextlru(nfr);
	  if (nfr >=0 && nfr < vmker.nrpages && lru == nfr)
          {
            pr_pftnfr(nfr);
	    printf("\nPress ENTER to continue: ");
	    getterm();
          }
          else
          {
	      if (lru == nfr)
		    printf("\n\nThe page frame must be >= 0 and < %08x\n",
				vmker.nrpages);
	      else
		    printf("\n\nPage frame %x isn't valid memory.\n", nfr);
          }
        } while (nfr < 0 || nfr > vmker.nrpages-1);

        break;

      case '2':
      case '3':
        printf("\n\nEnter the sid (in hex): ");
        sid = gethex();

        printf("\n\nEnter the pno (in hex): ");
        pno = gethex();

	if (choice == '2')
		nfr = v_lookup(sid,pno);
	else
		nfr = P_LOOKUP(sid,pno);
        if (nfr >=0 && nfr < vmker.nrpages)
        {
          pr_pftnfr(nfr);
	  printf("\nPress ENTER to continue: ");
	  getterm();
        }
	else
	{
	  if (nfr >= 0)
            printf("\nsid %08x pno %08x is hashed to %08x\n", sid,pno,nfr);
	  else
	    printf("\nThere is no page frame for sid %08x pno %08x\n", sid,pno);

	  printf("\nPress ENTER to continue: ");
	  getterm();
	}
        break;

      case '4':
        printf("\nFind all pfts whose swbits match (in hex): ");
        key = gethex();
	cnt = 0;
	for(x=0; x<vmker.nrpages; x++)
	{
	  x = nextlru(x);
	  if ((pft_swbits(x) & key) == key)
	  {
	    cnt++;
            pr_pftnfr(x);
	    printf("\nPress ENTER to display next pft, x to exit: ");
	    if (*getterm()=='x')
	      break;
	  }
	}
	printf("\n%x (hex) matches found with swbits %08x.\n", cnt, key);
	printf("\nPress ENTER to continue: ");
	getterm();
        break;

      case '5':
        printf("\nPage frames with pincount > 0:\n");
	lcnt = cnt = 0;
	first = last = -2;
	for(x=0; x<vmker.nrpages; x++)
	{
	  x = nextlru(x);
	  if (pft_inuse(x) && pft_pincount(x))
	  {
	    cnt++;
	    if (x!=last+1)
	    {
	       if (last != first)
	         printf("-%05x", last);

	       if (lcnt < 5)
	       {
		 lcnt++;
		 if (first != -2)
	           printf(", %05x", x);
		 else
		   printf("%05x", x);
	       }
	       else
	       {
		 lcnt = 0;
	         printf("\n%05x", x);
	       }
	       first = x;
	    }
	    last=x;
	  }
	}
	if (last != first)
	  printf("-%05x\n", last);
	printf("\n%x (hex) pages found with pincount > 0.\n", cnt);
	printf("\nPress ENTER to continue: ");
	getterm();
        break;

      case '6':
        printf("\nPage frames with xmemcnt > 0:\n");
	lcnt = cnt = 0;
	first = last = -2;
	for(x=0; x<vmker.nrpages; x++)
	{
	  x = nextlru(x);
	  if (pft_xmemcnt(x))
	  {
	    cnt++;
	    if (x!=last+1)
	    {
	       if (last != first)
	         printf("-%05x", last);

	       if (lcnt < 5)
	       {
		 lcnt++;
		 if (first != -2)
	           printf(", %05x", x);
		 else
		   printf("%05x", x);
	       }
	       else
	       {
		 lcnt = 0;
	         printf("\n%05x", x);
	       }
	       first = x;
	    }
	    last=x;
	  }
	}
	if (last != first)
	  printf("-%05x\n", last);
	printf("\n%x (hex) pages found with xmemcnt > 0.\n", cnt);
	printf("\nPress ENTER to continue: ");
	getterm();
	break;

      default:
        break;
    }
  }
}

/*
 * NAME: pr_pftnfr
 *
 * FUNCTION:    Display VMM PFT entry
 *
 * RETURN VALUE:  none.
 */
static
void
pr_pftnfr(nfr)

int nfr;
{
  struct hwinfo hwinfo;

  printf("\nVMM PFT Entry For Page Frame %05x of %05x\n", nfr,
		 vmker.nrpages-1);

  gethwnfr(nfr,&hwinfo);

  printf("h/w hashed sid : %08x  pno : %08x  key : %01x\n",
		 hwinfo.sid, hwinfo.pno, hwinfo.key);
  printf("source     sid : %08x  pno : %08x  key : %01x\n",
		 pft_ssid(nfr), pft_spage(nfr), pft_key(nfr));

  if (pft_inuse(nfr))    printf("> in use\n");
  if (pft_pageout(nfr))  printf("> page out\n");
  if (pft_pagein(nfr))   printf("> page in\n");
  if (pft_free(nfr))     printf("> on free list\n");
  if (pft_slist(nfr))    printf("> on scb list\n");
  if (pft_discard(nfr))  printf("> discard when I/O done\n");
  if (pft_fblru(nfr))    printf("> free list when I/O done (fblru)\n");
  if (pft_rdonly(nfr))   printf("> read only\n");
  if (pft_journal(nfr))  printf("> journalled\n");
  if (pft_homeok(nfr))   printf("> ok to write to home\n");
  if (pft_syncpt(nfr))   printf("> write to home\n");
  if (pft_newbit(nfr))   printf("> disk block uncommitted\n");
  if (pft_store(nfr))    printf("> store operation on client page\n");
  if (pft_pgahead(nfr))  printf("> hidden - trigger pageahead\n");
  if (pft_cow(nfr))      printf("> copy-on-write\n");
  if (pft_mmap(nfr))     printf("> mmap fault / client EOF\n");
  if (hwinfo.val)	 printf("> valid (h/w)\n");

  printf("> referenced (pft/pvt/pte): %1x/%1x/%1x\n", pft_refbit(nfr),
		hwinfo.swref, hwinfo.ref);
  printf("> modified (pft/pvt/pte): %1x/%1x/%1x\n", pft_modbit(nfr),
		hwinfo.swmod, hwinfo.mod);
#ifdef	_POWER_PC
  if (__power_pc()) {
     printf("> wimg h/w: %1x, s/w: %1x\n", hwinfo.wimg, pft_wimg(nfr));
  }
#endif	/* _POWER_PC */

  printf("out of order I/O      : %04x\n", pft_nonfifo(nfr));
  printf("index in PDT          : %04x\n", pft_devid(nfr));
  printf("page number in scb    : %08x\n", pft_pagex(nfr)   );
  printf("xmem hide count       : %01x\n", pft_xmemcnt(nfr) );
  printf("disk block number     : %08x\n", pft_dblock(nfr)  );
  printf("next page on scb list : %08x\n", pft_sidfwd(nfr)  );
  printf("prev page on scb list : %08x\n", pft_sidbwd(nfr)  );
  printf("freefwd/waitlist      : %08x\n", pft_freefwd(nfr) );
  printf("freebwd/logage/pincnt : %08x\n", pft_freebwd(nfr) );
  printf("next page on s/w hash : %08x\n", pft_next(nfr)  );

}

/*
 * NAME: pr_pte
 *
 * FUNCTION:    Display VMM PTEs
 *
 * RETURN VALUE:  none.
 */
static
void
pr_pte()

{
  char choice;
  int ptex, pte, sid, pno, nfr, hash, hash2, maxptex;
  int x, key, cnt, lcnt, first, last;
  int sptex, sptex2;
  uint idx1, idx2;
  struct hwinfo hw1, hw2;

#if defined(_POWER_RS1) || defined(_POWER_RSC)
    if (__power_set( POWER_RS1 | POWER_RSC ))
    {
	printf("\nThere are no PTEs on this processor type.\n\n");
	return;
    }
#endif /* _POWER_RS1 || _POWER_RSC */

  for (;;)
  {
    printf("\nVMM PTE\n\n");

    printf("Select the PTE to display by:\n\n");

    printf(" 1) index\n");
    printf(" 2) sid,pno\n");
    printf(" 3) page frame #\n\n");

    printf("Enter your choice or x to exit: ");

    if ((choice = *getterm()) == 'x')
      break;

    maxptex = PTEGSIZE*(1 << vmker.hashbits);
    switch(choice)
    {
      case '1':
	do
	{
          printf("\n\nEnter the index (in hex): ");
          ptex = gethex();
          if (ptex >=0 && ptex < maxptex)
          {
            pr_pteidx(ptex,1);
	    printf("\nPress ENTER to continue: ");
	    getterm();
          }
          else
          {
            printf("\n\nThe index must be >= 0 and < %08x\n", maxptex);
	  }
        } while (ptex < 0 || ptex > maxptex-1);

        break;

      case '2':
        printf("\n\nEnter the sid (in hex): ");
        sid = gethex();

        printf("\n\nEnter the pno (in hex): ");
        pno = gethex();

	hash = HASHF(sid,pno);
	sptex = FIRSTPTE(hash);

	hash = HASHF2(sid,pno);
	sptex2 = FIRSTPTE(hash);

	for (pte = 0; pte < PTEGSIZE; pte++)
	{
		pr_pteidx(sptex + pte, pte==0);
	}
	for (pte = 0; pte < PTEGSIZE; pte++)
	{
		pr_pteidx(sptex2 + pte, pte==0);
	}
	printf("\n");
	printf("\nPress ENTER to continue: ");
	getterm();

      case '3':
	do
	{
          printf("\n\nEnter the page frame number (in hex): ");
          nfr = gethex();
          if (nfr >=0 && nfr < vmker.nrpages)
          {
            pr_ptenfr(nfr);
	    printf("\nPress ENTER to continue: ");
	    getterm();
          }
          else
          {
            printf("\n\nThe page frame must be >= 0 and < %08x\n",
		   vmker.nrpages);
          }
        } while (nfr < 0 || nfr > vmker.nrpages-1);
	break;

      default:
        break;
    }
  }
}

/*
 * NAME: pr_pteidx
 *
 * FUNCTION:    Display hardware PTE fields
 *
 * RETURN VALUE:  none.
 */
static
void
pr_pteidx(idx,hdrflag)

int idx, hdrflag;
{
	struct hwinfo hw;

	if (hdrflag)
		printf("\n PTEX  v  SID   h  avpi  RPN  r c wimg pp\n");

	gethwinfo(idx,&hw);

	printf("%06x %01x %06x %01x   %02x  %05x %01x %01x   %01x   %01x\n",
		idx, hw.val, hw.sid, hw.hsel, hw.avpi, hw.rpn,
		hw.ref, hw.mod, hw.wimg, hw.key);
}

/*
 * NAME: pr_ptenfr
 *
 * FUNCTION:    Display hardware PTE fields
 *
 * RETURN VALUE:  none.
 */
static
void
pr_ptenfr(nfr)

int nfr;
{
	uint idx;
	struct hwinfo hw;

	printf("\n PTEX  v  SID   h  avpi  RPN  r c wimg pp\n");

#ifdef	_POWER_RS2
	if (__power_rs2()) {
		struct rs2pvt *rs2pvt;

		rs2pvt = (struct rs2pvt *) vmrmap_eaddr(RMAP_PVT);
		idx = (rs2pvt+nfr)->u1.s1._ptex;

		if (idx != PVNULL)
		{
			gethwinfo(idx,&hw);

	printf("%06x %01x %06x %01x   %02x  %05x %01x %01x   %01x   %01x\n",
		idx, hw.val, hw.sid, hw.hsel, hw.avpi, hw.rpn,
		hw.ref, hw.mod, hw.wimg, hw.key);
		}
	}
#endif	/* _POWER_RS2 */
#ifdef	_POWER_PC
	if (__power_pc()) {
		struct ppcpvt *ppcpvt;
		struct ppcpvlist *ppcpvlist;

		ppcpvt = (struct ppcpvt *) vmrmap_eaddr(RMAP_PVT);
		ppcpvlist = (struct ppcpvlist *) vmrmap_eaddr(RMAP_PVLIST);
		idx = (ppcpvt+nfr)->u1.s1._ptex;
		while (idx != PVNULL)
		{
			gethwinfo(idx,&hw);

	printf("%06x %01x %06x %01x   %02x  %05x %01x %01x   %01x   %01x\n",
		idx, hw.val, hw.sid, hw.hsel, hw.avpi, hw.rpn,
		hw.ref, hw.mod, hw.wimg, hw.key);

			idx = (ppcpvlist+idx)->u1.s1._next;
		}
	}
#endif	/* _POWER_PC */
}

/*
 * NAME: pr_apt
 *
 * FUNCTION:    Display VMM APTs
 *
 * RETURN VALUE:  none.
 */
static
void
pr_apt()

{
  char choice;
  int aptx, pte, sid, pno, ahash, maxaptx;
  int x, key, cnt, lcnt, first, last;
  int saptx, saptx2;
  uint idx1, idx2;
  struct hwinfo hw1, hw2;
  int nfr;

  for (;;)
  {
    printf("\nVMM APT\n\n");

    printf("Select the APT entry to display by:\n\n");

    printf(" 1) index\n");
    printf(" 2) sid,pno\n");
    printf(" 3) page frame #\n\n");

    printf("Enter your choice or x to exit: ");

    if ((choice = *getterm()) == 'x')
      break;

    maxaptx = vmrmap_size(RMAP_APT) / sizeof(struct apt);
    switch(choice)
    {
      case '1':
	do
	{
          printf("\n\nEnter the index (in hex): ");
          aptx = gethex();
          if (aptx >=0 && aptx < maxaptx)
          {
            pr_aptidx(aptx);
	    printf("\nPress ENTER to continue: ");
	    getterm();
          }
          else
          {
            printf("\n\nThe index must be >= 0 and < %08x\n", maxaptx);
	  }
        } while (aptx < 0 || aptx > maxaptx-1);

        break;

      case '2':
        printf("\n\nEnter the sid (in hex): ");
        sid = gethex();

        printf("\n\nEnter the pno (in hex): ");
        pno = gethex();

        ahash = AHASH(sid,pno);

	for (aptx = ahattab[ahash]; aptx != APTNULL; aptx = apt_next(aptx))
	{
		if (apt_sid(aptx) == sid && apt_pno(aptx) == pno)
			break;
	}

        if (aptx != APTNULL)
        {
          pr_aptidx(aptx);
	  printf("\nPress ENTER to continue: ");
	  getterm();
        }
	else
	{
	  printf("\nThere is no APT entry for sid %08x pno %08x\n", sid,pno);
	  printf("\nPress ENTER to continue: ");
	  getterm();
	}
        break;

      case '3':
	do
	{
          printf("\n\nEnter the page frame number (in hex): ");
          nfr = gethex();
          if (nfr >=0 && nfr < vmker.nrpages)
          {
	    for (aptx = pft_alist(nfr); aptx != APTNULL; aptx = apt_anext(aptx))
	    {
	      pr_aptidx(aptx);
	      printf("\nPress ENTER to display next apt, x to exit: ");
	      if (*getterm()=='x')
	        break;
	    }
          }
          else
          {
            printf("\n\nThe page frame must be >= 0 and < %08x\n",
		   vmker.nrpages);
          }
        } while (nfr < 0 || nfr > vmker.nrpages-1);
	break;

      default:
        break;
    }
  }
}

/*
 * NAME: pr_aptidx
 *
 * FUNCTION:    Display APT entry
 *
 * RETURN VALUE:  none.
 */
static
void
pr_aptidx(aptx)

int aptx;
{
  struct hwinfo hwinfo;

  printf("\nVMM APT Entry %04x of %04x\n", aptx,
		 vmker.nrpages-1);

  if (apt_valid(aptx))   printf("> valid\n");
  if (apt_pinned(aptx))  printf("> pinned\n");

  printf("segment identifier    : %08x\n", apt_sid(aptx));
  printf("page number           : %04x\n", apt_pno(aptx));
  printf("page frame            : %05x\n", apt_nfr(aptx));
  printf("protection key        : %01x\n", apt_key(aptx));
  printf("storage control attr  : %01x\n", apt_wimg(aptx));
  printf("next on hash          : %04x\n", apt_next(aptx));
  printf("next on alias list    : %04x\n", apt_anext(aptx));
  printf("next on free list     : %04x\n", apt_free(aptx));
}

extern struct proc ** compwlist;

/*
 * NAME: pr_vmwait
 *
 * FUNCTION:	Display VMM wait status
 *
 * RETURN VALUE: none
 */
static
void
pr_vmwait()

{
  int waddr, nfr, sidx, k;

  printf("\nVMM Wait Info\n\n");

  printf("Enter the wait address (in hex): ");
  waddr = gethex();

  /* See if one of VMM wait lists
   */
  if (waddr == (int) &pf_freewait)
  {
    printf("Waiting on a free page frame\n");
  }
  else if (waddr == (int) &pf_devwait)
  {
    printf("Waiting on all I/O for device to complete (unmount)\n");
  }
  else if (waddr == (int) &pf_extendwait)
  {
printf("Waiting on segment commit (tried to allocate disk while commit in progress)\n");
  }
  else if (waddr == (int) &pf_bufwait)
  {
    printf("Waiting on a free bufstruct\n");
  }
  else if (waddr == (int) &pf_deletewait)
  {
    printf("Waiting on segment inherit/delete\n");
  }
  else if (waddr == (int) &lanch.freewait)
  {
    printf("Waiting on free transaction blocks\n");
  }
  else if (waddr == (int) &lanch.tblkwait)
  {
    printf("Waiting on transactions to end to forward the log\n");
  } 
  else if (waddr == (int) &pf_dqwait)
  {
    printf("Waiting on quota to be unlocked \n");
  }
  else if (waddr == (int) &pf_pgspwait)
  {
    printf("Waiting on free paging space \n");
  }
  else if ((waddr >= (int) &lanch.tblk[0].waitors) &&
        (waddr <= (int) &lanch.tblk[NUMTIDS-1].waitors))
  {
    /* Waiting on transaction block anchor */
    k = (waddr - (int)&lanch.tblk[0].waitors) / sizeof(lanch.tblk[0]);
    printf("Waiting on transaction block number %08x\n", k);
  }
  else if ((waddr >= (int) &pft_waitlist(0)) && 
   	(waddr <= (int) &pft_waitlist(vmker.nrpages-1)))
  {
    /* Waiting on page frame, anchor in s/w pft
     */
    nfr = (waddr - (int)&pft_waitlist(0)) / sizeof(struct pftsw);
    printf("Waiting on page frame number %08x\n", nfr);
  }
  else if ((waddr >= (int) &scb_iowait(0)) && 
  	 (waddr < (int) &scb_iowait(pf_hisid)))
  {
    /* On an SCB wait list
     */
    sidx = (waddr - (int) &scb_iowait(0)) / sizeof(struct scb);
    if (waddr == (int) &scb_iowait(sidx))
    {
      printf("Waiting on segment I/O level (v_iowait), sidx = %08x\n", sidx);
    }
    else if (waddr == (int) &scb_delwait(sidx))
    {
      printf("Waiting on all I/O for segment to complete, sidx = %08x\n", sidx);
    }
    else if (waddr == (int) &scb_waitlist(sidx))
    {
      printf("Waiting on client EOF, hide of pinned page, or unhide), sidx = %08x\n",
	     sidx);
    }
    else printf("Waiting on UNRECOGNIZED SCB list, sidx = %08x\n", sidx);
  }
  else printf("The wait address %08x is not a VMM wait address.\n", waddr);

  printf("\nPress ENTER to continue: ");
  getterm();

  return;
}

/*
 * NAME: pr_vmaddr
 *
 * FUNCTION:	Display addresses of VMM data structures
 *
 * RETURN VALUE: none
 */
static
void
pr_vmaddr()

{
  printf("\nVMM Addresses\n\n");

#if defined(_POWER_RS1) || defined(_POWER_RSC)
  if (__power_set(POWER_RS1|POWER_RSC))
  {
	  printf("HAT        : %08x\n", &vmmdseg.hw.rs1.hat[0]);
	  printf("PFT        : %08x\n", &vmmdseg.hw.rs1.pft[0]);
  }
#endif	/* _POWER_RS1 || _POWER_RSC */
#ifdef	_POWER_RS2
  if (__power_rs2())
  {
	  printf("HTAB       : %08x\n", &vmmdseg.hw.rs2.pte[0]);
	  printf("PVT        : %08x\n", &vmmdseg.hw.rs2.pvt[0]);
  }
#endif	/* _POWER_RS2 */
#ifdef	_POWER_PC
  if (__power_pc())
  {
	  printf("HTAB       : %08x\n", &vmmdseg.hw.ppc.pte[0]);
	  printf("PVT        : %08x\n", &vmmdseg.hw.ppc.pvt[0]);
	  printf("PVLIST     : %08x\n", &vmmdseg.hw.ppc.pvlist[0]);
  }
#endif	/* _POWER_PC */

  printf("S/W HAT    : %08x\n", &vmmdseg.hat[0]);
  printf("S/W PFT    : %08x\n", &vmmdseg.pft[0]);
  printf("AHAT       : %08x\n", &vmmdseg.ahat[0]);
  printf("APT        : %08x\n", &vmmdseg.apt[0]);
  printf("RPHAT      : %08x\n", &vmmdseg.rphat[0]);
  printf("RPT        : %08x\n", &vmmdseg.rpt[0]);
  printf("PDT        : %08x\n", &vmmdseg.pdt[0]);
  printf("PFHDATA    : %08x\n", &vmmdseg.pf);
  printf("LOCKANCH   : %08x\n", &vmmdseg.lockanch);
  printf("SCBs       : %08x\n", &vmmdseg.scb[0]);
  printf("AMEs       : %08x\n", &vmmdseg.ame[0]);

  printf("\nPress ENTER to continue: ");
  getterm();

  return;
}

/*
 * NAME: pr_ames
 *
 * FUNCTION:    Display address map and address map entries
 *
 * RETURN VALUE:  none.
 */
static
void
pr_ames()

{
  vm_map_t	map;
  vm_map_entry_t e;
  char choice;
  int pid;
  struct proc *p;
  int srval;
  struct user *tu;

  for (;;)
  {
    printf("\nVMM AMEs\n\n");

    printf("Select the ame to display by:\n\n");

    printf(" 1) current process \n");
    printf(" 2) specified process \n\n");

    printf("Enter your choice or x to exit: ");

    if ((choice = *getterm()) == 'x')
      break;

    switch(choice)
    {
      case '1':
	map = (vm_map_t) u.u_map;
	if (map == VM_MAP_NULL)
	{
	  printf("No address map for current process\n");
	  printf("\nPress ENTER to continue: ");
	  getterm();
	}
	else
	{
	  pr_map(map);
	  for(e=vm_map_first_entry(map); e!=vm_map_to_entry(map);
	  	e=e->vme_next)
  	  {
            if (pr_ame(e))
	      break;
	  }
	}
        break;

      case '2':
	
        printf("\n\nEnter the pid (in hex): ");
	pid = gethex();
	p = VALIDATE_PID(pid);
	if (p == NULL)
	{
	  printf("%x is not a valid process id\n", pid);
	  printf("\nPress ENTER to continue: ");
	  getterm();
	  break;
	}
	srval = p->p_adspace;
	ldsr(TEMPSR, srval);
#ifndef _THREADS
	tu = (struct user *) ((TEMPSR << L2SSIZE) + ((int)&u & SOFFSET));
	map = (vm_map_t) tu->u_map;
#else /* _THREADS */
	tu = (struct user *) ((TEMPSR << L2SSIZE) + ((int)&U & SOFFSET));
	map = (vm_map_t) tu->U_map;
#endif
	if (map == VM_MAP_NULL)
	{
	  printf("No address map for process id %x\n", pid);
	  printf("\nPress ENTER to continue: ");
	  getterm();
	}
	else
	{
	  pr_map(map);
	  for(e=vm_map_first_entry(map); e!=vm_map_to_entry(map);
	  	e=e->vme_next)
  	  {
            if (pr_ame(e))
	      break;
	  }
	}
        break;

      default:
        break;
    }
  }
}

/*
 * NAME: pr_map
 
 * FUNCTION:    Display an address map.
 *
 * RETURN VALUE:  none.
 */
static
void
pr_map(map)

vm_map_t map;
{

    printf("\nVMM address map, address %08x\n\n", map);

    printf("previous entry      : %08x\n", map->vme_prev   );
    printf("next entry          : %08x\n", map->vme_next   );
    printf("minimum offset      : %08x\n", map->min_offset );
    printf("maximum offset      : %08x\n", map->max_offset );
    printf("number of entries   : %08x\n", map->nentries   );
    printf("size                : %08x\n", map->size       );
    printf("reference count     : %08x\n", map->ref_count  );
    printf("hint                : %08x\n", map->hint       );
    printf("first free hint     : %08x\n", map->first_free );
    printf("entries pageable    : %08x\n", map->entries_pageable );

    printf("\nPress ENTER to continue: ");
    getterm();
}

/*
 * NAME: pr_ame
 *
 * FUNCTION:    Display an address map entry.
 *
 * RETURN VALUE:  none.
 */
static
int
pr_ame(entry)

vm_map_entry_t entry;
{

    printf("\nVMM map entry, address %08x\n\n", entry);

    if (entry->copy_on_write)  printf("> copy-on-write\n");
    if (entry->needs_copy   )  printf("> needs-copy\n");

    printf("previous entry      : %08x\n", entry->vme_prev   );
    printf("next entry          : %08x\n", entry->vme_next   );
    printf("start address       : %08x\n", entry->vme_start  );
    printf("end address         : %08x\n", entry->vme_end    );
    printf("object (vnode ptr)  : %08x\n", entry->object     );
    printf("offset              : %08x\n", entry->offset     );
    printf("cur protection      : %08x\n", entry->protection );
    printf("max protection      : %08x\n", entry->max_protection );
    printf("inheritance         : %08x\n", entry->inheritance);
    printf("wired_count         : %08x\n", entry->wired_count);
    printf("source sid          : %08x\n", entry->source_sid);
    printf("mapping sid         : %08x\n", entry->mapping_sid);
    printf("paging sid          : %08x\n", entry->paging_sid);
    printf("original obj offset : %08x\n", entry->orig_offset);
    printf("xmem attach count   : %08x\n", entry->xmattach_count);

    printf("\nPress ENTER to display next ame, x to exit: ");
    if (*getterm()=='x')
      return(1);
    else	
      return(0);
}

/*
 * NAME: getdec
 *
 * FUNCTION:    Read the keyboard
 *
 * RETURN VALUE:  decimal value or -1.
 */
static
int
getdec()

{
  struct parse_out po;

  parse_line(getterm(), &po, " ");

  if (po.num_tok == -1)
    return(-1);
  else
    return(po.token[0].dv);

}

/*
 * NAME: gethex
 *
 * FUNCTION:    Read the keyboard
 *
 * RETURN VALUE:  hexadecimal value or -1.
 */
static
int
gethex()

{
  struct parse_out po;

  parse_line(getterm(), &po, " ");

  if (po.num_tok == -1)
    return(-1);
  else
    return(po.token[0].hv);

}

/*
 * NAME: check_address
 *
 * FUNCTION:  checks that there is write permission
 *		to the address specified
 *
 * RETURN VALUE:  
 *	TRUE - address has write perm
 *	FALSE - address is RDONLY
 */
int
check_address(srval, addr)
int	srval;
ulong	addr;
{

	int save_ksr, save_vmmsr, save_ptasr, save_tempsr, save_kexsr;
	int sid, pno, nfr, key;

	/*
	 * Set up to address kernel and VMM data
	 */
	save_ksr    = mfsr(0);
	save_vmmsr  = mfsr(VMMSR);
	save_ptasr  = mfsr(PTASR);
	save_tempsr = mfsr(TEMPSR);
	save_kexsr  = mfsr(KERNXSR);
	mtsr(0, g_ksrval);
	mtsr(VMMSR, vmker.vmmsrval);
	mtsr(PTASR, vmker.ptasrval);
	mtsr(TEMPSR, vmker.dmapsrval);
	mtsr(KERNXSR, vmker.kexsrval);

	/* translate srval to an index in segment control block */
	sid = SRTOSID(vm_vmid(srval));

	/* get the page number based on the address passed */
	pno = (addr & SOFFSET) >> L2PSIZE;


	/*find the real frame number to get the key */
	if( (nfr=v_lookup(sid, pno)) == -1 )
		key=RDONLY;
	else
	/* get the key */
		key = pft_key(nfr);

	/*
	 * Restore segment registers to original values
	 */
	mtsr(0, save_ksr);
	mtsr(VMMSR, save_vmmsr);
	mtsr(PTASR, save_ptasr);
	mtsr(TEMPSR, save_tempsr);
	mtsr(KERNXSR, save_kexsr);

	if (RDONLY == key)
		return(FALSE);
	else
		return(TRUE);
}

/*
 * NAME: pr_vmchk
 *
 * FUNCTION:    Check consistency of VMM data structures.
 *
 * RETURN VALUE:  none.
 */
static
void
pr_vmchk()
{
	int	nfree, nrecl;
	int	nfr, prv, cnt;
	int	sidx, head, iotail, iopart, rcnt;
	int	k, found, hash, maxhash, maxhashlen, thishashlen;
	struct	hwinfo hwinfo;
	ushort	ahash, aptx, x;

        int numptegs, totptes, tothash2, maxptes, pteg, pte, subtot;
        int ptex, ptemax, ptegdist[PTEGSIZE*2+1];
	int subtot2, maxptes2, ptemax2;
	int i, ncpus = NCPUS();
	int epsablks = 0, npsblks = 0, sumpsblks = 0;

	/*
	 * Check free list.
	 */
	prv = FBANCH;
	nfr = pft_freefwd(FBANCH);
	cnt = 0;
	while (nfr != FBANCH) {
		cnt++;
		if (pft_freebwd(nfr) != prv) {
printf("Free list corrupted at nfr = %x, prv = %x\n", nfr, prv);
			break;
		}
		if (!pft_free(nfr)) {
printf("Frame %x on free list but not free\n", nfr);
		}
		prv = nfr;
		nfr = pft_freefwd(nfr);
	}
#ifdef _VMM_MP_EFF
	/*
	 * count processor frame reservations
	 */
	for (i = 0; i < ncpus; ++i)
		cnt += -ppda[i].ppda_reservation;
#endif
	if (cnt != vmker.numfrb) {
printf("Wrong # of free frames, numfrb %x, free list %x\n", vmker.numfrb, cnt);
	}
printf("Free list verified.\n");

	/*
	 * Check page frame states.
	 */
	nfree = 0;
	nrecl = 0;
	for (nfr = 0; nfr < vmker.nrpages; nfr++) {
		nfr = nextlru(nfr);
		gethwnfr(nfr, &hwinfo);

		if (pft_free(nfr)) {
			nfree++;
			if (pft_slist(nfr)) {
				nrecl++;
				if (hwinfo.sid && !IOVADDR(hwinfo.sid)) {
printf("Reclaim frame %x not I/O\n", nfr);
				}
			}
		}

		if (pft_inuse(nfr)) {
			if (!pft_slist(nfr)) {
printf("In-Use frame %x not on SCB list\n", nfr);
			}
			if (hwinfo.sid && !hwinfo.val) {
printf("In-Use frame %x not valid\n", nfr);
			}
			if ((pft_xmemcnt(nfr) != 0) && pft_pincount(nfr)==0) {
printf("Hidden frame %x isn't pinned\n", nfr);
			}
		}

		if (pft_pagein(nfr) || pft_pageout(nfr)) {
			if (hwinfo.sid && !IOVADDR(hwinfo.sid)) {
printf("I/O frame %x doesn't have I/O sid\n", nfr);
			}
			if (!pft_slist(nfr)) {
printf("I/O frame %x not on SCB list\n", nfr);
			}
			if (hwinfo.sid && !hwinfo.val) {
printf("I/O frame %x not valid\n", nfr);
			}
		}

		if (pft_pgahead(nfr) && !pft_free(nfr)) {
			if (hwinfo.sid && !IOVADDR(hwinfo.sid)) {
printf("Pageahead frame %x not I/O\n", nfr);
			}
			if (!pft_slist(nfr)) {
printf("Pageahead frame %x not on SCB list\n", nfr);
			}
			if (hwinfo.sid && !hwinfo.val) {
printf("Pageahead frame %x not valid\n", nfr);
			}
		}
	}

	if (nfree != vmker.numfrb) {
printf("Wrong # of free frames, numfrb %x, # free %x, # reclaim %x\n",
	vmker.numfrb, nfree, nrecl);
	}
printf("Page frame states verified.\n");

	/*
	 * Check s/w hash.
	 */
	maxhash = 0;
	maxhashlen = 0;
	for (nfr = 0; nfr < vmker.nrpages; nfr++) {
		nfr = nextlru(nfr);
		if (!pft_slist(nfr))
			continue;
		hash = SWHASH(pft_ssid(nfr), pft_spage(nfr));
		found = 0;
		thishashlen = 0;
		for (k = SWFIRSTNFR(hash); k >= 0; k = pft_next(k)) {
			/*
			 * if the frame number greater than max number
			 * of frames in the system or if it is not a valid
			 * value ( -1 is ok but not any other -ve value),
			 * print a message.
			 */ 
			if (k >= vmker.nrpages || (k != -1 && k < 0)) {
printf("Bad s/w hash pointer, nfr %x ptr %x\n", nfr, k);
				break;
			}
			if (pft_ssid(k) == pft_ssid(nfr) &&
			    pft_spage(k) == pft_spage(nfr)) {
				found = 1;
				break;
			}
			thishashlen++;
			if (thishashlen > maxhashlen) {
				maxhash = hash;
				maxhashlen = thishashlen;
			}
		}

		if (!found) {
printf("Frame %x not on s/w hash %x\n", nfr, hash);
		}
	}
printf("Software hash chain verified.\n");
printf("Maximum s/w hash chain length is %d on hash 0x%x\n",
	maxhashlen, maxhash);

	/*
	 * Check alias hash.
	 */
	maxhash = 0;
	maxhashlen = 0;
	for (aptx = 0; aptx < vmrmap_size(RMAP_APT)/sizeof(struct apt); aptx++) {
		if (!apt_valid(aptx))
			continue;
		ahash = AHASH(apt_sid(aptx), apt_pno(aptx));
		found = 0;
		thishashlen = 0;
		for (x = ahattab[ahash]; x != APTNULL; x = apt_next(x)) {
			if (x >= vmrmap_size(RMAP_APT)/sizeof(struct apt)) {
printf("Bad alias hash index, aptx %x next %x\n", aptx, x);
				break;
			}
			thishashlen++;
			if (thishashlen > maxhashlen) {
				maxhash = ahash;
				maxhashlen = thishashlen;
			}
			if (apt_sid(x) == apt_sid(aptx) &&
			    apt_pno(x) == apt_pno(aptx)) {
				found = 1;
				break;
			}
		}

		if (!found) {
printf("APT entry %x not on alias hash %x\n", aptx, ahash);
		}
	}
printf("Alias hash chain verified.\n");
printf("Maximum alias hash chain length is %d on hash 0x%x\n",
	maxhashlen, maxhash);

	/*
	 * Check page frame alias lists.
	 */
	maxhash = 0;
	maxhashlen = 0;
	for (nfr = 0; nfr < vmker.nrpages; nfr++) {
		nfr = nextlru(nfr);
		found = 0;
		thishashlen = 0;
		for (aptx = pft_alist(nfr); aptx != APTNULL;
		     aptx = apt_anext(aptx)) {
			if (apt_nfr(aptx) != nfr) {
printf("Entry on alias list has wrong frame, aptx %x frame %x\n", aptx, nfr);
				break;
			}
			if (!apt_valid(aptx)) {
printf("Entry on alias list not valid, aptx %x frame %x\n", aptx, nfr);
				break;
			}
			thishashlen++;
			if (thishashlen > maxhashlen) {
				maxhash = nfr;
				maxhashlen = thishashlen;
			}
		}
	}
printf("Alias list verified.\n");
printf("Maximum alias chain length is %d for frame 0x%x\n",
	maxhashlen, maxhash);

	/*
	 * Check SCB lists.
	 */
	for (sidx = 0; sidx < pf_hisid; sidx++) {
		if (!scb_valid(sidx))
			continue;
		/*
		 * Accumulate paging space counts.
		 */
		if (scb_wseg(sidx))
		{
			if (scb_npsblks(sidx) < 0)
printf("Pgsp block count is negative, sidx %x\n", sidx);
			if (scb_psearlyalloc(sidx))
				epsablks += scb_npseablks(sidx);
			npsblks += scb_npsblks(sidx);
		}

		head = scb_sidlist(sidx);
		if (head < 0) {
			if (scb_npages(sidx) != 0) {
printf("Wrong # of pages on SCB list, sidx %x, npages %x, on list = 0\n",
	sidx, scb_npages(sidx));
			}
			continue;
		}
		iotail = pft_sidbwd(head);
		if (iotail >= 0)
			iopart = 1;
		else
			iopart = 0;
		cnt = 0;
		rcnt = 0;
		prv = iotail;
		nfr = head;
		while (nfr >= 0) {
			cnt++;
			if (pft_sidbwd(nfr) != prv) {
printf("SCB list corrupted, sidx %x, nfr %x, prv %x\n", sidx, nfr, prv);
				break;
			}
			if (iopart) {
				if (!(pft_pagein(nfr) || pft_pageout(nfr))) {
printf("Frame on I/O part of SCB list but not in I/O state, sidx %x, nfr %x\n",
	sidx, nfr);
				}
				if (nfr == iotail)
					iopart = 0;
			}
			if (!pft_slist(nfr)) {
printf("Frame on SCB list but not in slist state, sidx %x, nfr %x\n",
	sidx, nfr);
			} else if (pft_free(nfr))
				rcnt++;

			prv = nfr;
			nfr = pft_sidfwd(nfr);
		}
		if (scb_npages(sidx) != cnt - rcnt) {
printf("Wrong # of pages on SCB list, sidx %x, npages %x, on list %x, nrecl %x\n",
	sidx, scb_npages(sidx), cnt, rcnt);
		}
	}
printf("SCB lists verified.\n");

	if (epsablks < 0)
printf("Total early pgsp block count is negative, cnt = %x\n", epsablks);
	if (npsblks < 0)
printf("Total pgsp block count is negative, cnt = %x\n", npsblks);

	sumpsblks = vmker.psfreeblks + epsablks + npsblks;

	/* XXX  For some unexplained reason, the sum is off by 1.
	 * Also, the code doesn't account for lock word allocations.
	 */
printf("pgsp counts: sum = %x, actual = %x, free = %x, eps = %x, nps = %x\n",
	sumpsblks, vmker.numpsblks, vmker.psfreeblks, epsablks, npsblks);

#if 0
#ifdef	_POWER_PC
	if (__power_pc()) {
		/*
		 * Check for valid WIMG settings.
		 */
		for (ptex = 0; ptex < PTEGSIZE*(1<<vmker.hashbits); ptex++) {
			gethwinfo(ptex, &hwinfo);
			if (hwinfo.wimg != 0 &&		/* uninitialized */
			    hwinfo.wimg != 2 &&		/* normal */
			    hwinfo.wimg != 5)		/* int. register */
printf("Unusual WIMG settings for ptex %x frame %x\n", ptex, hwinfo.rpn);
		}
printf("WIMG settings verified.\n");
	}
#endif	/* _POWER_PC */
#endif

#if defined(_POWER_RS2) || defined(_POWER_PC)
	if (__power_set(POWER_RS2|POWER_PC_ALL)) {

		numptegs = (1 << vmker.hashbits);
		totptes = 0;
		tothash2 = 0;
		maxptes = 0;
		maxptes2 = 0;

		for (cnt = 0; cnt <= 2*PTEGSIZE; cnt++)
			ptegdist[cnt] = 0;

		/*
		 * Count the number of PTEs in use in each hash class
		 */
		for (pteg = 0; pteg < numptegs / 2; pteg++)
		{
			subtot = 0;
			subtot2 = 0;
			for (pte = 0; pte < PTEGSIZE; pte++)
			{
				ptex = FIRSTPTE(pteg) + pte;
				gethwinfo(ptex, &hwinfo);
				if (hwinfo.val)
				{
					subtot++;
					totptes++;
					if (hwinfo.hsel != 0)
					{
						tothash2++;
						subtot2++;
					}
				}
			}
			for (pte = 0; pte < PTEGSIZE; pte++)
			{
				ptex = FIRSTPTE(numptegs-pteg-1) + pte;
				gethwinfo(ptex, &hwinfo);
				if (hwinfo.val)
				{
					subtot++;
					totptes++;
					if (hwinfo.hsel != 0)
					{
						tothash2++;
						subtot2++;
					}
				}
			}
			if (subtot > maxptes)
			{
				ptemax = FIRSTPTE(pteg);
				maxptes = subtot;
			}
			if (subtot2 > maxptes2)
			{
				ptemax2 = FIRSTPTE(pteg);
				maxptes2 = subtot2;
			}
			ptegdist[subtot]++;
		}

		printf("\nPress ENTER to continue: ");
		getterm();

		printf("\n");
		printf("Number of page frames: 0x%08x (%d)\n",
			vmker.nrpages, vmker.nrpages);
		printf("Number of PTE groups : 0x%08x (%d)\n",
			numptegs, numptegs);
		printf("Number of valid PTEs : 0x%08x (%d)\n",
			totptes, totptes);
		printf("Number of 2nd entries : 0x%08x (%d)\n",
			tothash2, tothash2);
		printf("Fullest PTE group (ptex 0x%x) had %d entries\n",
			ptemax, maxptes);
		printf("PTE group (ptex 0x%x) with most 2nd entries (%d)\n",
			ptemax2, maxptes2);
		printf("PTE group distribution:\n");
		printf("# : total,percent\n");
		for (cnt = 0; cnt <= 2*PTEGSIZE; cnt++)
			printf("%d : %d,%d\n", cnt, ptegdist[cnt],
				(100*ptegdist[cnt])/(numptegs/2));
	}
#endif	/* _POWER_RS2 || _POWER_PC */

  printf("\nPress ENTER to continue: ");
  getterm();
}

/*
 * gethwnfr - passthru to gethwinfo given page frame number.
 */
static
void
gethwnfr(nfr, ptei)
uint nfr;
struct hwinfo *ptei;
{
	uint idx;

#if defined(_POWER_RS1) || defined(_POWER_RSC)
	if (__power_set(POWER_RS1 | POWER_RSC)) {
		struct rs1pft *rs1pft;

		rs1pft = (struct rs1pft *) vmrmap_eaddr(RMAP_PFT);
		idx = nfr;
		ptei->swref = (rs1pft+nfr)->u1.s1._refbit;
		ptei->swmod = (rs1pft+nfr)->u1.s1._modbit;
	}
#endif	/* _POWER_RS1 || _POWER_RSC */
#ifdef	_POWER_RS2
	if (__power_rs2()) {
		struct rs2pte *rs2pte;
		struct rs2pvt *rs2pvt;

		rs2pte = (struct rs2pte *) vmrmap_eaddr(RMAP_PFT);
		rs2pvt = (struct rs2pvt *) vmrmap_eaddr(RMAP_PVT);
		idx = (rs2pvt+nfr)->u1.s1._ptex;
		ptei->swref = (rs2pvt+nfr)->u1.s1._swref;
		ptei->swmod = (rs2pvt+nfr)->u1.s1._swmod;
	}
#endif	/* _POWER_RS2 */
#ifdef	_POWER_PC
	if (__power_pc()) {
		struct ppcpte *ppcpte;
		struct ppcpvt *ppcpvt;

		ppcpte = (struct ppcpte *) vmrmap_eaddr(RMAP_PFT);
		ppcpvt = (struct ppcpvt *) vmrmap_eaddr(RMAP_PVT);
		idx = (ppcpvt+nfr)->u1.s1._ptex;
		ptei->swref = (ppcpvt+nfr)->u1.s1._swref;
		ptei->swmod = (ppcpvt+nfr)->u1.s1._swmod;
	}
#endif	/* _POWER_PC */

	gethwinfo(idx, ptei);
}

/*
 * gethwinfo - machine-dependent routine to extract fields
 *		from a hardware page table entry.
 */
static
void
gethwinfo(idx, ptei)
uint idx;
struct hwinfo *ptei;
{

#if defined(_POWER_RS1) || defined(_POWER_RSC)
	if (__power_set(POWER_RS1 | POWER_RSC)) {
		struct rs1pft *rs1pft;

		rs1pft = (struct rs1pft *) vmrmap_eaddr(RMAP_PFT);
		ptei->rpn = idx;
		ptei->sid = (rs1pft+idx)->u1.s1._sidpno >> 3;
		ptei->pno = (rs1pft+idx)->u2.s1._page;
		ptei->key = (rs1pft+idx)->u1.s1._key;
		ptei->ref = (rs1pft+idx)->u1.s1._refbit;
		ptei->mod = (rs1pft+idx)->u1.s1._modbit;
		ptei->val = (rs1pft+idx)->u1.s1._valid;
		ptei->avpi = (rs1pft+idx)->u1.s1._sidpno >> 24;
		ptei->hsel = 0;
		ptei->wimg = 0;
	}
#endif	/* _POWER_RS1 || _POWER_RSC */
#ifdef	_POWER_RS2
	if (__power_rs2()) {
		if (idx != PVNULL) {
			struct rs2pte *rs2pte;

			rs2pte = (struct rs2pte *) vmrmap_eaddr(RMAP_PFT);
			ptei->val = (rs2pte+idx)->u1.s1._valid;
			ptei->sid = (rs2pte+idx)->u1.s1._sid;
			ptei->hsel = (rs2pte+idx)->u1.s1._hsel;
			ptei->avpi = (rs2pte+idx)->u1.s1._avpi;
			ptei->rpn = (rs2pte+idx)->u2.s2._rpn;
			ptei->ref = (rs2pte+idx)->u2.s2._refbit;
			ptei->mod = (rs2pte+idx)->u2.s2._modbit;
			ptei->key = (rs2pte+idx)->u2.s2._key;
			ptei->wimg = 0;
			ptei->pno =
	( (ptei->avpi << 11) |
	  (ptei->hsel ? ~((idx / PTEGSIZE) ^ ptei->sid) & 0x7ff
			  : ((idx / PTEGSIZE) ^ ptei->sid) & 0x7ff) );
		} else {
			ptei->rpn = 0;
			ptei->sid = 0;
			ptei->pno = 0;
			ptei->key = 0;
			ptei->ref = 0;
			ptei->mod = 0;
			ptei->val = 0;
			ptei->avpi = 0;
			ptei->hsel = 0;
			ptei->wimg = 0;
		}
	}
#endif	/* _POWER_RS2 */
#ifdef	_POWER_PC
	if (__power_pc()) {
		if (idx != PVNULL) {
			struct ppcpte *ppcpte;

			ppcpte = (struct ppcpte *) vmrmap_eaddr(RMAP_PFT);
			ptei->val = (ppcpte+idx)->u1.s1._valid;
			ptei->sid = (ppcpte+idx)->u1.s1._sid;
			ptei->hsel = (ppcpte+idx)->u1.s1._hsel;
			ptei->avpi = (ppcpte+idx)->u1.s1._avpi;
			ptei->rpn = (ppcpte+idx)->u2.s2._rpn;
			ptei->ref = (ppcpte+idx)->u2.s2._refbit;
			ptei->mod = (ppcpte+idx)->u2.s2._modbit;
			ptei->wimg = (ppcpte+idx)->u2.s2._wimg;
			ptei->key = (ppcpte+idx)->u2.s2._key;
			ptei->pno =
	( (ptei->avpi << 10) |
	  (ptei->hsel ? ~((idx / PTEGSIZE) ^ ptei->sid) & 0x3ff
			  : ((idx / PTEGSIZE) ^ ptei->sid) & 0x3ff) );
		} else {
			ptei->rpn = 0;
			ptei->sid = 0;
			ptei->pno = 0;
			ptei->key = 0;
			ptei->ref = 0;
			ptei->mod = 0;
			ptei->val = 0;
			ptei->avpi = 0;
			ptei->hsel = 0;
			ptei->wimg = 0;
		}
	}
#endif	/* _POWER_PC */
}

/*
 * NAME: pr_rmap
 *
 * FUNCTION:    Display real address range mappings.
 *              (structure vmrmap).
 *
 * RETURN VALUE:  none.
 */
static
void
pr_rmap()
{
  int i, x;
  int choice;

  printf("\nVMM Ranges\n\n");

  for (i=1; i<RMAP_MAX; i++)
  {
    printf("\nRMAP entry %02x of %02x\n\n", i, RMAP_MAX-1);

    printf("entry: ");
    switch(vmrmap_id(i))
    {
      case RMAP_KERN:	printf("Kernel\n"); break;
      case RMAP_IPLCB:	printf("IPL control block\n"); break;
      case RMAP_MST:	printf("MST\n"); break;
      case RMAP_RAMD:	printf("RAMD\n"); break;
      case RMAP_BCFG:	printf("BCFG\n"); break;
      case RMAP_HAT: 	printf("HAT\n"); break;
      case RMAP_PFT:	printf("PFT\n"); break;
      case RMAP_PVT:	printf("PVT\n"); break;
      case RMAP_PVLIST:	printf("PVLIST\n"); break;
      case RMAP_SWPFT:	printf("s/w PFT\n"); break;
      case RMAP_SWHAT:	printf("s/w HAT\n"); break;
      case RMAP_APT:	printf("APT\n"); break;
      case RMAP_AHAT:	printf("AHAT\n"); break;
      case RMAP_RPT:	printf("RPT\n"); break;
      case RMAP_RPHAT:	printf("RPHAT\n"); break;
      case RMAP_PDT:	printf("PDT\n"); break;
      case RMAP_PTAR:	printf("PTAR\n"); break;
      case RMAP_PTAD:	printf("PTAD\n"); break;
      case RMAP_PTAI:	printf("PTAI\n"); break;
      case RMAP_DMAP:	printf("DMAP\n"); break;
      case RMAP_IPLCBX:	printf("Extended IPL control block\n"); break;
      case RMAP_SYSREG: printf("SYSREG\n"); break;
      case RMAP_SYSINT: printf("SYSINT\n"); break;
      case RMAP_NVRAM:  printf("NVRAM\n"); break;
      case RMAP_TCE:    printf("TCE\n"); break;
      case RMAP_MCSR:   printf("MCSR\n"); break;
      case RMAP_MEAR:   printf("MEAR\n"); break;
      default:		printf("unknown\n"); break;
    }
    if (vmrmap_valid(i))	printf("> valid\n");
    if (vmrmap_ros(i))		printf("> loaded by ROS\n");
    if (vmrmap_holes(i))	printf("> may contain bad memory\n");
    if (vmrmap_io(i))		printf("> range is in I/O space\n");
    if (vmrmap_seg(i))		printf("> range assigned to unique segment\n");
    printf("Real address      : %08x\n", vmrmap_raddr(i));
    printf("Effective address : %08x\n", vmrmap_eaddr(i));
    printf("Size              : %08x\n", vmrmap_size(i));
    printf("Alignment         : %08x\n", vmrmap_align(i));
    printf("WIMG bits         : %01x\n", vmrmap_wimg(i));

    printf("\nPress ENTER to display next entry, x to exit: ");

    if (*getterm()=='x')
	break;
  }

  for (i=0; i<VMINT_TYPES; i++)
  {
    if (vmint_num(i) == 0)
	continue;

    printf("\nInterval entry %02x of %02x\n\n", i, VMINT_TYPES-1);
    printf("entry: ");
    switch(i)
    {
      case VMINT_BADMEM:	printf("Memory holes\n"); break;
      case VMINT_FIXMEM:	printf("Fixed kernel memory\n"); break;
      case VMINT_RELMEM:	printf("Released kernel memory\n"); break;
      case VMINT_FIXCOM:	printf("Fixed common memory\n"); break;
      default:			printf("unknown\n"); break;
    }
    printf("Number of intervals:  %08x\n", vmint_num(i));
    for (x=0; x<vmint_num(i); x++)
    {
	    printf("%x : [%06x,%06x)\n", x, vmint_start(i,x), vmint_end(i,x));
    }

    printf("\nPress ENTER to display next entry, x to exit: ");

    if (*getterm()=='x')
      return;
  }

}

/*
 * NAME: nextlru
 *
 * FUNCTION:    If specified page frame is bad or in a memory hole
 *		return next good frame.
 *
 * RETURN VALUE:  none.
 */
static
int
nextlru(nfr)
int nfr;
{
	int i;

	/*
	 * Scan the bad memory intervals to determine if the 
	 * given frame is good or not.
	 */
	for (i = 0; i < vmint_num(VMINT_BADMEM); i++)
	{
		if (nfr >= vmint_start(VMINT_BADMEM,i))
		{
			if (nfr < vmint_end(VMINT_BADMEM,i))
			{
				/*
				 * Frame is in a memory hole so
				 * skip to first good frame.
				 */
				nfr = vmint_end(VMINT_BADMEM,i);
				break;
			}
		}
		else
		{
			/*
			 * Frame is good.
			 */
			break;
		}
	}
	return(nfr);
}
/*
 * NAME: pr_elog
 *
 * FUNCTION:    Display the contents of vmmerrlog
 *
 * RETURN VALUE:  none.
 */

extern struct vmmerror
{
        struct  err_rec0        vmmerr;
        uint                    detail_data[4];
} vmmerrlog ;

static
void
pr_elog()
{
  int i, x;
  int choice;

  printf("\nMost recent VMM errorlog entry\n\n");

  printf("\nError id               =  %8s\n", vmmerrlog.vmmerr.error_id ==
						ERRID_DSI_PROC ? "DSI_PROC":
						vmmerrlog.vmmerr.error_id ==
						ERRID_ISI_PROC? "ISI_PROC":
						" ");
  printf(  "Exception DSISR/ISISR  =  %08x\n", vmmerrlog.detail_data[0]);
  printf(  "Exception srval        =  %08x\n", vmmerrlog.detail_data[1]);
  printf(  "Exception virt addr    =  %08x\n", vmmerrlog.detail_data[2]);
  printf(  "Exception value        =  %08x\n", vmmerrlog.detail_data[3]);

  printf("\nPress ENTER to continue: ");
  getterm();

}
