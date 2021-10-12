static char sccsid[] = "@(#)01  1.5.1.2  src/bos/kernext/dlc/lan/lanhash.c, sysxdlcg, bos411, 9428A410j 2/21/94 18:48:29";

/*
 * COMPONENT_NAME: (SYSXDLCG) Generic Data Link Control
 *
 * FUNCTIONS: lanhash.c
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
                                                                      */

#include <fcntl.h>
#include <sys/types.h>
#include <net/spl.h>
#include <sys/mbuf.h>
#include <sys/file.h>
#include <sys/malloc.h>
#include <sys/errno.h>
#include "dlcadd.h"
#include <sys/gdlextcb.h>
/* <<< defect 127819 >>> */
#ifdef   TRL
#include <sys/trlextcb.h>
#endif
#ifdef   FDL
#include <sys/fdlextcb.h>
#endif
/* <<< removed fddi, tok, and comio header files >>> */
/* <<< end defect 127819 >>> */
#include "lancomcb.h"
#include "lanstlst.h"
#include "lansplst.h"
#include "lanrstat.h"
#include "lanport.h"
#include "lanhasht.h"
find_sta_in_hash(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  find_sta_in_hash                                     */
/*                                                                    */
/* descriptive name:  find station in receive hash table.             */
/*                                                                    */
/* function:  finds and sets the station list stano and hashno indexes*/
/*            for the given input hash string.  the input string must */
/*            match the string found in the receive hash table, or    */
/*            both stano and hashno are set to "no match".            */
/*                                                                    */
/* input:  an 8-byte address string to be located via the receive     */
/*         buffer.  (source address, dsap, ssap)                      */
/*                                                                    */
/* output:  the station p->stano and hashno indexes.                  */
/*                                                                    */
/*** end of specifications ********************************************/

{
  int      ctr;                        /* scan counter                */
  union
    {
      struct
        {
          union
            {
              ulong    w8765;          /* RADDR word for bytes 8-7-6-5*/
              struct
                {
                  u_char   b8;
                  u_char   b7;
                  u_char   b6;
                  u_char   b5;
                } 
              hw1;
            } 
          u1;
          union
            {
              ulong    w4321;          /* RADDR bytes 4-3, and        
                                          DSAP/SSAP pair              */
              struct
                {
                  u_char   b4;
                  u_char   b3;
                  u_char   b2;
                  u_char   b1;
                } 
              hw2;
            } 
          u2;
        } 
      h1;
      struct
        {
          u_char   hash_raddr[6];      /* remote address              */
          u_char   hash_lsap;          /* local SAP                   */
          u_char   hash_rsap;          /* remote SAP                  */
        } 
      h2;
      u_char   rcv_hash_string[8];     /* remote SAP                  */
    } 
  hash;

  /********************************************************************/
  /* find and set the station list stationno and hashno for the given */
  /* input hash string, comparing it to the string in the receive     */
  /* hashing table. copy ssap, lsap, and rsap received into the hash  */
  /* string note - the rsap must have the response indicator reset.   */
  /********************************************************************/

  bcopy(p->rcv_data.raddr, hash.h2.hash_raddr, 6);
  hash.h2.hash_lsap = p->rcv_data.lsap;
  hash.h2.hash_rsap = (p->rcv_data.rsap&RESP_OFF);

  /********************************************************************/
  /* set the starting hash index to the ssap value received, and for  */
  /* each of the remaining received raddr(6) and dsap(1) bytes, set   */
  /* the next hash index to the exclusive-or of the hash table value  */
  /* and the received byte value.                                     */
  /********************************************************************/

    {
      p->common_cb.hashno = rcv_hash_tbl[hash.h1.u2.hw2.b1]^
         hash.h1.u2.hw2.b2;
      p->common_cb.hashno = rcv_hash_tbl[p->common_cb.hashno]^
         hash.h1.u2.hw2.b3;
      p->common_cb.hashno = rcv_hash_tbl[p->common_cb.hashno]^
         hash.h1.u2.hw2.b4;
      p->common_cb.hashno = rcv_hash_tbl[p->common_cb.hashno]^
         hash.h1.u1.hw1.b5;
      p->common_cb.hashno = rcv_hash_tbl[p->common_cb.hashno]^
         hash.h1.u1.hw1.b6;
      p->common_cb.hashno = rcv_hash_tbl[p->common_cb.hashno]^
         hash.h1.u1.hw1.b7;
      p->common_cb.hashno = rcv_hash_tbl[p->common_cb.hashno]^
         hash.h1.u1.hw1.b8;
    } 

  /********************************************************************/
  /* convert the resultant index to modulo the maximum number of link */
  /* stations allowed.                                                */
  /********************************************************************/

  p->common_cb.hashno %= MAX_SESSIONS;

  /********************************************************************/
  /* index to the next station, up to the maximum number of link      */
  /* stations.                                                        */
  /********************************************************************/
 

  for (ctr = 1; ctr <= MAX_SESSIONS+1; ctr++)
    {                                  /* repeats the first index to  
                                          insure exit                 */
 
      if                               /* the input string does equal 
                                          the string located in the   */

      /****************************************************************/
      /* receive station table at this index.                         */
      /****************************************************************/

         ((hash.h1.u1.w8765 == p->rcv_sta_tbl[p->common_cb.hashno].
         u_cs.s2.cmp_w8765) && (hash.h1.u2.w4321 == p->rcv_sta_tbl
         [p->common_cb.hashno].u_cs.s2.cmp_w4321))
        break;

      /****************************************************************/
      /* compute the next index.                                      */
      /****************************************************************/

      p->common_cb.hashno = (p->common_cb.hashno+1)%MAX_SESSIONS;
    }                                  /* end for ctr;                */
 
  if                                   /* the link station has not    
                                          been found                  */
     (ctr > MAX_SESSIONS+1)
    {

      /****************************************************************/
      /* set the return stationno and hash values to "no match".      */
      /****************************************************************/

      p->stano = NO_MATCH;
      p->common_cb.hashno = NO_MATCH;
    } 
 
  else                                 /* the station match has been  
                                          found.                      */
    {

      /****************************************************************/
      /* get the stationno from the receive station table.            */
      /****************************************************************/

      p->stano = p->rcv_sta_tbl[p->common_cb.hashno].sta_list_slot;
    } 
}                                      /* end find_sta_in_hash;       */
add_sta_to_hash(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  add_sta_to_hash                                      */
/*                                                                    */
/* descriptive name:  add station to the receive hash table.          */
/*                                                                    */
/* function:  adds the station list stano for the given input hash    */
/*            string to the receive hashing table.  if an empty       */
/*            entry is located, the hashno index is set to the table  */
/*            index.  if no empty entries are available, the hashno   */
/*            index is set to "no match".                             */
/*                                                                    */
/* input:  an 8-byte address string and its associated stano index.   */
/*                                                                    */
/* output:  the station stano is saved in the receive hash table,     */
/*          the hashno is set, and the station is declared active.    */
/*                                                                    */
/*** end of specifications ********************************************/

{
  int      n;                          /* input string index          */
  int      ctr;                        /* scan counter                */

  /********************************************************************/
  /* add the station list stationno for the given input hash string to*/
  /* the receive hashing table, and set the hashno value. set the     */
  /* starting hash index to the value in the 8th byte of the input    */
  /* hash string.                                                     */
  /********************************************************************/

  p->common_cb.hashno = p->common_cb.u_h.hash_string[7];

  /********************************************************************/
  /* for each of the remaining input string bytes from 7 to 1         */
  /********************************************************************/
 

  for (n = 6; n >= 0; n--)
    {

      /****************************************************************/
      /* set the next hash index to the exclusive-or of the hash table*/
      /* value and the input hash string value.                       */
      /****************************************************************/

      p->common_cb.hashno = rcv_hash_tbl[p->common_cb.hashno]^
         p->common_cb.u_h.hash_string[n];
    }                                  /* end do n;                   */

  /********************************************************************/
  /* convert the resultant index to modulo the maximum number of link */
  /* stations allowed.                                                */
  /********************************************************************/

  p->common_cb.hashno %= MAX_SESSIONS;

  /********************************************************************/
  /* index to the next station, up to the maximum number of link      */
  /* stations. ctr = 1 to max+1 repeats the first index to insure exit*/
  /********************************************************************/
 

  for (ctr = 1; ctr <= MAX_SESSIONS+1; ctr++)
    {

      /****************************************************************/
      /* the receive station table entry is found to be "in use" at   */
      /* this index.                                                  */
      /****************************************************************/
 

      if (p->rcv_sta_tbl[p->common_cb.hashno].in_use != TRUE)
        break;

      /****************************************************************/
      /* compute the next index.                                      */
      /****************************************************************/

      p->common_cb.hashno = (p->common_cb.hashno+1)%MAX_SESSIONS;
    }                                  /* end do ctr;                 */
 
  if                                   /* the there are no empty table
                                          entries                     */
     (ctr > MAX_SESSIONS+1)
    {

      /****************************************************************/
      /* set the return receive station table hash value to "no match"
      .                                                               */
      /****************************************************************/

      p->common_cb.hashno = NO_MATCH;
    } 
 
  else                                 /* an empty table entry has    
                                          been found.                 */
    {

      /****************************************************************/
      /* indicate that the located table entry is now "in use".       */
      /****************************************************************/

      p->rcv_sta_tbl[p->common_cb.hashno].in_use = TRUE;

      /****************************************************************/
      /* load the stationno into the receive station table at the     */
      /* calculated hash index.                                       */
      /****************************************************************/

      p->rcv_sta_tbl[p->common_cb.hashno].sta_list_slot = p->stano;

      /****************************************************************/
      /* load the input string into the receive station table at the  */
      /* calculated hash index.                                       */
      /****************************************************************/

      bcopy(p->common_cb.u_h.hash_string, p->rcv_sta_tbl
         [p->common_cb.hashno].u_cs.cmp_string, sizeof
         (p->common_cb.u_h.hash_string));

      /****************************************************************/
      /* save the receive station table hash value in the station     */
      /* list.                                                        */
      /****************************************************************/

      p->station_list[p->stano].sta_hash = p->common_cb.hashno;

      /****************************************************************/
      /* indicate that the station is now active.                     */
      /****************************************************************/

      p->station_list[p->stano].sta_active = TRUE;
    } 
}                                      /* end add_sta_to_hash;        */
landelsh(p)
  register struct port_dcl *p;

/*** start of specifications ******************************************/
/*                                                                    */
/* module name:  landelsh                                             */
/*                                                                    */
/* descriptive name:  delete station from the receive hash table.     */
/*                                                                    */
/* function:  removes the station entry associated with the given     */
/*            station index from the receive hash table.              */
/*                                                                    */
/* input:  the station index.                                         */
/*                                                                    */
/* output:  the station entry is removed from the receive hash table. */
/*                                                                    */
/*** end of specifications ********************************************/

{
 
  if                                   /* the specified station slot  
                                          is valid (ie. in use)       */
     (p->station_list[p->stano].in_use == TRUE)
    {

      /****************************************************************/
      /* locate the hash index from the station list.                 */
      /****************************************************************/

      p->common_cb.hashno = p->station_list[p->stano].sta_hash;
 
      if                               /* the located hash index is   
                                          valid (ie. in use)          */
         (p->rcv_sta_tbl[p->common_cb.hashno].in_use == TRUE)
        {

          /************************************************************/
          /* zero the receive station table entry.                    */
          /************************************************************/

          bzero(&p->rcv_sta_tbl[p->common_cb.hashno], sizeof
             (p->rcv_sta_tbl[p->common_cb.hashno]));

          /************************************************************/
          /* remove the station hash index from the station list.     */
          /************************************************************/

          p->station_list[p->stano].sta_hash = 0;
        } 
 
      else                             /* the station list hash value 
                                          is invalid, which may happen*/

        /**************************************************************/
        /* if a listen has not completed at the time of shutdown.     */
        /**************************************************************/

        {

          /************************************************************/
          /* fall thru.                                               */
          /************************************************************/

        } 
    } 
 
  else                                 /* error - the input station   
                                          slot number is invalid.     */
    {

      /****************************************************************/
      /* call error log to log error -- sap error, program error      */
      /****************************************************************/

      lanerrlg(p, ERRID_LAN8011, NON_ALERT, 0, 0, FILEN, LINEN);
    } 
}                                      /* end landelsh;               */
