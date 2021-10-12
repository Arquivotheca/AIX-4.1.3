static char sccsid[]="@(#)17   1.11  src/bos/kernext/cie/ciedata.c, sysxcie, bos411, 9438B411a 9/19/94 18:10:12";

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 * DESCRIPTION:
 *
 *    COMIO Emulator Static Data
 *
 *    Information contained in this module is mapped by the
 *    data structures defined in devconst.h.
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include "ciedd.h"

#include <sys/device.h>
#include <sys/uio.h>
#include <sys/comio.h>
#include <sys/devinfo.h>
#include <sys/ciouser.h>
#include <sys/err_rec.h>
#include <sys/tokuser.h>
#include <sys/fddiuser.h>
#include <sys/ioctl.h>
#include <sys/entuser.h>
#include <sys/adspace.h>
#include <sys/cdli.h>
#include <sys/cdli_fddiuser.h>
#include <sys/cdli_tokuser.h>
#include <sys/cdli_entuser.h>
#include <sys/tok_demux.h>
#include <sys/eth_demux.h>

#define  fddi_hdr cfddi_dmxhdr
#include <sys/fddi_demux.h>
#undef   fddi_hdr

#include "dev.h"
#include "chn.h"
#include "ses.h"
#include "dmalloc.h"

/*---------------------------------------------------------------------------*/
/*                Type Definition for generic DD Entry Point                 */
/*---------------------------------------------------------------------------*/

typedef int                  ddEntryPoint();

/*---------------------------------------------------------------------------*/
/*                  Device Driver Entry Point Declarations                   */
/*---------------------------------------------------------------------------*/

ddEntryPoint                 cieconfig;
ddEntryPoint                 ciempx;
ddEntryPoint                 cieopen;
ddEntryPoint                 cieclose;
ddEntryPoint                 cieread;
ddEntryPoint                 ciewrite;
ddEntryPoint                 cieioctl;
ddEntryPoint                 cieselect;
ddEntryPoint                 nodev;

/*---------------------------------------------------------------------------*/
/*              Static Array: Token Ring IOCTL Handler Routines              */
/*---------------------------------------------------------------------------*/

static IODT_ENTRY            tokIOCTLs[] =
{
   /*-----------------*/
   /*  Common IOCTLs  */
   /*-----------------*/

   {IOCINFO                , describeDevice              },
   {CIO_START              , startSession                },
   {CIO_HALT               , haltSession                 },
   {CIO_GET_STAT           , getChannelStatus            },
   {CIO_QUERY              , queryStatistics             },
   {CIO_CONNECT            , NULL                        },
   {CIO_DISCONN            , NULL                        },
   {CIO_GET_FASTWRT        , NULL                        },
   {CCC_DOWNLOAD           , NULL                        },
   {CCC_GET_VPD            , NULL                        },
   {CCC_TRCTBL             , NULL                        },
   {CCC_MEM_ACC            , NULL                        },
   {CCC_POS_ACC            , NULL                        },
   {CCC_REG_ACC            , NULL                        },

   /*------------------------------*/
   /*  Token-Ring Specific IOCTLS  */
   /*------------------------------*/

   {TOK_GRP_ADDR           , tokGroupAddr                },
   {TOK_FUNC_ADDR          , tokFuncAddr                 },
   {TOK_QVPD               , tokQueryVPD                 },
   {TOK_ACCESS_POS         , NULL                        },
   {TOK_SET_ADAP_IPARMS    , NULL                        },
   {TOK_SET_OPEN_PARMS     , NULL                        },
   {TOK_RING_INFO          , tokQueryTokenRingInfo       },
   {TOK_DOWNLOAD_UCODE     , NULL                        }
};

/*---------------------------------------------------------------------------*/
/*                 Static Array: FDDI IOCTL Handler Routines                 */
/*---------------------------------------------------------------------------*/

static IODT_ENTRY            fddiIOCTLs[] =
{
   /*-----------------*/
   /*  Common IOCTLs  */
   /*-----------------*/

   {IOCINFO                , describeDevice              },
   {CIO_START              , startSession                },
   {CIO_HALT               , haltSession                 },
   {CIO_GET_STAT           , getChannelStatus            },
   {CIO_QUERY              , queryStatistics             },
   {CIO_CONNECT            , NULL                        },
   {CIO_DISCONN            , NULL                        },
   {CIO_GET_FASTWRT        , NULL                        },
   {CCC_DOWNLOAD           , NULL                        },
   {CCC_GET_VPD            , NULL                        },
   {CCC_TRCTBL             , NULL                        },
   {CCC_MEM_ACC            , NULL                        },
   {CCC_POS_ACC            , NULL                        },
   {CCC_REG_ACC            , NULL                        },

   /*------------------------*/
   /*  FDDI-Specific IOCTLs  */
   /*------------------------*/

   {FDDI_SET_LONG_ADDR     , fddiGroupAddr               },
   {FDDI_QUERY_ADDR        , fddiQueryGroupAddr          },
   {FDDI_DWNLD_MCODE       , NULL                        },
   {FDDI_MEM_ACC           , NULL                        },
   {FDDI_HCR_CMD           , NULL                        }
};

/*---------------------------------------------------------------------------*/
/*               Static Array: Ethernet IOCTL Handler Routines               */
/*---------------------------------------------------------------------------*/

