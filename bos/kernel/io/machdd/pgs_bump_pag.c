static char sccsid[] = "@(#)73	1.7  src/bos/kernel/io/machdd/pgs_bump_pag.c, machdd, bos41J, bai15 2/14/95 17:08:31";
/*
 * @BULL_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: $
 * $EndLog$
 */
/*
 * COMPONENT_NAME:  (MACHDD) Machine Device Driver
 * 
 * FUNCTIONS: PEGASUS BUMP interface to Machine Device Driver
 * 
 * ORIGINS: 83 
 * 
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#ifdef _RS6K_SMP_MCA

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/mdio.h>
#include <sys/sys_resource.h>
#include <sys/m_intr.h>
#include <sys/processor.h>
#include <sys/thread.h>
#include <sys/inline.h>
#include "pgs_novram.h"
#include "pgs_bump.h"
#include "pegasus.h"

#define min(a, b)	((a) < (b) ? (a) : (b))
#define DISKETTE_INT_LVL	4

char *pgs_lcd;			/* lcd display address in NVRAM */
uint *pgs_electro_keyp;		/* electronic key address in NVRAM */
uint pgs_tp_buf_sz;		/* NVRAM buffer size  */
char *pgs_tp_buf;		/* NVRAM buffer address */
uint pgs_tp_buf_off;		/* NVRAM buffer offset */

extern int fd_mutex;
extern int pgs_SSGA_lvl;
extern void d_abort_fd();

/*
 * NAME: pgs_bumpstat_to_errno
 *
 * FUNCTION: converts the BUMP status to AIX errno.
 *
 * EXECUTION ENVIRONMENT:  Process; this routine can page fault.
 *
 * NOTES: stat is the BUMP status.
 *
 * RETURN VALUE: AIX errno
 *
 */
int pgs_bumpstat_to_errno(int stat)
{
	switch (stat) {
	case 0:			/* No error */
		return 0;
	case 0x01:		/* Non available processor */
		return ENXIO;
	case 0x02:		/* Key not in service mode */
	case 0xf6:		/* Attempt to write in a protected area */
	case 0xfa:		/* Service support not available */
	case 0xfc:		/* Bad Passwd */
	case 0xfd:		/* Bad customer Passwd */
		return EACCES;
	case 0xfb:		/* Info not valid */
	case 0xff:		/* Invalid command message or firmware file */
		return EINVAL;
	case 0x80:		/* CPU/BUMP transport error */
	case 0xfe:		/* i2c or Jtag communication error */
	default:		/* Unknown status */
		return EIO;
	}
}

/*
 * NAME: mdnvstrled
 *
 * FUNCTION: Sets the LEDs to the given ascii string on PEGASUS.
 *
 * EXECUTION ENVIRONMENT:  Process; this routine can page fault.
 *
 * RETURN VALUE: None
 *
 */

void mdnvstrled(char *str)
{
	int i;
/*
 * Pegasus NVRAM WA - See dma_ppc.c for details.
 *
 */

	int intpri ;
	cpu_t save_cpu;
	
	if (pgs_SSGA_lvl == 2) {
		save_cpu = CURTHREAD->t_cpuid;
		switch_cpu (MP_MASTER, SET_PROCESSOR_ID);
		pin ((uint) str,LCD_SZ);
		intpri = i_disable (DISKETTE_INT_LVL);
		if (fd_mutex)
			d_abort_fd();
	}

	for (i=0; i < LCD_SZ; i++) {
		pgs_lcd[i] = str[i];
		if (! str[i])
			break;
	}
	for (; i<LCD_SZ; i++)
		pgs_lcd[i] = '\0';

	if (pgs_SSGA_lvl == 2) {
		i_enable(intpri);
		unpin ((uint)str,LCD_SZ);
		switch_cpu (save_cpu,RESET_PROCESSOR_ID);
	}
}

/*
 * NAME: mdsetkey
 *
 * FUNCTION: Sets the electronic key on PEGASUS.
 *
 * EXECUTION ENVIRONMENT:  Process; this routine can page fault.
 *
 * RETURN VALUE: None
 *
 */
