# myfind
A GNU-like find command


This is a Uni Semester Porject from a group of two people. My partner and I are implenting or emulating a GNU-like Find command in linux. This is not to replace the GNU Find command. Imporve our linux Programming skills for the semester.
(Project Details and structure)[https://cis.technikum-wien.at/documents/bic/2/bes/semesterplan/lu/beispiel1.html]



* To run

```make
make all

//Clean up the object files

make distclean


//To create the documentation for the project, Doxygen and all the PDF files

make doc

```



* Usage

```bash
"Usage: myfind <file or directory> [ <options> ] ..."
                   "default path is the current directory if none is specified; default expression is -print"
                   "Options: (You can specify any of the following options)"
                   "       : -help                   Shows all necessary informations for this command"
                   "       : -user <name>/<uid>     file or directory belongig to specified user"
                   "       : -name <pattern>        file or directory with specified name"
                   "       : -type [bcdpfls]        All entry with specified file type"
                   "       : -print                 print entries with the path"
                   "       : -ls                    print entries with more details


```
