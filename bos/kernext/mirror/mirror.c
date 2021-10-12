static char sccsid[] = "@(#)15  1.3  src/bos/kernext/mirror/mirror.c, cmdmirror, bos412, 9443A412c 10/20/94 03:18:09";
/*
 * COMPONENT_NAME: CMDMIRROR: Console mirroring
 * 
 * FUNCTIONS: mirror_config, mirror_open, mirror_close, mirror_wput, 
 *     	      mirror_wsrv, mirror_rput, mirror_rsrv, flush_q
 * 
 * ORIGINS: 83 
 * 
 */
/*
 *  LEVEL 1, 5 Years Bull Confidential Information
 */


/************************************************************************
	Include files
************************************************************************/
#include <sys/types.h>
#include <sys/stropts.h>	/*	For FLUSHx, strioctl...		*/
#include <sys/stream.h>		/*	For module_info, qinit ..	*/
#include <sys/strconf.h>	/*	For SQLVL_MODULE		*/
#include <sys/errno.h>
#include <sys/device.h>
#include <sys/str_tty.h>	/*	For LPWRITE_ACK			*/
#include <sys/lpio.h>		/*	For LPWRITE_REQ			*/
#include <ldtty.h>		/*	For ldtty...			*/
#include <mirror.h>

/************************************************************************
	Function declarations
************************************************************************/
int mirror_config();
static int mirror_open();
static int mirror_close();
static int mirror_rput();
static int mirror_wput();
static int mirror_rsrv();
static int mirror_wsrv();
static int wsrv_transparent();
static int wsrv_echo();
static mblk_t *create_message();
static int flush_q();

/************************************************************************
	Global variables
************************************************************************/
int Echo;
/*
extern int *db_Echo;
*/
struct mir {
		queue_t *cust_wqptr;  /*  write queue of the module on S1*/ 
		queue_t *remot_wqptr;   /*  write queue of the module of S2 */ 
		unsigned int write_status_cust; /* Write status on customer line */
		unsigned int write_status_remot;/* Write status on remote line */
	   } mir;


#define IS_CUSTOM(n) ((Echo == ECHO_ON) && (n == mir.cust_wqptr))
#define IS_REMOTE(n) ((Echo == ECHO_ON) && (n == mir.remot_wqptr))


/************************************************************************
	Module declarations
************************************************************************/
#define MODULE_ID 2030
#define MODULE_NAME "mirror"
#define M_HIWAT 4096
#define M_LOWAT 256 

static struct module_info minfo = {
			MODULE_ID, MODULE_NAME, 0, INFPSZ, M_HIWAT, M_LOWAT };

static struct qinit rinit = {
	mirror_rput, mirror_rsrv, mirror_open, mirror_close, 0, &minfo, 0 };

static struct qinit winit = {
	mirror_wput, mirror_wsrv, 0, 0, 0, &minfo, 0 };

struct streamtab mirrorinfo = { &rinit, &winit, NULL, NULL };

void mir_trace ()
{
	int i;

	i++;
}

/* mirror module : this module is used when remote maintenance is done.
 * This module is managed by the daemon : mirrord .
 * This module can be in two modes depending on the variable
 * Echo. The modes are Transparent and Echo. In transparent mode the
 * mirror module is transparent and the lines S1 and S2 behave as normal
 * terminal lines. In Echo mode the module activates mirroring between
 * the two lines S1 and S2. The mirroring is activated by sending ioctls
 * to the mirror modules on lines S1 and S2. Ioctls can be directed to the S2 
 * line to configure the control attributes of the line S2. Writes directed
 * to the S1 line will be put on both the S1 and S2 lines. Reads 
 * directed to S1 will take input either from S1 or S2.
 */

/**********************************************************************/
/* NAME:  mirror_config                                               */
/* FUNCTION: configure the module mirror in the stream framework      */
/* RETURN VALUE: EINVAL if bad cmd                                    */
/*               error if str_install failed                          */
/*		 0 else                                               */
/**********************************************************************/

int mirror_config(cmd, uiop)
	int cmd;
	struct uio *uiop;
{