void mdsetkey(uint key)
{
/*
 * Pegasus NVRAM WA - See dma_ppc.c for details.
 *
 */
	int intpri;
	cpu_t save_cpu;

        if (pgs_SSGA_lvl == 2) {
                save_cpu = CURTHREAD->t_cpuid;
                switch_cpu (MP_MASTER, SET_PROCESSOR_ID);
                pin ((uint) &key,sizeof(uint));
                intpri = i_disable (DISKETTE_INT_LVL);
                if (fd_mutex)
                        d_abort_fd();
        }

	*pgs_electro_keyp = key;

	/* send an interrupt to BUMP */

	((struct pgs_sys_spec*)(sys_resource_ptr->sys_specific_regs))
		->set_int_bu = 0;
	__iospace_sync();		/* make sure seen */

	if (pgs_SSGA_lvl == 2) {
		i_enable(intpri);
		unpin ((uint)&key,sizeof(uint));
		switch_cpu (save_cpu,RESET_PROCESSOR_ID);
	}
}

#ifdef _POWER_MP


/*
 * NAME: mdcpuset
 *
 * FUNCTION: Sets the cpu status for next reboot on PEGASUS.
 *
 * EXECUTION ENVIRONMENT:  Process; this routine can page fault.
 *
 * NOTES: cpu_num is the physical cpu number
 *    
 * RETURN VALUE:
 *         0 : if no error
 */
int mdcpuset(uint cpu_num, uint cpu_stat)
{    
	pgs_bump_msg_t cmd, stat;
	extern int *start_bs_proc;
	register int rc;

	cmd.bump_portid = BUMP_SYSTEM_PORTID;
	cmd.command = cpu_stat;
	cmd.data.cpu.cpu_num = cpu_num;
	cmd.data.cpu.start_addr = start_bs_proc;
	if ((rc = mdbumpcmd(&cmd, &stat)) != 0)
		return(rc);
	return pgs_bumpstat_to_errno(stat.status);
}

/*
 * NAME: mdcpuget
 *
 * FUNCTION: Gets the cpu status for next reboot on PEGASUS.
 *
 * EXECUTION ENVIRONMENT:  Process; this routine can page fault.
 *
 * NOTES: cpu_num is the physical cpu number
 *    
 * RETURN VALUE:
 *         0 : if no error
 */
int mdcpuget(uint cpu_num, uint *cpu_stat)
{
	pgs_bump_msg_t cmd, stat;
	register int rc;

	cmd.command = PROCESSOR_STATUS;
	cmd.bump_portid = BUMP_SYSTEM_PORTID;
	cmd.data.cpu.cpu_num = cpu_num;
	if ((rc = mdbumpcmd(&cmd, &stat)) != 0)
	   return(rc);

	if (stat.status)
		return pgs_bumpstat_to_errno(stat.status);

	*cpu_stat = stat.data.cpu.cpu_status;
	return 0;
}
#endif /* _POWER_MP */

/*
 * NAME: mdeepromget
 *
 * FUNCTION: Read EEPROM on PEGASUS.
 *
 * EXECUTION ENVIRONMENT:  Process; this routine can page fault.
 *
 * NOTES: eenum: 	EEPROM number
 *        eep_off: 	offset in EEPROM
 *        iodata:	buffer address
 *        iosize:	buffer size
 *        length_addr: 	where to return the length effectively read 
 *    
 * RETURN VALUE:
 *         0 : if no error
 */
