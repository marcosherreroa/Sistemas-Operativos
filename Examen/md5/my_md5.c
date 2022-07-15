#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

char* loadstr(FILE * file)
{
	char* string = NULL;
	long int checkpoint = ftell(file);

	int size = 1, read; //empiezo con size en 1 para contar también \0
	while ((read = getc(file)) != EOF && read != '\0')++size;

	if(!ferror(file)){
        fseek(file, checkpoint, SEEK_SET);
		string = malloc(size);
		for (int i = 0; i < size; ++i) string[i] =  getc(file);
	}
	
	return string;
	// Completed
}

int my_system(char * command){
	pid_t pid = fork();

	if(pid == -1){//error en el fork
		fprintf(stderr, "Error en el fork\n" );
		return -1;
	}
	else if(pid == 0){ //proceso hijo
	    printf("%s\n",command);
		execl("/bin/bash", "bash", "-c",command, (char*)NULL) ;

		fprintf(stderr, "Error en el exec\n");
		return -1;
	}
	else{//proceso padre
		int status = 0;
		if( wait(&status) == -1){
			fprintf(stderr, "Error en el wait\n");
		};

		return status;
	}
}

int get_md5sum (char * md5, char * fname){
    FILE * f;

	char * command = malloc(100);
	snprintf(command, 100, "md5sum %s | sed -z 's/\\([a-f0-9]*\\) .*/\\1/' > tmpfile", fname);
    my_system(command);
	if ((f = fopen("tmpfile", "r")) == NULL) return -1;
	fgets(md5, 33, f);
	

    return 0;
}

int main(int argc, char* argv[])
{
	char md5[33];
	get_md5sum((char *)md5, argv[0]);
	printf("my md5sum is %s\n", (char *)md5);

}

