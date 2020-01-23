#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
int number=0;
int shownonprinting=0;
int showends=0;
int showtabs=0;
int squeezeblank=0;
char src[256];
int squeezeflag=0;
int numberflag=1;
void readfile(char* filename);
void print(char c,int *i);
//int checkstdi(char *arg);
int main(int argc, char *argv[]) {
    if(argc>1) {
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
    }
    return 0;
}
/*int checkstdi(char *arg){
    if (strlen(arg) == 1)
        if (strcmp(arg,"-")==0) {
            return 1;
        } else {
            return 0;
        }
    else
        return 0;
}*/

void printnonprinting(char c){
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
void printwithtabs(char c){
    if(c==9)
        printf("%s","^I");
    else
        printf("%c",c);
}
void printwithends(char c){
    if(c==10)
        printf("%s\n","$");
    else
        printf("%c",c);
}
void printwithnumberfile(char c, int *i){
    if (c == 10) {
        printf("\n%6d  ", *i);
        (*i)++;
    } else
        printf("%c", c);
}
void printwithnumberstdin(char c,int *i){
    if(numberflag){
        numberflag=0;
        printf("%6d  ", *i);
        printf("%c", c);
        (*i)++;
    } else
        printf("%c", c);
}
void printsqueezeblank(char c){
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
void print(char c, int *i){
    if(shownonprinting)
        printnonprinting(c);
    else if(showtabs)
        printwithtabs(c);
    else if(showends)
        printwithends(c);
    else if(number)
        printwithnumberfile(c, i);
    else if(squeezeblank)
        printsqueezeblank(c);
    else
        printf("%c",c);
}

void readfile(char* filename){
    int i=2;
    int j=1;
    int handle;
    char ch;
    if(strcmp(filename,"-")==0){//stdin
        char c;
        int n;
        int count=0;
        while ((n = read(STDIN_FILENO, &c, 1)) >= 0) {
            count++;
            if (n == 0) {
                break;
            } else {
                if (shownonprinting)
                    printnonprinting(c);
                else if (showtabs)
                    printwithtabs(c);
                else if (showends)
                    printwithends(c);
                else if (number) {
                    printwithnumberstdin(c, &j);
                    if(c==10){
                        numberflag=1;
                    }
                }
                else if (squeezeblank)
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
        if (number)
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
