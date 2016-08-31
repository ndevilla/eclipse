/*----------------------------------------------------------------------------*/
/**
  @file		rtd_i.c
  @author	N. Devillard
  @date		Jul 2001
  @version	$Revision: 2.5 $
  @brief	Interfaces to RTD.

   This module was freely adapted from the rtdRemote.c module written by Allan 
   Brighton. Most differences are related to error handling, to make it 
   compliant with eclipse.
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: rtd_i.c,v 2.5 2003/02/21 14:30:48 yjung Exp $
	$Author: yjung $
	$Date: 2003/02/21 14:30:48 $
	$Revision: 2.5 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <memory.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <signal.h>
#include "rtd_i.h"
#include "static_sz.h"
#include "comm.h"

/*-----------------------------------------------------------------------------
							Private type
 -----------------------------------------------------------------------------*/

/* Private struct allocated to manage the client's connection */
static struct {
    /* socket connection with display */
    int  socket;		
    /* pid of display on host */
    int  pid;			
    /* hostname where display is running */
    char host[64];		
    /* port number to use on host */
    int  port;			
} rtd_info ;

/*-----------------------------------------------------------------------------
							Functions prototypes
 -----------------------------------------------------------------------------*/

static int read_socket(int fd, char* ptr, int nbytes) ;
static int write_socket(int fd, char* ptr, unsigned long nbytes) ;
static int read_line(int fd, char* ptr, int maxlen) ;
static int write_line(int fd, char* ptr) ;
static int get_rtd_info(void) ;
static int rtd_get_result(int, char **) ;
static int rtd_send_engine(char * cmd) ;

/*-----------------------------------------------------------------------------
							Functions code
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	Open a connection to a running RTD.
  @param	pid		PID of the RTD session to connect to.
  @param	host	Name of the host on which RTD is running.
  @param	port	Port number to talk to.
  @return	int 0 for success, 1 for error.

  This function opens a connection to a currently running RTD display. The pid,i
  hostname and port number, if not specified (set to 0) are read from the file 
  $HOME/.rtd-remote, which is created by RTD on startup (see 
  rtdimage/src/RtdRemote.C in the RTD sources).
  The returned value is 0 for success, 1 for error.
 */
