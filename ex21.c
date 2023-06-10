//Yonatan Mandl 211399175
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {

    char *file1 = argv[1];
    char *file2 = argv[2];

    int fd1 = open(file1, O_RDONLY);
    int fd2 = open(file2, O_RDONLY);

    if (fd1 == -1 || fd2 == -1) {
        perror("Error in: open");
        exit(-1);
    }


    char buf1, buf2;
    int identity = 1;
    int similar=1;
    int read1,read2;
    int space1=0;
    int space2=0;

    while (1) {

        if(space1 && !space2)
        {
            read1=read(fd1, &buf1, 1);
        }
        else if(space2 && !space1)
        {
            read2=read(fd2, &buf2, 1);
        }
        else{
            read1=read(fd1, &buf1, 1);
            read2=read(fd2, &buf2, 1);
        }

        if (read1==-1 || read2==-1)
        {
            perror("Error in: read");
            exit(-1);
        }

        if(read1==0 || read2==0)
        {
            break;
        }

        if(buf1!=buf2)
        {
            identity=0;
        }

        space1=0;
        space2=0;
        
        if (buf1 == ' ' || buf1 == '\n' || buf1 == '\r') {
            space1=1;
        }
        
        if (buf2 == ' ' || buf2 == '\n' || buf2 == '\r') {
            space2=1;
        }

        if(!space1 && !space2){
        if(tolower(buf1)!=tolower(buf2))
        {
            similar=0;
            break;
        }
        }
    }


if(read1!=0)
    {
        identity=0;
        while (buf1 == ' ' || buf1 == '\n' || buf1 == '\r') {
            read1 = read(fd1, &buf1, 1);
            if (read1 == 0) {
                break;
            }
            if (read1 == -1) {
                perror("Error in: read");
                exit(-1);
            }
        }
        if (!(buf1 == ' ' || buf1 == '\n' || buf1 == '\r'))
        {
            similar=0;
        }
    }

    if(read2!=0)
    {
        identity=0;
        while (buf2 == ' ' || buf2 == '\n' || buf2 == '\r') {
            read1 = read(fd2, &buf2, 1);
            if (read1 == 0) {
                break;
            }
            if (read1 == -1) {
                perror("Error in: read");
                exit(-1);
            }
        }
        if (!(buf2 == ' ' || buf2 == '\n' || buf2 == '\r'))
        {
            similar=0;
        }
    }



    if (close(fd1)==-1)
    {
        perror("Error in: close");
        exit(-1);

    }
    if (close(fd2)==-1)
    {
        perror("Error in: close");
        exit(-1);
    }
    
    if(identity){
        return 1;
    }
    else if (similar) {
        return 3;
    }
    else {
        return 2;
    }

    
}
