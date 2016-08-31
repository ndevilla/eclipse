
/*===========================================================================
  Copyright (C) 1995 European Southern Observatory (ESO)
 
  This program is free software; you can redistribute it and/or 
  modify it under the terms of the GNU General Public License as 
  published by the Free Software Foundation; either version 2 of 
  the License, or (at your option) any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public 
  License along with this program; if not, write to the Free 
  Software Foundation, Inc., 675 Massachusetss Ave, Cambridge, 
  MA 02139, USA.
 
===========================================================================*/

/* 

 $Id: wdat.c,v 1.2 2001/01/12 14:23:30 ndevilla Exp $
 Revision 2.1b 1998/10/06 10:58:00  flacombe
 correct handling of Os9 devices as directories

 Revision 1.6  1997/04/04 11:41:29  ndevilla
 corrected many potential bugs, moved function declarations to ANSI

 Revision 1.5  1997/02/25 14:01:07  ndevilla
 New Version, complete change to a new scheme
 all modifications by Francois Lacombe
 integrated in the library by ndevilla

 Revision 1.4  1997/02/11 12:34:58  ndevilla
 *** empty log message ***

 Revision 1.3  1996/12/09 13:24:43  ndevilla
 initial release with RCS
 
*/ 
 

/*--------------------------------------------------------------------------

   File name    :   wdat.c
   Author       :   Francois Lacombe
   Created on   :   ?
   Description  :   DAT reader in Adonis format

 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

/*--------------------------------------------------------------------*/

/*
   does your machine support 'mt fsf <n>' command, put MT_FSF to 1,
   otherwise, set it to 0
*/

#define MT_FSF	 1

/*
   is your machine a VMS machine, directories have another syntax.
   VMS should then be set to 1.
*/

#define VMS      0

/*--------------------------------------------------------------------*/
#define VERSION  "WDAT version 2.1b \n"
#define REC_SIZE 1024
#define MAX_LEN	 80
#define NULL_CHAR '\0'
#define OS9_TRAILER 0x0d
/*--------------------------------------------------------------------*/
char device[80];
char tape_record[REC_SIZE];

char filename[80];
char listfile[80];

char selection_mask[80];
char exclusion_mask[80];
char selection_file[80];
char exclusion_file[80];

int path_tape;
int path_out;
int f_query;
int f_rewind;
int f_list;
int f_sel_mask;
int f_sel_file;
int f_exc_mask;
int f_exc_file;
int f_open;
int f_fits;
int f_device;
int f_list_file;
int f_log;
FILE *fp_out;
FILE *fp_list;

/*--------------------------------------------------------------------*/

void 	usage(void) ;
void 	Rewind_Tape(void) ;
int 	Open_Tape(void) ;
void 	Close_Tape(void) ;
int 	Skipp_File(void) ;
void 	Test_Label(void) ;
int 	Read_Record(int option) ;
int 	Is_Log(void) ;
int 	in_list(char *file) ;
int 	To_Copy(void) ;
void 	vms_name(char *name) ;
int 	my_mkdir(char *file) ;
int 	Open_Copy(void) ;
int 	Close_Copy(void) ;
int 	Copy_Text(void) ;
int 	Process_Tape(void) ;

/*--------------------------------------------------------------------*/

void usage(void)
{
  printf("Wdat <options>\n");
  printf("Options : \n");
  printf("-d <device> : selects a device, default is the TAPE environment variable\n");
  printf("-nr         : do not rewind tape before first access\n");  
  printf("-l          : ONLY list tape contents \n");  
  printf("-l= file    : ONLY list tape contents on file\n");  
  printf("-fits       : skipp files when not FITS\n");  
  printf("-log        : automatically translates logfiles (.LOG) from Os9\n");
  printf("-e= file    : read exclusion masks in file\n");  
  printf("-e MASK     : excludes all files matching the mask\n");  
  printf("-s= file    : read selection masks in file\n");  
  printf("-s MASK     : select only files matching the mask\n");  
  printf("-q          : ask before copying\n");  
  exit(0);
}