	int rc;
	static strconf_t conf = {
			"mirror", &mirrorinfo, STR_NEW_OPEN };
	conf.sc_sqlevel = SQLVL_MODULE;

	switch(cmd) {

			case CFG_INIT:
				rc = str_install(STR_LOAD_MOD, &conf);
				break;
			case CFG_TERM:
				rc = str_install(STR_UNLOAD_MOD, &conf);
				break;
			default:
				rc = EINVAL;
				break;
			}
	return(rc);
}

/**********************************************************************/
/* NAME:  mirror_open                                                 */
/* FUNCTION: Initialize the flags                                     */
/* RETURN VALUE: 0                                                    */
/**********************************************************************/

static int mirror_open(q, dev, flag, sflag, credp)
	queue_t *q;
	dev_t   *dev;
	int	flag;
	int	sflag;
	cred_t	*credp;
{

/*
	db_Echo = &Echo;
*/
	mir.write_status_remot &= ~MIR_WRITE_IN_PROCESS;
	mir.write_status_cust &= ~MIR_WRITE_IN_PROCESS;
	return(0);
}


/**********************************************************************/
/* NAME:  mirror_close                                                */
/* FUNCTION: Flush the remote module write queue                      */
/* RETURN VALUE: 0                                                    */
/**********************************************************************/

static int mirror_close(q, flag, credp)
	queue_t *q;
	int  	flag;
	cred_t	*credp;
{
	if(Echo != ECHO_OFF){
		Echo = ECHO_OFF;
		flush_q(mir.remot_wqptr);
	}
	return(0);
}



/**********************************************************************/
/* NAME:  mirror_wput                                                 */
/* FUNCTION: write put routine                                        */
/* RETURN VALUE: none                                                 */
/**********************************************************************/

static int mirror_wput(q, msg)
	queue_t *q;
	mblk_t 	*msg;

{

	struct iocblk *iocp;
	mblk_t 	*msgcpy;
	int buff;
	mblk_t	*msg_cre;


	switch(msg->b_datap->db_type) {
		case M_FLUSH:{
			if (*msg->b_rptr & FLUSHW){
				if (Echo == ECHO_OFF) {
					flushq(q, FLUSHDATA);
				}
				else{
					flushq(mir.remot_wqptr, FLUSHDATA);
					flushq(mir.cust_wqptr, FLUSHDATA);
					getq(mir.remot_wqptr);
					getq(mir.cust_wqptr);
					mir.write_status_remot &= ~MIR_WRITE_IN_PROCESS;
					mir.write_status_cust &= ~MIR_WRITE_IN_PROCESS;
				}
			}
		    	putnext(q, msg);
			break;
		}
		case M_STARTI:
		case M_STOPI:
		case M_START:
		case M_STOP:
			putnext(q, msg);
			break;
		case M_IOCTL:
			iocp = (struct iocblk *)msg->b_rptr;
			switch(iocp->ioc_cmd) {
				case MIR_PUT_WQ1:
					mir.cust_wqptr = q;
					msg->b_datap->db_type = M_IOCACK;	
					iocp->ioc_count = 0;
					qreply(q, msg);
					break;
				case MIR_PUT_WQ2:
					mir.remot_wqptr = q;
					msg->b_datap->db_type = M_IOCACK;	
					iocp->ioc_count = 0;
					qreply(q, msg);
					break;
				case MIR_ON_ECHO:
					Echo = ECHO_ON;
					msg->b_datap->db_type = M_IOCACK;	
					iocp->ioc_count = 0;
					qreply(q, msg);
					break;
				case MIR_OFF_ECHO:
					Echo = ECHO_OFF;
					flush_q(mir.remot_wqptr);
					msg->b_datap->db_type = M_IOCACK;	
					iocp->ioc_count = 0;
					qreply(q, msg);
					break;
				default:
				/*
					All other ioctls on the remote line
					are discarded.
				*/
					if IS_REMOTE(q) {
						freemsg(msg);
						break;
					}
					putq(q, msg);
					break;
			}
			break;
		default:
			/*
				No other messages are processed  if the line is
				S2(Remote).
			*/
			if IS_REMOTE(q) {
				freemsg(msg);
				break;
			}
			/*
				Line is S1 and high priority message
			*/
			if (msg->b_datap->db_type & QPCTL) {
				putnext(q, msg);
				break;
			}
			/* 	Echo is OFF so put the message in queue
				for processing by service procedure and come out
			*/
			if ((Echo == ECHO_OFF ) ||
					 (msg->b_datap->db_type != M_DATA))  {
				putq(q, msg);
				break;
			}
			/*
				Echo is ON and the line is S1(Customer console)
				So send a copy of message to remote console
			*/
			msgcpy = copymsg(msg);
			putq(q, msg);
			putq(mir.remot_wqptr, msgcpy);
			break;
	}
}

