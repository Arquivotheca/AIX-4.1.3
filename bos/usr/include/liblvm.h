/* @(#)71	1.16.1.8  src/bos/usr/include/liblvm.h, liblvm, bos411, 9428A410j 3/4/94 17:32:08 */
/*
 * COMPONENT_NAME: (liblvm) Logical Volume Manager
 *
 * FUNCTIONS: Header file for logical Volume Manger
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1990
 *               1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_LIBLVM
#define _H_LIBLVM

#include <lvm.h>
#include <sys/dasd.h>
#include <sys/bootrecord.h>

#ifndef TRUE
#define TRUE            1
#endif

#ifndef FALSE
#define FALSE           0
#endif

#ifndef NULL
#define NULL            ((void *) 0)
#endif


/*
 *   Error codes used internally by the library.  These are not returned
 *   to the user.  NOTE that these values start at 500 so they will not
 *   conflict with the error values in lvm.h which are returned to the
 *   user.
 */

#define LVM_BBRDERR      -500    /* read error on bad block directory  */
#define LVM_BBWRERR      -501    /* write error on bad block directory */
#define LVM_PVRDRELOC    -502    /* put PV in read only relocation     */
#define LVM_BBINSANE     -503    /* bad block directory is not sane    */


/*
 *   General defines
 */

#define LOCK_ALL 	0
#define CHECK_MAJ       1
#define NOCHECK         0
#define FIRST_INDEX 	0   
#define SEC_INDEX       1 
#define THIRD_INDEX 	2
#define NO_COPIES       0
#define ONE_COPY        1
#define LVM_FNAME 	72
#define LVM_NOLPSYET 	0
#define LVM_REDUCE 	1
#define LVM_EXTEND 	2
#define LVM_FIRST 	1
#define LVM_SEC 	2
#define LVM_THIRD 	3
#define LVM_LASTPV      0
#define LVM_CASEGEN     1
#define LVM_CASE2TO1    2
#define LVM_CASE3TO2    3
#define LVM_GETSTALE    1
#define LVM_NOSTALE     2
#define LVM_PVNAME      1
#define LVM_VGNAME      2
#define LVM_LVDDNAME	"hd_pin"
#define LVM_KMIDFILE	"/etc/vg/lvdd_kmid"
#define LVM_STREAMRD	"r"
#define LVM_STREAMWR	"w"
#define MAPNOTOPEN	-1		/* Mapped file is not open */
#define CFGPATH		"/etc/objrepos"

/*
*  GENERAL LV VALUES
*/
#define LVM_INITIAL_LPNUM 0



#define  LVM_LVMID       0x5F4C564D /* LVM id field = "_LVM" */
#define  LVM_SLASH       0x2F       /* hex value for ASCII slash */
#define  LVM_NULLCHAR    '\0'       /* null character */
#define  LVM_DEV        "/dev/"     /* concatenate to device names */
/*
#define  LVM_EXTNAME     (sizeof (LVM_NAMESIZ) + sizeof (LVM_DEV) + 1)
*/
				/* size of extended device names */
#define  LVM_EXTNAME     72
#define  LVM_ETCVG       "/etc/vg/vg"
				/* concatenate to VG id for map filename */
#define  LVM_RELOC_LEN   256    /* length in blocks of BB reloc pool */
#define  LVM_RELOCMASK   0x8    /* mask value to check BB relocation */
#define  LVM_FROMBEGIN   0      /* seek value is offset from beginning */
#define  LVM_NOVGDALSN   -1     /* no desc LSN defined for this entry */
#define  LVM_FILENOTOPN  -1     /* file is not currently open */
#define  LVM_WRITEDA     1      /* write VGDA for this PV */
#define  LVM_DALVMINOR   0      /* minor number of descriptor area LV */
#define  LVM_PRMRY       0      /* index for primary VGDA/VGSA LSN */
#define  LVM_SCNDRY      1      /* index for secondary VGDA/VGSA LSN */
#define  LVM_MAPFPERM    0664   /* permissions for open of mapped file */
#define  LVM_1STPV       1      /* PV number of first physical volume */
#define  LVM_BEGMIDEND   0      /* write order of beginning/middle/end */
#define  LVM_MIDBEGEND   1      /* write order of middle/beginning/end */
#define  LVM_ZEROETS     2      /* zero end timestamp, then write b/m/e */
#define  LVM_GREATER     1      /* first timestamp greater than second */
#define  LVM_EQUAL       2      /* first timestamp equals second */
#define  LVM_LESS        3      /* first timestamp less than second */
#define  LVM_TSRDERR     4      /* read error on timestamp */
#define  LVM_BTSEQETS    LVM_EQUAL     /* begin timestamp = end ts */
#define  LVM_BTSGTETS    LVM_GREATER   /* begin timestamp > end ts */
#define  LVM_DAPVS_TTL1  1      /* total of 1 PV with VGDA copies */
#define  LVM_TTLDAS_1PV  2      /* total number VGDA copies on 1 PV */
#define  LVM_DAPVS_TTL2  2      /* total of 2 PVs with VGDA copies */
#define  LVM_TTLDAS_2PV  3      /* total number VGDA copies on 2 PVs */
#define  LVM_DASPERPVGEN 1      /* number VGDAs per PV for general case */
#define  LVM_BBCHGRBLK   1      /* change relocation block of bad block */
#define  LVM_BBCHGSTAT   2      /* change status field of bad block */
#define  LVM_STRCMPEQ    0      /* string compare result of equal */
#define  LVM_BBRDONLY    1	/* read a bad block directory */
#define  LVM_BBRDINIT    2	/* read and initialize a bad block directory */
#define  LVM_BBRDRECV    3	/* read and recover bad block directories */
#define  LVM_BBPRIM      1      /* use the primary bad block directory */
#define  LVM_BBBACK      2      /* use the backup bad block directory */


/*
*  Macros
*/

#define  LVM_SIZTOBBND(Size)  ((((Size) + DBSIZE - 1) / DBSIZE) * DBSIZE)

#define  LVM_BBDIRLEN(Bb_hdr) (LVM_SIZTOBBND(sizeof(struct bb_hdr) + \
	  (Bb_hdr->num_entries * sizeof(struct bb_entry))))

#define  LVM_MAPFN(Mapfn, Vgid)  \
	       (sprintf ((Mapfn), "%s%8.8X%8.8X", LVM_ETCVG,            \
			 (Vgid) -> word1, (Vgid) -> word2))

#define LVM_BUILDNAME(name,maj,min) \
		(sprintf((name),"%s%c%c%s%d%c%d",LVM_DEV,'_','_',"pv",(maj),'.',(min)))

#define LVM_BUILDVGNAME(name,maj) \
		(sprintf((name),"%s%c%c%s%d",LVM_DEV,'_','_',"vg",(maj)))

