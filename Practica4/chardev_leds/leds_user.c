#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define PATH "/dev/leds"
#define ITERACIONES 1000

int main(){
    int fd = open(PATH, O_WRONLY);

    for(int i = 0; i < ITERACIONES; ++i){
        write(fd, "1", 1);
        sleep(1);
        write(fd, "2", 1);
        sleep(1);
        write(fd, "3", 1);
        sleep(1);

        write(fd,"12", 2);
        sleep(1);
        write(fd,"23", 2);
        sleep(1);
        write(fd,"31", 2);
        sleep(1);

        write(fd, "123", 3);
        sleep(3);
    }

    close(fd);

    return 0;
}