int mdeepromget(uint eenum, uint eep_off, char *iodata, uint iosize, uint *length_addr)
{
	pgs_bump_msg_t cmd, stat;
	register int rc;
/*
 * Pegasus NVRAM WA - See dma_ppc.c for details.
 *
 */
        int intpri;
        cpu_t save_cpu;

        if (pgs_SSGA_lvl == 2) {
                save_cpu = CURTHREAD->t_cpuid;
                switch_cpu (MP_MASTER, SET_PROCESSOR_ID);
                pin ((uint) iodata,iosize);
                intpri = i_disable (DISKETTE_INT_LVL);
                if (fd_mutex)
                        d_abort_fd();
        }

	if (iosize > pgs_tp_buf_sz) {
		if (pgs_SSGA_lvl == 2) {
			i_enable(intpri);
			unpin ((uint) iodata,iosize);
			switch_cpu (save_cpu,RESET_PROCESSOR_ID);
		}
		return ENOMEM;
	}

	cmd.command = EEPROM_READ;
	cmd.bump_portid = BUMP_EEPROM_PORTID;
	cmd.data.eeprom.eep_num = eenum;
	cmd.data.eeprom.data_length = iosize;
	cmd.data.eeprom.data_off = pgs_tp_buf_off;
	cmd.data.eeprom.eep_off = eep_off;
	cmd.data.eeprom.vpd_desc[0] = cmd.data.eeprom.vpd_desc[1] = 0;

	if ((rc = mdbumpcmd(&cmd, &stat)) != 0) {
		if (pgs_SSGA_lvl == 2) {
			i_enable(intpri);
			unpin ((uint) iodata,iosize);
			switch_cpu (save_cpu,RESET_PROCESSOR_ID);
		}
	   return(rc);
	}

	if (stat.status) {
		if (pgs_SSGA_lvl == 2) {
			i_enable(intpri);
			unpin ((uint) iodata,iosize);
			switch_cpu (save_cpu,RESET_PROCESSOR_ID);
		}
		return pgs_bumpstat_to_errno(stat.status);
	}

	/* status length should be < command length */
	*length_addr = min(iosize, stat.data.eeprom.data_length);
	bcopy(pgs_tp_buf, iodata, *length_addr);
	if (pgs_SSGA_lvl == 2) {
		i_enable(intpri);
		unpin ((uint)iodata,iosize);
		switch_cpu (save_cpu,RESET_PROCESSOR_ID);
	}

	return(0);
}

/*
 * NAME: mdeevpdget
 *
 * FUNCTION: Read vpd on PEGASUS.
 *
 * EXECUTION ENVIRONMENT:  Process; this routine can page fault.
 *
 * NOTES:
 *	eenum: 		EEPROM number
 *	vpd_desc: 	vpd string identifier
 *	iodata:		buffer address
 *	iosize:		buffer size
 *	length_addr: 	where to return the length effectively read 
 *    
 * RETURN VALUE:
 *         0 : if no error
 */
int mdeevpdget(uint eenum, char *vpd_desc, char *iodata, uint iosize, uint *length_addr)
{
	pgs_bump_msg_t cmd, stat;
	register int rc;

/*
 * Pegasus NVRAM WA - See dma_ppc.c for details.
 *
 */
        int intpri; 
        cpu_t save_cpu;

        if (pgs_SSGA_lvl == 2) {
                save_cpu = CURTHREAD->t_cpuid;
                switch_cpu (MP_MASTER, SET_PROCESSOR_ID);
                pin ((uint) iodata,iosize);
                intpri = i_disable (DISKETTE_INT_LVL);
                if (fd_mutex)
                        d_abort_fd();
        }

	if (iosize > pgs_tp_buf_sz) {
		if (pgs_SSGA_lvl == 2) {
			i_enable(intpri);
			unpin ((uint)iodata,iosize);
			switch_cpu (save_cpu,RESET_PROCESSOR_ID);
		}
		return ENOMEM;
	}

	cmd.command = EEPROM_READ;
	cmd.bump_portid = BUMP_EEPROM_PORTID;
	cmd.data.eeprom.eep_num = eenum;
	cmd.data.eeprom.data_length = iosize;
	cmd.data.eeprom.data_off = pgs_tp_buf_off;
	cmd.data.eeprom.vpd_desc[0] = vpd_desc[0];
	cmd.data.eeprom.vpd_desc[1] = vpd_desc[1];

	if ((rc = mdbumpcmd(&cmd, &stat)) != 0) {
		if (pgs_SSGA_lvl == 2) {
			i_enable(intpri);
			unpin ((uint)iodata,iosize);
			switch_cpu (save_cpu,RESET_PROCESSOR_ID);
		}
	   return(rc);
	}

	if (stat.status) {
		if (pgs_SSGA_lvl == 2) {
			i_enable(intpri);
			unpin ((uint)iodata,iosize);
			switch_cpu (save_cpu,RESET_PROCESSOR_ID);
		}
		return pgs_bumpstat_to_errno(stat.status);
	}

	/* status length should be < command length */
	*length_addr = min(iosize, stat.data.eeprom.data_length);
	bcopy(pgs_tp_buf, iodata, *length_addr);
	if (pgs_SSGA_lvl == 2) {
		i_enable(intpri);
		unpin ((uint)iodata,iosize);
		switch_cpu (save_cpu,RESET_PROCESSOR_ID);
	}
	return(0);}
	

