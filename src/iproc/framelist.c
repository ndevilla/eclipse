/*----------------------------------------------------------------------------*/
/**
   @file	framelist.c
   @author	N. Devillard
   @date	July 2000
   @version	$Revision: 1.23 $
   @brief	Framelist parsing routines
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: framelist.c,v 1.23 2002/08/20 08:08:40 yjung Exp $
	$Author: yjung $
	$Date $
	$Revision: 1.23 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <sys/types.h>
#include <sys/stat.h>

#include "static_sz.h"
#include "comm.h"
#include "file_handling.h"
#include "framelist.h"
#include "strlib.h"

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	Load a list of valid file names from an ASCII list.
  @param	filename		Name of the ASCII list to parse.
  @return	1 newly allocated ascii_list object.

  This function expects in input the name of a valid ASCII list, i.e. an
  ASCII file with the following format:

  \begin{itemize}
  \item First column contains a valid filename.
  \item Second column might contain a file type.
  \end{itemize}

  If a given file name does not correspond to a valid existing file,
  the list is not loaded and NULL is returned.

  The returned object must be deallocated using framelist_del().
 */
/*----------------------------------------------------------------------------*/
framelist * framelist_load(char * filename)
{
	framelist	*	loaded ;
	charmatrix	*	charm ;
	char		*	name ;
	char		*	type ;
	int				i ;
	int				nfiles ;

	if (is_ascii_list(filename)!=1) return NULL ;

	charm = charmatrix_read(filename);
	if (charm==NULL) {
		e_error("parsing input framelist [%s]", filename);
		return NULL ;
	}

	/* Test validity of input files */
	nfiles = charm->ly ;
	for (i=0 ; i<nfiles ; i++) {
		name = charmatrix_elem(charm, 0, i);
		if (file_exists(name)!=1) {
			e_error("file [%s] declared in %s does not exist", name, filename);
			charmatrix_del(charm);
			return NULL ;
		}
	}
	/* Initialize framelist structure */
	loaded = framelist_new(nfiles);
	loaded->filename = strdup(filename) ;
	if (charm->lx<2) {
		/* Single column input: no type declaration */
		free(loaded->type) ;
		loaded->type = NULL ;
	}

	/* Copy frame names into framelist */
	for (i=0 ; i<nfiles ; i++) {
		loaded->name[i] = strdup(charmatrix_elem(charm, 0, i)) ;
		if (loaded->type!=NULL) {
			type = charmatrix_elem(charm, 1, i);
			if (type!=NULL) {
				/* Switch frame type to lowercase and copy */
				loaded->type[i] = strdup(strlwc(type));
			} else {
				loaded->type[i] = NULL ;
			}
		}
	}
	charmatrix_del(charm);
	return loaded ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Get the first valid file name in an ASCII list.
  @param	filename	Name of the ASCII list to parse.
  @return	1 pointer to statically allocated string, or NULL.

  This function looks up an ASCII list file to localize the first valid
  FITS file name, and returns a pointer to a statically allocated string
  containing the file name. If an error occurs, it returns NULL.

  This function is actually implemented as a wrapper around framelist_load
  to avoid recoding a second ASCII list parser. So it costs just as much
  to call this function or framelist_load.
 */
/*----------------------------------------------------------------------------*/
char * framelist_firstname(char * filename)
{
	static char		firstname[FILENAMESZ];
	framelist	*	flist ;

	if (filename==NULL) return NULL ;
	/* Try to load the framelist */
	flist = framelist_load(filename);
	if (flist==NULL) return NULL ;
	strcpy(firstname, flist->name[0]);
	framelist_del(flist);
	return firstname ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Dump a framelist to an opened file pointer.
  @param	dumped	Framelist to dump.
  @param	out		Opened file pointer.
  @return	void

  This function dumps the information contained in a framelist object to an
  opened file pointer. It is Ok to provide stdout or stderr as file pointers.
 */
/*----------------------------------------------------------------------------*/
void framelist_dump(framelist * dumped, FILE * out)
{
	int		i ;

	if (dumped==NULL) return ;
	if (dumped->n<1) return ;
	if (dumped->name==NULL) return ;

	fprintf(out, "framelist: %s contains %d files\n",
			dumped->filename,
			dumped->n);
	for (i=0 ; i<dumped->n ; i++) {
		fprintf(out, "%s", dumped->name[i]);
		if (dumped->type!=NULL) {
			fprintf(out, "\t%s", dumped->type[i]);
		}
		fprintf(out, "\n");
	}
	return ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Allocate space to hold a new frame list.
  @param	n	Number of frames to hold in the list.
  @return	Pointer to newly allocated framelist object.

  This constructor will allocate the space for the new framelist object,
  set the number of frames to the required amount, and allocate space to
  hold names and types. Name and type entries are set to NULL.
 */
/*----------------------------------------------------------------------------*/
framelist * framelist_new(int n)
{
	framelist	*	f ;

	f = malloc(sizeof(framelist));
	f->filename = NULL ;
	f->n = n ;
	f->name  = calloc(n, sizeof(char*)) ;
	f->type  = calloc(n, sizeof(char*)) ;
	f->label = calloc(n, sizeof(int)) ;
	return f ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Deallocated a framelist object.
  @param	f	Framelist object to deallocate.
  @return	void

  This function frees all memory associated to a framelist object.
 */
/*----------------------------------------------------------------------------*/
void framelist_del(framelist * f)
{
	int		i ;

	if (f==NULL) return ;
	if (f->filename != NULL) free(f->filename);
	for (i=0 ; i<f->n ; i++) {
		if (f->name[i]!=NULL) {
			free(f->name[i]);
		}
	}
	free(f->name);
	if (f->type!=NULL) {
		for (i=0 ; i<f->n ; i++) {
			if (f->type[i]!=NULL) {
				free(f->type[i]);
			}
		}
		free(f->type);
	}
	if (f->label!=NULL) {
		free(f->label);
	}
	free(f);
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Copy contents of a framelist to a new framelist object.
  @param	f	Framelist object to copy.
  @return	1 newly allocated framelist object.

  All contents of a framelist are copied into a newly allocated framelist,
  which is returned to the caller. The returned object must be freed using
  framelist_del().
 */
/*----------------------------------------------------------------------------*/
framelist * framelist_copy(framelist * f)
{
	framelist	*	cpy ;
	int				i ;

	if (f==NULL) return NULL ;

	/* Allocate returned framelist */
	cpy = framelist_new(f->n);
	/* Copy filename */
	if (f->filename!=NULL) {
		cpy->filename = strdup(f->filename);
	}
	/* Copy list of names */
	if (f->name!=NULL) {
		for (i=0 ; i<f->n ; i++) {
			if (f->name[i]!=NULL) {
				cpy->name[i]=strdup(f->name[i]);
			}
		}
	}
	/* Copy list of types */
	if (f->type!=NULL) {
		for (i=0 ; i<f->n ; i++) {
			if (f->type[i]!=NULL) {
				cpy->type[i] = strdup(f->type[i]) ;
			}
		}
	}
	/* Copy list of labels */
	if (f->label!=NULL) {
		for (i=0 ; i<f->n ; i++) {
			cpy->label[i] = f->label[i];
		}
	}
	return cpy ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Select contents of a framelist to a new framelist object.
  @param    f       Framelist object to examine.
  @param    label   Label to use for selection
  @return   1 newly allocated framelist object.
 
  This function selects frames in a framelist which have their label
  set to the same value as 'label'. The returned object must be deallocated
  using framelist_del().
 */
/*----------------------------------------------------------------------------*/
framelist * framelist_select(
		framelist	*	f, 
		int 			label)
{
	framelist	*	selected ;
	int				i, j ;
	int				n_ok ;

	if (f==NULL) return NULL ;

	/* Count how many files match the given label */
	n_ok = 0 ;
	for (i=0 ; i<f->n ; i++) {
		if (f->label[i]==label)
			n_ok ++ ;
	}
	if (n_ok<1) return NULL ;

	selected = framelist_new(n_ok);
	if (f->type==NULL) {
		free(selected->type);
		selected->type = NULL ;
	}
	j=0 ;
	for (i=0 ; i<f->n ; i++) {
		if (f->label[i]==label) {
			/* Copy name */
			if (f->name[i]!=NULL) {
				selected->name[j] = strdup(f->name[i]);
			}
			/* Copy type */
			if (f->type!=NULL) {
				if (f->type[i]!=NULL) {
					selected->type[j] = strdup(f->type[i]);
				}
			}
			/* Copy label */
			selected->label[j] = f->label[i];
			j++ ;
		}
	}
	return selected ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Select only some frames in a list.
  @param	f			Framelist to check.
  @param	token		Token to identify for each frame.
  @param	token_get	Function to use to get token for each file.
  @return	Pointer to a newly allocated framelist.

  This function applies a 'token_get' function to each file in the input
  list, getting back a character token for each file. It compares the
  returned token with the value provided in 'token' and rejects from the
  list all non-matching frames. If no matching frame can be found, this
  function returns NULL.

  The returned framelist must be deallocated using framelist_del().
 */
/*----------------------------------------------------------------------------*/
framelist * framelist_select_tokenget(
		framelist	*	f,
		char		*	token,
		char    	*   (*token_get)(char*))
{
	int				i, j ;
	char		*	sval ;
	char		**	all_tokens ;
	int				nvalid ;
	framelist	*	selected ;

	if (f==NULL || token==NULL || token_get==NULL) return NULL ;
	if (f->n<1) return NULL ;
	
	/* Loop on all file names, get token for each frame */
	all_tokens = malloc(f->n * sizeof(char*));
	for (i=0 ; i<f->n ; i++) {
		sval = token_get(f->name[i]);
		if (sval==NULL) {
			all_tokens[i] = NULL ;
		} else {
			all_tokens[i] = strdup(sval);
		}
	}

	/* Loop through retrieved tokens, count valid matches */
	nvalid = 0 ;
	for (i=0 ; i<f->n ; i++) {
		if (all_tokens[i] != NULL) {
			if (!strcmp(all_tokens[i], token)) {
				nvalid++ ;
			}
		}
	}

	if (nvalid<1) {
		for (i=0 ; i<f->n ; i++) {
			if (all_tokens[i]!=NULL) free(all_tokens[i]);
		}
		free(all_tokens);
		return NULL ;
	}

	selected = framelist_new(nvalid);
	j=0 ;
	for (i=0 ; i<f->n ; i++) {
		if (all_tokens[i] != NULL) {
			if (!strcmp(all_tokens[i], token)) {
				selected->name[j] = strdup(f->name[i]);
				selected->type[j] = NULL ;
				if (f->type!=NULL) {
					if (f->type[i]!=NULL) {
						selected->type[j] = strdup(f->type[i]);
					}
				}
				j++ ;
			}
			free(all_tokens[i]);
		}
	}
	free(all_tokens);
	return selected ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Purge some frames in a list.
  @param	f			Framelist to check.
  @param	token		Token to identify for each frame.
  @param	token_get	Function to use to get token for each file.
  @return	Pointer to a newly allocated framelist.

  This function applies a 'token_get' function to each file in the input
  list, getting back a character token for each file. It compares the
  returned token with the value provided in 'token' and rejects from the
  list all matching frames. If no matching frame can be found, this
  function returns NULL.

  The returned framelist must be deallocated using framelist_del().
 */
/*----------------------------------------------------------------------------*/
framelist * framelist_purge_tokenget(
		framelist	*	f,
		char		*	token,
		char    	*   (*token_get)(char*))
{
	int				i, j ;
	char		*	sval ;
	char		**	all_tokens ;
	int				nvalid ;
	framelist	*	selected ;

	if (f==NULL || token==NULL || token_get==NULL) return NULL ;
	if (f->n<1) return NULL ;
	
	/* Loop on all file names, get token for each frame */
	all_tokens = malloc(f->n * sizeof(char*));
	for (i=0 ; i<f->n ; i++) {
		sval = token_get(f->name[i]);
		if (sval==NULL) {
			all_tokens[i] = NULL ;
		} else {
			all_tokens[i] = strdup(sval);
		}
	}

	/* Loop through retrieved tokens, count valid matches */
	nvalid = 0 ;
	for (i=0 ; i<f->n ; i++) {
		if (all_tokens[i] != NULL) {
			if (strcmp(all_tokens[i], token)) {
				nvalid++ ;
			}
		} else nvalid++ ;
	}

	if (nvalid<1) {
		for (i=0 ; i<f->n ; i++) {
			if (all_tokens[i]!=NULL) free(all_tokens[i]);
		}
		free(all_tokens);
		return NULL ;
	}

	selected = framelist_new(nvalid);
	j=0 ;
	for (i=0 ; i<f->n ; i++) {
		if (all_tokens[i] != NULL) {
			if (strcmp(all_tokens[i], token)) {
				selected->name[j] = strdup(f->name[i]);
				selected->type[j] = NULL ;
				if (f->type!=NULL) {
					if (f->type[i]!=NULL) {
						selected->type[j] = strdup(f->type[i]);
					}
				}
				j++ ;
			}
			free(all_tokens[i]);
		} else {
            selected->name[j] = strdup(f->name[i]);
            selected->type[j] = NULL ;
            if (f->type!=NULL) {
                if (f->type[i]!=NULL) {
                    selected->type[j] = strdup(f->type[i]);
                }
            }
            j++ ;
        }
	}
	free(all_tokens);
	return selected ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Separate a list of frames into groups, according to labels.
  @param	lnames		Input framelist.
  @param    compare     Pointer to comparison function to use.
  @return 	int -1 in error case, nb of settings otherwise 

  This function takes in input a framelist, and a comparison function to sort 
  the frames. It will sort the frames according to the labels found by the 
  comparison function.

  The comparison function receives two frame names, and is responsible for 
  fetching whatever keyword in each frame header and compare it. If keywords 
  match, the comparison function must return 1, otherwise 0. This function 
  will count the number of possible different values found by the comparison 
  function and update nsettings accordingly.

  See an example in ins/isaac/recipes/dark_average.c
 */
/*----------------------------------------------------------------------------*/
int framelist_labelize(
		framelist   *   lnames,
		int             (*compare)(char*,char*))
{
	int			nsettings ;
    int         current_setting_id ;
    int         comp ;
    int         i,
                j ;

	if (lnames==NULL || compare==NULL) return -1 ;

    if (lnames->n < 1) return -1 ;
    else if (lnames->n == 1) {
        nsettings = 1 ;
        lnames->label[0] = 0 ;
	} else {
        nsettings = 0;
        current_setting_id = 0 ;
        lnames->label[0] = current_setting_id ;

        for (i=1 ; i<lnames->n ; i++) {
            j = 0 ;
            do{
                comp=(*compare)(lnames->name[j], lnames->name[i]);
                switch(comp){
                case 1:
                    lnames->label[i] = lnames->label[j] ;
                    break;
                case 0:
                    j++ ;
                    if (j == i) lnames->label[i] = ++current_setting_id ;
                    break;
                case -1:
                default:
                    e_error("cannot compare settings between [%s] and [%s]",
							lnames->name[i], lnames->name[j]) ;
                    return -1 ;
                }
            } while ((j != i) && (comp != 1)) ;
        }
        nsettings = current_setting_id+1 ;
    }
    return nsettings ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Returns 1 if the file is an ASCII list, 0 else.
  @param    filename name of the file to check
  @return   int 0, 1, or -1

  Returns 1 if the file name corresponds to a valid ASCII list.
  Returns 0 else. If the file does not exist, returns -1.
 */
/*----------------------------------------------------------------------------*/
int is_ascii_list(char * filename)
{
    struct stat sta ;
	FILE *	in ;
    char	line[FILENAMESZ];
	char	col[FILENAMESZ];
	char *	name ;

    if (filename==NULL) return 0 ;

    /* Stat file */
    if (stat(filename, &sta)!=0)
        return 0 ;
    if (sta.st_size<1)
        return 0 ;
	if ((in=fopen(filename, "r"))==NULL)
		return 0 ;
	while (fgets(line, FILENAMESZ-1, in)!=NULL) {
        sscanf(line, "%s", col);
		name = strstrip(col);
		if ((name[0]!='#') && strlen(name)>1) {
			if (file_exists(name)!=1) {
				fclose(in);
				return 0 ;
			}
		}
	}
	fclose(in);
    return 1 ;
}


