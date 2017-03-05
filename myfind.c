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
#include <errno.h>
#include <libgen.h>
#include <fnmatch.h>
#include <assert.h>

/*
 * --------------------------------------------------------------- defines --
 */
#define STR_SIZE sizeof("?rwxrwxrwx")
#define LEN 12

/*
 * -------------------------------------------------------------- typedefs --
 */

typedef struct _alopts {
    char *name;
    int help;
    int print;
    char f_type;
    int ls;
    char *user;                     //	<name>/<uid> expt: user name can be a number too
    unsigned long user_id;
    struct _alopts *next;
} s_optns;

/*
 * --------------------------------------------------------------- globals --
 */

/*
 * ------------------------------------------------------------- functions --
 * */

void print_help(void);

void do_file(char *file_path, s_optns *parms, struct stat *atrr);

void do_dir(char *dir_path, s_optns *parms, struct stat sb);

s_optns *process_parms(const int len, char *spath[], char **parms);

char *get_smlink(const char *file_path, const struct stat *attr);

void print_ls(const char *file, const struct stat *atrr);

//int do_type(mode_t mode);

void startMyFind(char **p, s_optns *option1);

//int do_name(uid_t uid);
char ftype(mode_t mode);

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
    // clean_me(path_list);
    /* for (int i = 0; i < argc; ++i) {
         if(path_list[i] != NULL) {
             printf("%d---> %s\n",i,path_list[i]);
             free(path_list[i]);
         }
         //assert(path_list[i] == NULL);
     }*/


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


/**\brief
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
            fprintf(stderr, "myfind: lstat(%s): %s\n", path[i], strerror(errno));
            exit(EXIT_FAILURE);
        }

        do_file(path[i], op, &fattr);

        if (S_ISDIR(fattr.st_mode))
            do_dir(path[i], op, fattr);

        i++;
    }


}


/**
 * \brief This process all the passed in parameters from the *argv[]
 * and set status for the reqired parameters in the struct.
 * If in any case a wrong parameter is found EXIT_FAILURE otherwise continue to
 * the next parameter
 * */

