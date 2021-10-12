/* @(#)20 1.13  src/bos/usr/ccs/lib/libcfg/POWER/adapter.h, libcfg, bos41J, 9521A_all 5/18/95 13:22:49 */
/*
 * COMPONENT_NAME: (LIBCFG) BUSRESOLVE HEADER FILE
 *
 * FUNCTIONS: DEFINITIONS AND GLOBAL DATA STRUCTURES
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

/* prevent multiple inclusion */
#ifndef _H_ADAPTER
#define _H_ADAPTER

#include <cfgresid.h> /* For PNPIDSIZE */

/*****************************************************************************/
/*    DEFINITIONS FOR BUSRESOLVE                                             */
/*****************************************************************************/

#define SUCCESS	 0
#define FAIL    -1

#define LO 0
#define HI 1

#define NO_RESID_INDEX -1
#define NO_BUS_NUMBER  -1

/* Abstraction of packet types */

#define IRQPACKS 1
#define DMAPACKS 2
#define IOPACKS  3
#define MEMPACKS 4

/* Definitions for IRQ values and controllers */

#define INTR_CNTRLR_MASK  0xff000000
#define INTR_INSTNC_MASK  0x00ff0000
#define INTR_LEVEL_MASK   0x00008000
#define INTR_NUMBER_MASK  0x00007fff
#define INTR_RESOLVE_MASK 0xffff7fff
#define INTR_PRINT_MASK   0x000000ff
#define INTR_NOT_WIRED    0x0000ffff

#define INTR_CNTRLR(v)  (v >> 24)
#define INTR_INSTNC(v)  ((v & INTR_INSTNC_MASK) >> 16)
#define INTR_LEVEL(v)   (!(v & INTR_LEVEL_MASK))
#define INTR_EDGE(v)    (v & INTR_LEVEL_MASK)
#define INTR_NUMBER(v)  (v & INTR_NUMBER_MASK)
#define INTR_RESOLVE(v) (v & 0xffff7fff)
#define INTR_PRINT(v)   (v & INTR_PRINT_MASK)

#define AT_BYTE   0x01
#define MPIC_BYTE 0x02

#define EDGE  1
#define LEVEL 2

/*----------------------------------------------------------------------*/
/* The following enumeration is used to classify attribute value        */
/* representations so they can be processed properly. The NUMERIC value */
/* must be 0 since the reprsent member of the attribute_t struct is NOT */
/* explictly modified unless the representation is other than NUMERIC.  */
/*----------------------------------------------------------------------*/
typedef enum { 
	NOREP = 0,
	NULLVALS ,  /* Null values string in PdAt */
	LIST     ,
	RANGE
} representation_e;

/*----------------------------------------------------------------------*/
/* The following enumeration is used to classify bus resource types so  */
/* they can be processed with respect to the physical resources that    */
/* they represent. In the case of grouped attributes, a dummy attribute */
/* structure with type GROUP is used...                                 */ 
/*----------------------------------------------------------------------*/
typedef enum {
	NORES  = 0,
	ADDR      ,
	MADDR     , 
	BADDR     , 
	IOADDR    ,
	INTLVL    ,
	NSINTLVL  , 
	SINTLVL   ,
	DMALVL    ,
	GROUP     ,
} bus_resource_e;

/*---------------------------------------------------------*/
/* The following enumeration is used to classify bus types */
/* and bridge types. NOTE : the order of the values of this*/
/* enumeration defines the sort order of the adapter list. */
/* The adapter list is sorted to establish the order in    */
/* which adapters are resolved and configured.             */
/*---------------------------------------------------------*/
typedef enum {
	NONE  = 0,
	ISA      ,
	PCMCIA   ,
	PCI      ,
	MCA
} bus_type_e;

/*****************************************************************************/
/*    BUSRESOLVE STRUCTURES                                                  */
/*****************************************************************************/

/*=======================================================*/
/*  The adapter_t structure retains ADAPTER information  */
/*  pertinent to the allocation of bus resources. It is  */
/*  used by busresolve.                                  */
/*=======================================================*/

typedef void (*bus_specific_cb)(); 

typedef struct adapter {
	char             utype[UNIQUESIZE+1];  /* PdDv/PdAt unique type            */
	char             logname[NAMESIZE+1];  /* CuDv/CuAt logical name           */
	char             devid[DEVIDSIZE+1];   /* PdDv device id string            */
	char             status;               /* CuDv device status field         */
	char             unresolved;           /* Any attrib not resolved Bool     */
	char             base_adapter;         /* Phase 1 boot device Bool         */
	char             bus_extender;         /* Bus extender adapter Bool        */
	char             parent[NAMESIZE+1];   /* Parent device name               */
	char             connwhere[LOCSIZE+1]; /* ODM connwhere value              */
	char             pnpid[PNPIDSIZE+1];   /* CFG_DEVICE pnp ID string         */
	int              resid_index;          /* ROS residual device index        */
	unsigned int     resid_dev_flags;      /* ROS residual device flags        */
	unsigned long    bus_number;           /* bus_number attr val (bus only)   */
	bus_type_e       parent_bus_type;      /* PCI, ISA, etc. from odm subclass */
	bus_type_e       bridge_type;          /* from resid subtype (bridge only) */
	bus_specific_cb  share_algorithm_cb;   /* Algorithm to share interrupt     */
	struct adapter   *next;                /* Next adapter list struct         */
	struct adapter   *parent_bus;          /* Parent bus pointer               */
	struct attribute *attributes;          /* Attributes list                  */
} adapter_t;