#define  LVM_PPLENBLKS(Ppsize)   (1 << ((Ppsize) - DBSHIFT))

#define  LVM_PSNFSTPP(Lvmareastart, Lvmarealen)                         \
	  (TRK2BLK (BLK2TRK (Lvmareastart + Lvmarealen - 1) + 1))

/*
*   The following is the file header structure that gives indexes and
*   general information about the volume group descriptor area structures 
*/

struct da_info {
    daddr_t dalsn;             /* logical sector number of VGDA copy */
    struct timestruc_t ts;     /* timestamp of this VGDA copy */
    };

struct fheader {
    long vginx;                /* byte offset for vg header */
    long lvinx;                /* byte offset for lv entries */
    long pvinx;                /* byte offset for pv entries */
    long endpvs;               /* byte offset for end of last PV entry */
    long name_inx;             /* offset for the name area */
    long trailinx;             /* byte offset for the vg trailer */
    long major_num;            /* major number of volume group */
    long vgda_len;             /* length in blocks of the VGDA */
    char vgname[LVM_NAMESIZ];  /* name of volume group */
    long quorum_cnt;           /* number of vgdas needed for varyon */
    long pad1;		       /* pad */
    short int num_desclps;     /* number of LPs per PV for the VGDA LV */
    struct pvinfo {
	char pvname[LVM_NAMESIZ];  /* PV name */
	struct unique_id pv_id;    /* id of physical volume */
	long pvinx;                /* byte offset to PV header */
	dev_t device;              /* major/minor number */
	short pad2;		   /* pad */
	short pad3;		   /* pad */
	struct da_info da [LVM_PVMAXVGDAS];  /* info on this VGDA copy */
	} pvinfo[LVM_MAXPVS];  /* information about each PV */
    };




/*  II.Volume Group Descriptor Area  */


struct vg_header
{
      struct timestruc_t    vg_timestamp; /* time of last update */
      struct unique_id      vg_id;        /* unique id for volume group */ 
      short                 numlvs;       /* number of lvs in vg */
      short                 maxlvs;       /* max number of lvs allowed in vg */
      short                 pp_size;      /* size of pps in the vg */
      short                 numpvs;       /* number of pvs in the vg */
      short                 total_vgdas;  /* number of copies of vg */
					  /* descriptor area on disk */
      short                 vgda_size;    /* size of volume group descriptor */
   };
 
   struct lv_entries
   {
      short       lvname;  	  /* name of LV */
      short       res1;	   	  /* reserved area */
      long        maxsize;   	  /* maximum number of partitions allowed */
      char        lv_state; 	  /* state of logical volume */
      char        mirror;      	  /* none,single, or double */
      short       mirror_policy;  /* type of writing used to write */
      long        num_lps;	  /* number of logical partitions on the lv */
                          	  /* base 1 */
      char        permissions; 	  /* read write or read only */
      char        bb_relocation;  /* specifies if bad block */
                                  /* relocation is desired */
      char        write_verify;   /* verify all writes to the LV */
      char        mirwrt_consist; /* mirror write consistency flag */
      unsigned short  stripe_exp;  /* stripe size in exponent value */
      unsigned short  striping_width;   /* stripe width */
      double      res4;           /* reserved area on disk */
   };

 
   struct pv_header
   {
      struct unique_id      pv_id;      /* unique identifier of PV */
      unsigned short        pp_count;   /* number of physical partitions */
                                        /* on PV */
      char                  pv_state;   /* state of physical volume */
      char                  res1;       /* reserved area on disk */
      daddr_t               psn_part1;  /* physical sector number of 1st pp */
      short                 pvnum_vgdas;/* number of vg descriptor areas */
                                        /* on the physical volume */
      short                 pv_num;     /* PV number */
      long                  res2;       /* reserved area on disk */
    };
 
    struct pp_entries
    {
       short        lv_index;     /* index to lv pp is on */
       short        res_1;        /* reserved area on disk */
       long         lp_num;       /* log. part. number */
       char         copy;         /* the copy of the logical partition */
				  /* that this pp is allocated for */
       char         pp_state;     /* current state of pp */
       char         fst_alt_vol;  /* pv where partition allocation for*/
                                  /* first mirror begins */
       char         snd_alt_vol;  /* pv where partition allocation for*/
                                  /* second mirror begins */ 
       short        fst_alt_part; /* partition to begin first mirror */
       short        snd_alt_part; /*partition to begin second mirror */
       double       res_3;        /* reserved area  on disk */
       double       res_4;        /* reserved area on disk */
    };

struct namelist
{
   char       name[LVM_MAXLVS][LVM_NAMESIZ];
};
 
struct vg_trailer
{
      struct timestruc_t   timestamp; /*  time of last update */
      double                res_1;     /* reserved area on disk */
      double                res_2;     /* reserved area on disk */
      double                res_3;     /* reserved area on disk */
};



/*
 *  The following structures are used in lvm_varyonvg
 */

struct da_sa_info
	 /* structure to contain timestamp information about a volume
	    group descriptor or status area */
       {
       struct timestruc_t ts_beg;
	 /* beginning timestamp value */
       struct timestruc_t ts_end;
	 /* ending timestamp value */
       short int ts_status;
	 /* indicates if read error on either timestamp, or if both good
	    indicates if beginning ts equal or greater than ending ts */
       short int wrt_order;
	 /* indicates order in which to write this VGDA or VGSA copy */
       short int wrt_status;
	 /* indicates whether this VGDA or VGSA copy is to be written */
       };

struct inpvs_info
	 /* information structure for PVs in the user's input list */
       {
       struct
	      {
	      int fd;
		/* file descriptor for open of physical disk */
	      struct unique_id pv_id;
		/* the unique id for this physical volume */
	      dev_t device;
		/* the major/minor number of the physical volume */
	      daddr_t da_psn [LVM_PVMAXVGDAS];
		/* physical sector number (PSN) of beginning of the
		   volume group descriptor area (primary and secondary
		   copies), or 0 if none */
	      daddr_t sa_psn [LVM_PVMAXVGDAS];
		/* PSN of beginning of the volume group status area
		   (primary and secondary copies), or 0 if none */
	      daddr_t reloc_psn;
		/* PSN of the beginning of the bad block relocation
		   pool */
	      long reloc_len;
		/* the length in blocks of the bad block relocation
		   pool */
	      short int pv_num;
		/* the number of the physical volume */
	      short int pv_status;
		/* status of the physical volume */
#define  LVM_NOTVLDPV    0      /* non valid physical volume */
#define  LVM_VALIDPV     1      /* valid physical volume */
	      struct da_sa_info da [LVM_PVMAXVGDAS];
		/* array of structures to contain timestamp information
		   about VGDAs on one PV */
	      short int index_newestda;
		/* index of VGDA copy on PV which has newest timestamp */
	      short int index_nextda;
		/* index of VGDA copy on PV which is next written */
	      } pv [LVM_MAXPVS];
		  /* array of physical volumes, indexed by order in input
		     parameter list */
       long lvmarea_len;
	 /* the length of the entire LVM reserved area on disk */
       long vgda_len;
	 /* length of the volume group descriptor area */
       long vgsa_len;
	 /* length of the volume group status area */
       short int num_desclps;
	 /* the number of logical partitions per PV needed for the
	    descriptor / status area logical volume */
       short int pp_size;
	 /* the size of a physical partition for this volume group */
       };

