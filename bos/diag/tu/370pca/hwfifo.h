/* @(#)hwfifo.h	1.2 2/1/91 14:20:56 */
/*************************************************************************
* Meanings of bits in FIFO status word (MCI view)			 *
*************************************************************************/
#define MCI_REMPTY	0x01			/* means SFBF is all reserved */
#define MCI_EMPTY	0x04			/* means SFBF is empty */
#define MCI_RFULL	0x10			/* means RFBF is all reserved */
#define MCI_FULL	0x40			/* means RFBF is full */

/*************************************************************************
* Meanings of bits in FIFO status word (OBP view)			 *
*************************************************************************/
#define OBP_RF_FULL	0x01			/* means RFBF is full */
#define OBP_RF_EMPTY	0x02			/* means RFBF is empty */
#define OBP_RF_RESV	0x04			/* means RFBF is all reserved */
#define OBP_SF_RESV	0x08			/* means RFBF is all reserved */
#define OBP_SF_EMPTY	0x10			/* means RFBF is empty */
#define OBP_SF_FULL	0x20			/* means RFBF is full */

/*
 * This is the First Pass Gate Array defines.
 * This is also the definition of the Raleigh Altera cards.
 */
#ifdef NEWCARD
#define FIFO_SIZE	127			/* number of entries in hardware FIFOs */
#define FIFO_STAT	((SRAM) 0x3fd80)	/* addr to get status w/o a reservation */
#define RFBF_STAT	((SRAM) 0x3fc80)	/* get status w/reservation in RFBF */
#define SFBF_STAT	((SRAM) 0x3fd00)	/* get status w/reservation in SFBF */
#define RFBF_DATA	((SRAM) 0x3fc00)	/* addr to write RFBF data */
#define SFBF_DATA	((SRAM) 0x3fc00)	/* addr to read SFBF data */
#define DMA_START	((SRAM) 0x3f800)	/* addr to start DMA transfer */
#endif

/*
 * I don't know what this is, it was in the spec but
 * never became real.
 */
#ifdef NEWCARD2
#define FIFO_SIZE	127			/* number of entries in hardware FIFOs */
#define FIFO_STAT	((SRAM) 0x3ff80)	/* addr to get status w/o a reservation */
#define SFBF_STAT	((SRAM) 0x3fe80)	/* get status w/reservation in SFBF */
#define RFBF_STAT	((SRAM) 0x3fc80)	/* get status w/reservation in RFBF */
#define RFBF_DATA	((SRAM) 0x3fc00)	/* addr to write RFBF data */
#define SFBF_DATA	((SRAM) 0x3fe00)	/* addr to read SFBF data */
#define DMA_START	((SRAM) 0x3ff00)	/* addr to start DMA transfer */
#endif

/*
 * This is the last Austin made Altera card (#22, #23)
 */
#ifdef OLDCARD
#define FIFO_SIZE	127			/* number of entries in hardware FIFOs */
#define FIFO_STAT	((SRAM) 0x3ff00)	/* addr to get status w/o a reservation */
#define SFBF_STAT	((SRAM) 0x3fe00)	/* get status w/reservation in SFBF */
#define RFBF_STAT	((SRAM) 0x3fd00)	/* get status w/reservation in RFBF */
#define RFBF_DATA	((SRAM) 0x3fc00)	/* addr to write RFBF data */
#define SFBF_DATA	((SRAM) 0x3fc00)	/* addr to read SFBF data */
#define DMA_START	((SRAM) 0x3f800)	/* addr to start DMA transfer */
#endif

