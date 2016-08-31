qfits_header * qfits_header_read(char * filename);
qfits_header * qfits_header_readext(char * filename, int xtnum);
qfits_header * qfits_header_new(void);
qfits_header * qfits_header_default(void);
void qfits_header_add(
    qfits_header * hdr,
    char    * key,
    char    * val,
    char    * com,
    char    * lin);
void qfits_header_add_after(
    qfits_header * hdr,
    char    * after,
    char    * key,
    char    * val,
    char    * com,
    char    * lin);
void qfits_header_append(
    qfits_header *   hdr,
    char    * key,
    char    * val,
    char    * com,
    char    * lin);
void qfits_header_del(qfits_header * hdr, char * key);
void qfits_header_mod(qfits_header * hdr, char * key, char * val, char * com);
qfits_header * qfits_header_copy(qfits_header * src);
void qfits_header_touchall(qfits_header * hdr);
void qfits_header_consoledump(qfits_header * hdr);
void qfits_header_destroy(qfits_header * hdr);
char * qfits_header_getstr(qfits_header * hdr, char * key);
char * qfits_header_findmatch(qfits_header * hdr, char * key);

int qfits_header_getitem(
    qfits_header    *   hdr,
    int                 idx,
    char            *   key,
    char            *   val,
    char            *   com,
    char            *   lin
);
char * qfits_header_getline(qfits_header * hdr, char * key);
char * qfits_header_getcom(qfits_header * hdr, char * key);
int qfits_header_getint(qfits_header * hdr, char * key, int errval);
double qfits_header_getdouble(qfits_header * hdr, char * key, double errval);
int qfits_header_getboolean(qfits_header * hdr, char * key, int errval);
void keytuple2str(char * line, char * key, char * val, char * com);
int qfits_header_dump(qfits_header * hdr, FILE * out);
/* char * qfits_header_to_memblock(qfits_header * fh, int * hsize); */