struct mwc_info
	/* structure to contain timestamp information about a mirror
           write cache area */
	{
	struct timestruc_t ts;
  	  /* timestamp value */
	short int good_mwcc;
	  /* flag which indicates if the MWCC could not be read */
	short int wrt_status;
	  /* indicates whether this MWCC is to be written */
	};

struct defpvs_info
	 /* information structure for PVs defined into the kernel */
       {
       struct
	      {
	      short int in_index;
		/* corresponding index into the input PV information
		   structure for this PV */
	      short int pv_status;
		/* indicates if this PV is defined into the kernel */
#define  LVM_NOTDFND     0     /* this PV not defined in kernel */
#define  LVM_DEFINED     1     /* this PV defined in kernel */
	      struct da_sa_info sa[LVM_PVMAXVGDAS];
		/* array of structures to contain timestamp information
		   about VGSAs on one PV */
	      struct mwc_info mwc;
		/* structure to contain information about the mirror
 		   write consistency cache on this PV */
	      } pv [LVM_MAXPVS];
		  /* array of physical volumes indexed by PV number */
       int total_vgdas;
	 /* total number of volume group descriptor/status areas */
       struct timestruc_t newest_dats;
	 /* newest good timestamp for the volume group descriptor area */
       struct timestruc_t newest_sats;
	 /* newest good timestamp for the volume group status area */
       struct timestruc_t newest_mwcts;
	 /* timestamp for newwest mirror write consistency cach */
       };


/*
 *  Function declarations
 */

#ifndef _NO_PROTO

/*
 *  bbdirutl.c
 */

int lvm_bbdsane (
char * buf);
  /* buffer containing the directory to check */

int lvm_getbbdir (
int pv_fd,
  /* the file descriptor for this physical volume device */
char * buf,
  /* a buffer into which the bad block directory will be read */
int dir_flg);
  /* flags to indicate which directory to read */

int lvm_rdbbdir (
int pv_fd,
  /* the file descriptor for this physical volume device */
char * buf,
  /* a buffer into which the bad block directory will be read */
int act_flg);
  /* flag to indicate type of action requested */

int lvm_wrbbdir (
int pv_fd,
  /* the file descriptor for this physical volume device */
char * bbdir_buf,
  /* a buffer containing the bad block directory */
int dir_flg);
  /* flags to indicate which directory to write */


/*
 *  bblstutl.c
 */

void lvm_addbb (
struct bad_blk ** head_ptr,
  /* a pointer to the pointer to the head of the bad block linked list */
struct bad_blk * bb_ptr);
  /* pointer to the bad block structure which is to be added to the
     list */

int lvm_bldbblst (
int pv_fd,
  /* the file descriptor for this physical volume device */
struct pvol * pvol_ptr,
  /* a pointer to a structure which describes a physical volume for the
     logical volume device driver (LVDD) */
daddr_t reloc_psn);
  /* the physical sector number of the beginning of the bad block
     relocation pool */

void lvm_chgbb (
struct bad_blk * head_ptr,
  /* a pointer to the head of the bad block linked list */
daddr_t bad_blk,
  /* the bad block whose data is to be changed */
daddr_t reloc_blk,
  /* the new value for the relocation block, if it is to be changed */
int chgtype);
  /* type of change requested (change relocation block or status field)
     for this bad block */


/*
 *  chkquorum.c
 */

int lvm_chkquorum (
struct varyonvg * varyonvg,
  /* pointer to the structure which contains input parameter data for
     the lvm_varyonvg routine */
int vg_fd,
  /* the file descriptor for the volume group reserved area logical
     volume */
struct inpvs_info * inpvs_info,
  /* structure which contains information about the input list of PVs
     for the volume group */
struct defpvs_info * defpvs_info,
  /* structure which contains information about volume group descriptor
     areas and status areas for the defined PVs in the volume group */
caddr_t vgda_ptr,
  /* pointer to the volume group descriptor area */
struct vgsa_area **vgsa_ptr,
  /* pointer to the volume group status area */
daddr_t vgsa_lsn [LVM_MAXPVS] [LVM_PVMAXVGDAS],
  /* array in which to store the logical sector number addresses of the
     VGSAs for each PV */
char mwcc [DBSIZE]);
  /* buffer in which the latest mirror write consistency cache will be
     returned */

int lvm_vgsamwcc (
int vg_fd,
  /* file descriptor for the VG reserved area logical volume which
     contains the volume group descriptor area and status area */
struct inpvs_info * inpvs_info,
  /* structure which contains information about the input list of PVs for
     the volume group */
struct defpvs_info * defpvs_info,
  /* pointer to structure which contains information about PVs defined
     into the kernel */
caddr_t vgda_ptr,
  /* pointer to the volume group descriptor area */
long quorum,
  /* number of VGDAs/VGSAs needed to varyon in order to ensure that the
     volume group data is consistent with that from previous varyon */
struct vgsa_area ** vgsa_ptr,
  /* variable to contain the pointer to the buffer which will contain
     the volume group status area */
daddr_t vgsa_lsn [LVM_MAXPVS] [LVM_PVMAXVGDAS],
  /* array in which to store the logical sector number addresses of the
     VGSAs for each PV */
char mwcc [DBSIZE]);
  /* buffer in which the latest mirror write consistency cache will be
     returned */


/*
 *  comutl.c
 */

int lvm_chkvaryon (
struct unique_id * vg_id);
  /* the id of the volume group */

void lvm_mapoff (
struct fheader * mapfilehdr,
  /* a pointer to the mapped file header which contains the offsets of
     the different data areas within the mapped file */
caddr_t vgda_ptr);
  /* a pointer to the beginning of the volume group descriptor area */

int lvm_getvgdef (
struct unique_id * vg_id,
  /* a pointer to the volume group id */
int mapf_mode,
  /* the access mode with which to open the mapped file */
int * vgmap_fd,
  /* pointer to variable in which to return the file descriptor of the
     mapped file */
caddr_t * vgmap_ptr);
  /* pointer to the variable in which to return the pointer to the
     beginning of the mapped file */

