#define _XOPEN_SOURCE 220112L
#define _POSIX_C_SOURCE 200112L
#define _GNU_SOURCE


/**
 * @file myfind.c
 *
 * Beispiel 1
 *@brief This is a simple GNU-like Find command.
 * A clone of the Find -> myfind with 5 implementation of the find options
 *
 * @author Dennis Addo <ic16b026>
 * @author Robert Niedermaier <ic16b089>
 * @details More information about the project can be found here URL: https://cis.technikum-wien.at/documents/bic/2/bes/semesterplan/lu/beispiel1.html
 *
 * @date 16/02/2017
 *
 * @version 1.0
 *
 * @todo All implentations must be contained in one method structure unless other pieces of functions
 * @todo are required by other programs/methods
 * @todo ALL errors must be the GNU C EXIT_SUCCESS and FAILURE macros
 * @todo perror and sterror is used to display error informations, functions and error-types
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
#define LEN 12
#define ISSUE "Usage: myfind <file or directory> [ <options> ] ..."


/** \typedef
 * -------------------------------------------------------------- typedefs --
 *we are saving all the specififed options in a linked-list so that i can be efficient to access it
 * and set or store special inforation for later use in the do_file() function
 * there will be no memory allocation for the char pointers, we point to the
 * already set and given memory from argv[]
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

void do_file(char *fp_path, s_optns *parms, struct stat *atrr);

void do_dir(char *dp_path, s_optns *parms, struct stat sb);

s_optns *process_parms(const int len, char *spath[], char **parms);

char *get_smlink(const char *file_path, const struct stat *attr);

void print_ls(const char *file, const struct stat *atrr);

void startMyFind(char **p, s_optns *option1);

char ftype(mode_t mode);

void clean_parms(s_optns **pm);


/**
  *
  * \brief This is a clone of the GNU find command in pure c

  * \param argc the number of arguments
  * \param argv the arguments itselves (including the program name in argv[0])
  *
  * \return always "success"
  * \retval 0 always
  *
  */


int main(int argc, char *argv[]) {


    char *path_list[argc];
    memset(path_list, 0, argc);   // this is to prevent errors from valgrind


    s_optns *p = process_parms(argc, path_list, argv);

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
 * this initialise the whole myfind command
 * when no path is specified the the current working directory (cwd) is the default search path
 * We go through all the given start paths and pass it to the do_file() and if directory do_dir()
 *
 *
 * */
void startMyFind(char *path[], s_optns *op) {

    struct stat fattr;
    int ret = 0, i = 0;
    char cwd[] = "./";


    if (path[i] == NULL) {
        path[i] = cwd;
        path[i + 1] = NULL;
    }

    while (path[i]) {

        ret = lstat(path[i], &fattr);
        if (ret == -1) {
            /* is alright to fail for one or many paths given, but is not ok for you to stop there, report the error and continue buddy*/
            fprintf(stderr, "myfind: (%s): %s\n", path[i], strerror(errno));

        } else {

            do_file(path[i], op, &fattr);

            if (S_ISDIR(fattr.st_mode))
                do_dir(path[i], op, fattr);

        }
        i++;
    }


}


/**
 * \brief This check all the given command line arguements and use a pointer to the correct options
 *
 * \param len this is the number of command argruements passed to myfind
 * \param spath this is an array of pointers to save the list of search path passed to the myfind program
 * \param parms this is  the *argv[]
 *
 * We cannot garantee that the first command option will be the search path (there could be no given paths)
 * Flags isMemoryUsed is used to track the options of the commands.
 * If memory has not been used for an option(isMemoryUsed ==0) which beginns with '-'
 * then that can be the search path. otherwise (isMemoryUsed ==1) is not a correct command option.
 *
 * isOptions_set is used to limit or track the memory allocation for "linkedlist options"
 * Memory is only allocated for the set valid parameter options
 *
 * */

s_optns *process_parms(const int len, char *spath[], char **parms) {

    int index = 0, isOptions_set = 0, i, isMemoryUsed = 0;


    s_optns *op = malloc(sizeof(*op));
    memset(op, 0, sizeof(*op));
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
                fprintf(stderr, "myfind: missing argument to `%s`\n", parms[i - 1]);
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

                if (f == 'f' || f == 'b' || f == 'c'
                    || f == 'd' || f == 's' || f == 'p'
                    || f == 'l') {

                    op->f_type = f;
                    isOptions_set = isMemoryUsed = 1;
                    continue;
                } else {
                    fprintf(stderr, "myfind: Unknown argument to %s: %c\n", parms[i - 1], *parms[i]);
                    exit(EXIT_FAILURE);
                }


            } else {
                fprintf(stderr, "myfind: missing argument to `%s`\n", parms[i]);
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
                pd = getpwnam(op->user);
                if (pd != NULL) {
                    op->user_id = pd->pw_uid;
                    isOptions_set = isMemoryUsed = 1;
                    continue;
                } else if (isdigit(parms[i][0])) {
                    sscanf(op->user, "%lu", &op->user_id);
                    isOptions_set = isMemoryUsed = 1;
                    continue;
                } else if (pd == NULL) {
                    fprintf(stderr, "myfind: `%s` is not the name of a known user \n", parms[i]);
                    // fprintf(stderr,"myfind: %s\n",strerror(errno));
                    exit(EXIT_FAILURE);
                }


            } else {

                fprintf(stderr, "myfind: missing argument to `%s`\n", parms[i]);
                exit(EXIT_FAILURE);

            }


        } else if (isMemoryUsed != 0) {

            if (parms[i][0] == '-') {
                fprintf(stderr, "myfind: Unknown predicate `%s`\n", parms[i]);
                exit(EXIT_FAILURE);
            }

            fprintf(stderr, "%s\n"
                    "Run: myfind -help for more information\n", ISSUE);
            exit(EXIT_FAILURE);


        } else {
            isOptions_set = 0;
        }



        /*Getting the path here.
        * */
        if (!isMemoryUsed && parms[i][0] != '-') {
            spath[index] = parms[i];
            index++;


        } else {
            fprintf(stderr, "myfind: Unknown predicate `%s`\n", parms[i]);
            exit(EXIT_FAILURE);

        }

    }

    spath[index] = NULL;

    return first;
}


