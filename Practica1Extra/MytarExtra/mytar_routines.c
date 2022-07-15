#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mytar.h"

extern char *use;

/** Copy nBytes bytes from the origin file to the destination file.
 *
 * origin: pointer to the FILE descriptor associated with the origin file
 * destination:  pointer to the FILE descriptor associated with the destination file
 * nBytes: number of bytes to copy
 *
 * Returns the number of bytes actually copied or -1 if an error occured.
 */
int
copynFile(FILE * origin, FILE * destination, int nBytes, unsigned short * crc)
{
	int writtenBytes = 0;
	int read, written = 0;
	unsigned short sum1 = 0, sum2 = 0;
	while (writtenBytes < nBytes && (read = getc(origin)) != EOF && written != EOF){
		sum1 = (sum1 + (unsigned short) read) % 255;
		sum2 = (sum2 + sum1) % 255;

        written = putc(read, destination);

		if (written != EOF) ++writtenBytes;
	}

	if (ferror (origin) == 0 && ferror(destination) == 0){
		*crc = (sum2 << 8)| sum1;
		return writtenBytes;
	} 

    else return -1;
	//Completed
}

/** Loads a string from a file.
 *
 * file: pointer to the FILE descriptor 
 * 
 * The loadstr() function must allocate memory from the heap to store 
 * the contents of the string read from the FILE. 
 * Once the string has been properly built in memory, the function returns
 * the starting address of the string (pointer returned by malloc()) 
 * 
 * Returns: !=NULL if success, NULL if error
 */
char*
loadstr(FILE * file)
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

/** Read tarball header and store it in memory.
 *
 * tarFile: pointer to the tarball's FILE descriptor 
 * nFiles: output parameter. Used to return the number 
 * of files stored in the tarball archive (first 4 bytes of the header).
 *
 * On success it returns the starting memory address of an array that stores 
 * the (name,size) pairs read from the tar file. Upon failure, the function returns NULL.
 */
stHeaderEntry*
readHeader(FILE * tarFile, int *nFiles)
{
	stHeaderEntry* array = NULL;
	int nr_files = 0;

	int ret = 0;
	if (fread(&nr_files, sizeof(int), 1, tarFile) != 1)ret = -1;

	array = malloc(sizeof(stHeaderEntry)*nr_files);

	for(int i = 0; ret == 0 && i < nr_files; ++i){
		array[i].name = loadstr(tarFile);
		if (array[i].name == NULL)ret = -1;
		else if (fread(&array[i].size, sizeof(unsigned int), 1, tarFile) != 1) ret = -1;
		else if (fread(&array[i].crc, sizeof(unsigned short), 1, tarFile) != 1) ret = -1;
	}

    if (ret != 0) return NULL;
	else{
		(*nFiles) = nr_files;
		return array;
	} 
	
	// Completed
}

/** Creates a tarball archive 
 *
 * nfiles: number of files to be stored in the tarball
 * filenames: array with the path names of the files to be included in the tarball
 * tarname: name of the tarball archive
 * 
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First reserve room in the file to store the tarball header.
 * Move the file's position indicator to the data section (skip the header)
 * and dump the contents of the source files (one by one) in the tarball archive. 
 * At the same time, build the representation of the tarball header in memory.
 * Finally, rewind the file's position indicator, write the number of files as well as 
 * the (file name,file size) pairs in the tar archive.
 *
 * Important reminder: to calculate the room needed for the header, a simple sizeof 
 * of stHeaderEntry will not work. Bear in mind that, on disk, file names found in (name,size) 
 * pairs occupy strlen(name)+1 bytes.
 *
 */
