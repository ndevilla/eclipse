
/*
 * This program converts a list of ASCII star catalogs into a C header
 * file, ready to be included with the appropriate data structure
 * declarations and associated methods (see irstd.[ch]).
 * 
 * The format for each catalog is:
 * col  1: Name
 * col  2: RA (2000) hrs min sec
 * col  3: DEC (2000) deg ' ""
 * col  4: spectral type. A table for correspondance with temperature
 *      will be provided separately. Non 'standard' spectral types can just
 *      be disabled. In some cases the spectral type is not provided (--).
 * col  5: J
 * col  6: H
 * col  7: K
 * col  8: Ks
 * col  9: L
 * col 10: M
 * col 11: L'
 * col 12: M'
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#define STRINGSZ	50
#define LINESZ		512

#define	HEADERFILENAME	"irlist.h"
#define	WEBPAGENAME		"html/index.html"

#define NMAXCATALOG	50



char * get_basename(const char *filename)
{
  char *p ;
  p = strrchr (filename, '/');
  return p ? p + 1 : (char *) filename;
}

int is_catalog_file(char * filename)
{
	FILE	*	cat ;
	char		line[10];
	int			ret ;
	DIR		*	dir ;

	if (filename==NULL) return 0 ;
	if ((dir=opendir(filename))!=NULL) {
		closedir(dir);
		return 0 ;
	}
	ret=0 ;
	if ((cat=fopen(filename, "r"))!=NULL) {
		fread(line, sizeof(char), 10, cat);
		fclose(cat);
		if (!strncmp(line, "# CATALOG", 9)) {
			ret=1;
		}
	}
	return ret ;
}



void generate_header_file(char ** catalogs, int ncat)
{
	FILE *	header_file ;
	int		i ;
	int		lineno ;
	char	line[LINESZ+1] ;
	FILE *	tab ;
	char	star_name	[STRINGSZ],
			star_ra_1	[STRINGSZ],
			star_ra_2	[STRINGSZ],
			star_ra_3	[STRINGSZ],
			star_dec_1	[STRINGSZ],
			star_dec_2	[STRINGSZ],
			star_dec_3	[STRINGSZ],
			star_sptype	[STRINGSZ],
			star_J		[STRINGSZ],
			star_H		[STRINGSZ],
			star_K		[STRINGSZ],
			star_Ks		[STRINGSZ],
			star_L		[STRINGSZ],
			star_M		[STRINGSZ],
			star_Lp		[STRINGSZ],
			star_Mp		[STRINGSZ] ;
	int		scanned ;
	double	ra, dec ;

	header_file = fopen(HEADERFILENAME, "w");
	if (header_file==NULL) {
		printf("cannot create %s: aborting\n", HEADERFILENAME);
		return ;
	}

	/* Print out header */
	fprintf(header_file,
			"\n"
			"\n"
			"#ifndef _IRLIST_H_\n"
			"#define _IRLIST_H_\n"
			"\n");

	/* Print out catalog information */
	fprintf(header_file,
			"\n"
			"static const char * irstd_catalogs[] = {\n",
			ncat
			);
	for (i=0 ; i<ncat ; i++) {
		fprintf(header_file, "\t\"%s\",\n", get_basename(catalogs[i]));
	}
	fprintf(header_file, "\tNULL\n};\n\n");

	fprintf(header_file,
			"\n"
			"static irstd irstd_list[] =\n"
			"{\n");

	for (i=0 ; i<ncat ; i++) {
		if ((tab=fopen(catalogs[i], "r"))==NULL) {
			printf("cannot open catalog %s\n", catalogs[i]);
			continue ;
		}
		lineno = 0 ;
		while (fgets(line, LINESZ, tab)!=NULL) {
			if (line[0]=='#')
				continue;
			line[LINESZ] = (char)0;
			lineno++ ;
			scanned = 
				sscanf(line,
		"%s | %s %s %s | %s %s %s | %s | %s | %s | %s | %s | %s | %s | %s | %s",
				star_name,
				star_ra_1,
				star_ra_2,
				star_ra_3,
				star_dec_1,
				star_dec_2,
				star_dec_3,
				star_sptype,
				star_J,
				star_H,
				star_K,
				star_Ks,
				star_L,
				star_M,
                star_Lp,
                star_Mp);
			if (scanned != 16) {
				fprintf(stderr, "syntax error file %s line %d\n",
						catalogs[i], lineno);
			} else {

				ra = 15.0 *
					((double)atof(star_ra_1)+
					 (double)atof(star_ra_2)/60.0+
					 (double)atof(star_ra_3)/3600.0) ;
				if (star_ra_1[0]=='-') ra=-ra ;

				dec = fabs((double)atof(star_dec_1))+
					  fabs((double)atof(star_dec_2))/60.0+
					  fabs((double)atof(star_dec_3))/3600.0 ;

				if (star_dec_1[0]=='-') {
					dec=-dec ;
				}

				fprintf(header_file,
		"{1, \"%s\", %g, %g, \"%s\", %s, %s, %s, %s, %s, %s, %s, %s, %d},\n",
					star_name,
					ra,
					dec,
					star_sptype,
					star_J,
					star_H,
					star_K,
					star_Ks,
					star_L,
					star_M,
					star_Lp,
					star_Mp,
                    i);
			}
		}
		fclose(tab);
	}

	/* Output tail */
	fprintf(header_file,
			"{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}\n"
			"} ;\n"
			"\n"
			"\n"
			"#endif\n");
	fclose(header_file);
	return ;
}

