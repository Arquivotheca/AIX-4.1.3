/* @(#)99       1.1  src/bos/kernext/disp/ped/pedmacro/hw_iodefs.h, pedmacro, bos411, 9428A410j 3/24/94 13:59:40 */

/*
 * COMPONENT_NAME: PEDMACRO
 *
 * FUNCTIONS: I/O macros which retry after being interrupted by signals
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifndef MID_DD
extern  int     errno;                          /* System error number       */
extern  char    *sys_errlist[];                 /* Array of system errors    */
#endif /* not MID_DD */

/****************************************************************************/
/*                          I/O Macro Definitions                           */
/****************************************************************************/

#ifdef MID_DD

#define   INPUT_ERMSG(cmd)                                                   \
	  printf("'%s' error on input file\n",cmd);

#define   OUTPUT_ERMSG(cmd)                                                  \
	  printf("'%s' error on output file\n",cmd);

#define   FILEIO_ERMSG(cmd)                                                  \
	  printf("'%s' error on file\n",cmd);

#define   NAMEIO_ERMSG(cmd,filename)                                         \
	  printf("'%s' error on file '%s'\n",cmd);

#elif MID_TED

#include  "errdefs.h"

#define   INPUT_ERMSG(cmd)                                                   \
	  app_error(INPUT_ERROR,cmd,errno,sys_errlist[errno]);

#define   OUTPUT_ERMSG(cmd)                                                  \
	  app_error(OUTPUT_ERROR,cmd,errno,sys_errlist[errno]);

#define   FILEIO_ERMSG(cmd)                                                  \
	  app_error(FILEIO_ERROR,cmd,errno,sys_errlist[errno]);

#define   NAMEIO_ERMSG(cmd,filename)                                         \
	  app_error(NAMEIO_ERROR,cmd,filename,errno,sys_errlist[errno]);

#else /* not MID_TED and not MID_DD */

#define   INPUT_ERMSG(cmd)                                                   \
	  fprintf(stderr,"'%s' error on input file\nErrno %d: %s",           \
	    cmd,errno,sys_errlist[errno]);

#define   OUTPUT_ERMSG(cmd)                                                  \
	  fprintf(stderr,"'%s' error on output file\nErrno %d: %s",          \
	    cmd,errno,sys_errlist[errno]);

#define   FILEIO_ERMSG(cmd)                                                  \
	  fprintf(stderr,"'%s' error on file\nErrno %d: %s",                 \
	    cmd,errno,sys_errlist[errno]);

#define   NAMEIO_ERMSG(cmd,filename)                                         \
	  fprintf(stderr,"'%s' error on file '%s'\nErrno %d: %s",            \
	    cmd,filename,errno,sys_errlist[errno]);

#endif /* not MID_TED and not MID_DD */


#define   OPEN_ERMSG(filename,flags,fd)                                      \
{                                                                            \
  do                                                                         \
  {                                                                          \
    errno = 0;                                                               \
    fd = open(filename,flags);                                               \
  }                                                                          \
  while ((fd == -1) && (errno == EINTR));                                    \
  if (fd == -1)                                                              \
    NAMEIO_ERMSG("open",filename)                                            \
}

#define   WRITE(fd,buffer,nbytes)                                            \
{                                                                            \
  int     _rc;                                                               \
  do                                                                         \
  {                                                                          \
    errno = 0;                                                               \
    _rc = write(fd,buffer,nbytes);                                           \
  }                                                                          \
  while ((_rc == -1) && (errno == EINTR));                                   \
  if (_rc == -1)                                                             \
    OUTPUT_ERMSG("write")                                                    \
}

#define   IOCTL_ERMSG(fd,op,arg)                                             \
{                                                                            \
  int     _rc;                                                               \
  do                                                                         \
  {                                                                          \
    errno = 0;                                                               \
    _rc = ioctl(fd,op,arg);                                                  \
  }                                                                          \
  while ((_rc == -1) && (errno == EINTR));                                   \
  if (_rc == -1)                                                             \
    FILEIO_ERMSG("ioctl")                                                    \
}

#define   READ_KEY_ERMSG(buffer,nbytes,intflag)                              \
{                                                                            \
  int     _rc;                                                               \
  struct  sigaction     oldint;                                              \
  struct  sigaction     oldquit;                                             \
  struct  sigaction     newint;                                              \
  struct  sigaction     newquit;                                             \
	                                                                     \
  sigaction(SIGINT,NULL,&oldint);                                            \
  sigaction(SIGQUIT,NULL,&oldquit);                                          \
  newint = oldint;                                                           \
  newquit = oldquit;                                                         \
  newint.sa_flags &= ~SA_RESTART;                                            \
  newquit.sa_flags &= ~SA_RESTART;                                           \
  sigaction(SIGINT,&newint,NULL);                                            \
  sigaction(SIGQUIT,&newquit,NULL);                                          \
  do                                                                         \
  {                                                                          \
    errno = 0;                                                               \
    _rc = read(0,buffer,nbytes);                                             \
  } while ((_rc == -1) && (errno == EINTR) && (intflag == 0));               \
  sigaction(SIGINT,&oldint,NULL);                                            \
  sigaction(SIGQUIT,&oldquit,NULL);                                          \
  if (_rc == -1 && intflag == 0)                                             \
    INPUT_ERMSG("read")                                                      \
}