static IODT_ENTRY            entIOCTLs[] =
{
   /*-----------------*/
   /*  Common IOCTLs  */
   /*-----------------*/

   {IOCINFO                , describeDevice              },
   {CIO_START              , startSession                },
   {CIO_HALT               , haltSession                 },
   {CIO_GET_STAT           , getChannelStatus            },
   {CIO_QUERY              , queryStatistics             },
   {CIO_CONNECT            , NULL                        },
   {CIO_DISCONN            , NULL                        },
   {CIO_GET_FASTWRT        , NULL                        },
   {CCC_DOWNLOAD           , NULL                        },
   {CCC_GET_VPD            , entQueryVPD                 },
   {CCC_TRCTBL             , NULL                        },
   {CCC_MEM_ACC            , NULL                        },
   {CCC_POS_ACC            , NULL                        },
   {CCC_REG_ACC            , NULL                        },

   /*----------------------------*/
   /*  Ethernet-Specific IOCTLs  */
   /*----------------------------*/

   {ENT_SET_MULTI          , entMulticastAddr            },
   {ENT_CFG                , NULL                        },
   {ENT_NOP                , NULL                        },
   {ENT_POS                , NULL                        },
   {ENT_SELFTEST           , NULL                        },
   {ENT_DUMP               , NULL                        },
   {ENT_ALOC_BUF           , NULL                        },
   {ENT_COPY_BUF           , NULL                        },
   {ENT_FREE_BUF           , NULL                        },
   {ENT_CONF_BUF           , NULL                        },
   {ENT_LOCK_DMA           , NULL                        },
   {ENT_UNLOCK_DMA         , NULL                        },
   {ENT_PROMISCUOUS_ON     , entPromiscuousOn            },
   {ENT_PROMISCUOUS_OFF    , entPromiscuousOff           },
   {ENT_BADFRAME_ON        , NULL                        },
   {ENT_BADFRAME_OFF       , NULL                        },
   {ENT_RCV_SIG            , entSignalSupport            }
};

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                       Device Type Definition Table                        */
/*                                                                           */
/*  This is the master device type definition table for the COMIO Emulator.  */
/*  It consists of one entry per device type.  Each entry contains a         */
/*  pointer to to an array of IOCTL dispatch entries, along with a count of  */
/*  the number of entries.                                                   */
/*                                                                           */
/*  The IOCTL dispatch tables are statically declared above.  The list of    */
/*  IOCTL codes is given in a human-readable order, but each array is        */
/*  sorted during Emulator initialization, so that the array can be accessed */
/*  with a binary search for efficiency.                                     */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static DEV_TYPE_DEF          devData[] =
{
   /*--------------*/
   /*  Token Ring  */
   /*--------------*/
   {
      arraysize(tokIOCTLs),
      tokIOCTLs
   },

   /*--------*/
   /*  FDDI  */
   /*--------*/
   {
      arraysize(fddiIOCTLs),
      fddiIOCTLs
   },

   /*------------*/
   /*  Ethernet  */
   /*------------*/
   {
      arraysize(entIOCTLs),
      entIOCTLs
   },

   /*----------------------*/
   /*  End-of-List Marker  */  // DO NOT CHANGE THIS ENTRY!
   /*----------------------*/
   {
      -1,
      NULL
   }
};

/*---------------------------------------------------------------------------*/
/*                            Device Switch Table                            */
/*---------------------------------------------------------------------------*/

static const devsw_t         devSwitchTable =
{
   cieopen     , /* devsw.d_open     */
   cieclose    , /* devsw.d_close    */
   cieread     , /* devsw.d_read     */
   ciewrite    , /* devsw.d_write    */
   cieioctl    , /* devsw.d_ioctl    */
   nodev       , /* devsw.d_strategy */
   NULL        , /* devsw.d_ttys     */
   cieselect   , /* devsw.d_select   */
   cieconfig   , /* devsw.d_config   */
   nodev       , /* devsw.d_print    */
   nodev       , /* devsw.d_dump     */
   ciempx      , /* devsw.d_mpx      */
   nodev       , /* devsw.d_revoke   */
   NULL        , /* devsw.d_dsdptr   */
   NULL        , /* devsw.d_selptr   */
   DEV_MPSAFE    /* devsw.d_opts     */
};

/*---------------------------------------------------------------------------*/
/*                            Compile Time Stamp                             */
/*---------------------------------------------------------------------------*/

#define TIMESTAMP "Internal Build 0118.940427.145115"

/*---------------------------------------------------------------------------*/
/*                   Static External Anchor Data Structure                   */
/*---------------------------------------------------------------------------*/

extern volatile DEVMGR       dmgr =
{
   "DMGR",                                // Eye Catcher
   TIMESTAMP,                             // Compile Timestamp
   NULL,                                  // Ptr to Trace Table
   NULL,                                  // Ptr to memory debug arena
   &devData,                              // Ptr to Device Type Constants
   &devSwitchTable,                       // Ptr to Device Switch Table
   CIE_NOT_INITIALIZED,                   // Initialization State
   NULL,                                  // Device Table
   NULL,                                  // Channel Index
   LOCK_INIT                              // Global Data Lock
};

extern DEVMGR              * comioEmulator = &dmgr;
