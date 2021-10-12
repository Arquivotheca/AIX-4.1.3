static char sccsid[] = "@(#)07	1.1  src/bos/diag/util/udiskenh/udmedit.c, dsaudiskenh, bos411, 9435A411a 8/18/94 13:55:19";
#pragma options nosource
#include "udmhexit.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#pragma options source
/* #define DBG */

/*
 * NAME: main()
 *
 * FUNCTION: This procedure is the entry point for the hex editor.     
 * The first argument is the name of the executable.
 * The last argument contains the shared memory key where the edit 
 * buffer is and the return code from hex_edit is placed.
 * The other arguments to this main program 
 * are passed on to the hex_edit function.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This procedure is fork()ed and exec()ed by the udiskmnt program.
 *
 * (RECOVERY OPERATION:) In the event of a software error that prevents
 *      this routine from functioning, control will be returned to the
 *      diagnostic controller after presenting screens to the user
 *      informing them of the error.
 *
 * RETURNS: The return code is ASL_CANCEL, ASL_EXIT, or ASL_COMMIT.
 */

int
main (int argc, char* argv[]) 
{
    int *rc;
    int shm_id = atoi(argv[9]);
    char *io_buffer = shmat(shm_id, 0, 0);

    if (io_buffer == (char*)-1) {
#ifdef DBG
        fprintf(stderr,"UDMEDIT, shmat failed, errno = %d.\n", errno);
#endif
        return (-1);
    }
    else {    
        rc = (int*)(io_buffer+BUF_SIZE);
        *rc = hex_edit( argv[1],
                        argv[2],
                        argv[3],
                        argv[4],  
                        argv[5],
                        argv[6],
                        argv[7],
                        argv[8],  
                        io_buffer);
    }
#ifdef DBG
 fprintf(stderr,"UDMEDIT, *rc = %d.\n", *rc) ;
#endif
/* I believe a shmdt (shared memory detach) should go here, but it */
/* causes a segmentation fault, so I took it out.                  */
    exit(*rc);
} /* end main */