#define   FOPEN_NOMSG(filename,type,stream)                                  \
{                                                                            \
  do                                                                         \
  {                                                                          \
    errno = 0;                                                               \
    stream = fopen(filename,type);                                           \
  }                                                                          \
  while ((stream == NULL) && (errno == EINTR));                              \
}
	
#define   FOPEN_ERMSG(filename,type,stream)                                  \
{                                                                            \
  do                                                                         \
  {                                                                          \
    errno = 0;                                                               \
    stream = fopen(filename,type);                                           \
  }                                                                          \
  while ((stream == NULL) && (errno == EINTR));                              \
  if (stream == NULL)                                                        \
    NAMEIO_ERMSG("open",filename)                                            \
}
	
#define   FWRITE_NOMSG(buffer,size,count,stream,rc)                          \
{                                                                            \
  do                                                                         \
  {                                                                          \
    errno = 0;                                                               \
    rc = fwrite(buffer,size,count,stream);                                   \
  }                                                                          \
  while ((rc != count) && (errno == EINTR));                                 \
}
	
#define   FWRITE_ERMSG(buffer,size,count,stream)                             \
{                                                                            \
  int     _rc;                                                               \
  do                                                                         \
  {                                                                          \
    errno = 0;                                                               \
    _rc = fwrite(buffer,size,count,stream);                                  \
  }                                                                          \
  while ((_rc != count) && (errno == EINTR));                                \
  if (_rc != count)                                                          \
    OUTPUT_ERMSG("fwrite")                                                   \
}
	
#define   FREAD_NOMSG(buffer,size,count,stream,rc)                           \
{                                                                            \
  do                                                                         \
  {                                                                          \
    errno = 0;                                                               \
    rc = fread(buffer,size,count,stream);                                    \
  }                                                                          \
  while ((rc != count) && (errno == EINTR));                                 \
}
	
#define   FREAD_ERMSG(buffer,size,count,stream)                              \
{                                                                            \
  int     _rc;                                                               \
  do                                                                         \
  {                                                                          \
    errno = 0;                                                               \
    _rc = fread(buffer,size,count,stream);                                   \
  }                                                                          \
  while ((_rc != count) && (errno == EINTR));                                \
  if (_rc != count)                                                          \
    INPUT_ERMSG("fread")                                                     \
}
	
#define   FGETS_NOMSG(buffer,size,stream,rc)                                 \
{                                                                            \
  do                                                                         \
  {                                                                          \
    errno = 0;                                                               \
    rc = fgets(buffer,size,stream);                                          \
  }                                                                          \
  while ((rc != NULL) && (errno == EINTR));                                  \
}
	
#define   FGETS_ERMSG(buffer,size,stream)                                    \
{                                                                            \
  int     _rc;                                                               \
  do                                                                         \
  {                                                                          \
    errno = 0;                                                               \
    _rc = fgets(buffer,size,stream);                                         \
  }                                                                          \
  while ((_rc != NULL) && (errno == EINTR));                                 \
  if (_rc == NULL)                                                           \
    INPUT_ERMSG("fgets")                                                     \
}
	
#define   FPRINTF_NOMSG(parms,rc)                                            \
{                                                                            \
  do                                                                         \
  {                                                                          \
    errno = 0;                                                               \
    rc = fprintf parms ;                                                     \
  }                                                                          \
  while ((rc < 0) && (errno == EINTR));                                      \
}
	
#define   FPRINTF_ERMSG(parms)                                               \
{                                                                            \
  int     _rc;                                                               \
  do                                                                         \
  {                                                                          \
    errno = 0;                                                               \
    _rc = fprintf parms;                                                     \
  }                                                                          \
  while ((_rc < 0) && (errno == EINTR));                                     \
  if (_rc < 0)                                                               \
    OUTPUT_ERMSG("fprintf")                                                  \
}

#define   PRINTF_NOMSG(parms,rc)                                             \
{                                                                            \
  do                                                                         \
  {                                                                          \
    errno = 0;                                                               \
    rc = printf parms ;                                                      \
  }                                                                          \
  while ((rc < 0) && (errno == EINTR));                                      \
}
	
