#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#define BUF_SIZE 4096
typedef struct globalArgs_t{
    int number;// -n
    int shownonprinting;// -v
    int showends;// -E
    int showtabs;// -T
    int squeezeblank;// -s
    char **files;
    int numfiles;
}globalargs;
globalargs args={0, 0, 0, 0, 0, NULL, 0};
const char *optstring="nvETs";
int numberofrow=0;
int neednumber=1;
int emptyrow=0;
char src[256];
char *catname;
void readfromfile(char* filename);
void print(char* s, int bytes,char* filename);
void file_err(char* filename);

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
                readfromfile(args.files[i]);
            }
        else
            readfromfile("-");
    } else
        readfromfile("-");
    return 0;
}

void printrow(char* row,int count,char* filename){
    int tabsflag=0;
    int endsflag=0;
    int nonprintingflag=0;
    if(args.number){
        if(neednumber) {
            numberofrow++;
            printf("%6d  ", numberofrow);
        }
        else
            neednumber=1;
    }
    for(int i=0;i<count;i++){
        if(args.showtabs)
            if(row[i]==9) {
                if(write(STDOUT_FILENO, "^I", 2)==-1)
                    file_err(filename);
                tabsflag=1;
            }
        if(args.shownonprinting){
            unsigned char c = row[i];
            if(c<=37&&c!=9&&c!=10&&c!=12) {
                c = c + 64;
                if(write(STDOUT_FILENO, "^", 1)==-1)
                    file_err(filename);
                if(write(STDOUT_FILENO, &c, 1)==-1)
                    file_err(filename);
                nonprintingflag=1;
            }
            else if(c==177) {
                if(write(STDOUT_FILENO, "^?", 2)==-1)
                    file_err(filename);
                nonprintingflag=1;
            }
        }
        if(args.showends){
            if(row[i]==10){
                if(write(STDOUT_FILENO, "$\n", 2)==-1)
                    file_err(filename);
                endsflag=1;
            }
        }
        if(!endsflag&&!tabsflag&&!nonprintingflag)
            if(write(STDOUT_FILENO,&row[i],1)==-1)
                file_err(filename);
        tabsflag=0;
        endsflag=0;
        nonprintingflag=0;
    }
    if(row[count-1]!=10)//если последний символ строки не перенос
        neednumber=0;
}
char* getrow(int* i,int* count,char* s,int bytes){
    int k=1;
    int j;
    char* row = (char *) malloc(1);
    for (j=*i; j < bytes; j++) {
        if (j == *i) {
            row[0] = s[j];
            k++;
            if (s[j] == 10) {//empty row
                j++;
                emptyrow++;
                break;
            }
        } else {
            row = (char *) realloc(row, k);
            row[k - 1] = s[j];
            k++;
            if (s[j] == 10) {
                j++;
                emptyrow=0;
                break;
            }
        }
    }
    if(j==bytes){
        emptyrow=0;
    }
    *i=j;
    *count=k-1;
    return row;
}
void print(char* s, int bytes, char* filename){
   int i=0;
   int count;
   while (i<bytes) {
       char* row = getrow(&i,&count,s,bytes);
       if(args.squeezeblank){
           if(emptyrow<=1)
               printrow(row, count,filename);
       }
       else
           printrow(row, count,filename);
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
            print(buf, bytes,filename);
        }
        if (bytes == -1)
            file_err(filename);
        close(handle);
    }
    else{//stdin
        while ((bytes = read(STDIN_FILENO, buf, BUF_SIZE)) > 0) {
            print(buf, bytes,filename);
        }
        if (bytes == -1)
            file_err(filename);
    }
}
