static char sccsid[] = "@(#)83	1.1  src/bos/diag/tu/gga/pcitu.c, tu_gla, bos41J, 9515A_all 4/6/95 09:27:18";
/*
 *   COMPONENT_NAME: tu_gla
 *
 *   FUNCTIONS: get_machine_model
 *              pci_tu
 *              set_machine_model
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <sys/devinfo.h>
#include <sys/ioctl.h>
#include <sys/mode.h>
#include <sys/mdio.h>

#include "exectu.h"
#include "tu_type.h"
#include "ggamisc.h"
#include "ggaextern.h"
#include "ggapci.h"

static PCI_CONFIG_SPACE *GGA_PCI_config_space;

/*
 * NAME : pci_tu
 *
 * DESCRIPTION :
 *
 * Tests GGA PCI. PCI registers are accessed and read and contents verified
 * against Fairway spec dated 2/9/94.
 *
 * INPUT :
 *
 *  None.
 *
 * OUTPUT :
 *
 *  None.
 *
 * RETURNS :
 *
 *  Error code or SUCCESS.
 *
*/

int pci_tu(void)
{
  BOOL             found;
  int              rc, i, pci_slot_offset[] = {PCI_SLOT1_BASE, PCI_SLOT2_BASE};
  ULONG          reg_val;
  unsigned char    *eq, equip, csp_present[] = {PORT_80C_SLOT1, PORT_80C_SLOT2, 0};
  PCI_CONFIG_SPACE *csp_ptr;

  /* Read the PCI_EQUIP_REGISTER from the system I/O space */
  equip = *((char *)(PCI_addr + PCI_EQUIP_PRESNT));

  found = FALSE;
  for(i=0; i<NUM_PCI_SLOTS; i++)
    {
      if ((equip & csp_present[i]) == csp_present[i])
        {
          csp_ptr = (PCI_CONFIG_SPACE *) pci_slot_offset[i];
          if (csp_ptr->vendor_id == IBM_ID && csp_ptr->device_id == P9100_ID)
            {
              GGA_PCI_config_space = csp_ptr;
              found = TRUE;
            }
        }
    }
  if (!found)
    return(FAIRWAY_NOT_FOUND);

  disable_video ();                              /* do not show random data  */
  rc = SUCCESS;

  reg_val = RL(GGA_pcispace + VENDOR_ID_REG);
  if (reg_val != IBM_ID)
    rc = PCI_REG_COMPARE_ERR;

  reg_val = RL(GGA_pcispace + DEVICE_ID_REG);
  if (reg_val != P9100_ID)
    rc = PCI_REG_COMPARE_ERR;

  reg_val = RL(GGA_pcispace + COMMAND_REG);
  if (reg_val != MEM_ACCESS_ENABLED)
    rc = PCI_REG_COMPARE_ERR;

  reg_val = RL(GGA_pcispace + REVISION_ID_REG);
  if (reg_val != FIXED_AT_ZERO)
    rc = PCI_REG_COMPARE_ERR;

  reg_val = RL(GGA_pcispace + STD_PROG_INT_REG);
  if (reg_val != FIXED_AT_ZERO)
    rc = PCI_REG_COMPARE_ERR;

  reg_val = RL(GGA_pcispace + SUBCLASS_CODE_REG);
  if (reg_val != OTHER_VIDEO_ADAPTER)
    rc = PCI_REG_COMPARE_ERR;

  reg_val = RL(GGA_pcispace + CLASS_CODE_REG);
  if (reg_val != DISPLAY_CONTROLLER)
    rc = PCI_REG_COMPARE_ERR;

  reg_val = RL(GGA_pcispace + CACHE_LINE_SIZE_REG);
  if (reg_val != FIXED_AT_ZERO)
    rc = PCI_REG_COMPARE_ERR;

  reg_val = RL(GGA_pcispace + HEADER_TYPE);
  if (reg_val != FIXED_AT_ZERO)
    rc = PCI_REG_COMPARE_ERR;

  reg_val = RL(GGA_pcispace + BIST_CONTROL_REG);
  if (reg_val != NO_BIST_SUPPORT)
    rc = PCI_REG_COMPARE_ERR;

  reg_val = RL(GGA_pcispace + INTERRUPT_LINE_REG);
  if (reg_val != FIXED_AT_255)
    rc = PCI_REG_COMPARE_ERR;

  reg_val = RL(GGA_pcispace + INTERRUPT_PIN_REG);
  if (reg_val != INT_TIED_TO_INTA)
    rc = PCI_REG_COMPARE_ERR;

  reg_val = RL(GGA_pcispace + MIN_GRANT_REG);
  if (reg_val != FIXED_AT_ZERO)
    rc = PCI_REG_COMPARE_ERR;

  reg_val = RL(GGA_pcispace + MAX_GRANT_REG);
  if (reg_val != FIXED_AT_ZERO)
    rc = PCI_REG_COMPARE_ERR;

  enable_video ();
  return (rc);
}


/****************************************************************************/
/* FUNCTION: wr_byte

   DESCRIPTION: Uses the machine device driver to write ONE BYTE to
                the specified address

****************************************************************************/

int wr_byte(int fdes, unsigned char data, unsigned int addr)
{
        MACH_DD_IO iob;
        int rc;
        unsigned char ldata;
        char *pdata;

        ldata = data;
        pdata = (char *) &ldata;

        iob.md_data = pdata;
        iob.md_incr = MV_BYTE;
        iob.md_size = 1;
        iob.md_addr = addr;
        rc = ioctl(fdes, MIOBUSPUT, &iob);
        return (rc);
}

/****************************************************************************/
/* FUNCTION: rd_byte

   DESCRIPTION: Uses the machine device driver to read ONE BYTE from
                the specified address

****************************************************************************/

int rd_byte(int fdes, unsigned char *pdata, unsigned int addr)
{
        MACH_DD_IO iob;
        int rc;

        iob.md_data = pdata;
        iob.md_incr = MV_BYTE;
        iob.md_size = 1;
        iob.md_addr = addr;
        rc = ioctl(fdes, MIOBUSGET, &iob);
        return (rc);
}


/****************************************************************************/
/* FUNCTION: wr_word

   DESCRIPTION: Uses the machine device driver to write a WORD from
                pdata to the specified address

****************************************************************************/

int wr_word(int fdes, unsigned long data, unsigned int addr)
{
        MACH_DD_IO iob;
        int rc;
        unsigned long ldata;
        char *pdata;

        ldata = data;
        pdata = (char *) &ldata;

        iob.md_data = (char *)pdata;
        iob.md_incr = MV_WORD;
        iob.md_size = 1;
        iob.md_addr = addr;
        rc = ioctl(fdes, MIOBUSPUT, &iob);
        return (rc);
}

/****************************************************************************/
/* FUNCTION: rd_word

   DESCRIPTION: Uses the machine device driver to read a WORD from
                pdata to the specified address

****************************************************************************/

int rd_word(int fdes, unsigned long *pdata, unsigned int addr)
{
        MACH_DD_IO iob;
        int rc;

        iob.md_data = (char *)pdata;
        iob.md_incr = MV_WORD;
        iob.md_size = 1;
        iob.md_addr = addr;
        rc = ioctl(fdes, MIOBUSGET, &iob);
        return (rc);
}