/*----------------------------------------------------------------------------*/
int rtd_connect(int pid, char* host, int port)
{
    /* pointer to host info */
    struct  hostent *   hp ;		
    /* for peer socket address */
    struct  sockaddr_in addr ;	
	int	ret ;

    if (pid && host && port) {
		rtd_info.pid = pid;
		strncpy(rtd_info.host, host, sizeof(rtd_info.host));
		rtd_info.port = port;
    } else if (get_rtd_info() != 0) {
        /* get pid, hostname, port from ~/.rtd-remote file */
		return 1;
	}

    /* Clear out address */
    memset ((char *)&addr, 0, sizeof(struct sockaddr_in));

    /* Set up the peer address to which we will connect. */
    addr.sin_family = AF_INET;

    /* Get the host information for the rtd display */
    hp = gethostbyname (rtd_info.host) ;
    if (hp == NULL) {
		if (debug_active()) {
			perror("gethostbyname") ;
			e_error("cannot get hostname: aborting connection to rtd");
		}
		return 1 ;
	}

    addr.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
    addr.sin_port = htons(rtd_info.port) ;
    
    /* Create the socket. */
    rtd_info.socket = socket(AF_INET, SOCK_STREAM, 0) ;
    if (rtd_info.socket == -1)  {
		if (debug_active()) {
			perror("socket") ;
			e_error("creating socket: aborting connection to rtd") ;
		}
		return 1 ;
	}

    /* Try to connect to the remote Rtd display */
    ret = connect(rtd_info.socket, (struct sockaddr *)&addr,
				  sizeof(struct sockaddr_in)) ;
	if (ret==-1) {
		if (debug_active()) {
			perror("connect");
			e_error("connecting to rtd");
			return 1 ;
		}
	}
    return 0;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Disconnect from remote RTD.
  @return	void

  This function disconnects the current process from the running RTD session 
  it has been connected to, using rtd_connect().
 */
/*----------------------------------------------------------------------------*/
void rtd_disconnect(void)
{
    if (rtd_info.socket != -1) {
		close(rtd_info.socket) ;
		rtd_info.socket = -1 ;
    }
    return ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Evaluate an RTD command and return the command status.
  @param	cmd		Command to send to the remote RTD session.
  @return	int status of the command.

  Evaluate the given rtdimage subcommand in the remote rtd application and 
  return the status of the command.
  The command syntax is the same as for the "rtdimage" widget (image type),
  except that the instance name is missing.

  Example:
    
  @code
	char* result;
	int status = rtd_send("wcscenter", &result);
	if (status == 0) {
		if (sscanf(result, ...) ...) {...}
		...
	}
  @endcode
 
  On success, "result" points to a char buffer containing the result of the 
  command. The buffer is internal and should not be freed and will be 
  overwritten in the next call to this routine.
 
  If the command could not be sent, result is set to a NULL pointer and an 
  error status (1) is returned.
 */
/*----------------------------------------------------------------------------*/
int rtd_send(
        char    *   cmd, 
        char    **  result)
{
    if (rtd_info.socket == -1) {
		e_error("no connection to the image display");
		return 1 ;
	}
    if (rtd_send_engine(cmd) != 0) return 1 ;
    return rtd_get_result(rtd_info.socket, result) ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Send a command to a remote RTD display.
  @param	cmd		Command to send to RTD display.
  @return	int 0 if Ok, 1 otherwise.

  Write the command to the RTD socket and return 0 if Ok, 1 otherwise.
  "cmd" should not contain a newline, it will be added here.
 */
/*----------------------------------------------------------------------------*/
static int rtd_send_engine(char * cmd) 
{
    if (write_line(rtd_info.socket, cmd) <= 0) {
		e_error("error sending command to RTD") ;
		return -1 ;
	}
    return 0;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Read answer from the last command sent to RTD.
  @param	sock	Current RTD socket.
  @param	result	Pointer to modify to store result string.
  @return	int RTD status, -1 if error occurred.

  Read the socket with the answer from the last command sent to the remote RTD 
  display and return the command's status. The command's result is returned in 
  the 'result' argument, which points to local or static storage.

  The format of the message read from the socket is:

  @verbatim
  status length\n
  msg[length]\n
  @endverbatim

  where status is 0 (Ok) or 1 and length is the length of the result.
 */
/*----------------------------------------------------------------------------*/
static int rtd_get_result(
        int         sock, 
        char    **  result)
{
    static char     buf[ASCIILINESZ]; 
    static char *   rbuf ;	
    static int      rbufsize ;
    int             length ;	
    int             status ;			

    /* Initialize */
    rbuf = buf ; 
    rbufsize = sizeof(buf) - 1 ;
    buf[0] = '\0';		

    /* Test input */
    if (result) *result = rbuf;

    if (read_line(sock, buf, sizeof(buf)) <= 0) {
		e_error("reading result status from rtdimage");
		return -1 ;
	}
    if (sscanf(buf, "%d %d", &status, &length) != 2) {
		e_error("unknown result from rtdimage");
		return -1 ;
	}
    if (length == 0) return status;	
    if (length < 0) {
		e_error("bad length received from display application");
		return -1 ;
	}
    if (length >= rbufsize) {	
		if (rbufsize != sizeof(buf)) free(rbuf);
		rbuf = (char*)malloc(rbufsize=length+10);
	}
	if (result) *result = rbuf;
    if (read_socket(sock, rbuf, length) != length) {
		e_error("reading result from rtdimage");
		return -1 ;
	}
    rbuf[length] = '\0';
    return status;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Read "n" bytes from a descriptor.
  @param    fd      file descriptor
  @param    ptr     string 
  @param    nbytes  nb of bytes to read
  @return   nb of bytes read 
  To be used in place of read() when fd is a stream socket.
  Comes from the book : "UNIX Network Programming" by W. Richard Stevens.
 */
/*----------------------------------------------------------------------------*/
static int read_socket(
        int         fd, 
        char    *   ptr, 
        int         nbytes)
{
    int	    nleft, 
            nread ;
    
    nleft = nbytes ;
    while (nleft > 0) {
        nread = read(fd, ptr, nleft) ;
        if (nread < 0) return nread ;
        else if (nread == 0) break ;
        nleft -= nread ;
        ptr   += nread ;
    }
    return(nbytes - nleft) ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Write "n" bytes to a descriptor.
  @param    fd      file descriptor
  @param    ptr     string 
  @param    nbytes  nb of bytes to read
  @return   nb of bytes written 
  To be used in place of write() when fd is a stream socket.
  Comes from the book : "UNIX Network Programming" by W. Richard Stevens.
 */
/*----------------------------------------------------------------------------*/
static int write_socket(
        int                 fd, 
        char            *   ptr, 
        unsigned long       nbytes)
{
    int nleft, 
        nwritten ;

    nleft = nbytes ;
    while (nleft > 0) {
	    nwritten = write(fd, ptr, nleft) ;
	    if (nwritten <= 0) return nwritten ;	
        nleft -= nwritten ;
        ptr   += nwritten ;
    }
    return(nbytes - nleft) ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Read the line one byte at a time, looking for the newline
  @param    fd      file descriptor
  @param    ptr     string 
  @param    maxlen  maximum length
  @return   the nb of chars up to the null or < 0 upon errors
  We store the newline in the buffer, then follow it with a null (the same as
  fgets(3)).  Not very efficient but usefull for sockets.
  Comes from the book : "UNIX Network Programming" by W. Richard Stevens.
 */
/*----------------------------------------------------------------------------*/
static int read_line(
        int         fd, 
        char    *   ptr, 
        int         maxlen)
{
    int	    n, 
            rc ;
    char	c ;

    for (n=1 ; n<maxlen ; n++) {
        if ((rc = read(fd, &c, 1)) == 1) {
            *ptr++ = c ;
            if (c == '\n') break ;
        } else if (rc == 0) {
            if (n == 1) return 0 ;	
            else break ;		
        } else return -1 ;	
    }
    *ptr = 0 ;
    return n ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    write the given buffer to the given file followed by a newline
  @param    fd      file descriptor
  @param    ptr     string 
  @return   -1 in error case
  Comes from the book : "UNIX Network Programming" by W. Richard Stevens.
 */
/*----------------------------------------------------------------------------*/
static int write_line(int fd, char* ptr)
{
    return write_socket(fd, ptr, strlen(ptr)) + write_socket(fd, "\n", 1) ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Read the ~/.rtd-remote file
  @return   -1 in error case
  Read the ~/.rtd-remote file to get the pid, hostname and port number of the
  RTD, if it is running (check that it is running...)
 */
/*----------------------------------------------------------------------------*/
static int get_rtd_info(void)
{
    char        filename[FILENAMESZ] ;
    char    *   home ;
    FILE    *   f ;
    char        hostname[64] ;
	char        ret ;

    /* Initialize */
    home = getenv("HOME") ;
    
	/* Get values from rtd status file */
    sprintf(filename, "%s/.rtd-remote", (home ? home : "/tmp")) ; 
    if ((f=fopen(filename, "r")) == NULL) {
		e_error("cannot open status file: %s, is rtd running?", filename);
		return -1 ;
	}
    ret = fscanf(f, "%d %s %d",
				&rtd_info.pid,
				rtd_info.host,
				&rtd_info.port) ;
	fclose(f) ;
	if (ret != 3) {
		e_error("in Rtd status file: %s", filename) ;
		return -1 ;
	}

	/* See if process is still alive */
    if (kill(rtd_info.pid, 0) != 0) {
		e_error("rtd not running on this host?") ;
		return -1 ;
	}
	/* See if host name matches */
	if (gethostname(hostname, sizeof(hostname))==-1) {
		perror("gethostname") ;
		e_error("rtd not running on this host?") ; 
		return -1 ;
	}
	if (strcmp(hostname, rtd_info.host)) {
		e_error("rtd not running on this host?") ;
		return -1 ;
	}
    return 0 ;
}