int
createTar(int nFiles, char *fileNames[], char tarName[])
{
	FILE *origin ,* destination = NULL; int ret = EXIT_SUCCESS;

	if ((destination = fopen(tarName, "w")) == NULL) return EXIT_FAILURE;

	stHeaderEntry * infoHeader = malloc(nFiles*sizeof(stHeaderEntry));

	int headerSpace = sizeof(int) + nFiles*(sizeof(unsigned int)+sizeof(unsigned short)) + nFiles;
	for(int i = 0; i < nFiles; ++i){
		headerSpace += strlen(fileNames[i]);
	}

	fseek(destination, headerSpace, SEEK_SET);

	int fileSize = 0; unsigned short crc = 0;
	for(int i = 0; ret == EXIT_SUCCESS && i< nFiles; ++i){
		if((origin = fopen(fileNames[i], "r")) == NULL) ret = EXIT_FAILURE;

		else if ((fileSize = copynFile(origin, destination, INT_MAX, &crc)) == -1){
			ret = EXIT_FAILURE;	
			fclose(origin);	
		}

		else{
			infoHeader[i].name = fileNames[i];
			infoHeader[i].size = fileSize; 
			infoHeader[i].crc = crc;

			printf("[%d]: file %s, tam %u Bytes, CRC 0x%X\n", i, fileNames[i], fileSize, crc);

			fclose(origin);
		}
	}


	rewind(destination);
	if (fwrite(&nFiles, sizeof(int), 1, destination) != 1) ret = EXIT_FAILURE;

	if(ret == EXIT_SUCCESS){
	
		for(int i = 0; ret == EXIT_SUCCESS && i< nFiles; ++i){

			for(int j = 0; ret == EXIT_SUCCESS && j < strlen(infoHeader[i].name); ++j){
				if (putc(infoHeader[i].name[j], destination) == EOF) ret = EXIT_FAILURE; //se podria hacer con fwrite todo el string, ya que sabemos el tamaño
			}

			if(putc('\0', destination) == EOF) ret = EXIT_FAILURE;

			if (fwrite(&infoHeader[i].size, sizeof(unsigned int), 1, destination) != 1) ret = EXIT_FAILURE;
			if (fwrite(&infoHeader[i].crc, sizeof(unsigned short), 1, destination) != 1 ) ret = EXIT_FAILURE;
		}
	}

	free(infoHeader);
	fclose(destination);


	if (ret == EXIT_SUCCESS) printf("El fichero comprimido %s se ha creado con éxito.\n", tarName);
	else printf("No se ha podido crear el fichero comprimido %s\n", tarName);
	// Completed

	return ret;
}

/** Extract files stored in a tarball archive
 *
 * tarName: tarball's pathname
 *
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First load the tarball's header into memory.
 * After reading the header, the file position indicator will be located at the 
 * tarball's data section. By using information from the 
 * header --number of files and (file name, file size) pairs--, extract files 
 * stored in the data section of the tarball.
 *
 */
int
extractTar(char tarName[])
{
	FILE * origin, * destination = NULL; int ret = EXIT_SUCCESS;

	if ((origin = fopen(tarName, "r")) == NULL){
		printf("No se pudo abrir el fichero a descomprimir %s\n", tarName);
		return EXIT_FAILURE;
	} 

    int nFiles = 0;
	stHeaderEntry * header = readHeader(origin, &nFiles);
	if (header == NULL){
		printf("No se pudo leer la cabecera del fichero a descomprimir %s\n", tarName);
		fclose(origin);
		return EXIT_FAILURE;
	} 

	unsigned short crc = 0;
	for (int i = 0; i < nFiles; ++i){
		printf ("[%d]: Extrayendo %s, tam %u Bytes, CRC %X ...\n", i, header[i].name, header[i].size, header[i].crc);

		if ((destination = fopen(header[i].name, "w")) == NULL){
			printf("[%d]: Error de extracción: no se pudo abrir el fichero de escritura\n", i);
			ret = EXIT_FAILURE;
		}

		else if (copynFile(origin, destination, header[i].size, &crc) == -1){
			printf("[%d]: Error de extracción: no se pudo copiar la información al fichero de escritura\n", i);
			fclose(destination);
			ret = EXIT_FAILURE;
		}	

		else if (crc != header[i].crc){
			printf("[%d]: Error de extracción: CRC %X inesperado. La información extraída puede no ser correcta\n", i, crc);
			fclose(destination);
			ret = EXIT_FAILURE;
		} 

		else {
			printf("[%d]: Extracción correcta, CRC %X esperado\n", i, crc);
			fclose(destination);
		}
	}

	for(int i = 0; i< nFiles; ++i) free(header[i].name);
	free(header);
	fclose(origin);
	return ret;
}