/**********************************************************************/
/* NAME:  mirror_wsrv                                                 */
/* FUNCTION: write service routine                                    */
/* RETURN VALUE: none                                                 */
/**********************************************************************/
static int mirror_wsrv(q)
	queue_t *q;
{

	mblk_t *msg;

	if (Echo == ECHO_OFF) {
		wsrv_transparent(q);
	}
	else {
		wsrv_echo(q);
	}	

}
			

/**********************************************************************/
/* NAME:  mirror_rput                                                 */
/* FUNCTION: read  put routine                                        */
/* RETURN VALUE: none                                                 */
/**********************************************************************/
static int mirror_rput(q, msg)
        queue_t *q;
        mblk_t  *msg;

{
	mblk_t *msg_cre;

	switch(msg->b_datap->db_type) {
		case M_FLUSH:
                        if (*msg->b_rptr & FLUSHR) 
                                flushq(q, FLUSHDATA);
                        putnext(q, msg);
                        break;
		case M_CTL:{
			struct ldtty *tp = (struct ldtty *)q->q_next->q_ptr;

			if((Echo == ECHO_ON) && 
					(tp->t_state & TS_CARR_ON) &&
					(tp->t_state & TS_ISOPEN) &&
					/* ((tp->t_cflag & CLOCAL) == 0 )&& */
					(*((int *)msg->b_rptr) == cd_off)){
			/*
				ldterm module generates M_HANGUP message
				in response to this M_CTL message. M_HANGUP
				is a high priority message. So before M_HANGUP
				reaches stream head a high priority message 
				has to be sent to the mirror deamon so that
				it switches to transparent mode and pops the mirror
				module from the tty stack.
			*/	
				mir_trace ();
				Echo = ECHO_OFF;
				flush_q(mir.remot_wqptr);
				msg_cre = create_message(strlen(M_HANGUP_S1)+1);
				msg_cre->b_datap->db_type = M_PCPROTO;
				if (WR(q) == mir.cust_wqptr){
					strcpy(msg_cre->b_rptr, M_HANGUP_S1);
				}
				else{
					strcpy(msg_cre->b_rptr, M_HANGUP_S2);
				}
				msg_cre->b_wptr = msg_cre->b_rptr + strlen(M_HANGUP_S1)+1;
				putnext(RD(mir.remot_wqptr), msg_cre);
				if (WR(q) == mir.remot_wqptr){
					freemsg (msg);
					break;
				}
			}
			/*
				Send the message upstream for processing by ldterm 
				module
			*/
			putq(q, msg);
			break;	
		}
			
		case M_PCPROTO:{
			unsigned int *write_status;

			if IS_REMOTE(WR(q)) {
				write_status = &mir.write_status_remot;
			}
			else {
				write_status = &mir.write_status_cust;
			}

			if(*(int *)msg->b_rptr != LPWRITE_ACK){
				putnext(q, msg);
				break;
			}
			if(*write_status & MIR_WRITE_IN_PROCESS){
				*write_status &= ~MIR_WRITE_IN_PROCESS;
			}
			if((qsize(mir.cust_wqptr) > NULL) ||(qsize(mir.remot_wqptr)> NULL)){
			/*
				Always customer should get more info
				than remote maintenance people
				So keep 1 mesage more on the customer
				console than remote. 
			*/
				if((qsize(mir.cust_wqptr) + 1) > qsize(mir.remot_wqptr))
					qenable(mir.cust_wqptr);
				else{
					qenable(mir.remot_wqptr);
				}
			}
			freemsg(msg);
			break;
		}
		default :
			/*
				Put the message in the customer read queue
			*/
			if IS_REMOTE(WR(q)) {
				putq(RD(mir.cust_wqptr), msg);
				break;
			}

			/*
				Message is on Customer console
			*/
			if (msg->b_datap->db_type & QPCTL) {
                                putnext(q, msg);
			}
			else {
				putq(q, msg);
			}
			break;
	}
}