/*
 * NAME: mdeevpdput
 *
 * FUNCTION: Write vpd on PEGASUS.
 *
 * EXECUTION ENVIRONMENT:  Process; this routine can page fault.
 *
 * NOTES: eenum: 	EEPROM number
 *        vpd_desc: 	vpd string identifier
 *        iodata:	buffer address
 *        iosize:	buffer size
 *        length_addr: 	where to return the length effectively written
 *    
 * RETURN VALUE:
 *         0 : if no error
 */
int mdeevpdput(uint eenum, char *vpd_desc, char *iodata, uint iosize, uint *length_addr)
{
	pgs_bump_msg_t cmd, stat;
	register int rc;

/*
 * Pegasus NVRAM WA - See dma_ppc.c for details.
 *
 */
        int intpri ; 
        cpu_t save_cpu;

        if (pgs_SSGA_lvl == 2) {
                save_cpu = CURTHREAD->t_cpuid;
                switch_cpu (MP_MASTER, SET_PROCESSOR_ID);
                pin ((uint) iodata,iosize);
                intpri = i_disable (DISKETTE_INT_LVL);
                if (fd_mutex)
                        d_abort_fd();
        }

        if (((get_pksr() & 0xf) &0x00000003)!=2) {
		if (pgs_SSGA_lvl == 2) {
			i_enable(intpri);
			unpin ((uint)iodata,iosize);
			switch_cpu (save_cpu,RESET_PROCESSOR_ID);
		}
                return(EACCES);
	}
	if (iosize > pgs_tp_buf_sz) {
		if (pgs_SSGA_lvl == 2) {
			i_enable(intpri);
			unpin ((uint)iodata,iosize);
			switch_cpu (save_cpu,RESET_PROCESSOR_ID);
		}
		return ENOMEM;
	}
	bcopy(iodata, pgs_tp_buf, iosize);

	cmd.command = EEPROM_LOAD;
	cmd.bump_portid = BUMP_EEPROM_PORTID;
	cmd.data.eeprom.eep_num = eenum;
	cmd.data.eeprom.data_length = iosize;
	cmd.data.eeprom.data_off = pgs_tp_buf_off;
	cmd.data.eeprom.vpd_desc[0] = vpd_desc[0];
	cmd.data.eeprom.vpd_desc[1] = vpd_desc[1];

	if ((rc = mdbumpcmd(&cmd, &stat)) != 0) {
		if (pgs_SSGA_lvl == 2) {
			i_enable(intpri);
			unpin ((uint)iodata,iosize);
			switch_cpu (save_cpu,RESET_PROCESSOR_ID);
		}
		return(rc);
	}

	if (stat.status) {
		if (pgs_SSGA_lvl == 2) {
			i_enable(intpri);
			unpin ((uint)iodata,iosize);
			switch_cpu (save_cpu,RESET_PROCESSOR_ID);
		}
		return pgs_bumpstat_to_errno(stat.status);
	}

	*length_addr = stat.data.eeprom.data_length;
	if (pgs_SSGA_lvl == 2) {
		i_enable(intpri);
		unpin ((uint)iodata,iosize);
		switch_cpu (save_cpu,RESET_PROCESSOR_ID);
	}

	return(0);
}

/*
 * NAME: mdfepromput
 *
 * FUNCTION: Write FEPROM on PEGASUS.
 *
 * EXECUTION ENVIRONMENT:  Process; this routine can page fault.
 *
 * NOTES:
 *        iodata:	buffer address
 *        iosize:	buffer size
 *        length_addr: 	where to return the length effectively written
 *    
 * RETURN VALUE:
 *         0 : if no error
 */