int lvm_relocmwcc(
int pv_fd,
  /* file descriptor of physical volume where block containing the mirror
     write consistency cache needs to be relocated */
char mwcc [DBSIZE]);
  /* buffer which contains data to be written to mirror write consistency
     cache */

int lvm_rdiplrec (
caddr_t pvname_ptr,
int pv_fd,
  /* the file descriptor for the physical volume device */
IPL_REC_PTR ipl_rec);
  /* a pointer to the buffer into which the IPL record will be read */

int lvm_tscomp (
struct timestruc_t * ts1,
  /* first timestamp value */
struct timestruc_t * ts2);
  /* second timestamp value */

int lvm_updtime (
struct timestruc_t * beg_time,
  /* a pointer to the beginning timestamp to be updated */
struct timestruc_t * end_time);
  /* a pointer to the ending timestamp to be updated */


/*
 *  crtinsutl.c
 */

int lvm_initbbdir (
int pv_fd,
  /* the file descriptor for the physical volume device */
daddr_t reloc_psn);
  /* the physical sector number of the beginning of the bad block
     relocation pool */

void lvm_initlvmrec (
struct lvm_rec * lvm_rec,
  /* pointer to the LVM information record */
short int vgda_size,
  /* the length of the volume group descriptor area in blocks */
short int ppsize,
  /* physical partition size represented as a power of 2 */
long data_capacity);
  /* the data capacity of the disk in number of blocks */

int lvm_instsetup (
struct unique_id * vg_id,
  /* pointer to id of the volume group into which the PV is to be
     installed */
char * pv_name,
  /* a pointer to the name of the physical volume to be added to the
     volume group */
short int override,
  /* flag for which a true value indicates to override a VG member error,
     if it occurs, and install the physical volume into the indicated
     volume group */
struct unique_id * cur_vg_id,
  /* structure in which to return the volume group id, if this PV's
     LVM record indicates it is already a member of a volume group */
int * pv_fd,
  /* a pointer to where the file descriptor for the physical volume
     device will be stored */
IPL_REC *ipl_rec,
  /* a pointer to the block into which the IPL record will be read */
struct lvm_rec * lvm_rec,
  /* a pointer to the block into which the LVM information record will
     be read */
long * data_capacity);
  /* the data capacity of the disk in number of sectors */

void lvm_pventry (
struct unique_id * pv_id,
  /* pointer to a structure which contains id for the physical volume for
     which the entry is to be created */
struct vg_header * vghdr_ptr,
  /* a pointer to the volume group header of the descriptor area */
struct pv_header ** pv_ptr,
  /* a pointer to the beginning of the list of physical volume entries
     in the descriptor area */
long num_parts,
  /* the number of partitions available on this physical volume */
daddr_t beg_psn,
  /* the physical sector number of the first physical partition on this
     physical volume */
short int num_vgdas);
  /* the number of volume group descriptor areas which are to be placed
     on this physical volume */

int lvm_vgdas3to3 (
int lv_fd,
  /* the file descriptor of the LVM reserved area logical volume */
caddr_t vgmap_ptr,
  /* pointer to the beginning of the mapped file */
short int new_pv,
  /* the PV number of the new physical volume which is being added */
short int sav_pv_2,
  /* the PV number of the physical volume which previously had two copies
     of the VGDA */
short int sav_pv_1);
  /* the PV number of the physical volume which previously had one copy
     of the VGDA */

int lvm_vgmem (
struct unique_id * pv_id,
  /* pointer to id of the physical volume for which we are to determine
     membership in the specified VG */
caddr_t vgda_ptr);
  /* pointer to the beginning of the volume group descriptor area */

int lvm_zeromwc (
int pv_fd,
  /* the file descriptor of the physical volume */
short int newvg);
  /* flag to indicate if this is newly created volume group */

int lvm_zerosa (
int lv_fd,
  /* the file descriptor for the LVM reserved area logical volume */
daddr_t sa_lsn [LVM_PVMAXVGDAS]);
  /* the logical sector numbers within the LVM reserved area logical
     volume of where to initialize the copies of the volume group status
     area */


/*
 *  configutl.c
 */

int lvm_addmpv (
struct unique_id * vg_id,
  /* the volume group id of the volume group which is to be added into
     the kernel */
long vg_major,
  /* the major number where the volume group is to be added */
short int pv_num);
  /* number of the PV to be deleted from the volume group */

int lvm_addpv (
long partlen_blks,
  /* the length of a partition in number of 512 byte blocks */
short int num_desclps,
  /* the number of partitions needed on each physical volume to contain
     the LVM reserved area */
dev_t device,
  /* the major / minor number of the device */
int pv_fd,
  /* the file descriptor of the physical volume device */
short int pv_num,
  /* the index number for this physical volume */
long vg_major,
  /* the major number of the volume group */
struct unique_id * vg_id,
  /* the volume group id of the volume group to which the physical
     volume is to be added */
daddr_t reloc_psn,
  /* the physical sector number of the beginning of the bad block
     relocation pool */
long reloc_len,
  /* the length of the bad block relocation pool */
daddr_t psn_part1, 
  /* the phsyical sector number of the first partition on the physical
     volume */
daddr_t vgsa_lsn[LVM_PVMAXVGDAS],
short int quorum_cnt);
  /* the number of VGDAs/VGSAs needed for a quorum */

int lvm_chgqrm (
struct unique_id * vg_id,
  /* the volume group id of the volume group */
long vg_major,
  /* the major number of the volume group */
short int quorum_cnt);
  /* number of VGDA/VGSA copies needed for a quorum */

int lvm_chgvgsa (
struct unique_id * vg_id,
  /* the volume group id */
long vg_major,
  /* the major number of the volume group */
daddr_t vgsa_lsn [LVM_PVMAXVGDAS],
  /* array of logical sector number addresses of the VGSA copies on this
     PV */
short int pv_num,
  /* number of the PV which is to have changes to the number of VGSAs */
short int quorum_cnt,
  /* number of VGDAs/VGSAs needed for a quorum */
int command);
  /* command value which indicates the config routine to be called is
     that for adding/deleting VGSAs */

int lvm_chkvgstat (
struct varyonvg * varyonvg,
  /* pointer to the structure which contains input information for
     varyonvg */
int * vgstatus);
  /* pointer to variable to contain the varied on status of the volume
     group */

int lvm_config (
mid_t kmid,
  /* the module id for the object module which contains the logical
     volume device driver */
long vg_major,
  /* the major number of the volume group */
int request,
  /* the request for the configuration routine to be called within the
     kernel hd_config routine */
struct ddi_info * cfgdata);
  /* structure to contain the input parameters for the configuration
     device driver */

