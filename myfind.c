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

/*
 * --------------------------------------------------------------- defines --
 */

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

void do_ls(const struct stat atrr);

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


    printf("I");

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