/*--------------------------------------------------------------------*/
void Rewind_Tape(void)
{
  char command[80];
  
  printf("Rewinding...\n");
  if (VMS)
  {
     sprintf(command,"dismount/nounload %s",device);
     system(command);
     sprintf(command,"mount/for/rec=1024/blo=1024/nowrite %s",device);
     if (system(command) != 1) exit(0);
  }
  else
  {
     sprintf(command,"mt -t %s rewind",device);
     if (system(command)) exit(0);
  }
}

int Open_Tape(void)
{
  f_open = 0;
  
  path_tape = fileno(fopen(device,"r"));

  if (path_tape < 0)
    {
      printf("Can't open device\n");
      return(0);
    }
  
  f_open = 1;
  
  return(1);
}

void Close_Tape(void)
{
  if (f_open) close(path_tape);
  f_open = 0;
}

int Skipp_File(void)
{
int nr;
char command[80];

  if ((MT_FSF) && (!VMS))
    {
      Close_Tape();
      sprintf(command,"mt -t %s fsf 1",device);
      if (system(command)) return(0);
    }
  else
    while((nr = read(path_tape,tape_record,REC_SIZE)) > 0); 

  return(1);
}


/*--------------------------------------------------------------------*/
void Test_Label(void)
{
  char answer[80];
  
  if (!Open_Tape()) exit(0);
  if (!Read_Record(2)) exit(0);

  tape_record[79] = NULL_CHAR;
  printf("Label is : '%s' , do you agree ? [Yes] ",tape_record);

  scanf("%s", answer);
  if (toupper(answer[0]) == 'N')
    {
      Close_Tape();
      exit(0);
    }

  if (!Skipp_File()) exit(0);
  Close_Tape();
}

/*--------------------------------------------------------------------*/
int Read_Record(int option)
{
  int nr;
  
  if ((nr = read(path_tape,tape_record,REC_SIZE)) == REC_SIZE) return(1);

  if (option == 1) printf("READ ERROR \n");
  if (option == 2) printf("\nEND of DATA\n");
  
  return(0);
}

/*--------------------------------------------------------------------*/
int Is_Log(void)
{
  if (strstr(filename,".log") != NULL) return(1);
  if (strstr(filename,".LOG") != NULL) return(1);
  
  return(0);
}

int Is_Fits()
  {
    char *ptr;
    int nr;
    
    for (nr = REC_SIZE, ptr = tape_record; nr > 0 ; ptr += 80,nr -= 80)
      if (!strncmp(ptr,"BITPIX",6)) return(1);

    return(0);
  }
  
/*--------------------------------------------------------------------*/
int in_list(char *file)
{
  FILE *fp;
  char tst[80];
  int ok = 0;
  
  fp = fopen(file,"r");
  while(fgets(tst,80,fp) != NULL)
    {
      if (strlen(tst) > 1)
	{
	  tst[strlen(tst) - 1] = NULL_CHAR;
	  if (strstr(filename,tst) != NULL)
	    {
	      ok = 1;
	      break;
	    }
	}
    }
  fclose(fp);
  return(ok);
}

