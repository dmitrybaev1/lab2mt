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
}globalArgs;
globalArgs args={0,0,0,0,0,NULL,0};
const char *optstring="nvETs";
char src[256];
int squeezeflag=0;
int numberflag=1;
void readfile(char* filename);
void print(unsigned char c,int *i);
//int checkstdi(char *arg);
int main(int argc, char *argv[]) {
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
                default:
                    break;
            }
            opt = getopt(argc, argv, optstring);
        }
        args.files = argv + optind;
        args.numfiles = argc - optind;
        for (int i = 0; i < args.numfiles; i++) {
            readfile(args.files[i]);
        }
    } else
        readfile("-");
   /* if(argc>1) {
        for (int i = 1; i < argc; i++) {//ПЕРЕБОР ПАРАМЕТРОВ
            if(i==1&&argc!=2) {
                if (strlen(argv[i]) == 2) {//ВОЗМОЖНО, ЗДЕСЬ КЛЮЧИ
                    if (strcmp(argv[i],"-E")==0)
                        showends = 1;
                    else if (strcmp(argv[i],"-n")==0)
                        number = 1;
                    else if (strcmp(argv[i],"-s")==0)
                        squeezeblank = 1;
                    else if (strcmp(argv[i],"-T")==0)
                        showtabs = 1;
                    else if (strcmp(argv[i],"-v")==0)
                        shownonprinting = 1;
                    else
                        readfile(argv[i]);
                }
                else {
                    readfile(argv[i]);
                }
            }
            else{
                readfile(argv[i]);
            }
        }
    }
    else{
        readfile("-");
    }*/
    return 0;
}

void printnonprinting(unsigned char c){
    if(c>=0&&c<=37&&c!=9&&c!=10&&c!=12) {
        c = c+64;
        printf("%c", '^');
        printf("%c",c);
    }
    else if(c==177)
        printf("%s","^?");
    else
        printf("%c",c);
}
void printwithtabs(unsigned char c){
    if(c==9)
        printf("%s","^I");
    else
        printf("%c",c);
}
void printwithends(unsigned char c){
    if(c==10)
        printf("%s\n","$");
    else
        printf("%c",c);
}
void printwithnumberfile(unsigned char c, int *i){
    if (c == 10) {
        printf("\n%6d  ", *i);
        (*i)++;
    } else
        printf("%c", c);
}
void printwithnumberstdin(unsigned char c,int *i){
    if(numberflag){
        numberflag=0;
        printf("%6d  ", *i);
        printf("%c", c);
        (*i)++;
    } else
        printf("%c", c);
}
void printsqueezeblank(unsigned char c){
    if(squeezeflag > 1) {
        if (c != 10){
            printf("%c",c);
            squeezeflag=0;
        }
    }
    else{
        if(c==10)
            squeezeflag++;
        else
            squeezeflag=0;
        printf("%c",c);
    }
}
void print(unsigned char c, int *i){
    if(args.shownonprinting)
        printnonprinting(c);
    else if(args.showtabs)
        printwithtabs(c);
    else if(args.showends)
        printwithends(c);
    else if(args.number)
        printwithnumberfile(c, i);
    else if(args.squeezeblank)
        printsqueezeblank(c);
    else
        printf("%c",c);
}

void readfile(char* filename){
    int i=2;
    int j=1;
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
                if (args.shownonprinting)
                    printnonprinting(c);
                else if (args.showtabs)
                    printwithtabs(c);
                else if (args.showends)
                    printwithends(c);
                else if (args.number) {
                    printwithnumberstdin(c, &j);
                    if(c==10){
                        numberflag=1;
                    }
                }
                else if (args.squeezeblank)
                    printsqueezeblank(c);
                else
                    printf("%c", c);
            }
        }
    }
    else {//file
        if ((handle = open(filename, O_RDONLY)) == -1) {
            strcat(src, "mycat: ");
            strcat(src, filename);
            perror(src);
            exit(1);
        }
        if (args.number)
            printf("\n     1  ");
        while (!eof(handle)) {
            if (read(handle, &ch, 1) == -1) {
                strcat(src, "mycat: ");
                strcat(src, filename);
                perror(src);
            }
            print(ch, &i);
        }
        close(handle);
    }
}