/*============================================================*/
/*  The attribute_t structure retains ATTRIBUTE information   */
/*  for each bus resource required by an adapter. It is used  */
/*  by busresolve.                                            */
/*============================================================*/

	/*------------------------------------------------------*/
	/* The value_list_t is used to construct a linked list  */
	/* of values which are explicitly specified (as opposed */
	/* to computed from a range specification).             */
	/*------------------------------------------------------*/

typedef struct value_list {
	struct value_list *next;             /* Next element in list         */
	unsigned long     value;             /* The value itself             */
} value_list_t;

	/*------------------------------------------------------*/
	/* The value_range_t is used to construct a linked list */
	/* of ranges which define the valid values to use for   */
	/* the attribute.                                       */
	/*------------------------------------------------------*/

typedef struct value_range {
	struct value_range *next;            /* Next element in list         */
	unsigned long      lower;            /* Lowest value of valid range  */
	unsigned long      upper;            /* Upper value of valid range   */
	unsigned long      incr;             /* Increment or boundry value   */
} value_range_t;

	/*------------------------------------------------------*/
	/* The value_u is a union of a RANGE value and a LIST   */
	/* element pointer. It is used to reserve storage for   */
	/* either type.                                         */
	/*------------------------------------------------------*/

typedef union value_union {
	struct {
		value_list_t *ptr;
	} list;
	struct {
		unsigned long value;
	} range;
} value_u;

	/*------------------------------------------------------*/
	/* The valuespec_u is a union of a value_list and range */
	/* specification. It is used to carry the specification */ 
	/* for the possible values for the attribute.           */
	/*------------------------------------------------------*/

typedef union values_specification {
	struct {
		value_list_t   *head;              /* Head of values list          */
		value_list_t   *current;           /* Current list element         */
	} list;
	struct {
		value_range_t  *head;              /* Head of range list           */
		value_range_t  *current;           /* Current range element        */
	} range;
} valuespec_u;

	/*------------------------------------------------------*/
	/* The statedata_t is used to save state data for the   */
	/* attributes. Type definitions for the members must    */
	/* match thier counterparts in the attribute structure. */
	/*------------------------------------------------------*/

typedef struct state_data {
	char      changed;                     /* Value changed Bool               */
	value_u   start;                       /* Starting ptr/value for wrap chk  */
	value_u   current;                     /* Attribute's current ptr/value    */
} statedata_t;
 
/*======================================================*/
/* Finally define the attribute structure itself.       */
/*======================================================*/

typedef struct attribute {
	char             name[ATTRNAMESIZE+1]; /* PdAt/CuAt attribute name     */
	bus_resource_e   resource;             /* Attribute bus resource       */
	bus_resource_e   specific_resource;    /* Attribute specific bus resrc */
	representation_e reprsent;             /* LIST | RANGE                 */
	unsigned long    current;              /* Current value                */
	unsigned long    width;                /* Amount of resource needed    */
	unsigned long    valllim;              /* Lower boundry for attr vals  */
	unsigned long    valulim;              /* Upper boundry for attr vals  */
	unsigned long    busllim;              /* Lower boundry for bus vals   */
	unsigned long    busulim;              /* Upper boundry for bus vals   */
	unsigned long    addrmask;             /* Sig'nfcnt val bits, never 0! */
	valuespec_u      values;               /* Values for the attribute     */
	value_u          start;                /* Starting value for wrap chk  */
	char             changed;              /* Value changed Bool           */
	char             priority;             /* Interrupt priority class     */
	char             share_head;           /* Head of share list Bool      */ 
	char             ignore;               /* Ignore attr flag Bool        */
	char             user;                 /* User settable attribute Bool */
	char             cuat_append;          /* Append ,char to CuAt value   */
	char             trigger;              /* Intrupt trigr EDGE/LEVEL     */ 
	struct attribute *next;                /* Exhaustive attribute list    */
	struct attribute *share_ptr;           /* Shared attributes list       */
	struct attribute *group_ptr;           /* Grouped attributes list      */
	struct adapter   *adapter_ptr;         /* Pointer to this attr's adapt */
} attribute_t;


/*==============================================================*/
/* The share_list type is used to record shared with attributes */
/* temporarily within build_lists().                            */
/*==============================================================*/

typedef struct sharel {
	int           processed;                 /* Flag for processing Bool   */
	char          shar_name[ATTRNAMESIZE+1]; /* Share with/share with name */
	char          attr_name[ATTRNAMESIZE+1]; /* Attribute name             */ 
	struct sharel *next;                     /* Temp shared with attr list */ 
	adapter_t     *adapter;                  /* Adapter struct this node   */
} share_list_t;

/*============================================================*/
/* The inttbl_t type is used to construct a table of assigned */
/* interrupt levels for for the interrupt sharing algorithm.  */
/*============================================================*/

typedef struct
{
	unsigned long intnumber;
	unsigned long priority;
	unsigned short usecount;
	unsigned char trigger;
} inttbl_t;

#endif /* _H_ADAPTER */