int mdfepromput(char *iodata, uint iosize, uint *length_addr)
{
	pgs_bump_msg_t cmd, stat;
	register int rc;

/*
 * Pegasus NVRAM WA - See dma_ppc.c for details.
 *
 */
        int intpri; 
        cpu_t save_cpu;

        if (pgs_SSGA_lvl == 2) {
                save_cpu = CURTHREAD->t_cpuid;
                switch_cpu (MP_MASTER, SET_PROCESSOR_ID);
                pin ((uint) iodata,iosize);
                intpri = i_disable (DISKETTE_INT_LVL);
                if (fd_mutex)
                        d_abort_fd();
        }

        if (((get_pksr() & 0xf) &0x00000003)!=2) {
		if (pgs_SSGA_lvl == 2) {
			i_enable(intpri);
			unpin ((uint)iodata,iosize);
			switch_cpu (save_cpu,RESET_PROCESSOR_ID);
		}
                return(EACCES);
	}

	if (iosize > pgs_tp_buf_sz) {
		if (pgs_SSGA_lvl == 2) {
			i_enable(intpri);
			unpin ((uint)iodata,iosize);
			switch_cpu (save_cpu,RESET_PROCESSOR_ID);
		}
		return ENOMEM;
	}
	bcopy(iodata, pgs_tp_buf, iosize);

	cmd.command = FEPROM_LOAD;
	cmd.bump_portid = BUMP_FEPROM_PORTID;
	cmd.data.feprom.data_length = iosize;
	cmd.data.feprom.data_off = pgs_tp_buf_off;

	if ((rc = mdbumpcmd(&cmd, &stat)) != 0) {
		if (pgs_SSGA_lvl == 2) {
			i_enable(intpri);
			unpin ((uint)iodata,iosize);
			switch_cpu (save_cpu,RESET_PROCESSOR_ID);
		}
	   return(rc);
	}

	if (stat.status) {
		if (pgs_SSGA_lvl == 2) {
			i_enable(intpri);
			unpin ((uint)iodata,iosize);
			switch_cpu (save_cpu,RESET_PROCESSOR_ID);
		}
		return pgs_bumpstat_to_errno(stat.status);
	}

	*length_addr = stat.data.feprom.data_length;
	if (pgs_SSGA_lvl == 2) {
		i_enable(intpri);
		unpin ((uint)iodata,iosize);
		switch_cpu (save_cpu,RESET_PROCESSOR_ID);
	}

	return(0);
}

/*
 * NAME: mdpowerset
 *
 * FUNCTION: send a command on PEGASUS power supply.
 *
 * EXECUTION ENVIRONMENT:  Process; this routine can page fault.
 *
 * NOTES:
 *        cbnum:	cabinet number
 *        pwr_cmd:	command
 *        iodata:	buffer address
 *        iosize:	buffer size
 *        length_addr: 	where to return effectively written
 *    
 * RETURN VALUE:
 *         0 : if no error
 */
int mdpowerset(uint cbnum, uint pwr_cmd, char *iodata, uint iosize, uint *length_addr)
{
	pgs_bump_msg_t cmd, stat;
	register int rc;

/*
 * Pegasus NVRAM WA - See dma_ppc.c for details.
 *
 */
        int intpri; 
        cpu_t save_cpu;

        if (pgs_SSGA_lvl == 2) {
                save_cpu = CURTHREAD->t_cpuid;
                switch_cpu (MP_MASTER, SET_PROCESSOR_ID);
                pin ((uint) iodata,iosize);
                intpri = i_disable (DISKETTE_INT_LVL);
                if (fd_mutex)
                        d_abort_fd();
        }

	if (iosize > pgs_tp_buf_sz) {
		if (pgs_SSGA_lvl == 2) {
			i_enable(intpri);
			unpin ((uint)iodata,iosize);
			switch_cpu (save_cpu,RESET_PROCESSOR_ID);
		}
		return ENOMEM;
	}
	bcopy(iodata, pgs_tp_buf, iosize);

	cmd.command = POWER_STATUS_CMD;
	cmd.data.power.func = pwr_cmd;
	cmd.bump_portid = BUMP_POWER_PORTID;
	cmd.data.power.data_length = iosize;
	cmd.data.power.data_off = pgs_tp_buf_off;
	cmd.data.power.cabinet_num = cbnum;

	if ((rc = mdbumpcmd(&cmd, &stat) != 0)) {
		if (pgs_SSGA_lvl == 2) {
			i_enable(intpri);
			unpin ((uint)iodata,iosize);
			switch_cpu (save_cpu,RESET_PROCESSOR_ID);
		}
	   return(rc);
	}

	if (stat.status) {
		if (pgs_SSGA_lvl == 2) {
			i_enable(intpri);
			unpin ((uint)iodata,iosize);
			switch_cpu (save_cpu,RESET_PROCESSOR_ID);
		}
		return pgs_bumpstat_to_errno(stat.status);
	}

	*length_addr = stat.data.power.data_length;
	if (pgs_SSGA_lvl == 2) {
		i_enable(intpri);
		unpin ((uint)iodata,iosize);
		switch_cpu (save_cpu,RESET_PROCESSOR_ID);
	}
	return(0);
}

