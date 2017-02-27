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

#include <sys/types.h>
#include <limits.h>
#include <unistd.h>

#include <dirent.h>

#include <sys/stat.h>
#include <malloc.h>
#include <memory.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#include <ctype.h>
#include <stdlib.h>

/*
 * --------------------------------------------------------------- defines --
 */
#define STR_SIZE sizeof("?rwxrwxrwx")
#define LEN 12

/* #define DEBUG_SWITCH */
/*
 * -------------------------------------------------------------- typedefs --
 */

typedef struct _params {
    char *spath;
    int help;
    int print;
    char f_type;
    int ls;
    char *user;                     //	<name>/<uid> expt: user name can be a number too
    unsigned long user_id;
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

parms process_parms(const int len, char **pms);

char *get_smlink(const char *file_path, const struct stat attr);

void print_ls(const char *file, const struct stat atrr);

int do_type(mode_t mode);

int do_name(uid_t uid);
void clean_parms(parms *pm);


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


int main(int argc, char *argv[]) {

    int ret;
    struct stat sb;

    if (argc <= 2) {
        fprintf(stderr, "usage: %s -option <file-path>\n", argv[0]);
        return 1;
    }
    if (!argv[2]) {
        fprintf(stderr, "%s\nusage: %s -option <file-path>\n", "Missing file-path", argv[0]);
        return 1;
    }

    parms p = process_parms(argc,argv);

    if(p.help){
        print_help();
        clean_parms(&p);
        return 0;
    }

    if(p.ls){
        ret = lstat(argv[1], &sb);
        if (ret) {
            perror("lstat processing error");
        }

        print_ls(argv[1], sb);
        

        /*  test call do_dir*/
        if((sb.st_mode & S_IFMT) == S_IFDIR)
            do_dir(argv[1], &p);

    }
    printf("\n");
    return 0;
}

/**
 * \brief This display all the necessary help information for the programm
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


/**
 * \brief This process all the passed in parameters from the *argv[]
 * and set status for the reqired parameters in the struct.
 * If in any case a wrong parameter is found EXIT_FAILURE otherwise continue to
 * the next parameter
 * */

parms process_parms(const int len, char **pms) {
    parms p = {0};        // to prevent uninitialise message

    for (int i = 1; (i < len); ++i) {

        /*if is the first argv and is not one of the
         * options, assign it to the spath.
         * will check later in do_file and do_dir when other options are
         * set and they require a specific type
         * */
        if ((i == 1) && *(*(pms + 1)) != '-') {
            size_t l = strlen(pms[i] + 1);

            p.spath = malloc(sizeof(char) * l);
            strcpy(p.spath, pms[i]);
            p.spath[l + 1] = '\0';

            continue;
        } else


        if (strcmp(pms[i], "-name") == 0) {
            size_t l = strlen(pms[i]);
            p.name = malloc(sizeof(char) * l + 1);
            strcpy(p.name, pms[i]);
            p.name[l + 1] = '\0';

            continue;
        } else


        if (strcmp(pms[i], "-help") == 0) {
            p.help = 1;

            continue;
        } else


        if (strcmp(pms[i], "-print") == 0) {
            p.help = 1;

            continue;

        } else

        if (strcmp(pms[i], "-ls") == 0) {
            p.ls = 1;

            continue;
        } else


        if (strcmp(pms[i], "-type") == 0) {
            char f = *(pms[i + 1]);
            if (f == 'f' || f == 'b' || f == 'c' ||
                f == 'd' || f == 's' || f == 'p' || f == 'l') {
                p.f_type = f;
                continue;
            } else {
                printf("myfind: Unknown argument to %s: %c\n", pms[i], f);
                exit(EXIT_FAILURE);
            }


        } else

            /*First confirm the user in the passwd database
             * getpwnam return NULL if no match is found in the database
             * if no match is found then check if is a digit and extract it as user_id
             * otherwise report error and exit failure
             * This check cases where in the passwd database a username is a number
             * */

        if (strcmp(pms[i], "-user") == 0) {

            struct passwd *pd;
            if (pms[i + 1]) {
                p.user = strcpy(malloc(sizeof(strlen(pms[i + 1]))), pms[i + 1]);
                if ((pd = getpwnam(pms[i + 1]))) {
                    p.user_id = pd->pw_uid;
                    continue;
                } else if (isdigit(pms[i][0])) {
                    sscanf(p.user, "%lu", &p.user_id);
                    continue;
                } else {
                    printf("myfind: `%s` is not a the name of a known user \n", pms[i + 1]);
                    exit(EXIT_FAILURE);
                }


            } else {

                printf("myfind: missing argument to `%s`\n", pms[i]);
                exit(EXIT_FAILURE);

            }


        } else {
            printf("myfind: Unknown predicate `%s`\n", pms[i]);

        }


    }

    return p;




}


/** \brief
 * emulating the -l in the linux command ls -l wth expected output format:
 * -rwxr-xr-x. 1 root root 3756 Feb  5 20:18 filename.extension
 * putting everything in the main function here as one method.
 * */
void print_ls(const char *filename, const struct stat sb) {

    struct group *gp;
    struct passwd *pd;
    char ftpe;

    switch (sb.st_mode & S_IFMT) {
        case S_IFREG:
            ftpe = '-';
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

    ntime[strlen(ntime) - 9] = '\0';


    /*Allocating memory for a null-terminated string for the file permission format
     * A maximun size to hold or the file permission bits inluding the SUID and the sticky-bit
     * using snprntf to store the results in the required formatted output
     * put a null-terminator at the end of the char array
     * */
    char *permstr = malloc(sizeof(char) * LEN);

    snprintf(permstr, STR_SIZE, "%c%c%c%c%c%c%c%c%c%c", ftpe, (sb.st_mode & S_IRUSR) ? 'r' : '-',
             (sb.st_mode & S_IWUSR) ? 'w' : '-', (sb.st_mode & S_ISUID) ? (sb.st_mode & S_IXUSR ? 's' : 'S') :
                                                 (sb.st_mode & S_IXUSR ? 'x' : '-'),

             (sb.st_mode && S_IRGRP) ? 'r' : '-', (sb.st_mode & S_IWGRP) ? 'w' : '-',
             (sb.st_mode & S_ISGID) ? (sb.st_mode & S_IXGRP ? 's' : 'S') : (sb.st_mode & S_IXGRP ? 'x' : '-'),
             (sb.st_mode & S_IROTH) ? 'r' : '-', (sb.st_mode & S_IWOTH) ? 'w' : '-',
             (sb.st_mode & S_ISVTX) ? (sb.st_mode & S_IXOTH ? 't' : 'T') : (sb.st_mode & S_IXOTH ? 'x' : '-'));

    permstr[LEN - 1] = '\0';


    char *symlink = get_smlink(filename,sb);

    printf("\n%s  %ld %s %s %lld %s %s %s %s",
           permstr, sb.st_nlink,
           pd->pw_name, gp->gr_name, (long long) sb.st_size,
           ntime, filename, (symlink?"->":""),(symlink?symlink:""));



    free(permstr);
    free(symlink);

}

void clean_parms(parms *pm){
    free(pm->user);
    free(pm->name);
    free(pm->spath);

}

/** \brief
 * gathering informations about the target of the symbolic link and return them in the aproparate format of "find":
 * example of return string "-> boot/vmlinuz-4.4.0-64-generic"
 * */
char *get_smlink(const char *file_path, const struct stat attr){

    char *sym_link = NULL;
    ssize_t r, bufsiz;;


    bufsiz = attr.st_size + 1;

    if (S_ISLNK(attr.st_mode)) {
        if (attr.st_size == 0)
            bufsiz = PATH_MAX;


        sym_link = malloc(sizeof(char) * bufsiz);
        if (sym_link == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }


        while((r = readlink(file_path, sym_link, bufsiz)) > 1 && (r > bufsiz)){
            bufsiz *=2;
            if((sym_link = realloc(sym_link,sizeof(char)*bufsiz)) == NULL){
                printf("Not enough memory to continue\n");
                exit(EXIT_FAILURE);
            }

        }
        if (r == -1) {
            perror("readlink");
            exit(EXIT_FAILURE);
        }




        sym_link[r] = '\0';



        return sym_link;
    }

    return NULL;

}


/** \brief
 * gathering informations about the given directory and print them out:
 * example of test-output "inode number: [1587860]	-> file: [mail]"
 * */
void do_dir(const char *dir_name, const parms *parms){

/*	int return_val;	*/
    struct stat sb;
    struct dirent *entry;
    DIR *dir;




    dir = opendir (dir_name);

    printf("contents of direchtory: [%s]\n", parms->spath);
    while ((entry = readdir (dir)) != NULL) {
#ifdef DEBUG_SWITCH
        printf("inode number: [%ld]	-> file: [%s]\n", entry->d_ino, entry->d_name);
#endif
        printf("%s", entry->d_name);
        lstat(dir_name, &sb);
    }

#ifdef DEBUG_SWITCH
    if (!entry)
		perror ("readdir");
#endif

    closedir (dir);

}
