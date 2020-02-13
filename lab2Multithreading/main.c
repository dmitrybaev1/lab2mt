#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
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
int isstdinfirstinqueue=1;
//
int i=0;
void readfromfile(char* filename);
void printfromfile(unsigned char c);
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

int printnonprinting(unsigned char c){
    if(c>=0&&c<=37&&c!=9&&c!=10&&c!=12) {
        c = c+64;
        write(STDOUT_FILENO,"^",1);
        write(STDOUT_FILENO,&c,1);
        return 1;
    }
    else if(c==177) {
        write(STDOUT_FILENO, "^?", 2);
        return 1;
    }
    return 0;
}
int printwithtabs(unsigned char c){
    if(c==9) {
        write(STDOUT_FILENO, "^I", 2);
        return 1;
    }
    return 0;
}
int printwithends(unsigned char c){
    if(c==10) {
        if(!squeezeflag1) {
            write(STDOUT_FILENO, "$", 1);
            write(STDOUT_FILENO, &c, 1);
        }
        squeezeflag1=0;
        return 1;
    }
    squeezeflag1=0;
    return 0;
}
int printwithnumberfromfile(unsigned char c){
    if(firstrowfileflag){
        i++;
        printf("%6d  ", i);
        firstrowfileflag=0;
    }
    if (c == 10) {
        if(!squeezeflag2) {
            if (!keyflags.endsflag)
                write(STDOUT_FILENO, &c, 1);
            i++;
            printf("%6d  ", i);
        }
        squeezeflag2=0;
        return 1;
    }
    return 0;
}
int printwithnumberfromstdin(unsigned char c){
    if(numberflag){
        numberflag=0;
        i++;
        printf("%6d  ", i);
        write(STDOUT_FILENO,&c,1);
        return 1;
    }
    if(c==10){
        numberflag=1;
    }
    return 0;
}
int printsqueezeblank(unsigned char c){
    if(squeezecount > 1) {
        if (c != 10){
            write(STDOUT_FILENO,&c,1);
            squeezecount=0;
        }
        else {
            squeezecount++;
            squeezeflag1=1;
            squeezeflag2=1;
        }
        return 1;
    }
    else{
        if(c==10)
            squeezecount++;
        else
            squeezecount=0;
        //write(STDOUT_FILENO,&c,1);
        return 0;
    }
}
void resetkeyflags(){
    keyflags.nonprintingflag=0;
    keyflags.tabsflag=0;
    keyflags.endsflag=0;
    keyflags.numberflag=0;
    keyflags.squeezeblankflag=0;
}
void printfromfile(unsigned char c){//порядок if-ов важен!
    if(args.squeezeblank)
        keyflags.squeezeblankflag=printsqueezeblank(c);
    if(args.shownonprinting)
        keyflags.nonprintingflag=printnonprinting(c);
    if(args.showtabs)
        keyflags.tabsflag=printwithtabs(c);
    if(args.showends)
        keyflags.endsflag=printwithends(c);
    if(args.number)
        keyflags.numberflag= printwithnumberfromfile(c);
    if(!keyflags.nonprintingflag && !keyflags.tabsflag && !keyflags.endsflag && !keyflags.numberflag && !keyflags.squeezeblankflag)
        write(STDOUT_FILENO,&c,1);
    resetkeyflags();
}
void printfromstdin(unsigned char c){
    if (args.squeezeblank)
        keyflags.squeezeblankflag=printsqueezeblank(c);
    if (args.shownonprinting)
        keyflags.nonprintingflag=printnonprinting(c);
    if (args.showtabs)
        keyflags.tabsflag=printwithtabs(c);
    if (args.showends)
        keyflags.endsflag=printwithends(c);
    if (args.number) {
        keyflags.numberflag=printwithnumberfromstdin(c);
    }
    if(!keyflags.nonprintingflag && !keyflags.tabsflag && !keyflags.endsflag && !keyflags.numberflag && !keyflags.squeezeblankflag)
        write(STDOUT_FILENO,&c,1);
    resetkeyflags();
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
    unsigned char ch;
    if(strcmp(filename,"-")==0){//stdin
        if(!isstdinfirstinqueue){
            i--;
        }
        isstdinfirstinqueue=1;
        firstrowfileflag=1;
        unsigned char c;
        int n;
        while ((n = read(STDIN_FILENO, &c, 1)) >= 0) {
            if (n == 0) {
                break;
            } else {
                printfromstdin(c);
            }
        }
    }
    else {//file
        isstdinfirstinqueue=0;
        if ((handle = open(filename, O_RDONLY)) == -1) {
            file_err(filename);
        }
        /*if (args.number&&firstrowfileflag) {
            write(STDOUT_FILENO, "     1  ", 9);
            firstrowfileflag=0;
        }*/
        while (!eof(handle)) {
            if (read(handle, &ch, 1) == -1) {
                file_err(filename);
            }
            printfromfile(ch);
        }
        close(handle);
    }
}