/*
 * NAME: mdpowerget
 *
 * FUNCTION: get information from PEGASUS power supply.
 *
 * EXECUTION ENVIRONMENT:  Process; this routine can page fault.
 *
 * NOTES:
 *        cbnum:	cabinet number
 *        pwr_cmd:	command
 *        iodata:	buffer address
 *        iosize:	buffer size
 *        length_addr: 	where to return the length effectively read
 *    
 * RETURN VALUE:
 *         0 : if no error
 */
int mdpowerget(uint cbnum, char *iodata, uint iosize, uint *length_addr)
{
	pgs_bump_msg_t cmd, stat;
	register int rc;

/*
 * Pegasus NVRAM WA - See dma_ppc.c for details.
 *
 */
        int intpri ; 
        cpu_t save_cpu;

        if (pgs_SSGA_lvl == 2) {
                save_cpu = CURTHREAD->t_cpuid;
                switch_cpu (MP_MASTER, SET_PROCESSOR_ID);
                pin ((uint) iodata,iosize);
                intpri = i_disable (DISKETTE_INT_LVL);
                if (fd_mutex)
                        d_abort_fd();
        }

	if (iosize > pgs_tp_buf_sz) {
		if (pgs_SSGA_lvl == 2) {
			i_enable(intpri);
			unpin ((uint)iodata,iosize);
			switch_cpu (save_cpu,RESET_PROCESSOR_ID);
		}
		return ENOMEM;
	}

	cmd.command = POWER_STATUS_READ;
	cmd.bump_portid = BUMP_POWER_PORTID;
	cmd.data.power.data_length = iosize;
	cmd.data.power.data_off = pgs_tp_buf_off;
	cmd.data.power.cabinet_num = cbnum;

	if ((rc = mdbumpcmd(&cmd, &stat) != 0)) {
		if (pgs_SSGA_lvl == 2) {
			i_enable(intpri);
			unpin ((uint)iodata,iosize);
			switch_cpu (save_cpu,RESET_PROCESSOR_ID);
		}
	   return(rc);
	}

	if (stat.status) {
		if (pgs_SSGA_lvl == 2) {
			i_enable(intpri);
			unpin ((uint)iodata,iosize);
			switch_cpu (save_cpu,RESET_PROCESSOR_ID);
		}
		return pgs_bumpstat_to_errno(stat.status);
	}
	
	/* status length should be < command length */
	*length_addr = min(iosize, stat.data.power.data_length);
	bcopy(pgs_tp_buf, iodata, *length_addr);
	if (pgs_SSGA_lvl == 2) {
		i_enable(intpri);
		unpin ((uint)iodata,iosize);
		switch_cpu (save_cpu,RESET_PROCESSOR_ID);
	}
	return(0);
}

/*
 * NAME: mdrdsset
 *
 * FUNCTION: Remote Disk Switch command on PEGASUS disk.
 *
 * EXECUTION ENVIRONMENT:  Process; this routine can page fault.
 *
 * NOTES:
 *	cbnum: 	cabinet number
 *	dknum:	disk number
 *	rds_state:	RDS_ON / RDS_OFF
 *    
 * RETURN VALUE:
 *         0
 */
