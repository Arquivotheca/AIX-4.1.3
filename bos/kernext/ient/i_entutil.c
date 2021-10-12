static char sccsid[] = "@(#)47  1.11  src/bos/kernext/ient/i_entutil.c, sysxient, bos411, 9439C411e 9/30/94 21:48:26";
/****************************************************************************/
/*
 *   COMPONENT_NAME: SYSXIENT
 *
 *   FUNCTIONS: port
 *              retry_put
 *              ient_cdt_func
 *              ient_trace
 *              retry_get
 *              reverse_long
 *              reverse_short
 *              ient_dmp_add
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/****************************************************************************/

#include <stddef.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/device.h>
#include <sys/intr.h>
#include <sys/timer.h>
#include <sys/watchdog.h>
#include <sys/dma.h>
#include <sys/malloc.h>
#include <sys/intr.h>
#include <sys/adspace.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/sleep.h>
#include <sys/trchkid.h>
#include <sys/err_rec.h>
#include <sys/mbuf.h>
#include <sys/dump.h>
#include <sys/ndd.h>

#include <sys/cdli.h>
#include <sys/generic_mibs.h>
#include <sys/ethernet_mibs.h>
#include <sys/cdli_entuser.h>

#include "i_entdds.h"
#include "i_entmac.h"
#include "i_enthw.h"
#include "i_entsupp.h"
#include "i_ent.h"
#include "i_enterrids.h"

extern ient_dev_ctl_t *p_dev_ctl;
extern ient_trace_t ient_trace_tbl;

void retry_put(ulong, ulong, ulong);
void retry_get(ulong, ulong, void *);


/****************************************************************************/
/*
 *
 * NAME: port
 *
 * FUNCTION:  This routine issues a port command to the ethernet device
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *    This routine runs under both the process thread and interrupt thread.
 *
 * NOTES:
 *
 *    Output: none
 *
 *    Notes:
 *      must delay for 10 system clocks and five transmit clocks
 *      after a reset or any port access has been issued.
 *      after that, issue a CA signal to start initialization sequence.
 *
 *
 * CALLED FROM: ient_start
 *              ient_restart
 *
 * CALLS TO:    io_att
 *              io_det
 *
 * INPUTS:      ioa       -  pointer to device's BUS I/O address space
 *              ivalue    -  port value 
 *
 * RETURNS: 0 (success) always. 
 *
 */
/****************************************************************************/

int
port(ulong ioa, ulong ivalue)
{
    uchar   value[4];
    ulong   lvalue;

    TRACE_SYS(HKWD_IENT_OTHER, "AptB", (ulong)p_dev_ctl, ivalue, 0);

    *(int *)&value = ivalue;

    if (WRK.machine == MACH_MEM_BASED)
    {
        ioa = (void *) io_att((DDS.bus_id | BUSMEM_IO_SELECT), 0);
        lvalue = (value[3] << 24) | (value[2] << 16);
        BUSPUTL(DDS.io_port, lvalue);
        DELAYMS(10);
        lvalue = (value[1] << 24) | (value[0] << 16);
        BUSPUTL(DDS.io_port, lvalue);
        io_det(ioa);
    }
    else
    {
        BUSPUTC(PORT_DATA_LOW, value[3]);
        BUSPUTC(PORT_DATA_HIGH, value[2]);
        BUSPUTC(PORT_CONTROL, 0x01);

        DELAYMS(10);

        BUSPUTC(PORT_DATA_LOW, value[1]);
        BUSPUTC(PORT_DATA_HIGH, value[0]);
        BUSPUTC(PORT_CONTROL, 0x01);
    }

    DELAYMS(10);

    TRACE_SYS(HKWD_IENT_OTHER, "AptE", 0, 0, 0);
    return(0);
}

/*****************************************************************************/
/*
 * NAME:     ient_cdt_func
 *
 * FUNCTION: return the address of the component dump table
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * CALLED FROM:
 *
 * CALLS TO:
 *
 * INPUT:
 *      none.
 *
 * RETURNS:  the address of the component dump table
 */
/*****************************************************************************/
struct cdt *ient_cdt_func()
{

   return((struct cdt *) &CDT.head);

}

/*****************************************************************************/
/*
 * NAME:     ient_dmp_add
 *
 * FUNCTION: builds the component dump table for Integrated Ethernet
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * CALLED FROM: ient_open
 *
 * CALLS TO:    dmp_add
 *
 * INPUT: component name
 *        address of area to dump
 *        and it's length
 *
 * RETURNS:  None
 */