/**********************************************************************/
/* NAME:  mirror_rsrv                                                 */
/* FUNCTION: read service routine                                     */
/* RETURN VALUE: none                                                 */
/**********************************************************************/

static int mirror_rsrv (q)
	queue_t *q;

{

	mblk_t *msg;

	
	while ((msg = getq(q)) != NULL) {
		if (canput (q->q_next)) {
			putnext(q, msg);
			continue;
		}
		putbq(q, msg);
		break;
	}
}



/**********************************************************************/
/* NAME:  flush_q                                                     */
/* FUNCTION: Passes the messages to rs driver till canput returns 1.  */
/*	     Rest of the messages are flushed.			      */
/* RETURN VALUE: none                                                 */
/**********************************************************************/

static int flush_q (q)
	queue_t *q;

{

	mblk_t *msg;

	while ((msg = getq(q)) != NULL) {
		if (canput (q->q_next)) {
			putnext(q, msg);
			continue;
		}
                else {
                        putbq(q, msg);
			break;
		}	
	}
	flushq(q, FLUSHALL);
}




/*********************************************************************/
/* NAME:  wsrv_transparent                                           */
/* FUNCTION: write service routine when the module runs in	     */
/* transparent mode 						     */
/* RETURN VALUE: none                                                */
/*********************************************************************/

static int wsrv_transparent(q)
	queue_t *q;
{
	mblk_t *msg;

	while((msg = getq(q)) != NULL) {
        	if (canput(q->q_next)) {
                	putnext(q, msg);
                        continue;
                }
                else {
                        putbq(q, msg);
                        break;
                }
          }
}

/****************************************************************************/
/* NAME:  wsrv_echo                                                         */
/* FUNCTION: write service routine when the module runs in echo mode        */
/* RETURN VALUE: none                                                       */
/****************************************************************************/

static int wsrv_echo(q)
	queue_t *q;
{
	mblk_t *msg;
	mblk_t *mproto_mp;
	unsigned int *write_status;

	if IS_REMOTE(q) {
		write_status = &mir.write_status_remot;
	}
	else {
		write_status = &mir.write_status_cust;
	}

	while ((msg = getq(q)) != NULL) {
		if((!canput(q->q_next)) ||
			(msg->b_datap->db_type == M_DATA) && (*write_status & MIR_WRITE_IN_PROCESS)){
			putbq(q, msg);
			break;
		}
		/*
			Synchronize only  M_DATA messages
		*/
		if(msg->b_datap->db_type == M_DATA) {
			mproto_mp = create_message(sizeof(int));
			mproto_mp->b_datap->db_type = M_PROTO;
			*(int *)mproto_mp->b_rptr = LPWRITE_REQ;
			mproto_mp->b_wptr = mproto_mp->b_rptr + sizeof(int);
			linkb(mproto_mp, msg);
			*write_status |= MIR_WRITE_IN_PROCESS;
			putnext(q, mproto_mp);
		} else
                	putnext(q, msg);
	}
}


/****************************************************************************/
/* NAME:  create_message                                                    */
/* FUNCTION: Allocates a message of requested size 		            */
/* RETURN VALUE: Pointer to the allocated message                           */
/****************************************************************************/
mblk_t *create_message(sz_mess) 
	int sz_mess;
{
	mblk_t *new_mess;

	if ((new_mess = allocb(sz_mess, BPRI_MED)) == NULL) {
		bufcall(sz_mess, BPRI_MED, create_message, sz_mess);
	}
	return(new_mess);
}