void generate_web_page(char ** catalogs, int ncat)
{
	FILE *	wpage ;
	int		i ;
	int		lineno ;
	char	line[LINESZ+1] ;
	FILE *	tab ;
	char	star_name	[STRINGSZ],
			star_ra_1	[STRINGSZ],
			star_ra_2	[STRINGSZ],
			star_ra_3	[STRINGSZ],
			star_dec_1	[STRINGSZ],
			star_dec_2	[STRINGSZ],
			star_dec_3	[STRINGSZ],
			star_sptype	[STRINGSZ],
			star_J		[STRINGSZ],
			star_H		[STRINGSZ],
			star_K		[STRINGSZ],
			star_Ks		[STRINGSZ],
			star_L		[STRINGSZ],
			star_M		[STRINGSZ],
			star_Lp		[STRINGSZ],
			star_Mp		[STRINGSZ] ;
	int		scanned ;
	double	ra, dec ;
	char	wpage_name[STRINGSZ];
	
	if (mkdir("html", 0777)==-1) {
		printf("cannot create directory [html]\n");
		return ;
	}
	wpage = fopen(WEBPAGENAME, "w");
	if (wpage==NULL) {
		printf("cannot create %s: aborting\n", WEBPAGENAME);
		return ;
	}

	/* Print out header */
	fprintf(wpage,
			"<html>\n"
			"<title>Infrared standard star catalog</title>\n"
			"<body bgcolor=\"#ffffff\" text=\"#000000\">\n"
			"<p>\n"
			"List of all catalogs:\n"
			"</p>\n"
			"<ul>\n");
	for (i=0 ; i<ncat ; i++) {
		fprintf(wpage,
				"<li><a href=\"%s.html\">%s</a></li>\n",
				get_basename(catalogs[i]),
				get_basename(catalogs[i]));
	}
	fprintf(wpage,
			"</ul>\n"
			"</body>\n"
			"</html>\n");

	fclose(wpage);


	for (i=0 ; i<ncat ; i++) {
		if ((tab=fopen(catalogs[i], "r"))==NULL) {
			printf("cannot open catalog %s\n", catalogs[i]);
			continue ;
		}
		sprintf(wpage_name, "html/%s.html", get_basename(catalogs[i]));
		if ((wpage=fopen(wpage_name, "w"))==NULL) {
			printf("cannot create file %s\n", wpage_name);
			fclose(tab);
			continue;
		}

		/* Print out header */
		fprintf(wpage,
				"<html>\n"
				"<title>Catalog: %s</title>\n"
				"<body bgcolor=\"#ffffff\" text=\"#000000\">\n",
				get_basename(catalogs[i]));
		fprintf(wpage,
				"<h2>Catalog: %s</h2>\n",
				get_basename(catalogs[i]));
		fprintf(wpage,
				"<p>\n"
				"<table "
				"cols=\"10\" "
				"width=\"90%\" "
				"cellpadding=\"2\" "
				"cellspacing=\"2\" "
				"border=\"2\">\n"
				);


		fprintf(wpage,
				"<tr>\n"
				"<td><b>Name</b></td>\n"
				"<td><b>RA</b></td>\n"
				"<td><b>Dec</b></td>\n"
				"<td><b>SpType</b></td>\n"
				"<td><b>J</b></td>\n"
				"<td><b>H</b></td>\n"
				"<td><b>K</b></td>\n"
				"<td><b>Ks</b></td>\n"
				"<td><b>L</b></td>\n"
				"<td><b>M</b></td>\n"
				"<td><b>L'</b></td>\n"
				"<td><b>M'</b></td>\n"
				"</tr>\n");

		lineno = 0 ;
		while (fgets(line, LINESZ, tab)!=NULL) {
			if (line[0]=='#')
				continue;
			line[LINESZ] = (char)0;
			lineno++ ;
			scanned = 
				sscanf(line,
		"%s | %s %s %s | %s %s %s | %s | %s | %s | %s | %s | %s | %s | %s | %s",
				star_name,
				star_ra_1,
				star_ra_2,
				star_ra_3,
				star_dec_1,
				star_dec_2,
				star_dec_3,
				star_sptype,
				star_J,
				star_H,
				star_K,
				star_Ks,
				star_L,
				star_M,
				star_Lp,
				star_Mp);
			if (scanned != 16) {
				printf("syntax error file %s line %d\n", catalogs[i], lineno);
			} else {
				if (!strcmp(star_J, "99")) {
					strcpy(star_J, "--");
				}
				if (!strcmp(star_H, "99")) {
					strcpy(star_H, "--");
				}
				if (!strcmp(star_K, "99")) {
					strcpy(star_K, "--");
				}
				if (!strcmp(star_Ks, "99")) {
					strcpy(star_Ks, "--");
				}
				if (!strcmp(star_L, "99")) {
					strcpy(star_L, "--");
				}
				if (!strcmp(star_M, "99")) {
					strcpy(star_M, "--");
				}
				if (!strcmp(star_Lp, "99")) {
					strcpy(star_Lp, "--");
				}
				if (!strcmp(star_Mp, "99")) {
					strcpy(star_Mp, "--");
				}
				fprintf(wpage,
						"<tr>\n"
						"<td>%s</td>\n"
						"<td>%s:%s:%s</td>\n"
						"<td>%s:%s:%s</td>\n"
						"<td>%s</td>\n"
						"<td>%s</td>\n"
						"<td>%s</td>\n"
						"<td>%s</td>\n"
						"<td>%s</td>\n"
						"<td>%s</td>\n"
						"<td>%s</td>\n"
						"<td>%s</td>\n"
						"<td>%s</td>\n"
						"</tr>\n",
						star_name,
						star_ra_1, star_ra_2, star_ra_3,
						star_dec_1, star_dec_2, star_dec_3,
						star_sptype,
						star_J,
						star_H,
						star_K,
						star_Ks,
						star_L,
						star_M,
						star_Lp,
						star_Mp);
			}
		}
		fclose(tab);
		fprintf(wpage,
				"</table>\n"
				"</p>\n"
				"\n");
		/* Output tail */
		fprintf(wpage,
				"</body>\n"
				"</html>\n");
		fclose(wpage);
	}

	return ;
}


