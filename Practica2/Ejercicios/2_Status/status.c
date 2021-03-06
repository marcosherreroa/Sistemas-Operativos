#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>

typedef enum{
	NONE,
	FOLLOWLINKS
}flags;

char permissions[] = {'x', 'w', 'r'};

int status(char *, flags);

int main(int argc, char *argv[]) {
	// See if the number of arguments in the command line is correct
	if (argc < 2)
		fprintf(stderr, "Usage: %s [-L] files...\n", argv[0]), exit(-1);

	flags flag = NONE; int opt;

	//Parse command-line options
	while((opt = getopt(argc,argv,"L")) != -1){
		switch(opt){
			case 'L':
				flag = FOLLOWLINKS;
				break;
			default:
				fprintf(stderr, "Usage: %s [-L] files...\n", argv[0]), exit(-1);
				break;
		}
	}

	// Show the status of each file

	int i = 1;
	if (flag == FOLLOWLINKS) ++i;
	while(i<argc){
		status(argv[i], flag);
		++i;
	}

	exit(0);
}

int status(char *filename, flags flag) {
	struct stat buf;
	struct passwd *pw;
	struct group *gr;
	int i;

	// Fills buf with the stat structure containing file attributes. If the file is the file is a symbolic link and the option -L was given, we get 
	//the attributes for the referenced file. Otherwise, we get the link's attributes
	switch(flag){
		case FOLLOWLINKS:
			if (stat(filename, &buf) == -1)
				perror(filename), exit(-1);
			break;
		case NONE:
			if (lstat(filename, &buf) == -1)
				perror(filename), exit(-1);
			
			break;
	}

	
	printf("File: %s\n", filename);
	
	// The st_dev field describes the device on which this file resides. 
	// The following technique is useful to decompose the device ID (pair major-minor) in this field.
	printf("Resides in the device: %d, %d\n",
		(int)(buf.st_dev & 0xff00) >> 8, (int)buf.st_dev & 0x00ff);

	// Shows the inode number. With the previous value
	//	allows us to completely identify the file
	printf(" inode number: %ld\n", buf.st_ino);

	// The field 'mode' informs about the file type
	//	(S_ISxxx are bit masks)
	printf(" Type: ");
	switch(buf.st_mode & S_IFMT) {
        case S_IFREG:
            printf("regular.\n"); break;
        case S_IFDIR:
            printf("directory.\n"); break;
        case S_IFCHR:
            printf("character device.\n"); break;
        case S_IFBLK:
            printf("block device.\n"); break;
        case S_IFIFO:
            printf("FIFO (named pipe).\n"); break;
        case S_IFLNK:
            printf("symbolic link.\n"); break;
        default:
            printf("UNKNOWN.\n"); break;
	}
	if (buf.st_mode & S_ISUID)
		printf(" This is the set-user-ID on execute bit, usually 04000.\n");
	if (buf.st_mode & S_ISGID)
		printf(" This is the set-group-ID on execute bit, usually 02000.\n");
	if (buf.st_mode & S_ISVTX)
		printf(" This is the sticky bit, usually 01000.\n");

	// Show the permissions in octal
	printf(" Permissions: 0%o ", buf.st_mode & 0777);

	// Show permissions on the form 'rwxrwxrwx'
	for (i=0; i<9; i++)
		if (buf.st_mode & (0400 >> i))
			printf("%c", permissions[(8-i)%3]);
		else
			printf("-");

	// Show the number of hard links of this inode
	printf("\n");
	printf(" Links: %ld\n", (long int)buf.st_nlink);

	// Show identifications of user and group, numerically and symbolically
	//	getpwuid() check the file /etc/passwd
	//	getgrgid() check the file /etc/group
	// 	where these relations between identifications are saved
	printf(" User ID: %d; Name: ", buf.st_uid);
	if ((pw = getpwuid(buf.st_uid)) == NULL)
		printf("???\n");
	else
		printf("%s\n", pw->pw_name);
	printf(" Group ID: %d; Name: ", buf.st_gid);
	if ((gr = getgrgid(buf.st_gid)) == NULL)
		printf("???\n");
	else
		printf("%s\n", gr->gr_name);

	// If this file is a device file, show the numbers 
	//	'major' and 'minor' 
	switch (buf.st_mode & S_IFMT) {
        case S_IFCHR:
        case S_IFBLK:
            printf(" Device numbers: %d, %d\n",
                (int)(buf.st_rdev & 0xff00) >> 8, (int)buf.st_rdev & 0x00ff);
	}

	// Show file size
	printf(" Size: %ld bytes.\n", buf.st_size);

	// Show date of the file in string format.
	//	stat() give date and time as numbers of type time_t
	//	localtime() transforms a time_t variable into a tm structure
	//	asctime() transforms a tm structure into a regular string
	printf(" Last access: %s", asctime(localtime(&buf.st_atime)));
	printf(" Last modification: %s", asctime(localtime(&buf.st_mtime)));
	printf(" Last change: %s", asctime(localtime(&buf.st_ctime)));

	return 0;
}
