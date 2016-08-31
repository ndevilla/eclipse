
/*-------------------------------------------------------------------------*/
/**
  @file		userid.c
  @author	N. Devillard
  @date		Sep 1999
  @version	$Revision: 1.8 $
  @brief	Portable user identification routine.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: userid.c,v 1.8 2002/01/21 10:30:12 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2002/01/21 10:30:12 $
	$Revision: 1.8 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief	Returns the user login name.
  @return	Pointer to statically allocated character string.

  Finds out what the login name of the current user is. The result is
  placed in a static character string inside this module and a pointer
  to the first character in this string is returned. Do not modify or
  free the returned string!

  If the user name cannot be determined, the returned pointer will
  point to a string which first character is a null character.
 */
/*--------------------------------------------------------------------------*/

char * get_login_name(void)
{
    struct passwd * pw ;
    static char name[32];

    pw = getpwuid(getuid());
    if (pw!=NULL) {
        strcpy(name, pw->pw_name);
    } else {
        name[0]=0 ;
    }
    return name ;
}

#ifdef TEST
int main(void)
{
	printf("your user ID is %s\n", get_login_name());
	return 0 ;
}
#endif
/* vim: set ts=4 et sw=4 tw=75 */