int lvm_defvg (
long partlen_blks,
  /* the length of a partition in number of 512 byte blocks */
short int num_desclps,
  /* the number of partitions needed on each physical volume to contain
     the LVM reserved area */
mid_t kmid,
  /* the module id which identifies where the LVDD code is loaded */
long vg_major,
  /* the major number where the volume group is to be added */
struct unique_id * vg_id,
  /* the volume group id of the volume group which is to be added into
     the kernel */
short int ppsize,
  /* the physical partition size, represented as a power of 2 of the
     size in bytes, for partitions in this volume group */

long noopen_lvs,
  /* flag to indicate if logical volumes in the volume group are not
     allowed to be opened */
long noquorum
);

int lvm_delpv (
struct unique_id * vg_id,
  /* the volume group id of the volume group which is to be added into
     the kernel */
long vg_major,
  /* the major number where the volume group is to be added */
short int pv_num,
  /* number of the PV to be deleted from the volume group */
short int num_desclps,
  /* number of logical partitions in the descriptor / status area
     logical volume for this PV */
int flag, 
  /* flag to indicate whether the PV is being deleted from the volume
     group or just temporarily removed */
short int quorum_cnt);
  /* quorum count of logical volume */

void lvm_delvg (
struct unique_id * vg_id,
  /* the volume group id of the volume group which is to be added into
     the kernel */
long vg_major);
  /* the major number where the volume group is to be added */


/*
 *  lvmrecutl.c
 */

void lvm_cmplvmrec(
struct unique_id *vgid,                 /* pointer to volume group id */
char             *match,                /* indicates a matching vgid */
char		 pvname[LVM_NAMESIZ]);  /* name of pv to read lvm rec from */

int lvm_rdlvmrec (
int pv_fd,
  /* the file descriptor for the physical volume device */
struct lvm_rec * lvm_rec);
  /* a pointer to the buffer into which the LVM information record will
     be read */

int lvm_wrlvmrec (
int pv_fd,
  /* the file descriptor for the physical volume device */
struct lvm_rec * lvm_rec);
  /* a pointer to the buffer which contains the LVM information record
     to be written */

void lvm_zerolvm (
int pv_fd);
  /* the file descriptor for the physical volume device */

short lvm_getversion (
struct fheader *fhead);

void lvm_updlvmrec (
char *vgptr,
short version);


/*
 *  queryutl.c
 */

extern int lvm_chklvclos (
struct lv_id * lv_id,
  /* logical volume id */
long major_num);
  /* major number of volume group */

extern int lvm_getpvda (
char * pv_name,
  /* a pointer to the name of the physical volume to be added to the
     volume group */
char ** map_ptr, 
  /* a pointer to where the pointer to the memory area containing the
     mapped file information will be stored */
int rebuild);
  /* indicates we are rebuilding the vg file */

extern int lvm_gettsinfo(
int     pvfd,	/* file descriptor for physical volume */
daddr_t psn[LVM_PVMAXVGDAS],
		/* array of physical sector numbers for VGDAS */
long    vgdalen,
		/* length of volume group descriptor area */
int     *copy,  /* copy of VGDA with newest timestamp */
int     rebuild); 
		/* indicates we are rebuilding the vg file */


/*
 * rdex_com.c
 */

extern int rdex_proc(
 struct lv_id     *lv_id,       /* logical volume id */
 struct ext_redlv *ext_red,     /* maps of pps to be extended or reduced */
 char             *vgfptr,      /* pointer to volume group mapped file */
 int              vgfd,         /* volume group file descriptor */
 short            minor_num,    /* minor number of logical volume */
 int              indicator);   /* indicator for extend or reduce operation */


/*
 *  revaryon.c
 */

int lvm_revaryon (
struct varyonvg * varyonvg,
  /* pointer to a structure which contains the input information for
     the lvm_varyonvg subroutine */
int *vgmap_fd,
  /* the file descriptor for the mapped file */
struct inpvs_info * inpvs_info,
  /* a pointer to the structure which contains information about PVs
     from the input list */
struct defpvs_info * defpvs_info);
  /* structure which contains information about volume group descriptor
     areas and status areas for the defined PVs in the volume group */

int lvm_vonmisspv (
struct varyonvg * varyonvg,
  /* pointer to a structure which contains the input information for
     the lvm_varyonvg subroutine */
struct inpvs_info * inpvs_info,
  /* a pointer to the structure which contains information about PVs
     from the input list */
struct defpvs_info * defpvs_info,
  /* structure which contains information about volume group descriptor
     areas and status areas for the defined PVs in the volume group */
struct fheader * maphdr_ptr,
  /* pointer to the mapped file header */
caddr_t vgda_ptr,
  /* pointer to the beginning of the volume group descriptor area */
struct pv_header * pv_ptr,
  /* a pointer to the header of a physical volume entry in the volume
     group descriptor area */
int vg_fd,
  /* the file descriptor for the volume group reserved area logical
     volume */
short int in_index,
  /* index into the input list of a physical volume */
int * chkvvgout);
  /* flag to indicate if the varyonvg output structure should be
     checked */


/*
 *  setupvg.c
 */

int lvm_setupvg (
struct varyonvg * varyonvg,
  /* pointer to the structure which contains input information for
     varyonvg */
struct inpvs_info * inpvs_info,
  /* a pointer to the structure which contains information about PVs
     from the input list */
struct defpvs_info * defpvs_info,
  /* pointer to structure which contains information about the physical
     volumes defined into the kernel */
struct fheader * maphdr_ptr,
  /* a pointer to the file header portion of the mapped file */
int vg_fd,
  /* file descriptor of the volume group reserved area logical volume */
caddr_t vgda_ptr,
  /* a pointer to the in-memory copy of the volume group descriptor
     area */
struct vgsa_area * vgsa_ptr,
  /* a pointer to the volume group status area */
daddr_t vgsa_lsn [LVM_MAXPVS] [LVM_PVMAXVGDAS],
  /* array of logical sector number addresses of all VGSA copies */
struct mwc_rec * mwcc);
  /* buffer which contains latest mirror write consistency cache */

int lvm_bldklvlp (
caddr_t vgda_ptr,
  /* a pointer to the volume group descriptor area */
struct vgsa_area * vgsa_ptr,
  /* a pointer to the volume group status area */
struct lvol * lvol_ptrs [LVM_MAXLVS]);
  /* array of pointers to the LVDD logical volume structures */

