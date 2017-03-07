#define _XOPEN_SOURCE 220112L
#define _POSIX_C_SOURCE 200112L
#define _GNU_SOURCE

//
// Created by seekaddo on 2/16/17.
//


/**
 * @file myfind.c
 *
 * Beispiel 0
 *
 * @author Dennis Addo <ic16b026>
 * @author Robert Niedermaier <ic16b089>
 * @details More information about the project can be found here URL: https://cis.technikum-wien.at/documents/bic/2/bes/semesterplan/lu/beispiel1.html
 *
 * @date 16/02/2017
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
#include <errno.h>
#include <libgen.h>
#include <fnmatch.h>
#include <assert.h>

/**\def
 * --------------------------------------------------------------- defines --
 */
#define STR_SIZE sizeof("?rwxrwxrwx")
#define LEN 12
#define ISSUE "Usage: myfind <file or directory> [ <options> ] ..."

#define f_type(mode) if (S_ISBLK(mode)) { return 'b';} if (S_ISCHR(mode)) {return 'c';}\
                        if (S_ISDIR(mode)) {return 'd';} if (S_ISFIFO(mode)) {return 'p';}\
                        if (S_ISREG(mode)) {return 'f';} if (S_ISLNK(mode)) {return 'l';}\
                        if (S_ISSOCK(mode)) {return 's';} return '?';

/** \typedef
 * -------------------------------------------------------------- typedefs --
 *
 */

typedef struct ss_alopts {
    char *name;
    int help;
    int print;
    char f_type;
    int ls;
    char *user;                     //	<name>/<uid> expt: user name can be a number too
    unsigned long user_id;
    struct ss_alopts *next;
} s_optns;

/*
 * --------------------------------------------------------------- globals --
 */

/*
 * ------------------------------------------------------------- functions --
 * */

inline void print_help(void);

void do_file(char *file_path, s_optns *parms, struct stat *atrr);

void do_dir(char *dir_path, s_optns *parms, struct stat *sb);

s_optns *process_parms(const int len, char *spath[], char **parms);

char *get_smlink(const char *file_path, const struct stat *attr);

void print_ls(const char *file, const struct stat *atrr);

//int do_type(mode_t mode);

void startMyFind(char **p, s_optns *option1);

//int do_name(uid_t uid);
inline char ftype(mode_t mode);

void clean_parms(s_optns **pm);
/*void clean_me(char **av){
    int i =0;

    while(av[i]){
        assert(av[i] != NULL);
        free(av[i]);
        i++;
    }
}*/


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


    char *path_list[argc];
    memset(path_list,0,argc);   // this is to prevent errors from valgrind


    s_optns *p = NULL;
    p = process_parms(argc, path_list, argv);

    //printf("Print is vidible here in main----->%d\n",p->next->print);
    if (p->help) {
        print_help();
        clean_parms(&p);
        return 0;
    }

    startMyFind(path_list, p);
    clean_parms(&p);


    return 0;
}

/**
 * \brief This display all the necessary help information for the programm
 * when user specify -help for the program "myfind"
 * Default:displays for  inproper command options
 * */