int To_Copy(void)
{
  char answer[80];
  
  if ((f_list) || (f_list_file))
    {
      fflush(stdout);
      return(0);
    }
  
  if (f_fits)
    {
      if (!Is_Fits()) 	
	{
	  printf("NO FITS file, Skipping...");
	  fflush(stdout);
	  return(0);
	}
    }
  
  if (f_exc_file)
    {
      if (in_list(exclusion_file))
	{
	  printf("In exclusion list, Skipping...");
	  fflush(stdout);
	  return(0);
	}
    }
  
  if (f_exc_mask)
    {
      if (strstr(filename,exclusion_mask) != NULL)
	{
	  printf("Excluded, Skipping...");
	  fflush(stdout);
	  return(0);
	}
    }
  
  if (f_sel_file)
    {
      if (in_list(selection_file))
	{
	  printf("In selection list, ");
	  fflush(stdout);
	}
      else
	{
	  printf("NOT in selection list, Skipping...");
	  fflush(stdout);
	  return(0);
	}
    }
  
  if (f_sel_mask)
    {
      if (strstr(filename,selection_mask) != NULL)
	{
	  printf("Selected, ");
	  fflush(stdout);
	}
      else
	{
	  printf("NOT Selected, Skipping...");
	  fflush(stdout);
	  return(0);
	}
    }
  
  if (f_query)
    {
      printf("Copy ? [NO] : ");
      fgets(answer, 80, stdin);
      if (toupper(answer[0]) == 'Y') 
	{
	  printf("Copying... ");
	  fflush(stdout);
	  return(1);
	}
      
      printf("Skipping... ");
      fflush(stdout);
      return(0);
    }
  

  printf("Copying... ");
  fflush(stdout);
  return(1);
}

/*--------------------------------------------------------------------*/
void vms_name(char *name)
{
char tmp[80];
int n;

  strcpy(tmp,name);
  for (n = 0; n < strlen(tmp) ; n++) if (tmp[n] == '.') tmp[n] = '_';
  for (n = 0; n < strlen(tmp) ; n++) if (tmp[n] == '/') tmp[n] = '.';
  sprintf(name,"[.%s]",tmp);
}
 
int my_mkdir(char *file)
{
char dir_name[80];
char fil_name[80];
char command[80];
int n;
int level = 0;

  if (VMS) strcpy(fil_name,file);

  for (n = 0; n < strlen(file); n++)
    {
      if (file[n] == '/')
	{
	  level++;
	  
	  strcpy(dir_name,file);
	  dir_name[n] = NULL_CHAR;

	  if (VMS) 
	  {
		vms_name(dir_name);
	  	sprintf(fil_name,"%s%s",dir_name,file + n + 1);
	  }

	  if (chdir(dir_name) < 0)
	    {
	      sprintf(command,"%s %s",VMS ? "create/dir" : "mkdir",dir_name);
	      printf("Creating %s\n",dir_name);
	      
	      if (system(command) != (VMS ? 1 : 0)) return(0);
	    }
	  else
	    {
	      int p;
	      for (p = 0; p < level; p++) chdir(VMS ? "[-]" : "..");
	    }
	}
    }  
  
  if (VMS) strcpy(file,fil_name);
  return(1);
}
/*-----------------------------------------------------------------------*/
int Open_Copy(void)
{
/*
   if (!my_mkdir(filename)) return(0);
   
   fp_out = fopen(filename,"w");
*/

char *good_boy;

   good_boy = filename[0] == '/' ? filename + 1 : filename;

   if (!my_mkdir(good_boy)) return(0);
   
   fp_out = fopen(good_boy,"w");

   path_out = fileno(fp_out);
   if (path_out > 0) return(1);

   printf("OPEN ERROR \n");

   return(0);
}

 /*--------------------------------------------------------------------*/
int Close_Copy(void)
{
   fclose(fp_out);
   return(1); 
}

 /*--------------------------------------------------------------------*/