int mdrdsset(uint cbnum, uint dknum, uint rds_state)
{
	pgs_bump_msg_t cmd, stat;
	register int rc;

	cmd.command = RDS_COMMAND;
	cmd.bump_portid = BUMP_RDS_PORTID;
	cmd.data.rds.rds_state = rds_state;
	cmd.data.rds.cabinet_num = cbnum;
	cmd.data.rds.disk_num = dknum;

	if ((rc = mdbumpcmd(&cmd, &stat) != 0))
		return(rc);

	return pgs_bumpstat_to_errno(stat.status);
}

/*
 * NAME: mdrdsget
 *
 * FUNCTION: get Remote Disk Switch status on PEGASUS disk.
 *
 * EXECUTION ENVIRONMENT:  Process; this routine can page fault.
 *
 * NOTES:
 *	cbnum: 		cabinet number
 *	dknum:		disk number
 *	rds_state:	where to return the disk status
 *    
 * RETURN VALUE:
 *         0
 */
int mdrdsget(uint cbnum, uint dknum, uint *rds_state)
{
	pgs_bump_msg_t cmd, stat;
	register int rc;

	cmd.command = RDS_STATE_READ;
	cmd.bump_portid = BUMP_RDS_PORTID;
	cmd.data.rds.cabinet_num = cbnum;
	cmd.data.rds.disk_num = dknum;

	if ((rc = mdbumpcmd(&cmd, &stat) != 0))
	   return(rc);

	if (stat.status)
		return pgs_bumpstat_to_errno(stat.status);

	*rds_state = stat.data.rds.rds_state;
	return(0);
}

/*
 * NAME: mdinfoset
 *
 * FUNCTION: set PEGASUS diagnostic information.
 *
 * EXECUTION ENVIRONMENT:  Process; this routine can page fault.
 *
 * NOTES:
 *	type:		info type
 *	iodata:		buffer address
 *	iosize:		buffer size
 *	length_addr: 	where to return the length effectively written
 *    
 * RETURN VALUE:
 *         0 : if no error
 */
int mdinfoset(uint type, char *iodata, uint iosize, uint *length_addr)
{
	pgs_bump_msg_t cmd, stat;
	register int rc;

/*
 * Pegasus NVRAM WA - See dma_ppc.c for details.
 *
 */
        int intpri ; 
        cpu_t save_cpu;

        if (pgs_SSGA_lvl == 2) {
                save_cpu = CURTHREAD->t_cpuid;
                switch_cpu (MP_MASTER, SET_PROCESSOR_ID);
                pin ((uint) iodata,iosize);
                intpri = i_disable (DISKETTE_INT_LVL);
                if (fd_mutex)
                        d_abort_fd();
        }

	if (iosize > pgs_tp_buf_sz) {
		if (pgs_SSGA_lvl == 2) {
			i_enable(intpri);
			unpin ((uint)iodata,iosize);
			switch_cpu (save_cpu,RESET_PROCESSOR_ID);
		}
		return ENOMEM;
	}
	bcopy(iodata, pgs_tp_buf, iosize);

	cmd.command = DIAG_INFO_WRITE;
	cmd.bump_portid = BUMP_AUTODIAL_PORTID;
	cmd.data.dinfo.type = type;
	cmd.data.dinfo.data_length = iosize;
	cmd.data.dinfo.data_off = pgs_tp_buf_off;

	if ((rc = mdbumpcmd(&cmd, &stat) != 0)) {
		if (pgs_SSGA_lvl == 2) {
			i_enable(intpri);
			unpin ((uint)iodata,iosize);
			switch_cpu (save_cpu,RESET_PROCESSOR_ID);
		}
	   return(rc);
	}

	if (stat.status) {
		if (pgs_SSGA_lvl == 2) {
			i_enable(intpri);
			unpin ((uint)iodata,iosize);
			switch_cpu (save_cpu,RESET_PROCESSOR_ID);
		}
		return pgs_bumpstat_to_errno(stat.status);
	}

	*length_addr = stat.data.dinfo.data_length;
	if (pgs_SSGA_lvl == 2) {
		i_enable(intpri);
		unpin ((uint)iodata,iosize);
		switch_cpu (save_cpu,RESET_PROCESSOR_ID);
	}
	return(0);
}

