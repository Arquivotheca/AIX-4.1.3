static char sccsid[]="@(#)33   1.10  src/bos/kernext/cie/status.c, sysxcie, bos411, 9428A410j 5/18/94 15:55:32";

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 *   mapFDDIHardFail
 *   mapFDDILimboEnter
 *   mapFDDINDDStatus
 *   mapTokHardFail
 *   mapTokLimboEnter
 *   mapTokNDDStatus
 *   mapAsyncStatus
 *   new_STATBLK
 *   free_STATBLK
 *   mkStartDoneStatus
 *   mkHaltDoneStatus
 *   mkNullBlkStatus
 *   mkLostStatusStatus
 *   mkTXDoneStatus
 *   mkCDLIStatus
 *
 * DESCRIPTION:
 *
 *    Status Block Implementation
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

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/mbuf.h>

#include <sys/comio.h>
#include <sys/ciouser.h>
#include <sys/err_rec.h>
#include <sys/tokuser.h>
#include <sys/entuser.h>
#include <sys/fddiuser.h>

#include <sys/cdli.h>
#include <sys/ndd.h>
#include <sys/cdli_tokuser.h>
#include <sys/cdli_entuser.h>
#include <sys/cdli_fddiuser.h>

#include "ciedd.h"
#include "dev.h"
#include "status.h"
#include "dmalloc.h"

/*---------------------------------------------------------------------------*/
/*                       Status Code Mapping Structure                       */
/*---------------------------------------------------------------------------*/

#define  NUM_STAT  6         /* size of map structure per device type */

typedef struct CODEMAP       CODEMAP;
typedef struct SUBCODEMAP    SUBCODEMAP;

struct CODEMAP
{
   CIE_DEV_TYPE              devType;
   struct
   {
      int                    cdli;        // CDLI Code
      int                    cio;         // COMIO Code
      int                 (* mapper)(
                                cio_stat_blk_t      * cioStat,
                                const ndd_statblk_t * cdliStat
                             );           // Additional Mapping Function
   } map[NUM_STAT];

};

/*---------------------------------------------------------------------------*/
/*                  Mapping Structure for Status Sub Codes                   */
/*---------------------------------------------------------------------------*/

struct SUBCODEMAP
{
   int                       cdli;        // CDLI SubCode
   int                       cio;         // COMIO SubCode
};

/*---------------------------------------------------------------------------*/
/*              Custom Mapping for FDDI Hard Fail Status Block               */
/*---------------------------------------------------------------------------*/

static
int
   mapFDDIHardFail(
      cio_stat_blk_t       * cioStat     ,//  O-COMIO Status Block
      const ndd_statblk_t  * cdliStat     // I -CDLI Status Block
   )
{
   FUNC_NAME(mapFDDIHardFail);

   static SUBCODEMAP         map[] =
   {
      {NDD_PIO_FAIL             ,FDDI_PIO_FAIL             },
      {CFDDI_SELF_TEST          ,FDDI_SELF_TEST            },
      {CFDDI_REMOTE_DISCONNECT  ,FDDI_REMOTE_DISCONNECT    },
      {0                        ,0                         }
   };

   int                       i;
   int                       rc = 0;

   /*--------------------------------*/
   /*  Search subcode mapping table  */
   /*--------------------------------*/

   for (i=0; map[i].cdli && map[i].cdli!=cdliStat->option[0]; i++);

   /*--------------------------------------------------*/
   /*  If found, perform the mapping; otherwise error  */
   /*--------------------------------------------------*/

   if (map[i].cdli)
   {
      cioStat->option[1] = map[i].cio;
      cioStat->option[2] = cdliStat->option[1];
      cioStat->option[3] = cdliStat->option[2];
   }
   else
   {
      cioStat->option[1] = cdliStat->option[0];
      cioStat->option[2] = cdliStat->option[1];
      TRC_OTHER(stma,cdliStat->option[0],cdliStat->option[1],0);

      rc = ERANGE;
   }

   return rc;
}

/*---------------------------------------------------------------------------*/
/*             Custom Mapping for FDDI Limbo Enter Status Block              */
/*---------------------------------------------------------------------------*/