s_optns *process_parms(const int len, char *spath[], char **parms) {

    //parms p = {0};        // to prevent uninitialise message
    //option vp = {0};
    int index = 0, flag = 0, i;


    s_optns *op = malloc(sizeof(*op));
    s_optns *first = op;

    for (i = 1; (i < len); ++i) {

        if (flag == 1) {
            //op = op->next;
            op->next = calloc(1, sizeof(*op));
            if (op->next == NULL) {
                fprintf(stderr, "myfind: calloc(): %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }

            op = op->next;
            op->next = NULL;
        }


        if (strcmp(parms[i], "-name") == 0) {
            if (parms[++i]) {
                op->name = parms[i];
                flag = 1;
                continue;
            } else {
                printf("myfind: missing argument to `%s`\n", parms[i - 1]);
                exit(EXIT_FAILURE);
            }

        } else if (strcmp(parms[i], "-help") == 0) {

            op->help = 1;
            flag = 1;
            continue;

        } else if (strcmp(parms[i], "-print") == 0) {
            op->print = 1;
            flag = 1;

            continue;

        } else if (strcmp(parms[i], "-ls") == 0) {
            op->ls = 1;
            flag = 1;
            continue;


        } else if (strcmp(parms[i], "-type") == 0) {
            char f = *(parms[++i]);
            if (f == 'f' || f == 'b' || f == 'c' ||
                f == 'd' || f == 's' || f == 'p' || f == 'l') {
                op->f_type = f;
                flag = 1;
                continue;
            } else {
                printf("myfind: Unknown argument to %s: %c\n", parms[i - 1], *parms[i]);
                exit(EXIT_FAILURE);
            }


        } else

            /*First confirm the user in the passwd database
             * getpwnam return NULL if no match is found in the database
             * if no match is found then check if is a digit and extract it as user_id
             * otherwise report error and exit failure
             * This check cases where in the passwd database a username is a number
             * */

        if (strcmp(parms[i], "-user") == 0) {

            struct passwd *pd;
            if (parms[++i]) {
                op->user = parms[i];
                if ((pd = getpwnam(op->user))) {
                    op->user_id = pd->pw_uid;
                    flag = 1;
                    continue;
                } else if (isdigit(parms[i][0])) {
                    sscanf(op->user, "%lu", &op->user_id);
                    flag = 1;
                    continue;
                } else {
                    printf("myfind: `%s` is not a  name of a known user \n", parms[i]);
                    exit(EXIT_FAILURE);
                }


            } else {

                printf("myfind: missing argument to `%s`\n", parms[i]);
                exit(EXIT_FAILURE);

            }


        } else {
            flag = 0;
        }



        /*Getting the path here.
        * */
        if (parms[i][0] != '-') {

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


/** \brief
 * emulating the -l in the linux command ls -l wth expected output format:
 * -rwxr-xr-x. 1 root root 3756 Feb  5 20:18 filename.extension
 * putting everything in the main function here as one method.
 * */
void print_ls(const char *filename, const struct stat *sb) {

    struct group *gp;
    struct passwd *pd;
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


    /*Getting the group details*/
    gp = getgrgid(sb->st_gid);
    pd = getpwuid(sb->st_uid);



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




    printf("%7lu %8lld %10s %3li %-8s %-8s %8lld %12s  %s %s %s\n",
           sb->st_ino, nblks, permstr, sb->st_nlink,
           pd->pw_name, gp->gr_name, (long long) sb->st_size,
           ntime, filename, (symlink ? "->" : ""), (symlink ? symlink : ""));


    //free(permstr);
    free(symlink);

}

/*The other macros is not working so i decided to use this macros*/
char ftype(mode_t mode) {

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

    int flag = 0;

    do {
        if (p->f_type) {
            if (ftype(attr->st_mode) != p->f_type) { //todo: this works
                return;
            }

        }

        if (p->name) {
            char *f = basename(file_path);   //todo: it works now

            if (fnmatch(p->name, f, 0) != 0) {
                return;
            }

        }

        if (p->user) {                  //Todo: I have not tested yet
            struct passwd *pd;
            pd = getpwuid(attr->st_uid);

            if (strcmp(p->user, pd->pw_name) != 0) {
                return;

            } else if (p->user_id != attr->st_uid) {

                return;

            }
        }
        if (p->print) {                       //todo: works fine
            printf("%s\n", file_path);
            flag = 1;
        }

        if (p->ls) {                        //todo:it works fine
            print_ls(file_path, attr);
            flag = 1;
        }


        p = p->next;

    } while (p != NULL);


    if (flag == 0) {             //todo: i use it in case there is no parameter set, example find or myfind
        printf("%s\n", file_path);
    }


}

/** \brief
 * gathering informations about the given directory and print them out:
 * example of test-output "inode number: [1587860]	-> file: [mail]"
 *
 * this is just to test my do_file, robert is working on the right do_dir()
 * for the file prject
 * */
void do_dir(char *dir_path, s_optns *params, struct stat sb) {
    DIR *dir;
    struct dirent *e;
    size_t len = 0;
    char *sl = "";
    char *new_path = NULL;


    if ((dir = opendir(dir_path)) == NULL) {
        fprintf(stderr, "myfind: opendir(%s): %s\n", dir_path, strerror(errno));
        return; //exit(EXIT_FAILURE);                                       /*go to the next dir*/
    }

    while ((e = readdir(dir)) != NULL) {
        /* skipping all current directory and previous directory '.' and '..' */
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) {
            continue;
        }

        len = strlen(dir_path);

        if (dir_path[len - 1] != '/') {
            sl = "/";
        }

        len += strlen(e->d_name) + 2;
        new_path = malloc(sizeof(char) * len);

        if (!new_path) {
            fprintf(stderr, "malloc: malloc(): %s\n", strerror(errno));
            break;
        }

        //snprintf(full_path, length, "%s%s%s", path, slash, entry->d_name)
        snprintf(new_path, len, "%s%s%s", dir_path, sl, e->d_name);


        if (lstat(new_path, &sb) == 0) {

            do_file(new_path, params, &sb);


            if (S_ISDIR(sb.st_mode)) { // recursive call for all subdirs
                //printf("dir--> %s\n",new_path);
                do_dir(new_path, params, sb);
            }


        } else {
            fprintf(stderr, "myfind: lstat(%s): %s\n", new_path, strerror(errno));
            free(new_path);
            new_path = NULL;
            continue;
        }

        free(new_path);
        new_path = NULL;
    }

    if (closedir(dir) != 0) {
        fprintf(stderr, "myfind: closedir(%s): %s\n", dir_path, strerror(errno));
        exit(EXIT_FAILURE);
    }

}


/**\brief
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

/** \brief
 * gathering informations about the target of the symbolic link and return them in the aproparate format of "find":
 * example of return string "-> boot/vmlinuz-4.4.0-64-generic"
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