/*****************************************************************************/

void
ient_dmp_add(register char *name, register char *ptr, register int len)
{
    struct cdt_entry temp_entry;
    int    num_elems;

    TRACE_SYS(HKWD_IENT_OTHER, "IdaB", (ulong) name, (ulong) ptr, (ulong) len);

    strncpy(temp_entry.d_name, name, sizeof(temp_entry.d_name));
    temp_entry.d_len = len;
    temp_entry.d_ptr = ptr;
    temp_entry.d_xmemdp = NULL;

    num_elems =
        (CDT.head._cdt_len - sizeof(CDT.head)) / sizeof(struct cdt_entry);

    if (num_elems < MAX_CDT_ENTRIES)
    {
        CDT.entry[num_elems] = temp_entry;
        CDT.head._cdt_len += sizeof(struct cdt_entry);
    }

    TRACE_SYS(HKWD_IENT_OTHER, "IdaE", num_elems, 0, 0);
    return;
}

/*****************************************************************************/
/*
 * NAME:     ient_dmp_del
 *
 * FUNCTION: delete an entry from the component dump table
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  nothing
 *
 */
/*****************************************************************************/

void
ient_dmp_del(register char *name, register char *ptr, register int len)
{
    struct cdt_entry temp_entry;
    int num_elems;
    int index = 0;

    TRACE_DBG(HKWD_IENT_OTHER, "IddB", (ulong)name, (ulong)ptr,(ulong)len);

    strncpy(temp_entry.d_name, name, sizeof(temp_entry.d_name));
    temp_entry.d_len = len;
    temp_entry.d_ptr = ptr;
    temp_entry.d_xmemdp = NULL;

    num_elems =
        (CDT.head._cdt_len - sizeof(CDT.head)) / sizeof(struct cdt_entry);


    /* find the element in the array (match only the memory pointer) */

   
    while (index < num_elems)
    {
        if (temp_entry.d_ptr == CDT.entry[index].d_ptr) break;
        index++;
    }

    /* re-pack the array to remove the element if it is there */
    if (index < num_elems)
    {
        index++;
        while (index < num_elems)
        {
            CDT.entry[index-1] = CDT.entry[index];
            index++;
        }

        bzero(&CDT.entry[index-1], sizeof(struct cdt_entry));
        CDT.head._cdt_len -= sizeof(struct cdt_entry);
    }

    TRACE_DBG(HKWD_IENT_OTHER, "IddE", num_elems, index, 0);
    return;
}



/****************************************************************************/
/*
 *
 * NAME: retry_put
 *
 * FUNCTION:  This routine tries repeated puts
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *    This routine runs under both the process thread and interrupt thread.
 *
 * NOTES:
 *
 *    Output: none
 *
 * CALLED FROM:
 *              ient_slih
 *              ack_interrupt
 *
 * CALLS TO:
 *
 * INPUT:
 *              ioa
 *              size
 *              value
 *
 * RETURN:  N/A
 *
 */
/****************************************************************************/

void
retry_put(ulong ioa, ulong size, ulong value)
{
    int     i;
    int     rc;

    TRACE_SYS(HKWD_IENT_OTHER, "PrpB", value, 0, 0 );

    for (rc = 1, i = 0; rc != 0 && i < 10; i++)
    {
        switch (size)
        {
        case 1:
            rc = BUS_PUTCX((char *)ioa, (char)value);
            break;
        case 2:
            rc = BUS_PUTSX((short *)ioa, (short)value);
            break;
        case 4:
            rc = BUS_PUTLX((long *)ioa, (long)value);
            break;
        }
    }

    if (!WRK.dump_started)
    {
        if (rc)
        {
            ient_logerr(ERRID_IENT_PIOFAIL, __LINE__, __FILE__, ioa,
                        rc, 0);
#ifdef DEBUG
            panic("retry_put");
#endif
        }
    }

    TRACE_SYS(HKWD_IENT_OTHER, "PrpE", 0, 0, 0);
    return;
}

/****************************************************************************/
/*
 *
 * NAME: retry_get
 *
 * FUNCTION:  This routine tries repeated puts
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *    This routine runs under both the process thread and interrupt thread.
 *
 * NOTES:
 *
 *    Output: none
 *
 * CALLED FROM:
 *
 * CALLS TO:
 *
 * INPUT:
 *              ioa
 *              size
 *              value
 *
 * RETURN:  N/A
 *
 */

