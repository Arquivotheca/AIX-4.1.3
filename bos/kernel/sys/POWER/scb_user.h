/* @(#)94	1.1  src/bos/kernel/sys/POWER/scb_user.h, sysxscsi, bos411, 9428A410j 9/29/93 08:50:12 */
/*
 * COMPONENT_NAME: (SYSSCSI) - SCB Subsystem User Include File
 *
 * FUNCTIONS: scb_user.h
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_SCB_USER
#define _H_SCB_USER

/*
 *  TRANSMIT BUFFER STRUCTURES
 *
 *
 *  This contains a pointer to the control element and also a pointer to all
 *  of the buffers that need to be allocated.
 */
struct ctl_elem_blk {
    struct ctl_elem_blk	*next;
    struct ctl_elem_blk	*prev;
    uchar		flags;          /* Flags for this ctl_elem_blk      */
    int			key;            /* used by protocol head            */
    int			status;         /* control element status           */
    int			num_pd_info;    /* number of pd_info structs        */
    int			pds_data_len;   /* Length of data in pd_infos       */
    caddr_t		reply_elem;     /* reply element                    */
    int			reply_elem_len; /* Length of reply element buffer   */
    caddr_t		ctl_elem;       /* pointer to ctl elem              */
    struct pd_info	*pd_info;       /* PD-1 information                 */
};
typedef struct ctl_elem_blk	ctl_elem_blk_t; 

/*
 *  Flags for the flags field in ctl_elem_blk structure.
 */
#define			CANCEL		0x01 /* This is a cancel ctl_elem    */
#define                 EXPEDITE	0x02 /* Expedited transmit request   */
#define			NO_REPLY	0x04 /* Can suppress reply on EMBED  */

/*
 *  SCB Error definitions for the status field in ctl_elem_blk structure.
 */

/*
 * indicates a Host I/O Bus error condition
 */
#define	SCB_HOST_IO_BUS_ERR	0x01
/*
 * the adapter is indicating a hardware failure
 */
#define	SCB_ADAPTER_HDW_FAILURE	0x10
/*
 * the adapter is indicating a microcode failure
 */
#define	SCB_ADAPTER_SFW_FAILURE	0x20
/*
 * There was not enough space to copy the entire reply element.  Copied
 * as much as possible.
 */
#define	SCB_PARTIAL_REPLY_ELEM	0x90
/*
 * The control element was cancelled by the adapter driver because it
 * received a CANCEL control element for that ctl_elem_blk.
 */
#define	SCB_CTL_ELEM_CANCELLED	0xa0
/*
 *  The reply element is suppressed but the packet was written to the adapter
 *  successfully.
 */
#define	SCB_REPLY_SUPPRESSED	0xb0

/*
 *  Pre-defined SCB entity IDs
 */
#define SCB_8022                (0x04)
#define SCB_IPI3_SLAVE          (0x06)
#define SCB_IPI3_MASTER         (0x07)
#define SCB_SCSI_INITIATOR      (0x08)
#define SCB_SCSI_TARGET         (0x09)


/*
 *  There will be one of these structures for each data PD-1 in the control 
 *  element.
 */
struct pd_info {
    struct pd_info	*next;
    uint		buf_type;	/* Type of buffer in p_buf_list   */
    uint		pd_ctl_info;    /* First word of PD-1             */
    uint		mapped_addr;    /* bus address                    */
    int			total_len;      /* total length of PD             */
    int			num_tcws;	/* number of TCWs		  */
    caddr_t		p_buf_list;	/* List of buffers for this PD    */
};
typedef struct pd_info	pd_info_t;

/*
 *  Values for the buf_type field in pd_info structure.
 */
#define		P_BUF	0x1     /* p_buf_list points to bufstruct chain   */
#define		P_MBUF	0x2     /* p_buf_list points to an mbuf chain     */

/*
 *  RECEIVE STRUCTURES
 * 
 *
 *  This structure contains information about a specific buffer pool
 *  element. 
 */

struct buf_pool_elem {
    struct buf_pool_elem	*next;
    struct buf_pool_elem	*prev;
    int				buffer_size;   /* size of this buffer       */
    caddr_t			virtual_addr;  /* virtual address of buffer */
    caddr_t			mapped_addr;   /* mapped address of buffer  */
    void			(*buf_pool_free)();/* buf_pool_free func    */
    caddr_t			adap_dd_info;  /* parm to buf_pool_free */
};
typedef struct buf_pool_elem	buf_pool_elem_t;

/*
 *
 *  SCB CONTROL STRUCTURES
 *
 *
 *  This structure is used by the adpater driver to recognize a new entity and
 *  to set up a buffer pool for that entity based on that information.
 *  The protocol head will fill in the entity number, num_buffers, buffer_size,
 *  and the buf_pool_free and pool_free_param fields will be filled in by
 *  the adapter driver.
 */
struct scb_entity_info {
    int			entity_number;     /* Entity number for protocol  */
    int			buf_threshold;     /* use small or big bufs       */
    int			num_buffers1;      /* Number of buffers in pool 1 */
    int			buffer_size1;      /* Size of buffers in pool 1   */
    int			num_buffers2;      /* Number of buffers in pool 2 */
    int			buffer_size2;      /* Size of buffers in pool 2   */
};
typedef struct scb_entity_info	scb_entity_info_t;

/*
 *  Structure passed to demuxer on an ns_add_filter()
 */
struct scb_filter {
    int				filtertype;      /* filter type */
    struct scb_entity_info	entity_info;     /* SCB filter info */
};
typedef struct scb_filter	scb_filter_t;


/*
 *   SCB Control element and configuration record definitions.
 */

/*
 *  Configuration record
 */
struct config_record {
    ushort		reserved1;
    ushort		length;
    ushort		config_status;
    ushort		unit_ids;	/* SYS unit id | adap unit id */
    uint		sys_sig_addr;
    uint		adap_sig_addr;
    ushort		reserved2;
    ushort		base_io_addr;
    char		sys_mgmt_id;
    char		time_unit;
    ushort		timer_freq;
    ushort		sys_config_opts;
    ushort		adap_config_opts;
    ushort		in_pipe_size;
    ushort		out_pipe_size;
    uint		p_in_pipe;
    uint		p_in_deque_status;
    uint		p_in_soe;
    uint		p_in_enque_status;
    uint		p_in_sof;
    uint		p_out_pipe;
    uint		p_out_deque_status;
    uint		p_out_soe;
    uint		p_out_enque_status;
    uint		p_out_sof;
};


/*
 * control element header
 */
struct ctl_elem_hdr {
    ushort		format;
    ushort		length;
    ushort		options;
    ushort		reserved;
    char		src_unit;
    char		src_entity;
    char		dest_unit;
    char		dest_entity;
    uint		correlation_id;
};
typedef struct ctl_elem_hdr ctl_elem_hdr_t;

/*
 *  Definition for Generic Parameter Descriptor
 */
struct pd {
    ushort		desc_number;
    ushort		ctl_info;
    uint		word1;
    uint		word2;
    uint		word3;
};
typedef struct pd	pd_t;

/*
 *  Control Element Defines
 */
#define 		CTL_ELEM_FORMAT		0

#endif /* _H_SCB_USER */