/** \brief displaying  ls format with expected output format:
 * -rwxr-xr-x. 1 root root 3756 Feb  5 20:18 filename.extension
 * \param filename this is the path to apply -ls
 * \param file attributes from the stat struct
 * putting everything in the main function here as one method.
 *
 * */
void print_ls(const char *filename, const struct stat *sb) {


    struct group *gp;
    struct passwd *pd;
    char *username;
    char *groupname;

    char ftpe;

    switch (sb->st_mode & S_IFMT) {
        case S_IFREG: ftpe = '-'; break;
        case S_IFDIR: ftpe = 'd'; break;
        case S_IFBLK: ftpe = 'b'; break;
        case S_IFCHR: ftpe = 'c'; break;
        case S_IFIFO: ftpe = 'p'; break;
        case S_IFLNK: ftpe = 'l'; break;
        case S_IFSOCK:ftpe = 's'; break;
        default:
            ftpe = '?';
    }


    /**
     * getting the user and the group information
     * if user is NULL, then user_id will be the default username
     * */
    gp = getgrgid(sb->st_gid);
    pd = getpwuid(sb->st_uid);

    if (pd == NULL) {
        username = alloca(12);
        snprintf(username, 12, "%u", sb->st_uid);

    } else {
        username = pd->pw_name;
    }


    if (gp == NULL) {
        groupname = alloca(12);
        snprintf(groupname, 12, "%u", sb->st_gid);
    } else {
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
    // char *permstr = malloc(sizeof(char) * LEN);

    snprintf(permstr, LEN, "%c%c%c%c%c%c%c%c%c%c", ftpe,
             (sb->st_mode & S_IRUSR) ? 'r' : '-',
             (sb->st_mode & S_IWUSR) ? 'w' : '-',
             (sb->st_mode & S_ISUID) ? (sb->st_mode & S_IXUSR ? 's' : 'S') :
             (sb->st_mode & S_IXUSR ? 'x' : '-'),

             (sb->st_mode & S_IRGRP) ? 'r' : '-',
             (sb->st_mode & S_IWGRP) ? 'w' : '-',
             (sb->st_mode & S_ISGID) ? (sb->st_mode & S_IXGRP ? 's' : 'S') :
             (sb->st_mode & S_IXGRP ? 'x' : '-'),

             (sb->st_mode & S_IROTH) ? 'r' : '-',
             (sb->st_mode & S_IWOTH) ? 'w' : '-',
             (sb->st_mode & S_ISVTX) ? (sb->st_mode & S_IXOTH ? 't' : 'T') :
             (sb->st_mode & S_IXOTH ? 'x' : '-')
    );


    char *symlink = get_smlink(filename, sb);

    long long nblks = S_ISLNK(sb->st_mode) ? 0 : sb->st_blocks / 2;


    printf("%7lu %8lld %10s %3lu %-8s %-8s %8lu %12s  %s %s %s\n",
           sb->st_ino, nblks, permstr, (unsigned long) sb->st_nlink,
           username, groupname, sb->st_size,
           ntime, filename, (symlink ? "->" : ""),
           (symlink ? symlink : "")
    );


    // free(permstr);
    free(symlink);

}

/**
 * \brief The other macros is not working so i decided to use this inline
 * \param mode the file mode from the struct stat
 * */

char ftype(mode_t mode) {

    if (S_ISBLK(mode)) { return 'b'; }

    if (S_ISCHR(mode)) { return 'c'; }

    if (S_ISDIR(mode)) { return 'd'; }

    if (S_ISFIFO(mode)) { return 'p'; }

    if (S_ISREG(mode)) { return 'f'; }

    if (S_ISLNK(mode)) { return 'l'; }

    if (S_ISSOCK(mode)) { return 's'; }

    return '?';
}


/**\brief Checking all the specified options against each passed directory or file
 * The idea is simple, options are checked against the file_path using the attributes if it pass
 * we move to the next option in the linked-list if it fails then we can't continue
 * if one of the options fails then we move on to the next file by returning void
 * \param fp_path a pointer to the specified or passed in search file or given path to
 * test the list of options on.
 * */

void do_file(char *fp_path, s_optns *p, struct stat *attr) {

    int isPrintOrls_set = 0;

    do {
        if (p->f_type) {
            if (ftype(attr->st_mode) != p->f_type)
                return;


        }

        if (p->name) {
            char *f = basename(fp_path);

            if (fnmatch(p->name, f, 0) != 0)
                return;


        }

        if (p->user) {

            if (p->user_id != attr->st_uid)
                return;


        }
        if (p->print) {
            printf("%s\n", fp_path);
            isPrintOrls_set = 1;
        }

        if (p->ls) {
            print_ls(fp_path, attr);
            isPrintOrls_set = 1;
        }


        p = p->next;

    } while (p != NULL);


    if (isPrintOrls_set == 0)
        printf("%s\n", fp_path);


}

/** \brief
 * main task of this function is to gather informations about the given directory and process them:
 *\param dp_path the given directory to open a stream on
 * \param params a linked-list of all the specified options
 * \param sb the attributes of the passed dir_path
 * it is called once in main and processes the by commandline given object
 * it opens the Directory and goes rekursively trough all levels
 * it loops trough all objects in the directory
 * handle '..' and '.' to avoid endless loops
 * get attributes of actual object
 * handle if the given path has a '/' at the end
 * and calls recursivly do_dir() after do_file() if an directory found, else it only calls do_file()
 **/

void do_dir(char *dp_path, s_optns *params, struct stat sb) {

    DIR *dirp;
    struct dirent *dire;
    int ret = 0;


    errno = 0;
    dirp = opendir(dp_path);
    if (dirp == NULL) {
        fprintf(stderr, "myfind:%s %s\n", dp_path,strerror(errno));
        return;
    }

    errno = 0;
    while ((dire = readdir(dirp)) != NULL) {

        if ((strcmp(dire->d_name, ".")) == 0 || (strcmp(dire->d_name, "..")) == 0)
            continue;


        unsigned long pfadlaenge = strlen(dp_path) + strlen(dire->d_name);
        char pfad[pfadlaenge];

        if (dp_path[strlen(dp_path) - 1] == '/')
            sprintf(pfad, "%s%s", dp_path, dire->d_name);
        else
            sprintf(pfad, "%s/%s", dp_path, dire->d_name);


        ret = lstat(pfad, &sb);
        if (ret == -1) {
            fprintf(stderr, "myfind: (%s): %s\n", pfad, strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (S_ISDIR(sb.st_mode)) {
            do_file(pfad, params, &sb);
            do_dir(pfad, params, sb);
        } else
            do_file(pfad, params, &sb);


    }

    if (closedir(dirp) == -1) {
        fprintf(stderr, "myfind: %s %s \n", dp_path,strerror(errno));
        exit(EXIT_FAILURE);
    }

}


/**\brief this is to clean the linkedlist for the options
 * \param pm a pointer to the address of the first struct
 * this is to clean the linkedlist for the options
 * full documentation will come soon
 *
 * */

void clean_parms(s_optns **pm) {

    s_optns *first = *pm;
    while (first) {
        s_optns *temp = first->next;
        free(first);
        first = temp;
    }

}

/** \brief  getting the symbolic link from the the specified path and return a NULL-terminator string(char pointer)
 * if not link NULL is returned others a pointer to a NULL-terminating string(char pointer)
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
                fprintf(stderr,"Not enough memory to continue\n");
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