int Copy_Text(void)
{
  int b_level;
  static char buffer_out[MAX_LEN];
  char *b_ptr;
  int nr,i;
  
  b_ptr = buffer_out;
  b_level = 0;
  nr      = REC_SIZE;
  
  while(1)
    {
      for (i = 0; i < nr; i++)
	{
	  if ((tape_record[i] == OS9_TRAILER) || (b_level >= MAX_LEN))
	    {
	      *b_ptr++ = NULL_CHAR;
	      fprintf(fp_out,"%s\n",buffer_out);
	      b_ptr = buffer_out;
	      b_level = 0;
	    }
	  else
	    {
	      *b_ptr++ = tape_record[i];
	      b_level++;
	    }      
	}

      if (!Read_Record(0)) 
	{
	  *b_ptr++ = NULL_CHAR;
	  fprintf(fp_out,"%s\n",buffer_out);
	  break;
	}
    }
    Close_Copy();
    printf("Translated ");
    return(1);
  }

  int Copy_Bin()
  {
    int nw;

    write(path_out,tape_record,REC_SIZE);
    while(Read_Record(0)) 
      {
	nw = write(path_out,tape_record,REC_SIZE);
	if (nw != REC_SIZE) 
	  {
	    printf("WRITE ERROR ");
	    return(0);
	  }
      }

    Close_Copy();
    printf("Copied ");

    return(1);
  }

 int Copy_File()
 {
   int ret;
   
   if (!Open_Copy()) return(0);

   if ((f_log) && (Is_Log()))
       ret = Copy_Text();
   else
       ret = Copy_Bin();

   return(ret);
 }

 /*--------------------------------------------------------------------*/
 int Process_File()
 {
   int ret;

   printf("%s ",filename);

   if (!Read_Record(0))
	{
	  printf("File is empty\n");
	  return(1);
      	}

   if (To_Copy()) 
     ret = Copy_File();
   else
     ret = Skipp_File();

   printf("\n");
   return(ret);
 }

/*--------------------------------------------------------------------*/
int Process_Tape(void)
{
  int ok = 1;
  
  printf("Processing tape...\n");

  if (f_list_file) 
  {
	fp_list = fopen(listfile,"w");
	if (fp_list == NULL) exit(0);
  }

  while(ok)
    {
      if (!Open_Tape()) return(0);
      
      if (Read_Record(2))
	{
	      tape_record[79] = NULL_CHAR;
	      strcpy(filename,tape_record);

	      if (f_list_file) fprintf(fp_list,"%s\n",filename);
	  
	      if (!Process_File()) return(0);
		      
	      Close_Tape();
	}
      else
        {
	      Close_Tape();
	      break;
        }
    }

  if (f_list_file) fclose(fp_list);
  return 1;
}

/*--------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
char *ptr;

  printf(VERSION);
  printf("%s\n",MT_FSF ? "Fast skipp" : "Slow skipp");
  
  f_rewind = 1;
  f_query  = 0;
  f_list = 0;
  f_fits = 0;
  f_device = 0;
  f_log  = 0;

  f_sel_mask = f_sel_file = f_exc_mask = f_exc_file = 0;
  
  ptr = getenv("TAPE");
  
  while(*++argv)
    {
      if (!strcmp(*argv,"-d"))
      {
	f_device = 1;
	ptr = *++argv;
      }
      else if (!strcmp(*argv,"-nr"))
	f_rewind = 0;
      else if (!strcmp(*argv,"-fits"))
	f_fits = 1;
      else if (!strcmp(*argv,"-log"))
	f_log = 1;
      else if (!strcmp(*argv,"-q"))
	f_query = 1;
      else if (!strcmp(*argv,"-l"))
	f_list = 1;
      else if (!strcmp(*argv,"-l="))
	{
	  f_list_file = 1;
	  strcpy(listfile,*++argv);
	}
      else if (!strcmp(*argv,"-s"))
	{
	  f_sel_mask = 1;
	  strcpy(selection_mask,*++argv);
	}
      else if (!strcmp(*argv,"-s="))
	{
	  f_sel_file = 1;
	  strcpy(selection_file,*++argv);
	}
      else if (!strcmp(*argv,"-e"))
	{
	  f_exc_mask = 1;
	  strcpy(exclusion_mask,*++argv);
	}
      else if (!strcmp(*argv,"-e="))
	{
	  f_exc_file = 1;
	  strcpy(exclusion_file,*++argv);
	}
      
      else
	usage();
      
    }

  if ((!f_device) && (ptr == NULL))
  {
	printf("Device not specified...\n");
	exit(0);
  }

  printf("Device : %s\n",ptr);
  strcpy(device,ptr);

  if (f_rewind) Rewind_Tape();

  Test_Label();

  Process_Tape();
  return 0 ;
}
/*--------------------------------------------------------------------*/