inline void print_help(void) {
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


/**\brief this initialise the programm
 * \param path this is the list of specified search paths/locations set to cwd if none is specified
 * \param op this is the linkedlist of all passed command options
 * Documentation will come soon
 * this initialise the whole myfind command
 *
 *
 * */
void startMyFind(char *path[], s_optns *op) {

    struct stat fattr;
    int ret = 0, i = 0;
    char cwd[] = "./";

    //char **p;

    //char **sSource = p;
    if (path[i] == NULL) {
        path[i] = cwd;
        path[i + 1] = NULL;
        //p = path;
    }

    while (path[i]) {

        ret = lstat(*path, &fattr);
        if (ret == -1) {
            fprintf(stderr, "myfind: (%s): %s\n", path[i], strerror(errno));
            exit(EXIT_FAILURE);
        }

        do_file(path[i], op, &fattr);

        if (S_ISDIR(fattr.st_mode))
            do_dir(path[i], op, &fattr);

        i++;
    }


}


/**
 * \brief This check all the given command line arguements and use a pointer to the correct options
 *
 * \param len this is the number of command argruements passed to myfind
 * \param spath this is an array of pointers to save the list of search path specified by the user
 * \param parms this is  the *argv[]
 *
 * We cannot garantee that the first command option will be the search path
 * Flags isMemoryUsed is used to track the options of the commands.
 * If memory has not been used for an option(isMemoryUsed ==0) which beginns with '-'
 * then that can be the search path. otherwise (isMemoryUsed ==1) is not a correct command option.
 *
 * isOptions_set is used to limit or track the memory allocation for "linkedlist options"
 * Memory is only allocated for the set valid parameter options
 *
 * */

s_optns *process_parms(const int len, char *spath[], char **parms) {

    //parms p = {0};        // to prevent uninitialise message
    //option vp = {0};
    int index = 0, isOptions_set = 0, i,isMemoryUsed = 0;


    /* use calloc or memset op to prevent errors from valgrind passing uninitialise value to syscall*/
    s_optns *op = malloc(sizeof(*op));
    memset(op,0,sizeof(*op));
    s_optns *first = op;

    for (i = 1; (i < len); ++i) {

        if (isOptions_set == 1) {
            //op = op->next;
            op->next = calloc(1, sizeof(*op));
            if (op->next == NULL) {
                fprintf(stderr, "myfind: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }

            op = op->next;
            op->next = NULL;
        }


        if (strcmp(parms[i], "-name") == 0) {
            if (parms[++i]) {
                op->name = parms[i];
                isOptions_set = isMemoryUsed = 1;
                continue;
            } else {
                printf("myfind: missing argument to `%s`\n", parms[i - 1]);
                exit(EXIT_FAILURE);
            }

        } else if (strcmp(parms[i], "-help") == 0) {

            op->help = 1;
            isOptions_set = isMemoryUsed = 1;
            continue;

        } else if (strcmp(parms[i], "-print") == 0) {
            op->print = 1;
            isOptions_set = isMemoryUsed = 1;

            continue;

        } else if (strcmp(parms[i], "-ls") == 0) {
            op->ls = 1;
            isOptions_set = isMemoryUsed = 1;
            continue;


        } else if (strcmp(parms[i], "-type") == 0) {
            if (parms[++i]) {

                if (strlen(parms[i]) > 1) {
                    fprintf(stderr, "myfind: Arguments to -type should contain only one letter\n");
                    exit(EXIT_FAILURE);
                }

                char f = *(parms[i]);

                if (f == 'f' || f == 'b' || f == 'c' ||
                    f == 'd' || f == 's' || f == 'p' || f == 'l') {
                    op->f_type = f;
                    isOptions_set = isMemoryUsed = 1;
                    continue;
                } else {
                    fprintf(stderr, "myfind: Unknown argument to %s: %c\n", parms[i - 1], *parms[i]);
                    exit(EXIT_FAILURE);
                }


            } else {
                printf("myfind: missing argument to `%s`\n", parms[i]);
                exit(EXIT_FAILURE);
            }


        } else

            /*First confirm the user in the passwd database
             * getpwnam return NULL if no match is found in the database
             * if no match is found then check if is a digit and extract it as user_id
             * otherwise report error and exit failure
             * This also checks cases where the passwd database a username is a number
             * */

        if (strcmp(parms[i], "-user") == 0) {

            struct passwd *pd;
            if (parms[++i]) {
                op->user = parms[i];
                if ((pd = getpwnam(op->user))) {
                    op->user_id = pd->pw_uid;
                    isOptions_set = isMemoryUsed = 1;
                    continue;
                } else if (isdigit(parms[i][0])) {
                    sscanf(op->user, "%lu", &op->user_id);
                    isOptions_set = isMemoryUsed = 1;
                    continue;
                } else {
                    printf("myfind: `%s` is not a  name of a known user \n", parms[i]);
                    exit(EXIT_FAILURE);
                }


            } else {

                printf("myfind: missing argument to `%s`\n", parms[i]);
                exit(EXIT_FAILURE);

            }


        } else if(isMemoryUsed != 0){

            if(parms[i][0] == '-'){
                printf("myfind: Unknown predicate `%s`\n", parms[i]);
                exit(EXIT_FAILURE);
            }

            fprintf(stderr,"%s\n"
                    "Run: myfind -help for more information\n",ISSUE);
            exit(EXIT_FAILURE);


        } else {
            isOptions_set = 0;
        }



        /*Getting the path here.
        * */
        if (!isMemoryUsed && parms[i][0] != '-') {

            // size_t l = strlen(parms[i])+1;
            // spath[index] = malloc(sizeof(char) * l);

            //strcpy(spath[index], parms[i]);
            // spath[index][l + 1] = '\0';
            spath[index] = parms[i];
            index++;


        } else {
            printf("myfind: Unknown predicate `%s`\n", parms[i]);
            //docs/ doc1/ doc2/ -ls -type m -user james
            exit(EXIT_FAILURE);

        }

    }

    spath[index] = NULL;

    return first;
}


/** \brief emulating the -l in the linux command ls -l wth expected output format:
 * -rwxr-xr-x. 1 root root 3756 Feb  5 20:18 filename.extension
 * \param filename this is the path to apply -ls
 * \param file attributes from the stat struct
 * putting everything in the main function here as one method.
 * */
void print_ls(const char *filename, const struct stat *sb) {

    struct group *gp;
    struct passwd *pd;
    char *username;
    char *groupname;
    char ftpe;

    switch (sb->st_mode & S_IFMT) {
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


    /*Getting the groupname details*/
    gp = getgrgid(sb->st_gid);
    pd = getpwuid(sb->st_uid);
    if(pd == NULL){
        username = alloca(12);
        snprintf(username,12,"%u",sb->st_uid);

    } else{
        username = pd->pw_name;
    }


    if(gp == NULL){
        groupname = alloca(12);
        snprintf(groupname,12,"%u",sb->st_gid);
    } else{
        groupname = gp->gr_name;
    }



    /*Get a formated last modified date with ctime() in timesec format
     * removing the day at the beginning fo the time
     * and removing the '\n' which ctime() already adds from the string and additional millisec with -9 the length
     * the pointer ntime point to month instead of the initial day
     * PN: ctime() is a MT-Unsafe functions, that means is not safe to call/use in a multithreaded programm
     * in a multithreaded programm strftime() is recommended
     * */

    char *ntime = ctime(&sb->st_mtim.tv_sec) + 4;

    ntime[strlen(ntime) - 9] = '\0';


    /*Allocating memory for a null-terminated string for the file permission format
     * A maximun size to hold or the file permission bits inluding the SUID and the sticky-bit
     * using snprntf to store the results in the required formatted output
     * put a null-terminator at the end of the char array
     * */
    char *permstr = alloca(sizeof(char) * LEN);
    //char *permstr = malloc(sizeof(char) * LEN);

    snprintf(permstr, STR_SIZE, "%c%c%c%c%c%c%c%c%c%c", ftpe, (sb->st_mode & S_IRUSR) ? 'r' : '-',
             (sb->st_mode & S_IWUSR) ? 'w' : '-', (sb->st_mode & S_ISUID) ? (sb->st_mode & S_IXUSR ? 's' : 'S') :
                                                  (sb->st_mode & S_IXUSR ? 'x' : '-'),

             (sb->st_mode && S_IRGRP) ? 'r' : '-', (sb->st_mode & S_IWGRP) ? 'w' : '-',
             (sb->st_mode & S_ISGID) ? (sb->st_mode & S_IXGRP ? 's' : 'S') : (sb->st_mode & S_IXGRP ? 'x' : '-'),
             (sb->st_mode & S_IROTH) ? 'r' : '-', (sb->st_mode & S_IWOTH) ? 'w' : '-',
             (sb->st_mode & S_ISVTX) ? (sb->st_mode & S_IXOTH ? 't' : 'T') : (sb->st_mode & S_IXOTH ? 'x' : '-'));

    permstr[LEN - 1] = '\0';


    char *symlink = get_smlink(filename, sb);

    long long nblks = S_ISLNK(sb->st_mode) ? 0 : sb->st_blocks / 2; //find show half the blocks




    printf("%7lu %8lld %10s %3ld %-8s %-8s %8lu %12s  %s %s %s\n",
           sb->st_ino, nblks, permstr, sb->st_nlink,
           username, groupname, sb->st_size,
           ntime, filename, (symlink ? "->" : ""), (symlink ? symlink : ""));


    //free(permstr);
    free(symlink);

}

/*The other macros is not working so i decided to use this inline*/
inline char ftype(mode_t mode) {

    if (S_ISBLK(mode)) {
        return 'b';
    }

    if (S_ISCHR(mode)) {
        return 'c';
    }

    if (S_ISDIR(mode)) {
        return 'd';
    }

    if (S_ISFIFO(mode)) {
        return 'p';
    }

    if (S_ISREG(mode)) {
        return 'f';
    }

    if (S_ISLNK(mode)) {
        return 'l';
    }

    if (S_ISSOCK(mode)) {
        return 's';
    }

    return '?';
}


void do_file(char *file_path, s_optns *p, struct stat *attr) {

    int isPrintOrls_set = 0;

    do {
        if (p->f_type) {
            if (ftype(attr->st_mode) != p->f_type) { //todo: it works
                return;
            }

        }

        if (p->name) {
            char *f = basename(file_path);   //todo: it works

            if (fnmatch(p->name, f, 0) != 0) {
                return;
            }

        }

        if (p->user) {                  //Todo: it works

            #if 0
            struct passwd *pd;
            pd = getpwuid(attr->st_uid);

            if (strcmp(p->user, pd->pw_name) != 0) {
                return;

            } else

            #endif

            if (p->user_id != attr->st_uid) {

                return;

            }
        }
        if (p->print) {                       //todo: works fine
            printf("%s\n", file_path);
            isPrintOrls_set = 1;
        }

        if (p->ls) {                        //todo:it works fine
            print_ls(file_path, attr);
            isPrintOrls_set = 1;
        }


        p = p->next;

    } while (p != NULL);


    if (isPrintOrls_set == 0) {             //todo: i use it in case there is no parameter set, example find or myfind
        printf("%s\n", file_path);
    }


}

/** \brief
 * gathering informations about the given directory and print them out:
 * example of test-output "inode number: [1587860]	-> file: [mail]"
 *
 * this is just to test my do_file, todo: robert is working on the final do_dir() version
 * for the file prject
 * */
void do_dir(char *dir_path, s_optns *params, struct stat *sb) {
    

}


/**\brief this is to clean the linkedlist for the options
 * \param pm a pointer to the address of the first struct
 * this is to clean the linkedlist for the options
 * full documentation will come soon
 *
 * */

void clean_parms(s_optns **pm) {

    /* if(*pm ==NULL){
         return;
     }
     clean_parms(&(*pm)->next);
     free((*pm)->name);
     free((*pm)->user);
     free(*pm);*/

    s_optns *first = *pm;
    while (first) {
        s_optns *temp = first->next;
        free(first);
        first = temp;
    }


}

/** \brief  gathering informations about the target of the symbolic link and return them in the aproparate format of "find":
 * example of return string "-> boot/vmlinuz-4.4.0-64-generic"
 *
 *\param file_path this is the path to the linked_path
 * \param attr the attributes of the given/ passed file_path
 * */
char *get_smlink(const char *file_path, const struct stat *attr) {

    char *sym_link = NULL;
    ssize_t r, bufsiz;;


    bufsiz = attr->st_size + 1;

    if (S_ISLNK(attr->st_mode)) {
        if (attr->st_size == 0)
            bufsiz = PATH_MAX;


        sym_link = malloc(sizeof(char) * bufsiz);
        if (sym_link == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }




        while ((r = readlink(file_path, sym_link, bufsiz)) > 1 && (r > bufsiz)) {
            bufsiz *= 2;
            if ((sym_link = realloc(sym_link, sizeof(char) * bufsiz)) == NULL) {
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