int main(int argc, char *argv[])
{
	char **	catalogs ;
	int		ncat ;
	int		nstars ;
	int		i, j ;
	int		web_page ;

	if (argc<2) {
		printf("usage: %s <list of table files>\n", argv[0]);
		return 1 ;
	}

	/* Identify catalogs in command-line arguments */
	web_page = 0 ;
	ncat = 0 ;
	for (i=1 ; i<argc ; i++) {
		if (is_catalog_file(argv[i])!=0)
			ncat ++ ;
		if (!strcmp(argv[i], "-w"))
			web_page=1 ;
	}
	if (ncat<1) {
		printf("none of the command-line arguments is a catalog\n");
		return -1 ;
	}
	catalogs = malloc(ncat * sizeof(char*));
	printf("\n");
	printf("-------------------------------------------------------\n");
	j=0 ;
	for (i=1 ; i<argc ; i++) {
		if (is_catalog_file(argv[i])!=0) {
			catalogs[j] = argv[i] ;
			printf("registered catalog: %s\n", catalogs[j]);
			j++ ;
		}
	}
	printf("-------------------------------------------------------\n");
	printf("\n");

	if (web_page) {
		generate_web_page(catalogs, ncat);
	} else {
		generate_header_file(catalogs, ncat);
	}
	printf("done.\n");
	return 0 ;
}