/****************************************************************************/

void
retry_get(ulong ioa, ulong size, void *value)
{
    int     i;
    int     rc;

    TRACE_SYS (HKWD_IENT_OTHER, "PrgB", value, 0, 0 );

    for ( rc = 1, i = 0; rc != 0 && i < 10; i++ )
    {
        switch (size)
        {
        case 1:
            rc = BUS_GETCX((char *)ioa, (char *)value);
            break;
        case 2:
            rc = BUS_GETSX((short *)ioa, (short *)value);
            break;
        case 4:
            rc = BUS_GETLX((long *)ioa, (long *)value);
            break;
        }
    }

    if (rc)
    {
        ient_logerr(ERRID_IENT_PIOFAIL, __LINE__, __FILE__, ioa,
                    rc, 0);
        #ifdef DEBUG
        panic("retry_get");
        #endif
    }

    TRACE_SYS (HKWD_IENT_OTHER, "PrgE", 0, 0, 0 );
    return;
}

/*****************************************************************************/
/*
 *
 * NAME: reverse_long
 *
 * FUNCTION: byte reverse a 32-bit value
 *
 * EXECUTION ENVIRONMENT:
 *      This routine runs under the process thread or interrupt level.
 *
 * NOTES:
 *
 *    Input: 32-bit word
 *
 *    Output: byte reversed word
 *
 */
/*****************************************************************************/

int
reverse_long(be_addr)
ulong be_addr;            /* addr in big endian format */
{
    int le_addr;

    TRACE_SYS(HKWD_IENT_OTHER, "PrlB", be_addr, 0, 0);

    le_addr =  be_addr << 24;
    le_addr |= (be_addr & 0x0000FF00 ) << 8;
    le_addr |= (be_addr & 0x00FF0000 ) >> 8;
    le_addr |= be_addr >> 24;
    TRACE_SYS(HKWD_IENT_OTHER, "PrlE", le_addr, 0, 0);
    return(le_addr);

}

/****************************************************************************/
/*
 *
 * NAME: reverse_short
 *
 * FUNCTION: byte reverse a 16-bit value
 *
 * EXECUTION ENVIRONMENT:
 *      This routine runs under the process thread or interrupt level.
 *
 * NOTES:
 *
 *    Output: byte reversed word
 *
 * CALLED FROM:
 *              ient_RU_complete
 *
 * CALLS TO:
 *
 * INPUT: 32-bit word
 *
 */
/****************************************************************************/

short
reverse_short(be_addr)
ushort  be_addr;                        /* addr in big endian format */
{
    short le_addr;

    TRACE_SYS(HKWD_IENT_OTHER, "PrsB", 0, 0, 0);

    le_addr =  be_addr << 8;
    le_addr |= (be_addr & 0xFF00 ) >> 8;
    TRACE_SYS(HKWD_IENT_OTHER, "PrsE", 0, 0, 0);
    return(le_addr);
}

/*****************************************************************************/
/*
 * NAME:     ient_trace
 *
 * FUNCTION: Put a trace into the internal trace table and the external
 *           system trace.
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * NOTES:
 *      This routine is only called through macros when the DEBUG is defined.
 *
 * CALLED FROM:
 *
 * INPUT:
 *      hook            - trace hook
 *      tag             - four letter trace ID
 *      arg1 to arg 3   - arguments to trace
 *
 * RETURNS:  none.
 *
 */
/*****************************************************************************/

int
ient_trace(ulong hook, char *tag, ulong arg1, ulong arg2, ulong arg3)
{

    int i;

    /*
    ** Copy the trace point into the internal trace table
    */
    i = ient_trace_tbl.next_entry;
    ient_trace_tbl.table[i++] = *(ulong *)tag;
    ient_trace_tbl.table[i++] = arg1;
    ient_trace_tbl.table[i++] = arg2;
    ient_trace_tbl.table[i++] = arg3;

    if (i < TRACE_TABLE_SIZE)
    {
        ient_trace_tbl.table[i] = IENT_TRACE_END;
        ient_trace_tbl.next_entry = i;
    }
    else
    {
        ient_trace_tbl.table[0] = IENT_TRACE_END;
        ient_trace_tbl.next_entry = 0;
    }

    /*
    ** Call the external trace routine
    */
    TRCHKGT((hook << 20) | HKTY_GT | 4, *(ulong *)tag, arg1, arg2, arg3, 0);
    return;
}