/*
 * NAME: mdinfoget
 *
 * FUNCTION: get PEGASUS diagnostic information.
 *
 * EXECUTION ENVIRONMENT:  Process; this routine can page fault.
 *
 * NOTES:
 *	type:		number type
 *	iodata:		buffer address
 *	iosize:		buffer size
 *	length_addr: 	where to return the length effectively read
 *    
 * RETURN VALUE:
 *         0 : if no error
 */
int mdinfoget(uint type, char *iodata, uint iosize, uint *length_addr)
{
	pgs_bump_msg_t cmd, stat;
	register int rc;

/*
 * Pegasus NVRAM WA - See dma_ppc.c for details.
 *
 */
        int intpri ; 
        cpu_t save_cpu;

        if (pgs_SSGA_lvl == 2) {
                save_cpu = CURTHREAD->t_cpuid;
                switch_cpu (MP_MASTER, SET_PROCESSOR_ID);
                pin ((uint) iodata,iosize);
                intpri = i_disable (DISKETTE_INT_LVL);
                if (fd_mutex)
                        d_abort_fd();
        }

	if (iosize > pgs_tp_buf_sz) {
		if (pgs_SSGA_lvl == 2) {
			i_enable(intpri);
			unpin ((uint)iodata,iosize);
			switch_cpu (save_cpu,RESET_PROCESSOR_ID);
		}
		return ENOMEM;
	}

	cmd.command = DIAG_INFO_READ;
	cmd.bump_portid = BUMP_AUTODIAL_PORTID;
	cmd.data.dinfo.type = type;
	cmd.data.dinfo.data_length = iosize;
	cmd.data.dinfo.data_off = pgs_tp_buf_off;

	if ((rc = mdbumpcmd(&cmd, &stat) != 0)) {
		if (pgs_SSGA_lvl == 2) {
			i_enable(intpri);
			unpin ((uint)iodata,iosize);
			switch_cpu (save_cpu,RESET_PROCESSOR_ID);
		}
	   return(rc);
	}

	if (stat.status) {
                i_enable(intpri);
                unpin ((uint) iodata, iosize);
                switch_cpu (save_cpu,RESET_PROCESSOR_ID);
		return pgs_bumpstat_to_errno(stat.status);
	}

	/* status length should be < command length */
	*length_addr = min(iosize, stat.data.dinfo.data_length);
	bcopy(pgs_tp_buf, iodata, *length_addr);
	if (pgs_SSGA_lvl == 2) {
		i_enable(intpri);
		unpin ((uint)iodata,iosize);
		switch_cpu (save_cpu,RESET_PROCESSOR_ID);
	}
	return(0);
}

/*
 * NAME: mdsurvset
 *
 * FUNCTION: set cpu surveillance by BUMP on PEGASUS.
 *
 * EXECUTION ENVIRONMENT:  Process; this routine can page fault.
 *
 * NOTES:
 *	mode:	soft/hard reset on delay expiration
 *	delay:	in secs
 *    
 * RETURN VALUE:
 *         0
 */
int mdsurvset(uint mode, uint delay)
{
	pgs_bump_msg_t cmd, stat;
	register int rc;

        cmd.bump_portid = BUMP_SYSTEM_PORTID;
	cmd.command = CPU_SURVEILLANCE;
	cmd.data.surv.start_stop = 0;
	cmd.data.surv.mode = mode;
	cmd.data.surv.delay = delay;
	
	if ((rc = mdbumpcmd(&cmd, &stat) != 0)) {
	   return(rc);
	}
	return pgs_bumpstat_to_errno(stat.status);
}

/*
 * NAME: mdsurvreset
 *
 * FUNCTION: reset cpu surveillance by BUMP on PEGASUS.
 *
 * EXECUTION ENVIRONMENT:  Process; this routine can page fault.
 *
 * RETURN VALUE:
 *         0
 */
int mdsurvreset()
{
	pgs_bump_msg_t cmd, stat;
	register int rc;


	cmd.bump_portid = BUMP_SYSTEM_PORTID;
	cmd.command = CPU_SURVEILLANCE;
	cmd.data.surv.start_stop = 1;
	
	if ((rc = mdbumpcmd(&cmd, &stat) != 0)) {
		return(rc);
	}

	return pgs_bumpstat_to_errno(stat.status);
}

#endif /* _RS6K_SMP_MCA */
