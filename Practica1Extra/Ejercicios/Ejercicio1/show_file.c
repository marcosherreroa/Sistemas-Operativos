#include <stdio.h>
#include <stdlib.h>
#include <err.h>

int main(int argc, char* argv[]) {
	FILE* file=NULL;
	
	if (argc!=3) {
		fprintf(stderr,"Usage: %s <file_name> <block_size>\n",argv[0]);
		exit(1);
	}
    
	int n = atoi(argv[2]);
	if (n <= 0) err(4,"El segundo argumento debe ser un entero positivo");

	/* Open file */
	if ((file = fopen(argv[1], "r")) == NULL)
		err(2,"The input file %s could not be opened",argv[1]);

	char * c = malloc(n);
	int ret;
	/* Read file byte by byte */
	int bytes = fread(c, 1, n, file);
	while (bytes != 0) {
		/* Print byte to stdout */
		ret = fwrite(c, 1, bytes, stdout);

		if (ret!= bytes){
			free(c);
			fclose(file);
			err(3,"fwrite() failed!!");
		}

		bytes = fread(c, 1, n, file);
	}
    
	free(c);
	fclose(file);
	return 0;
}