int lvm_mwcinfo (
struct varyonvg * varyonvg,
  /* pointer to the structure which contains input information for
     varyonvg */
struct inpvs_info * inpvs_info,
  /* a pointer to the structure which contains information about PVs
     from the input list */
struct defpvs_info * defpvs_info,
  /* pointer to structure which contains information about the physical
     volumes defined into the kernel */
struct fheader * maphdr_ptr,
  /* a pointer to the file header portion of the mapped file */
int vg_fd,
  /* file descriptor of the volume group reserved area logical volume */
caddr_t vgda_ptr,
  /* a pointer to the volume group descriptor area */
struct vgsa_area * vgsa_ptr,
  /* a pointer to the volume group status area */
daddr_t vgsa_lsn [LVM_MAXPVS] [LVM_PVMAXVGDAS],
  /* array of logical sector number addresses of all VGSA copies */
struct lvol * lvol_ptrs [LVM_MAXLVS],
  /* array of pointers to LVDD logical structures */
struct mwc_rec * mwcc,
  /* buffer which contains latest mirror write consistency cache */
struct mwc_rec * kmwcc,
  /* buffer to contain list of logical track groups from the MWCC which
     need to be resynced in the kernel */
short int * num_entries);
  /* number of logical track group entries in the kernel MWCC buffer */


/*
 * synclp.c
 */

extern int synclp(
 int                lvfd,       /* logical volume file descriptor */
 struct lv_entries  *lv,        /* pointer to logical volume entry */
 struct unique_id   *vg_id,     /* volume group id */
 char               *vgptr,     /* pointer to the volume group mapped file */
 int                vgfd,       /* volume group mapped file file descriptor */
 short              minor_num,  /* minor number of the logical volume */
 long               lpnum,      /* logical partition number to sync */
 int		    force);	/* resync any non-stale lp if TRUE */


/*
 *  utilities.c
 */

extern int get_lvinfo(
 struct lv_id     *lv_id,       /* logical volume id */
 struct unique_id *vg_id,       /* volume group id */
 short            *minor_num,   /* logical volume minor number */
 int              *vgfd,        /* volume group file descriptor */
 char             **vgptr,      /* pointer to volume group mapped file */
 int              mode);        /* how to open the vg mapped file */

extern int get_ptrs(
char *vgmptr,               /* pointer to the beginning of the volume */
                            /* group mapped file */
struct fheader **header,    /* points to the file header */
struct vg_header **vgptr,   /* points to the volume group header */
struct lv_entries **lvptr,  /* points to the logical volume entries */
struct pv_header **pvptr,   /* points to the physical volume header */
struct pp_entries **pp_ptr, /* points to the physical partition entries */
struct namelist **nameptr); /* points to the name descriptor area */

extern int lvm_errors(
 char     failing_rtn[LVM_NAMESIZ],     /* name of routine with error */
 char     calling_rtn[LVM_NAMESIZ],     /* name of calling routine */
 int      rc);                          /* error returned from failing rtn */

extern int get_pvandpp(
 struct pv_header   **pv,       /* pointer to the physical volume header */
 struct pp_entries  **pp,       /* pointer to the physical partition entry */
 short               *pvnum,     /* pv number of physical volume id sent in */
 char               *vgfptr,    /* pointer to the volume group mapped file */
 struct unique_id   *id);       /* id of pv you need a pointer to */

extern int bldlvinfo(
 struct logview     **lp,       /* pointer to logical view of a logical vol */
 char               *vgfptr,    /* pointer to volume group mapped file */
 struct lv_entries  *lv,        /* pointer to a logical volume */
 long               *cnt,       /* number of pps per copy of logical volume */
 short              minor_num,  /* minor number of logical volume */
 int 		    flag);	/* GETSTALE if info on stale pps is desired */
				/* NOSTALE of not */

extern int status_chk(
 char    *vgptr,        /* pointer to volume group mapped file */
 char    *name,         /* name of device to be checked */
 int     flag,          /* indicator to check the major number */
 char    *rawname);     /* pointer to new raw device name */

extern int lvm_special_3to2(
 char                   *pvname,
				/* name of physical volume being removed */
 struct unique_id       *vgid,
				/* pointer to the volume group id */
 int                    lvfd,
				/* reserved area logical volume file desc */
 char                   *vgdaptr,
				/* pointer to vgda area of the vg file */
 short 			pvnum0, /* number of pv to delete/remove */ 
 short 			pvnum1, /* number of pv to keep one copy */ 
 short 			pvnum2, /* number of pv to keep two copies */ 
 char                   *match,
				/* indicates the vgid in the lvm record */
				/* matches the one passed in */
 char                   delete, /* indicaes we are called by deletepv */
 struct fheader         *fhead); /* pointer to vg mapped file header */

extern  int getstates(
 struct vgsa_area   *vgsa,      /* pointer to buffer for volume group status */
				/* area */
 char               *vgfptr);   /* pointer to volume group mapped file */

extern int timestamp(
 struct vg_header *vg,		/* pointer to volume group header */
 char		  *vgptr,	/* pointer to volume group file */
 struct fheader   *fhead);	/* pointer to vg file header */

extern int timestamp(
 struct vg_header *vg,		/* pointer to volume group header */
 char             *vgptr,	/* pointer to volume group file */
 struct fheader   *fhead);	/* pointer to file header of vg file */

extern int buildname(
 dev_t	dev,			/* device info for physical volume */
 char	name[LVM_EXTNAME],	/* array to store name we create for pv */
 int    mode, 			/* mode to set the device entry to */ 
 int    type);			/* type of name to build */

extern int getlvmrec(
char * pv_name,
struct lvm_rec * lvm_rec_ptr);

/*
 *  varyonvg.c
 */

int lvm_forceqrm (
caddr_t vgda_ptr,
  /* pointer to the volume group descriptor area */
struct defpvs_info * defpvs_info,
  /* structure which contains information about volume group descriptor
     areas and status areas for the defined PVs in the volume group */
struct inpvs_info * inpvs_info,
  /* a pointer to the structure which contains information about PVs
     from the input list */
daddr_t vgsa_lsn [LVM_MAXPVS] [LVM_PVMAXVGDAS]);
  /* array of logical sector number addresses of all VGSA copies */

void lvm_mapfile (
struct varyonvg * varyonvg,
  /* pointer to a structure which contains the input information for
     the lvm_varyonvg subroutine */
struct inpvs_info * inpvs_info,
  /* a pointer to the structure which contains information about PVs
     from the input list */
struct defpvs_info * defpvs_info,
  /* pointer to structure which contains information about PVs which are
     defined into the kernel */
struct fheader * mapfilehdr,
  /* a pointer to the mapped file header which contains the offsets of
     the different data areas within the mapped file */
caddr_t vgda_ptr);
  /* pointer to the volume group descritptor area */

void lvm_pvstatus (
struct varyonvg * varyonvg,
  /* pointer to the structure which contains input parameter data for
     the lvm_varyonvg routine */
struct defpvs_info * defpvs_info,
  /* pointer to structure which contains information about PVs which are
     defined into the kernel */
caddr_t vgda_ptr,
  /* pointer to the volume group descriptor area */
int * missname,
  /* flag to indicate if there are any PV names missing from the input
     list */
int * misspv);
  /* flag to indicate if there are any PVs missing from the varied-on
     volume group (i.e., PVs that could not be defined into the kernel */