static
int
   mapFDDILimboEnter(
      cio_stat_blk_t       * cioStat     ,//  O-COMIO Status Block
      const ndd_statblk_t  * cdliStat     // I -CDLI Status Block
   )
{
   FUNC_NAME(mapFDDILimboEnter);

   static SUBCODEMAP         map[] =
   {
      {NDD_ADAP_CHECK           ,FDDI_ADAP_CHECK           },
      {NDD_CMD_FAIL             ,FDDI_CMD_FAIL             },
      {NDD_PIO_FAIL             ,FDDI_PIO_FAIL             },
      {NDD_TX_ERROR             ,FDDI_TX_ERROR             },
      {CFDDI_TX_ERROR           ,FDDI_TX_ERROR             },
      {CFDDI_REMOTE_SELF_TEST   ,FDDI_REMOTE_SELF_TEST     },
      {CFDDI_PATH_TEST          ,FDDI_PATH_TEST            },
      {0                        ,0                         }
   };

   int                       i;
   int                       rc = 0;

   /*--------------------------------*/
   /*  Search subcode mapping table  */
   /*--------------------------------*/

   for (i=0; map[i].cdli && map[i].cdli!=cdliStat->option[0]; i++);

   /*--------------------------------------------------*/
   /*  If found, perform the mapping; otherwise error  */
   /*--------------------------------------------------*/

   if (map[i].cdli)
   {
      cioStat->option[1] = map[i].cio;
      cioStat->option[2] = cdliStat->option[1];
   }
   else
   {
      cioStat->option[1] = cdliStat->option[0];
      cioStat->option[2] = cdliStat->option[1];
      TRC_OTHER(stmb,cdliStat->option[0],cdliStat->option[1],0);

      rc = ERANGE;
   }

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                 Custom Mapping for FDDI Ring Status Block                 */
/*---------------------------------------------------------------------------*/

static
int
   mapFDDINDDStatus(
      cio_stat_blk_t       * cioStat     ,//  O-COMIO Status Block
      const ndd_statblk_t  * cdliStat     // I -CDLI Status Block
   )
{
   FUNC_NAME(mapFDDIRingStatus);

   static SUBCODEMAP         mapSmtEvent[] =
   {
      {CFDDI_LLC_DISABLE        ,FDDI_LLC_DISABLE          },
      {CFDDI_LLC_ENABLE         ,FDDI_LLC_ENABLE           },
      {CFDDI_NO_ROP             ,FDDI_NO_ROP               },
      {CFDDI_ROP                ,FDDI_ROP                  },
      {0                        ,0                         }
   };

   static SUBCODEMAP         mapSmtError[] =
   {
      {CFDDI_RTT                ,FDDI_RTT                  },
      {CFDDI_REMOTE_T_REQ       ,FDDI_REMOTE_T_REQ         },
      {CFDDI_PORT_STUCK         ,FDDI_PORT_STUCK           },
      {CFDDI_BYPASS_STUCK       ,FDDI_BYPASS_STUCK         },
      {0                        ,0                         }
   };

   int                       i;
   int                       rc = 0;
   SUBCODEMAP              * m = NULL;

   /*--------------------------------------------------------*/
   /*  Decide which mapping table to use based on option[0]  */
   /*--------------------------------------------------------*/

   switch(cdliStat->option[0])
   {
      case CFDDI_SMT_EVENT:
         cioStat->option[1] = FDDI_SMT_EVENT;
         m = mapSmtEvent;
         break;
      case CFDDI_SMT_ERROR:
         cioStat->option[1] = FDDI_SMT_ERROR;
         m = mapSmtError;
         break;
      default:
         cioStat->option[1] = cdliStat->option[0];
         cioStat->option[2] = cdliStat->option[1];
         TRC_OTHER(stmc,cdliStat->option[0],cdliStat->option[1],0);

         rc = ERANGE;
         break;
   }

   if (m)
   {
      /*---------------------------------------------*/
      /*  Search the table for a matching option[1]  */
      /*---------------------------------------------*/

      for (i=0; m[i].cdli && m[i].cdli!=cdliStat->option[1]; i++);

      /*-------------------------------------------------*/
      /*  Perform the mapping if found; otherwise error  */
      /*-------------------------------------------------*/

      if (m[i].cdli)
      {
         cioStat->option[2] = m[i].cio;
         cioStat->option[3] = cdliStat->option[2];
      }
      else
      {
         cioStat->option[2] = cdliStat->option[1];
         cioStat->option[3] = cdliStat->option[2];
         TRC_OTHER(stmd,cdliStat->option[0],cdliStat->option[1],0);

         rc = ERANGE;
      }
   }
   else
   {
      cioStat->option[2] = cdliStat->option[1];
      cioStat->option[3] = cdliStat->option[2];
   }

   return rc;
}

/*---------------------------------------------------------------------------*/
/*           Custom Mapping for Token Ring Hard Fail Status Block            */
/*---------------------------------------------------------------------------*/

static
int
   mapTokHardFail(
      cio_stat_blk_t       * cioStat     ,//  O-COMIO Status Block
      const ndd_statblk_t  * cdliStat     // I -CDLI Status Block
   )
{


   FUNC_NAME(mapTokHardFail);


   static SUBCODEMAP         map[] =
   {
      {NDD_PIO_FAIL             ,TOK_PIO_FAIL             },
      {TOK_RECOVERY_THRESH      ,TOK_RCVRY_THRESH            },
      {0                        ,0                         }
   };

   int                       i;
   int                       rc = 0;

   /*--------------------------------*/
   /*  Search subcode mapping table  */
   /*--------------------------------*/

   for (i=0; map[i].cdli && map[i].cdli!=cdliStat->option[0]; i++);

   /*--------------------------------------------------*/
   /*  If found, perform the mapping; otherwise error  */
   /*--------------------------------------------------*/

   if (map[i].cdli)
   {
      cioStat->option[1] = map[i].cio;
      cioStat->option[2] = cdliStat->option[1];
   }
   else
   {
      cioStat->option[1] = cdliStat->option[0];
      cioStat->option[2] = cdliStat->option[1];
      TRC_OTHER(stme,cdliStat->option[0],cdliStat->option[1],0);

      rc = ERANGE;
   }

   return rc;
}

/*---------------------------------------------------------------------------*/
/*          Custom Mapping for Token Ring Limbo Enter Status Block           */
/*---------------------------------------------------------------------------*/

static
int
   mapTokLimboEnter(
      cio_stat_blk_t       * cioStat     ,//  O-COMIO Status Block
      const ndd_statblk_t  * cdliStat     // I -CDLI Status Block
   )
{
   FUNC_NAME(mapTokLimboEnter);

   static SUBCODEMAP         map[] =
   {
      {NDD_ADAP_CHECK           ,TOK_ADAP_CHECK           },
      {NDD_AUTO_RMV             ,TOK_AUTO_REMOVE           },
      {NDD_BUS_ERROR            ,TOK_MC_ERROR           },
      {NDD_CMD_FAIL             ,TOK_CMD_FAIL           },
      {NDD_TX_ERROR             ,TOK_TX_ERROR           },
      {NDD_TX_TIMEOUT           ,/*TOK_ADAP_TIMEOUT*/TOK_ADAP_CHECK           },
      {TOK_DMA_FAIL             ,TOK_ADAP_CHECK           },
      {TOK_RING_SPEED           ,TOK_RING_STATUS           },
      {TOK_RMV_ADAP             ,TOK_REMOVED_RECEIVED           },
      {TOK_WIRE_FAULT           ,TOK_LOBE_WIRE_FAULT           },
      {0                        ,0                         }
   };

   int                       i;
   int                       rc = 0;

   /*--------------------------------*/
   /*  Search subcode mapping table  */
   /*--------------------------------*/

   for (i=0; map[i].cdli && map[i].cdli!=cdliStat->option[0]; i++);

   /*--------------------------------------------------*/
   /*  If found, perform the mapping; otherwise error  */
   /*--------------------------------------------------*/

   if (map[i].cdli)
   {
      cioStat->option[1] = map[i].cio;
      cioStat->option[2] = cdliStat->option[1];
   }
   else
   {
      cioStat->option[1] = cdliStat->option[0];
      cioStat->option[2] = cdliStat->option[1];
      TRC_OTHER(stmf,cdliStat->option[0],cdliStat->option[1],0);

      rc = ERANGE;
   }

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                Custom Mapping for Token Ring Status Block                 */
/*---------------------------------------------------------------------------*/

static
int
   mapTokNDDStatus(
      cio_stat_blk_t       * cioStat     ,//  O-COMIO Status Block
      const ndd_statblk_t  * cdliStat     // I -CDLI Status Block
   )
{
   FUNC_NAME(mapNDDStatus);

   static SUBCODEMAP         map[] =
   {
      {NDD_STATUS             ,TOK_RING_BEACONING             },
      {0                        ,0                         }
   };

   int                       i;
   int                       rc = 0;

   /*--------------------------------*/
   /*  Search subcode mapping table  */
   /*--------------------------------*/

   for (i=0; map[i].cdli && map[i].cdli!=cdliStat->option[0]; i++);

   /*--------------------------------------------------*/
   /*  If found, perform the mapping; otherwise error  */
   /*--------------------------------------------------*/

   if (map[i].cdli)
   {
      cioStat->option[1] = map[i].cio;
      cioStat->option[2] = cdliStat->option[1];
   }
   else
   {
      cioStat->option[1] = cdliStat->option[0];
      cioStat->option[2] = cdliStat->option[1];
      TRC_OTHER(stmg,cdliStat->option[0],cdliStat->option[1],0);

      rc = ERANGE;
   }

   return rc;
}

/*---------------------------------------------------------------------------*/
/*               Map A Status Block from CDLI to COMIO Format                */
/*---------------------------------------------------------------------------*/

int
   mapAsyncStatus(
      CIE_DEV_TYPE           devType     ,// I -Device Type Code
      cio_stat_blk_t       * cioStat     ,//  O-COMIO Status Block
      const ndd_statblk_t  * cdliStat     // I -CDLI Status Block
   )
{
   FUNC_NAME(mapAsyncStatus);

   /*-------------------*/
   /*  Status Code Map  */
   /*-------------------*/

   static CODEMAP               codeMap[] =
   {
      /*------------*/
      /*  Ethernet  */
      /*------------*/

      {
         CIE_DEV_ENT,
         {
            {NDD_HARD_FAIL        ,CIO_HARD_FAIL        ,NULL                },
            {NDD_LIMBO_ENTER      ,0                    ,NULL                },
            {NDD_LIMBO_EXIT       ,0                    ,NULL                },
            {NDD_STATUS           ,0                    ,NULL                },
            {NDD_BAD_PKTS         ,0                    ,NULL                },
            {NDD_CONNECTED        ,0                    ,NULL                }
         }
      },

      /*--------*/
      /*  FDDI  */
      /*--------*/

      {
         CIE_DEV_FDDI,
         {
            {NDD_HARD_FAIL        ,CIO_HARD_FAIL        ,mapFDDIHardFail     },
            {NDD_LIMBO_ENTER      ,CIO_NET_RCVRY_ENTER  ,mapFDDILimboEnter   },
            {NDD_LIMBO_EXIT       ,CIO_NET_RCVRY_EXIT   ,NULL                },
            {NDD_STATUS           ,FDDI_RING_STATUS     ,mapFDDINDDStatus    },
            {NDD_BAD_PKTS         ,0                    ,NULL                },
            {NDD_CONNECTED        ,0                    ,NULL                }
         }
      },

      /*--------------*/
      /*  Token Ring  */
      /*--------------*/

      {
         CIE_DEV_TOK,
         {
            {NDD_HARD_FAIL        ,CIO_HARD_FAIL        ,mapTokHardFail      },
            {NDD_LIMBO_ENTER      ,CIO_NET_RCVRY_ENTER  ,mapTokLimboEnter    },
            {NDD_LIMBO_EXIT       ,CIO_NET_RCVRY_EXIT   ,NULL                },
            {NDD_STATUS           ,TOK_RING_STATUS      ,mapTokNDDStatus     },
            {NDD_BAD_PKTS         ,0                    ,NULL                },
            {NDD_CONNECTED        ,0                    ,NULL                }
         }
      },

      /*---------------*/
      /*  End-of-List  */
      /*---------------*/

      {
         CIE_DEV_NULL
      }
   };

   int                       dx;          // Device Index in map table
   int                       sx;          // Stat code index in map table

   int                       rc = 0;

   /*------------------------------------------------------------*/
   /*  Lookup device type in mapping table; EINVAL if not found  */
   /*------------------------------------------------------------*/

   for (dx=0; codeMap[dx].devType != CIE_DEV_NULL; dx++)
      if (codeMap[dx].devType == devType) break;

   if (codeMap[dx].devType == CIE_DEV_NULL)
   {
      TRC_OTHER(stmh,cdliStat->option[0],cdliStat->option[1],0);
      return EINVAL;
   }

   /*----------------------------------------------------*/
   /*  Lookup Status Code in table; ERANGE if not found  */
   /*----------------------------------------------------*/

   for (sx=0; sx<NUM_STAT; sx++)
      if (codeMap[dx].map[sx].cdli == cdliStat->code) break;

   if (sx == NUM_STAT)
   {
      TRC_OTHER(stmi,cdliStat->option[0],cdliStat->option[1],0);
      return ERANGE;
   }

   /*---------------------------------------------*/
   /*  Initialize the COMIO status block to zero  */
   /*---------------------------------------------*/

   memset(cioStat,0x00,sizeof(*cioStat));

   /*-----------------------------------*/
   /*  Convert the primary Status Code  */
   /*-----------------------------------*/

   cioStat->code      = CIO_ASYNC_STATUS;
   cioStat->option[0] = codeMap[dx].map[sx].cio;

   /*------------------------------------------------------*/
   /*  Call the custom mapping function if one is defined  */
   /*------------------------------------------------------*/

   if (codeMap[dx].map[sx].mapper != NULL)
   {
      rc = (codeMap[dx].map[sx].mapper)(cioStat,cdliStat);
   }

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                Allocate and clear a status block structure                */
/*---------------------------------------------------------------------------*/

STATBLK *
   new_STATBLK(
      STATBLK_FORMAT         format      ,// I -Status Block Format Code
      int                    waitFlag     // I -Wait flag (borrowed from mbuf)
   )
{
   FUNC_NAME(new_STATBLK);

   register mbuf_t         * m;           // mbuf pointer
   register STATBLK        * s;           // ptr to status block in mbuf

   if ((m = m_getclr(waitFlag,MT_DATA)) == NULL)
   {
      TRC_OTHER(stmj,0,0,0);
      return NULL;
   }

   (s = MTOD(m,STATBLK *))->format = format;

   return s;
}

/*---------------------------------------------------------------------------*/
/*                       Free a Status Block Structure                       */
/*---------------------------------------------------------------------------*/

void
   free_STATBLK(
      STATBLK              * s            // IO-Ptr to statblk to be freed
   )
{
   FUNC_NAME(free_STATBLK);

   m_free(DTOM(s));
}

/*---------------------------------------------------------------------------*/
/*                   Create a CIO_START_DONE status block                    */
/*---------------------------------------------------------------------------*/

STATBLK *
   mkStartDoneStatus(
      register DEV         * dev         ,// I -Device
      register netid_t       netid        // I -Net ID
   )
{
   FUNC_NAME(mkStartDoneStatus);

   STATBLK                 * s = NULL;

   union
   {
      unsigned int              netAddrInt[2];
      char                      netAddrCharTOK[TOK_NADR_LENGTH];
      char                      netAddrCharENT[ent_NADR_LENGTH];
      char                      netAddrCharFDDI[FDDI_NADR_LENGTH];
   } temp = { {0,0} };

   /*-------------------------------*/
   /*  Allocate a new status block  */
   /*-------------------------------*/

   if ((s = new_STATBLK(STAT_COMIO,M_WAIT)) != NULL)
   {
      /*---------------------------*/
      /*  Initialize Status Block  */
      /*---------------------------*/

      s->stat.cio.code      = CIO_START_DONE;
      s->stat.cio.option[0] = CIO_OK;

      /*-------------------------------------*/
      /*  Copy in NetID and Network Address  */
      /*-------------------------------------*/

      switch(dev->dds.devType)
      {
         case CIE_DEV_ENT:
            memcpy(temp.netAddrCharENT, dev->ds.ent.curAddr, ent_NADR_LENGTH);
            s->stat.cio.option[1] = temp.netAddrInt[0];
            s->stat.cio.option[2] = temp.netAddrInt[1] >> 16;
            break;

         case CIE_DEV_FDDI:
            memcpy(temp.netAddrCharFDDI,dev->ds.fddi.curAddr,FDDI_NADR_LENGTH);
            s->stat.cio.option[1] = netid;
            s->stat.cio.option[2] = temp.netAddrInt[0];
            s->stat.cio.option[3] = temp.netAddrInt[1];
            break;

         case CIE_DEV_TOK:
            memcpy(temp.netAddrCharTOK, dev->ds.tok.curAddr, TOK_NADR_LENGTH);
            s->stat.cio.option[1] = netid;
            s->stat.cio.option[2] = temp.netAddrInt[0];
            s->stat.cio.option[3] = temp.netAddrInt[1];
            break;

         default:
         {
            TRC_OTHER(stmx,dev->dds.devType,0,0);
            break;
         }
      }
   }

   return s;
}

/*---------------------------------------------------------------------------*/
/*                    Create a CIO_HALT_DONE status block                    */
/*---------------------------------------------------------------------------*/

STATBLK *
   mkHaltDoneStatus(
      register DEV         * dev         ,// I -Device
      register netid_t       netid        // I -Net ID
   )
{
   FUNC_NAME(mkHaltDoneStatus);

   STATBLK                 * s = NULL;

   /*-------------------------------*/
   /*  Allocate a new status block  */
   /*-------------------------------*/

   if ((s = new_STATBLK(STAT_COMIO,M_WAIT)) != NULL)
   {
      /*---------------------------*/
      /*  Initialize Status Block  */
      /*---------------------------*/

      s->stat.cio.code      = CIO_HALT_DONE;
      s->stat.cio.option[0] = CIO_OK;

      switch(dev->dds.devType)
      {
         case CIE_DEV_ENT:
            break;

         case CIE_DEV_TOK:
            s->stat.cio.option[1] = netid;
            break;

         case CIE_DEV_FDDI:
            s->stat.cio.option[1] = netid;
            break;

         default:
         {
            TRC_OTHER(stmy,dev->dds.devType,0,0);
            break;
         }
      }
   }

   return s;
}

/*---------------------------------------------------------------------------*/
/*                    Create a CIO_NULL_BLK status block                     */
/*---------------------------------------------------------------------------*/

STATBLK *
   mkNullBlkStatus(
      void
   )
{
   STATBLK                 * s = NULL;

   /*-------------------------------*/
   /*  Allocate a new status block  */
   /*-------------------------------*/

   if ((s = new_STATBLK(STAT_COMIO,M_WAIT)) != NULL)
   {
      s->stat.cio.code = CIO_NULL_BLK;
   }

   return s;
}

/*---------------------------------------------------------------------------*/
/*                   Create a CIO_LOST_STATUS status block                   */
/*---------------------------------------------------------------------------*/

STATBLK *
   mkLostStatusStatus(
      void
   )
{
   STATBLK                 * s = NULL;

   /*-------------------------------*/
   /*  Allocate a new status block  */
   /*-------------------------------*/

   if ((s = new_STATBLK(STAT_COMIO,M_WAIT)) != NULL)
   {
      s->stat.cio.code = CIO_LOST_STATUS;
   }

   return s;
}

/*---------------------------------------------------------------------------*/
/*                     Create a CIO_TX_DONE status block                     */
/*---------------------------------------------------------------------------*/

STATBLK *
   mkTXDoneStatus(
      register int           status      ,// I -Status Code
      register int           write_id     // I -Write Correlator from Extension
   )
{
   FUNC_NAME(mkTXDoneStatus);

   STATBLK                 * s = NULL;

   /*-------------------------------*/
   /*  Allocate a new status block  */
   /*-------------------------------*/

   if ((s = new_STATBLK(STAT_COMIO,M_WAIT)) != NULL)
   {
      s->stat.cio.code      = CIO_TX_DONE;
      s->stat.cio.option[0] = status;
      s->stat.cio.option[1] = write_id;
   }

   return s;
}

/*---------------------------------------------------------------------------*/
/*               Encapsulate an NDD status block for queueing                */
/*---------------------------------------------------------------------------*/

STATBLK *
   mkCDLIStatus(
      register const ndd_statblk_t  * ns           // I -NDD Status Block
   )
{
   FUNC_NAME(mkCDLIStatus);

   STATBLK                 * s = NULL;

   /*-------------------------------*/
   /*  Allocate a new status block  */
   /*-------------------------------*/

   if ((s = new_STATBLK(STAT_CDLI,M_DONTWAIT)) != NULL)
   {
      s->stat.ndd = *ns;
   }

   return s;
}