#define   PRINTF_ERMSG(parms)                                                \
{                                                                            \
  int     _rc;                                                               \
  do                                                                         \
  {                                                                          \
    errno = 0;                                                               \
    _rc = printf parms;                                                      \
  }                                                                          \
  while ((_rc < 0) && (errno == EINTR));                                     \
  if (_rc < 0)                                                               \
    OUTPUT_ERMSG("printf")                                                   \
}
	
#define   FSEEK_NOMSG(stream,offset,whence,rc)                               \
{                                                                            \
  do                                                                         \
  {                                                                          \
    errno = 0;                                                               \
    rc = fseek(stream,offset,whence);                                        \
  }                                                                          \
  while ((rc != 0) && (errno == EINTR));                                     \
}

#define   FSEEKI_ERMSG(stream,offset,whence)                                 \
{                                                                            \
  int     _rc;                                                               \
  do                                                                         \
  {                                                                          \
    errno = 0;                                                               \
    _rc = fseek(stream,offset,whence);                                       \
  }                                                                          \
  while ((_rc != 0) && (errno == EINTR));                                    \
  if (_rc != 0)                                                              \
    INPUT_ERMSG("fseek")                                                     \
}
	
#define   FSEEKO_ERMSG(stream,offset,whence)                                 \
{                                                                            \
  int     _rc;                                                               \
  do                                                                         \
  {                                                                          \
    errno = 0;                                                               \
    _rc = fseek(stream,offset,whence);                                       \
  }                                                                          \
  while ((_rc != 0) && (errno == EINTR));                                    \
  if (_rc != 0)                                                              \
    OUTPUT_ERMSG("fseek")                                                    \
}
	
#define   FTELL_ERMSG(stream,location)                                       \
{                                                                            \
  do                                                                         \
  {                                                                          \
    errno = 0;                                                               \
    location = ftell(stream);                                                \
  }                                                                          \
  while ((location == -1) && (errno == EINTR));                              \
  if (location == -1)                                                        \
    FILEIO_ERMSG("ftell")                                                    \
}
	
#define   REWIND_ERMSG(stream)                                               \
{                                                                            \
  do                                                                         \
  {                                                                          \
    errno = 0;                                                               \
    rewind(stream);                                                          \
  }                                                                          \
  while (errno == EINTR);                                                    \
  if (errno != 0)                                                            \
    FILEIO_ERMSG("rewind")                                                   \
}
	
#define   FSYNC_NOMSG(stream)                                                \
{                                                                            \
  int     _rc;                                                               \
  do                                                                         \
  {                                                                          \
    errno = 0;                                                               \
    _rc = fsync(fileno(stream));                                             \
  }                                                                          \
  while ((_rc == -1) && (errno == EINTR));                                   \
}
	
#define   FSYNC_ERMSG(stream)                                                \
{                                                                            \
  int     _rc;                                                               \
  do                                                                         \
  {                                                                          \
    errno = 0;                                                               \
    _rc = fsync(fileno(stream));                                             \
  }                                                                          \
  while ((_rc == -1) && (errno == EINTR));                                   \
  if (_rc == -1)                                                             \
    OUTPUT_ERMSG("fsync")                                                    \
}

#define   FCLOSEO_NOMSG(stream)                                              \
{                                                                            \
  int     _rc;                                                               \
  if (stream != NULL)                                                        \
  {                                                                          \
    FSYNC_ERMSG(stream)                                                      \
    do                                                                       \
    {                                                                        \
      errno = 0;                                                             \
      _rc = fclose(stream);                                                  \
    }                                                                        \
    while ((_rc != 0) && (errno == EINTR));                                  \
    stream = NULL;                                                           \
  }                                                                          \
}

#define   FCLOSEO_ERMSG(stream)                                              \
{                                                                            \
  int     _rc;                                                               \
  if (stream != NULL)                                                        \
  {                                                                          \
    FSYNC_ERMSG(stream)                                                      \
    do                                                                       \
    {                                                                        \
      errno = 0;                                                             \
      _rc = fclose(stream);                                                  \
    }                                                                        \
    while ((_rc != 0) && (errno == EINTR));                                  \
    if (_rc != 0)                                                            \
      OUTPUT_ERMSG("fclose")                                                 \
    stream = NULL;                                                           \
  }                                                                          \
}

#define   FCLOSEI_ERMSG(stream)                                              \
{                                                                            \
  int     _rc;                                                               \
  if (stream != NULL)                                                        \
  {                                                                          \
    do                                                                       \
    {                                                                        \
      errno = 0;                                                             \
      _rc = fclose(stream);                                                  \
    }                                                                        \
    while ((_rc != 0) && (errno == EINTR));                                  \
    if (_rc != 0)                                                            \
      INPUT_ERMSG("fclose")                                                  \
    stream = NULL;                                                           \
  }                                                                          \
}
