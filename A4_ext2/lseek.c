#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    int fd, len;
    char buf[128];
    fd = open(argv[1], O_RDONLY);
    len = read(fd, buf, 10);
    buf[len] = '\0';
    printf("%s\n", buf);
    lseek(fd, 2, SEEK_CUR);
    len = read(fd, buf, 10);
    buf[len] = '\0';
    printf("%s\n", buf);
    return 0;
}