int lvm_update (
struct varyonvg * varyonvg,
  /* pointer to the structure which contains input information for
     varyonvg */
struct inpvs_info * inpvs_info,
  /* a pointer to the structure which contains information about PVs
     from the input list */
struct defpvs_info * defpvs_info,
  /* structure which contains information about volume group descriptor
     areas and status areas for the defined PVs in the volume group */
struct fheader * maphdr_ptr,
  /* a pointer to the file header portion of the mapped file */
int vg_fd,
  /* the file descriptor for the volume group reserved area logical
     volume */
caddr_t vgda_ptr,
  /* pointer to the volume group descriptor area */
struct vgsa_area * vgsa_ptr,
  /* pointer to the volume group status area */
daddr_t vgsa_lsn [LVM_MAXPVS] [LVM_PVMAXVGDAS],
  /* array which contains the logical sector number addresses of all
     the VGSAs */
char mwcc [DBSIZE],
  /* buffer containing the latest mirror write consistency cache */
int forceqrm,
  /* flag to indicate if the quorum has been forced */
int misspv);
  /* flag to indicate if there were any missing PVs */


/*
 *  verify.c
 */

int lvm_verify (
struct varyonvg * varyonvg,
  /* pointer to the structure which contains input parameter data for
     the lvm_varyonvg routine */
int * vg_fd,
  /* pointer to the variable to contain the file descriptor for the
     volume group reserved area logical volume */
struct inpvs_info * inpvs_info,
  /* structure which contains information about the input list of PVs
     for the volume group */
struct defpvs_info * defpvs_info,
  /* structure which contains information about volume group descriptor
     areas and status areas for the defined PVs in the volume group */
caddr_t * vgda_ptr,
  /* pointer to variable where the pointer to the volume group descriptor
     area is to be returned */
struct vgsa_area  **vgsa_ptr, 
  /* variable to contain the pointer to the buffer which will contain the
     volume group status area */
daddr_t vgsa_lsn [LVM_MAXPVS] [LVM_PVMAXVGDAS],
  /* array in which to store the logical sector number addresses of the
     VGSAs for each PV */
char mwcc [DBSIZE]);
  /* buffer in which the latest mirror write consistency cache will be
     returned */

int lvm_defpvs (
struct varyonvg * varyonvg,
  /* pointer to the structure which contains input parameter data for
     the lvm_varyonvg routine */
int vg_fd,
  /* file descriptor of the volume group reserved area logical volume */
struct inpvs_info * inpvs_info,
  /* structure which contains information about the input list of PVs for
     the volume group */
struct defpvs_info * defpvs_info);
  /* structure which contains information about the volume group
     descriptor and status areas for PVs defined into the kernel */

void lvm_getdainfo (
int vg_fd,
  /* file descriptor for the VG reserved area logical volume which
     contains the volume group descriptor area and status area */
short int pv_index,
  /* index variable for looping on physical volumes in input list */
struct inpvs_info * inpvs_info,
  /* structure which contains information about the input list of PVs for
     the volume group */
struct defpvs_info * defpvs_info);
  /* pointer to structure which contains information about PVs defined
     into the kernel */

int lvm_readpvs (
struct varyonvg * varyonvg,
  /* pointer to the structure which contains input parameter data for
     the lvm_varyonvg routine */
struct inpvs_info * inpvs_info);
  /* structure which contains information about the input list of PVs
     for the volume group */

int lvm_readvgda (
int vg_fd,
  /* the file descriptor for the volume group reserved area logical
     volume */
short int override,
  /* flag which indicates if no quorum error is to be overridden */
struct inpvs_info * inpvs_info,
  /* structure which contains information about the input list of PVs
     for the volume group */
struct defpvs_info * defpvs_info,
  /* structure which contains information about volume group descriptor
     areas and status areas for the defined PVs in the volume group */
caddr_t * vgda_ptr);
  /* pointer to buffer in which to read the volume group descriptor
     area */

/*
 *  vonutl.c
 */

void lvm_clsinpvs (
struct varyonvg * varyonvg,
  /* pointer to the structure which contains input parameter data for
     the lvm_varyonvg routine */
struct inpvs_info * inpvs_info);
  /* structure which contains information about the input list of PVs
     for the volume group */

int lvm_deladdm (
struct varyonvg * varyonvg,
  /* pointer to the structure which contains input information for
     varyonvg */
struct inpvs_info * inpvs_info,
  /* a pointer to the structure which contains information about PVs
     from the input list */
struct defpvs_info * defpvs_info,
  /* pointer to structure which contains information about the physical
     volumes defined into the kernel */
short int pv_num);
  /* the PV number of PV being changed to a missing PV */

int lvm_vonresync (
struct unique_id * vg_id);
  /* pointer to the volume group id */



/*
 *  wrtutl.c
 */

int lvm_diskio (
caddr_t vg_mapptr,
  /* a pointer to the mapped file for this volume group */
int vg_mapfd);
  /* the file descriptor of the mapped file for this volume group */

int lvm_updvgda (
int lv_fd,
  /* the file descriptor of the descriptor area logical volume, if it
     is already open */
struct fheader * maphdr_ptr,
  /* a pointer to the file header portion of the mapped file */
caddr_t vgda_ptr);
  /* a pointer to the memory location which holds the volume group
     descriptor area */

int lvm_wrtdasa (
int vg_fd,
  /* the file descriptor of the LVM reserved area logical volume */
caddr_t area_ptr,
  /* a pointer to the memory location which holds the volume group
     descriptor or status area */
struct timestruc_t * e_timestamp,
  /* a pointer to the end timestamp for the area */
long area_len,
  /* the length in sectors of the area */
daddr_t lsn,
  /* the logical sector number within the LVM reserved area logical
     volume of where to write a copy of the area */
short int write_order);
  /* flag which indicates whether the area is to be written in the order
     of beginning/middle/end or middle/beginning/end */

int lvm_wrtnext (
int lv_fd,
  /* the file descriptor of the LVM reserved area logical volume */
caddr_t vgda_ptr,
  /* a pointer to the memory location which holds the volume group
     descriptor area */
struct timestruc_t *etimestamp,
  /* pointer to the ending timestamp in the vg trailer */
short int pvnum,
  /* the PV number of the PV to which the VGDA is to be written */
struct fheader * maphdr_ptr,
  /* pointer to the mapped file header */
short int pvnum_vgdas);
  /* number of volume group descriptor areas to be written to the PV */

void get_odm_pvname(
char *pvid,
char *pvname);

#else

/*
 *  bbdirutl.c
 */

