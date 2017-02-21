//
// Created by seekaddo on 2/16/17.
//


/**
 * @file myfind.c
 *
 * Beispiel 0
 *
 * @author Dennis Addo <ic16b026@technikum-wien.at>
 * @author Robert Niedermaier <ic16b089@technikum-wien.at>
 * @details More information about the project can be found here URL: https://cis.technikum-wien.at/documents/bic/2/bes/semesterplan/lu/beispiel1.html
 *
 * @date 15/02/2017
 *
 * @version 0.1
 *
 * @todo All implentations must be contained in one method structure unless other pieces of functions
 * @todo are required by other programs/methods
 * @todo ALL errors must be int or enums 0 fur SUCCESS or 1 for FAIL. You can also use the GNU C EXIT_SUCCESS anf FAILURE macros
 * @todo perror is used to display error informations, functions and error-types
 *
 *
 */

/*
 * -------------------------------------------------------------- includes --
 */

#include <stdio.h>
#include <sys/stat.h>
#include <malloc.h>
#include <memory.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>

/*
 * --------------------------------------------------------------- defines --
 */
#define STR_SIZE sizeof("?rwxrwxrwx")
#define LEN 12
/*
 * -------------------------------------------------------------- typedefs --
 */

typedef struct _params {
    char *spath;
    int help;
    int print;
    char f_type;
    int ls;
    char *user;
    unsigned long usr_id;
    char *name;

} parms;

/*
 * --------------------------------------------------------------- globals --
 */

/*
 * ------------------------------------------------------------- functions --
 * */

void print_help(void);

void do_file(const char *file_name, const parms *parms);

void do_dir(const char *dir_name, const parms *parms);

char *get_smlink(char file_path, const struct stat attr);

void do_ls(const char * file ,const struct stat atrr);

int do_type(mode_t mode);

int do_name(uid_t uid);


/**
  *
  * \brief This is a clone of the GNU find command in pure c
  *
  *
  *
  * \param argc the number of arguments
  * \param argv the arguments itselves (including the program name in argv[0])
  *
  * \return always "success"
  * \retval 0 always
  *
  */


int maint(int argc, char *argv[]) {

int ret;
    struct stat sb;

    if(argc <= 2){
        fprintf(stderr,"usage: %s -option <file-path>\n",argv[0]);
        return 1;
    }
    if(!argv[2]){
        fprintf(stderr,"%s\nusage: %s -option <file-path>\n","Missing file-path",argv[0]);
        return 1;
    }

    ret = stat(argv[1],&sb);
    if(ret){
        perror("stat processing error");
    }

    print_ls(argv[1],sb);

    return 0;
}

/*\brief This display all the necessary help information for the programm
 * when user specify -help for the program "myfind"
 * Default:displays for  inproper command options
 * */
void print_help(void) {
    printf("Usage: myfind <file or directory> [ <options> ] ...\n"
                   "default path is the current directory if none is specified; default expression is -print\n"
                   "Options: (You can specify any of the following options)\n"
                   "       : -help                   Shows all necessary informations for this command\n"
                   "       : -user <name>/<uid>     file or directory belongig to specified user\n"
                   "       : -name <pattern>        file or directory with specified name\n"
                   "       : -type [bcdpfls]        All entry with specified file type\n"
                   "       : -print                 print entries with the path\n"
                   "       : -ls                    print entries with more details\n"
                   "");

}


/** \brief
 * emulating the -l in the linux command ls -l wth expected output format:
 * -rwxr-xr-x. 1 root root 3756 Feb  5 20:18 filename.extension
 * putting everything in the main function here as one method.
 * */
void print_ls(const char *filename,const struct stat sb){

    struct group *gp;
    struct passwd *pd;
    char ftpe;

    switch (sb.st_mode & S_IFMT){
        case S_IFREG:
            ftpe =  '-';
            break;
        case S_IFDIR:
            ftpe = 'd';
            break;
        case S_IFBLK:
            ftpe = 'b';
            break;
        case S_IFCHR:
            ftpe = 'c';
            break;
        case S_IFIFO:
            ftpe = 'p';
            break;
        case S_IFLNK:
            ftpe = 'l';
            break;
        case S_IFSOCK:
            ftpe = 's';
            break;
        default:
            ftpe = '?';
    }


    /*Getting the group details*/
    gp = getgrgid(sb.st_gid);
    pd = getpwuid(sb.st_uid);



    /*Get a formated last modified date with ctime() in timesec format
     * removing the day at the beginning fo the time
     * and removing the '\n' which ctime() already adds from the string and additional millisec with -9 the length
     * the pointer ntime point to month instead of the initial day
     * PN: ctime() is a MT-Unsafe functions, that means is not safe to call/use in a multithreaded programm
     * in a multithreaded programm strftime() is recommended
     * */
    char *ntime = ctime(&sb.st_mtim.tv_sec) + 4;

    ntime[strlen(ntime)-9] = '\0';


    /*Allocating memory for a null-terminated string for the file permission format
     * A maximun size to hold or the file permission bits inluding the SUID and the sticky-bit
     * using snprntf to store the results in the required formatted output
     * put a null-terminator at the end of the char array
     * */
    char *permstr = malloc(sizeof(char)*LEN);

    snprintf(permstr,STR_SIZE, "%c%c%c%c%c%c%c%c%c%c",ftpe, (sb.st_mode  & S_IRUSR) ? 'r' : '-',
             (sb.st_mode  & S_IWUSR) ? 'w':'-',(sb.st_mode & S_ISUID)?(sb.st_mode & S_IXUSR ? 's':'S'):
                                               (sb.st_mode & S_IXUSR ?'x':'-'),

             (sb.st_mode  && S_IRGRP)?'r':'-',(sb.st_mode  & S_IWGRP)?'w':'-',
                                (sb.st_mode  & S_ISGID)?(sb.st_mode & S_IXGRP ?'s':'S'):(sb.st_mode & S_IXGRP?'x':'-'),
             (sb.st_mode  & S_IROTH)?'r':'-',(sb.st_mode  & S_IWOTH)?'w':'-',
                                (sb.st_mode  & S_ISVTX)?(sb.st_mode & S_IXOTH ?'t':'T'):(sb.st_mode & S_IXOTH? 'x':'-'));

    permstr[LEN-1] = '\0';


    printf("%s  %ld %s %s %lld %s %s\n",
           permstr, sb.st_nlink,
           pd->pw_name, gp->gr_name, (long long)sb.st_size,
           ntime,filename);


    free(permstr);
    permstr = NULL;

}

