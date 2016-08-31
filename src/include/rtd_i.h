/*----------------------------------------------------------------------------*/
/**
  @file     rtd_i.h
  @author   N. Devillard
  @date     Jul 2001
  @version  $Revision: 1.5 $
  @brief    Interfaces to RTD.

   This module was freely adapted from the rtdRemote.c module written by Allan 
   Brighton. Most differences are related to error handling, to make it 
   compliant with eclipse.
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: rtd_i.h,v 1.5 2003/02/21 14:30:47 yjung Exp $
	$Author: yjung $
	$Date: 2003/02/21 14:30:47 $
	$Revision: 1.5 $
*/

#ifndef _RTD_I_H_
#define _RTD_I_H_

/*-----------------------------------------------------------------------------
   							Function prototypes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Open a connection to a running RTD.
  @param    pid     PID of the RTD session to connect to.
  @param    host    Name of the host on which RTD is running.
  @param    port    Port number to talk to.
  @return   int 0 for success, 1 for error.

  This function opens a connection to a currently running RTD display. The pid,i
  hostname and port number, if not specified (set to 0) are read from the file 
  $HOME/.rtd-remote, which is created by RTD on startup (see 
  rtdimage/src/RtdRemote.C in the RTD sources).
  The returned value is 0 for success, 1 for error.
 */
/*----------------------------------------------------------------------------*/
int rtd_connect(int pid, char* host, int port) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Disconnect from remote RTD.
  @return   void

  This function disconnects the current process from the running RTD session 
  it has been connected to, using rtd_connect().
 */
/*----------------------------------------------------------------------------*/
void rtd_disconnect(void) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Evaluate an RTD command and return the command status.
  @param    cmd     Command to send to the remote RTD session.
  @return   int status of the command.

  Evaluate the given rtdimage subcommand in the remote rtd application and 
  return the status of the command.
  The command syntax is the same as for the "rtdimage" widget (image type),
  except that the instance name is missing.

  Example:
    
  @code
    char* result;
    int status = rtdRemoteCmd("wcscenter", &result);
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
        char    **  result) ;

#endif
