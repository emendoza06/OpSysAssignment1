#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

/*-------------------------------------------PROTOTYPES----------------------------------------*/
int depthfirstapply(char *path, int pathfun(char *path, char *options, int scale), char *options, int scale, int depth, int ino, int max_depth); 

int sizepathfun(char *path, char *options, int scale); 

void humanReadable(int size, char *pathname, char *options, int scale); 





/*-------------------------------------------SHOW TREE SIZE--------------------------------------*/
int showtreesize(char *path, int pathfun(char *path, char *options, int scale), char *options, int scale, int ino, int depth, int max_depth){
    int t_size = depthfirstapply(path, sizepathfun, options, scale, depth, ino, max_depth);

    //Print as human readable 
    if((strstr(options, "H") != NULL) || (strstr(options, "m") != NULL) || (strstr(options, "B") != NULL)) { 
        humanReadable(t_size, path, options, scale);
    }
    else{ 
        printf("%d %s\n", t_size, path);
    }

    return t_size;
}





/*----------------------------------------HUMAN READABLE FUNCTION------------------------------------*/
void humanReadable(int size, char *path, char *options, int scale) {
    //Readables will either be G, M, or K
    const char *sizesuffix = " "; 

    if(strstr(options, "H") != NULL) {
        if(size >= 1000000000) { 
            size = (long long) (size/1000000000);
            sizesuffix = "G";
        }
	else if(size >= 1000000){
	    size = (long long)(size/1000000); 
	    sizesuffix = "M";
	}
	else if(size >= 1000){
	    size = (long long)(size/1000);
	    sizesuffix = "K";
	}
	printf("%d%-7s %s\n", size, sizesuffix, path);
    }

   //Scale sizes by M before printing 
   if(strstr(options, "B") != NULL) { 
       size = size/scale;
       if(size <1) {
	size = 1;
	}
	printf("%d %s\n", size, path);
   } 

    //Same as -B
    if(strstr(options, "m") != NULL){
        size = size/1000000;
        if(size < 1){
	    size = 1;
        }
	printf("%d%-7s %s\n", size, "M", path);
    }
} 




/*-------------------------------------MAIN---------------------------------------------*/
int main(int argc, char **argv) {
    //holds command line arguments 
    char option_string[10];

    //Options
    int opt; 
    
    //Optional variables 
    int scale = 0; 
    int max_depth = 0; 

    //Parses arguments entered 
    while((opt = getopt(argc, argv, "haB:bmcd:HLs")) !=-1){
        
        //Specify what each option will do 
        switch(opt) {
            case 'h':
	        printf("\n USAGE:\n");
		printf("\n mydu [-h]");
		printf("\n mydu [-a] [-B M | -b -m] [-c] [-d N] [-H] [-L] [-s] <dir1> <dir2>");
		printf("\n");
		printf("\n DESCRIPTION:\n");
		printf("\n -a Write count for all files, not just directories.");
		printf("\n -B M Scale sizes by M before printing; for example, BM prints size in units of 1,048,576 bites.");
		printf("\n -b Print size in bytes.");
		printf("\n -c Print a grand total");
		printf("\n -d N Print the total for a directory only if it is N or fewer levels below the command line argument.");
		printf("\n -h Print a help message or usage, and exit");
		printf("\n -H Human readable; print size in human readable format, for example, 1K, 234M, 2G.");	
		printf("\n -L Dereference all symbolic links. By default, you will not dereference symbolic links."); 
		printf("\n -m Same as -B 1048576.");
		printf("\n -s Display only a total for each argument.");
		return EXIT_SUCCESS;

	    case 'a':
		strcat(option_string, "a");
		break;
	    case 'B':
		strcat(option_string, "B");
		//Get the value of the option argument
		scale = atoi(optarg);
		break;
	    case 'b':
		strcat(option_string, "b");
		break;
	    case 'm':
		strcat(option_string, "m");
		break;
	    case 'c':
		strcat(option_string, "c");
		break;
	    case 'd':
		strcat(option_string, "d");
		//Get the value of the option argument
		max_depth = atoi(optarg);
		break;
	    case 'H': 
		strcat(option_string, "H");
		break; 
	    case 'L': 
		strcat(option_string, "L");
		break;
	    case 's': 
		strcat(option_string, "s");
		break; 
	    default: 
		fprintf(stderr, "%s: Please use \"-h\" option for more info. \n", argv[0]);
		return EXIT_FAILURE;
 
        }
    }
    
    //State info for file "."
    struct stat stats; 
    stat(".", &stats);
    //file's unique inode number 
    int inode = stats.st_ino;
    int totalsize = 0;

    //If no directory given, default to current working directory
    char *topdir, *targetdir, current[2]=".";
    if(argv[optind] == NULL){
        char origin[4096];
        getcwd(origin, sizeof(origin));
	topdir = origin; 
	targetdir = current; 
	printf("Directory traverse of: %s\n", targetdir);
	int size = sizepathfun(".", option_string, scale);
	if(size >=0){
	    totalsize += size;
	}
	else{
	    totalsize += showtreesize(targetdir, sizepathfun, option_string, scale, inode, 0, max_depth);
	}	
    }
    else { 
        topdir = argv[optind]; 
	targetdir = topdir;
	printf("Directory traverse of: %s\n", targetdir);
	int size = sizepathfun(targetdir, option_string, scale);
	if(size >=0) { 
	    totalsize += size;
	}
	else { 
	    totalsize += showtreesize(targetdir, sizepathfun, option_string, scale, inode, 0, max_depth);
	}
    }

    if(((strstr(option_string, "c") != NULL)) && (strstr(option_string, "H") != NULL)) { 
	humanReadable(totalsize, "TOTAL", option_string, scale);
    } 
    else if(strstr(option_string, "c") != NULL) { 
	printf("%d%s\n", totalsize, "TOTAL");
    }

    return EXIT_SUCCESS;
}


