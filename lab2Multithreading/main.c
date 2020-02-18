#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#define BUF_SIZE 4096
//АРГУМЕНТЫ
typedef struct globalArgs_t{
    int number;// -n
    int shownonprinting;// -v
    int showends;// -E
    int showtabs;// -T
    int squeezeblank;// -s
    char **files;
    int numfiles;
}globalargs;
//ФЛАГИ ПЕЧАТИ
typedef struct flags_t{
    int numberflag;
    int nonprintingflag;
    int endsflag;
    int tabsflag;
    int squeezeblankflag;
}printflags;
//ОСТАЛЬНЫЕ ПЕРЕМЕННЫЕ
globalargs args={0, 0, 0, 0, 0, NULL, 0};
printflags keyflags={0, 0, 0, 0, 0};
const char *optstring="nvETs";
char src[256];
char *catname;
int squeezecount=0;
//ФЛАГИ
int squeezeflag1=0;
int squeezeflag2=0;
int numberflag=1;
int firstrowfileflag=1;
//int isstdinfirstinqueue=1;
//
//int i=0;
void readfromfile(char* filename);
void print(char* s, int bytes);
//
int main(int argc, char *argv[]) {
    catname=argv[0];
    int opt=0;
    if(argc>1) {
        opt = getopt(argc, argv, optstring);
        while (opt != -1) {
            switch (opt) {
                case 'n':
                    args.number = 1;
                    break;
                case 'v':
                    args.shownonprinting = 1;
                    break;
                case 'E':
                    args.showends = 1;
                    break;
                case 'T':
                    args.showtabs = 1;
                    break;
                case 's':
                    args.squeezeblank = 1;
                    break;
                case '?':
                    exit(1);
                    break;
            }
            opt = getopt(argc, argv, optstring);
        }
        args.files = argv + optind;
        args.numfiles = argc - optind;
        if(args.numfiles>0)
            for (int i = 0; i < args.numfiles; i++) {
                squeezecount=0;
                squeezeflag1=0;
                numberflag=1;
                readfromfile(args.files[i]);
            }
        else
            readfromfile("-");
    } else
        readfromfile("-");
    return 0;
}

void printrow(char* row,int count){
    write(STDOUT_FILENO,row,count);
}
char* getrow(int* i,int* count,char* s,int bytes){
    int k=1;
    int j;
    char* row = (char *) malloc(1);
    for (j=*i; j < bytes; j++) {
        if (j == *i) {
            row[0] = s[j];
            k++;
            if (s[j] == 10) {
                j++;
                break;
            }
        } else {
            row = (char *) realloc(row, k);
            row[k - 1] = s[j];
            k++;
            if (s[j] == 10) {
                j++;
                break;
            }
        }
    }
    *i=j;
    *count=k-1;
    return row;
}
void print(char* s, int bytes){//порядок if-ов важен!
   int i=0;
   int count;
   while (i<bytes) {
       //printf("i is %d, bytes is %d",i,bytes);
       char* row = getrow(&i,&count,s,bytes);
       printrow(row, count);
       free(row);
   }
}
void file_err(char* filename){
    strcat(src, catname);
    strcat(src,": ");
    strcat(src, filename);
    perror(src);
    exit(1);
}
void readfromfile(char* filename){
    int handle;
    char buf[BUF_SIZE];
    int bytes;
    if(strcmp(filename,"-")!=0) {//file
        if ((handle = open(filename, O_RDONLY)) == -1) {
            file_err(filename);
        }
        while ((bytes = read(handle, buf, BUF_SIZE)) > 0) {
            print(buf, bytes);
        }
        if (bytes == -1)
            file_err(filename);
        close(handle);
    }
    else{//stdin
        while ((bytes = read(STDIN_FILENO, buf, BUF_SIZE)) > 0) {
            print(buf, bytes);
        }
        if (bytes == -1)
            file_err(filename);
    }
}
