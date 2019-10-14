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
copynFile(FILE * origin, FILE * destination, int nBytes)
{
	// Complete the function
	if (origin == NULL || destination == NULL)
		return -1;

	int size = 0, c;
	
	do {
		c = getc(origin);
		if (c == EOF)
			break;
		putc((unsigned char) c, destination);
		size++;
	} while (size < nBytes && c != EOF);
	return (size);
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
char* loadstr(FILE * file)
{
	int n, size = 0;
	char* buf;

	do {
		n = getc(file);
		size++;
	} while ((n != (int) '\0')
		 && (n != EOF));

	if (n == EOF)
		return NULL;

	if ((buf = (char *) malloc(size)) == NULL)
		return NULL;

	fseek(file, -size, SEEK_CUR);

	fread(buf, 1, size, file);

	return buf;
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
	fread(nFiles, sizeof(int), 1, tarFile);
	stHeaderEntry * headers;

	headers = (stHeaderEntry *) malloc(sizeof(stHeaderEntry) * (*nFiles));

	for(int i = 0; i < *nFiles; ++i){
		headers[i].name = loadstr(tarFile);
		fread(&headers[i].size, sizeof(int), 1, tarFile);
	}


	return headers;
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
	FILE * f = fopen(tarName, "w");

	unsigned int hs;
	hs = sizeof(int);
	stHeaderEntry * headers = malloc(sizeof(stHeaderEntry) * nFiles);

	for (int i = 0; i < nFiles; ++i){
		int ns = strlen(fileNames[i]) + 1;
		headers[i].name = malloc(ns);
		strcpy(headers[i].name, fileNames[i]);
		hs += ns + sizeof(headers->size);
	}

	fseek(f, hs, SEEK_SET);

	for (int i = 0; i < nFiles; ++i){
		FILE * fi = fopen(fileNames[i], "r");
		headers[i].size = copynFile(fi, f, INT_MAX);
		fclose(fi);
	}

	fseek(f, 0, SEEK_SET);
	fwrite(&nFiles, sizeof(int), 1, f);
	for(int i = 0; i < nFiles; ++i){
		fwrite(headers[i].name, 1, strlen(headers[i].name) + 1, f);
		fwrite(&headers[i].size, sizeof(headers[i].size), 1, f);
	}

	for(int i = 0; i < nFiles; ++i)
		free(headers[i].name);
	free(headers);

	fclose(f);
	

	return EXIT_SUCCESS;
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
	FILE * f;
	f = fopen(tarName, "r");
	int nFiles;
	
	stHeaderEntry * heads = readHeader(f, &nFiles);

	for(int i = 0; i < nFiles; ++i){
		FILE * fi = fopen(heads[i].name, "w");
		copynFile(f, fi, heads[i].size);
		fclose(fi);
	}

	return EXIT_FAILURE;
}

int 
listTar(char tarName[])
{
	FILE * f;
	f = fopen(tarName, "r");
	int nFiles;
	
	stHeaderEntry * heads = readHeader(f, &nFiles);

	for(int i = 0; i < nFiles; ++i){
		printf("%s %d\n", heads[i].name, heads[i].size);
	}

	return 0;
}