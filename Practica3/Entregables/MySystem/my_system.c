#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char* argv[])
{
	if (argc!=2){
		fprintf(stderr, "Usage: %s <command>\n", argv[0]);
		exit(1);
	}

	pid_t pid = fork();

	if(pid == -1){//error en el fork
		fprintf(stderr, "Error en el fork\n" );
		return -1;
	}
	else if(pid == 0){ //proceso hijo
		execl("/bin/bash", "bash", "-c",argv[1], (char*)NULL) ;

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

