#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
typedef struct globalArgs_t{
    int number;// -n
    int shownonprinting;// -v
    int showends;// -E
    int showtabs;// -T
    int squeezeblank;// -s
    char **files;
    int numfiles;
}globalargs;
typedef struct flags_t{
    int numberflag;
    int nonprintingflag;
    int endsflag;
    int tabsflag;
    int squeezeblankflag;
}printflags;
globalargs args={0, 0, 0, 0, 0, NULL, 0};
printflags flags={0,0,0,0,0};
const char *optstring="nvETs";
char src[256];
char *name;
int squeezecount=0;
int squeezeflag1=0;
int squeezeflag2=0;
int numberflag=1;
int firstrowflag=1;
int i=1;//file
int j=1;//stdin
void readfile(char* filename);
void print(unsigned char c);
int main(int argc, char *argv[]) {
    name=argv[0];
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
        for (int i = 0; i < args.numfiles; i++) {
            squeezecount=0;
            squeezeflag1=0;
            numberflag=1;
            readfile(args.files[i]);
        }
    } else
        readfile("-");
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
int printwithnumberfile(unsigned char c){
    if(firstrowflag){
        printf("%6d  ", i);
        i++;
        firstrowflag=0;
    }
    if (c == 10) {
        if(!squeezeflag2) {
            if (!flags.endsflag)
                write(STDOUT_FILENO, &c, 1);
            printf("%6d  ", i);
            i++;
        }
        squeezeflag2=0;
        return 1;
    }
    return 0;
}
int printwithnumberstdin(unsigned char c){
    if(numberflag){
        numberflag=0;
        printf("%6d  ", i);
        write(STDOUT_FILENO,&c,1);
        i++;
        return 1;
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
void resetflags(){
    flags.nonprintingflag=0;
    flags.tabsflag=0;
    flags.endsflag=0;
    flags.numberflag=0;
    flags.squeezeblankflag=0;
}
void print(unsigned char c){//порядок if-ов важен!
    if(args.squeezeblank)
        flags.squeezeblankflag=printsqueezeblank(c);
    if(args.shownonprinting)
        flags.nonprintingflag=printnonprinting(c);
    if(args.showtabs)
        flags.tabsflag=printwithtabs(c);
    if(args.showends)
        flags.endsflag=printwithends(c);
    if(args.number)
        flags.numberflag=printwithnumberfile(c);
    if(!flags.nonprintingflag&&!flags.tabsflag&&!flags.endsflag&&!flags.numberflag&&!flags.squeezeblankflag)
        write(STDOUT_FILENO,&c,1);
    resetflags();
}
void printstdin(unsigned char c){
    if (args.shownonprinting)
        printnonprinting(c);
    else if (args.showtabs)
        printwithtabs(c);
    else if (args.showends)
        printwithends(c);
    else if (args.number) {
        printwithnumberstdin(c);
        if(c==10){
            numberflag=1;
        }
    }
    else if (args.squeezeblank)
        printsqueezeblank(c);
    else
        write(STDOUT_FILENO,&c,1);
}
void file_err(char* filename){
    strcat(src,name);
    strcat(src,": ");
    strcat(src, filename);
    perror(src);
    exit(1);
}
void readfile(char* filename){
    int handle;
    unsigned char ch;
    if(strcmp(filename,"-")==0){//stdin
        unsigned char c;
        int n;
        int count=0;
        while ((n = read(STDIN_FILENO, &c, 1)) >= 0) {
            count++;
            if (n == 0) {
                break;
            } else {
                printstdin(c);
            }
        }
    }
    else {//file
        if ((handle = open(filename, O_RDONLY)) == -1) {
            file_err(filename);
        }
        /*if (args.number&&firstrowflag) {
            write(STDOUT_FILENO, "     1  ", 9);
            firstrowflag=0;
        }*/
        while (!eof(handle)) {
            if (read(handle, &ch, 1) == -1) {
                file_err(filename);
            }
            print(ch);
        }
        close(handle);
    }
}