/*--------------------------------SIZE PATH FUN---------------------------------*/
int sizepathfun(char *path, char *options, int scale) { 
    //list properties of a file identified by path 
    struct stat stval; 
    if(stat(path, &stval) ==-1) { 
        perror("Path does not correspond to an ordinary file"); 
	return -1;
    }
    if(S_ISREG(stval.st_mode) ==0) { 
	return -1;
    }
    else { 
	//Check if b, B, or m is one of the selected option 
	if((strstr(options, "b") != NULL) || (strstr(options, "B") != NULL) || (strstr(options, "m") != NULL)) { 
	  return stval.st_size;
	}
	else { 
	    return (stval.st_blocks/2);
	}
    }
}



/*----------------------------------------DEPTH FIRST APPLY----------------------------*/ 
int depthfirstapply(char *path, int pathfun(char *path, char *options, int scale), char *options, int scale, int depth, int ino, int max_depth) { 
    //Directory stram data type 
    DIR *dir; 
    //Struct with member to facilitate directory traversing 
    struct dirent *entry; 
    //Struct with members to facilitate getting info about files 
    struct stat filestat; 

    //opendir to point to first entry in directory strea. If dir is NULL or does not exist then return 
    if(!(dir = opendir(path))){ 
        printf("ERROR: %s\n", strerror(errno));
	return -1;
    }

    int result = 0; 
    //Change current working directory 
    chdir(path); 

    //Get path of current directory 
    char cwd[4096]; 
    getcwd(cwd, sizeof(cwd)); 

    //Read directories until end of stream is reached 
    while((entry = readdir(dir)) !=NULL) { 
        //Save current dir name 
        char *name = entry->d_name; 
	
	//Returns attributes of the file without following symbolic links 
	lstat(entry->d_name, &filestat);

	//If L option selected then use stat instead of lstat 
	if(strstr(options, "L") != NULL) { 
	    stat(name, &filestat); 
	}	

	//Found a directory, but ignore . and .. 
	if(strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) ==0){ 
	    continue;
	}

	//Print 
	printf("%s\n", name);

	//Recursion 
	//If a directory 
	if(S_ISDIR(filestat.st_mode)){ 
	     //size is depthfirstapply result 
	     int size = depthfirstapply(name, pathfun, options, scale, depth + 1, ino, max_depth); 
	    if(size >=-0) { 
	        //Add results form recursive calls
	        result += size;

		//Check for s option 
		if(strstr(options, "s")!= NULL) {
		    continue;
		}

		//Check for H option and send to human readable 
		if((strstr(options, "H") != NULL) || (strstr(options, "B") != NULL) || (strstr(options, "m") != NULL)) { 
		    humanReadable(size, name, options, scale);
		}

		//Check for d option 
		else if((strstr(options, "d") != NULL) && depth >= max_depth) { 
		   humanReadable(size, name, options, scale); 
		}
		else{
		    printf("%d %s\n", size, name);
		}
	    }
	}

	else if(S_ISLNK(filestat.st_mode)){ 
	    //pathfun applied to each file encountered in traversal 
	    int size = pathfun(name, options, scale); 

	    if(size >0) { 
		result += size;
	    }
	    //Check for s option 
	    if(strstr(options, "S") != NULL) { 
		continue;
	    }
	    if(((strstr(options, "B") != NULL) || (strstr(options, "m") != NULL)) && size < 1) { 
		size = 1;
	    }
	    if(strstr(options, "a") != NULL) { 
	        if((strstr(options, "H") != NULL) || (strstr(options, "B") != NULL) || (strstr(options, "m") != NULL)) { 
		    humanReadable(size, name, options, scale);
		}
		else if((strstr(options, "d") != NULL) && depth >= max_depth) { 
		    humanReadable(size, name, options, scale);
		}
		else{ 
		    printf("%d%s\n", size, name);
		}
	    }
	}
    }

	//Change current working directory one level up and close directory one level down 
	chdir(".."); 
	closedir(dir); 

//	printf("\nResult in depthfirst is %d", result); 
	return result;
}