int lvm_bbdsane ( );

int lvm_getbbdir ( );

int lvm_rdbbdir ( );

int lvm_wrbbdir ( );


/*
 *  bblstutl.c
 */

void lvm_addbb ( );

int lvm_bldbblst ( );

void lvm_chgbb ( );


/*
 *  chkquorum.c
 */

int lvm_chkquorum ( );

int lvm_vgsamwcc();


/*
 *  comutl.c
 */

int lvm_chkvaryon ( );

int lvm_mapoff ( );

int lvm_getvgdef ();

int lvm_relocmwcc();

int lvm_rdiplrec ( );

int lvm_tscomp ( );

int lvm_updtime ( );


/*
 *  crtinsutl.c
 */

int lvm_initbbdir ( );

void lvm_initlvmrec ( );

int lvm_instsetup ( );

void lvm_pventry ( );

int lvm_vgdas3to3 ( );

int lvm_vgmem ( );

int lvm_zeromwc ( );

int lvm_zerosa ( );


/*
 *  configutl.c
 */

int lvm_addmpv ( );

int lvm_addpv ( );

int lvm_chgvgsa ( );

int lvm_chkvgstat ( );

int lvm_config ( );

int lvm_defvg ( );

int lvm_delpv ( );

void lvm_delvg ( );


/*
 *  lvmrecutl.c
 */

void lvm_cmplvmrec ( );

int lvm_rdlvmrec ( );

int lvm_wrlvmrec ( );

void lvm_zerolvm ( );

short lvm_getversion ( );

void lvm_updlvmrec ( );


/*
 *  queryutl.c
 */

extern int lvm_chklvclos ( );

extern int lvm_getpvda ( );

extern int lvm_gettsinfo( );


/*
 *  rdex_com.c
 */

extern int rdex_proc();
/* struct lv_id     *lv_id       logical volume id
   struct ext_redlv *ext_red     maps of pps to be extended or reduced
   int              indicator    indicator for extend or reduce operation */


/*
 *  revaryon.c
 */

int lvm_revaryon ( );

void lvm_vonmisspv ( );


/*
 *  setupvg.c
 */

int lvm_setupvg();

int lvm_bldklvlp ( );

int lvm_mwcinfo ( );


/*
 *  synclp.c
 */

extern int synclp();
/*  int                lvfd      logical volume file descriptor
    struct lv_entries  *lv       pointer to logical volume entry
    char               *vgptr    pointer to the volume group mapped file
    int                vgfd      volume group mapped file file descriptor
    short              minor_num  minor number of the logical volume
    int                lpnum     logical partition number to sync
    struct unique_id   *vg_id    volume group id
    int		       force     resync any non-stale lp if TRUE */


/*
 *  utilities.c
 */

extern int get_lvinfo();
/* struct unique_id *vg_id      volume group id
   struct lv_id     *lv_id      logical volume id
   int              *vgfd       volume group file descriptor
   short            *minor_num  logical volume minor number
   char             **vgptr     pointer to volume group mapped file
   int              mode        how to open the vg mapped file */

extern int get_ptrs();
 /*  struct fheader    **header    points to the file header
     struct vgheader   **vgptr     points to the volume group header
     struct lv_entries **lvptr     points to the logical volume entries
     struct pv_header  **pvptr     points to the physical volume header
     struct pp_entries **pp_ptr    points to the physical partition ents
     struct namelist   **nameptr   points to the name descriptor area */

extern int lvm_errors();
 /*  char     failing_rtn[LVM_NAMESIZ]   name of routine with error
     char     calling_rtn[LVM_NAMESIZ]   name of calling routine
     int      rc                         error returned from failing rtn */

extern int get_pvandpp();
 /* struct pv_header   **pv    pointer to the physical volume header
    struct pp_entries  **pp    pointer to the physical partition entry
    short              *pvnum  pv number of physical volume id sent in
    char               *vgfptr pointer to the volume group mapped file
    struct unique_id   *id     id of pv you need a pointer to

extern int bldlvinfo();
 /*  struct logview     **lp pointer to logical view of a logical vol
     char               *vgfptr pointer to volume group mapped file
     struct lv_entries  *lv pointer to a logical volume
     long               *cnt number of pps per copy of logical volume
     short              minor_num  minor number of logical volume */

extern int status_chk();
/*  char    *vgptr       pointer to volume group mapped file
    char    *name        name of device to be checked
    int     flag         indicator to check the major number
    char    *rawname     pointer to new raw device name  */

extern int lvm_special_3to2();
/* char                 *pvname,
				 name of physical volume being removed
 struct unique_id       *vgid,
				 pointer to the volume group id
 int                    lvfd,
				 reserved area logical volume file desc
 char                   *vgdaptr,
				 pointer to vgda area of the vg file
 short			pvnum0,	 number of pv to delete/remove
 short			pvnum1,	 number of pv to to keep one copy
 short			pvnum2,	 number of pv to keep two copies
 char                   match
				indicates the vgid in the lvm record matches
				the vgid passed in
 char                   delete  indicates we are called by deletepv
 struct fheader         *fhead  pointer to vg mapped file header
*/

extern int getstates();
/* struct vgsa_area   *vgsa      pointer to buffer for volume group status 
				 area 
 char               *vgfptr      pointer to volume group mapped file 
*/

extern int timestamp();
/* struct vg_header *vg,        pointer to volume group header 
 char		  *vgptr,	pointer to volume group file 
 struct fheader   *fhead);	pointer to vg file header 
*/

extern int timestamp();
/*
 struct vg_header *vg,	 	pointer to volume group header
 char             *vgptr, 	pointer to volume group file 
 struct fheader   *fhead) 	pointer to file header of vg file
*/

extern int buildname();
/* dev_t	dev,		 device info for physical volume 
 char	name[LVM_EXTNAME],	 array to store name we create for pv 
 int    mode, 			 mode to set the device entry to 
 int    type);                   type of name to build
*/

extern int getlvmrec();
/*
char * pv_name,
struct lvm_rec * lvm_rec_ptr
*/

/*
 *  varyonvg.c
 */

int lvm_deladdm ( );

int lvm_forceqrm ( );

void lvm_mapfile ( );

void lvm_pvstatus ( );

int lvm_update ( );


/*
 *  verify.c
 */

int lvm_verify ( );

void lvm_clsinpvs ( );

int lvm_defpvs ( );

void lvm_getdainfo ( );

int lvm_readpvs ( );

int lvm_readvgda ( );


/*
 *  wrtutl.c
 */

int lvm_diskio ( );

void lvm_updvgda ( );

int lvm_wrtdasa();

int lvm_wrtnext ( );

void get_odm_pvname( );

#endif  /* _NO_PROTO */

#endif  /* _H_LIBLVM */

