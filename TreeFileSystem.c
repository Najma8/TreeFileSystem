#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <libgen.h>
#include <limits.h>

#define MAX_NAME_LENGTH 1024

typedef struct Item {
	char name[MAX_NAME_LENGTH];
	struct Item* parent;
	char type;
} Item;

typedef struct File {
	Item item;
	char textContent[MAX_NAME_LENGTH];
} File;

typedef struct Directory {
	Item item;
	struct Item* children[100];
	int numChildren;
} Directory;

typedef struct FileSystem {
	Directory* self;
	Directory* currentDirectory;
	Directory* currentDirectoryPath[100];
	int currentDirectoryPathSize;
} FileSystem;


FileSystem* createFileSystem() {
	FileSystem* fs = (FileSystem*)malloc(sizeof(FileSystem));
	
	Directory* root = (Directory*)malloc(sizeof(Directory));
	strcpy(root->item.name, "root");
	root->item.parent = NULL;
	root->numChildren = 0;
	
	fs->self = root;
	fs->currentDirectory = root;
	fs->currentDirectoryPath[0] = root;
	fs->currentDirectoryPathSize = 1;
	return fs;
}

File* createFile(const char* fileName, const char* textContent) {
	File* file = (File*)malloc(sizeof(File));
	strcpy(file->item.name, fileName);
	file->item.parent = NULL;
	file->item.type='f';
	strcpy(file->textContent, textContent);
	return file;
}

Directory* createDirectory(const char* dirName) {
	Directory* dir = (Directory*)malloc(sizeof(Directory));
	strcpy(dir->item.name, dirName);
	dir->item.parent = NULL;
	dir->item.type='d';
	dir->numChildren = 0;
	return dir;
}

void insertItem(Directory* dir, Item* item) {
	dir->children[dir->numChildren] = item;
	dir->numChildren++;
	item->parent = dir;
}

void printCurrentDirectory(FileSystem* fs) {
	printf("\n[%s]:", fs->currentDirectoryPath[fs->currentDirectoryPathSize - 1]->item.name);
	if (fs->currentDirectory->numChildren == 0) {
		printf("\n(empty)");
	} else {
		int i;
		for ( i = 0; i < fs->currentDirectory->numChildren; i++) {
			Item* item = fs->currentDirectory->children[i];
			printf("\n[%c]-> %s", item->type, item->name);
		}
	}
	printf("\n");
	writePath(fs);
}

void openDirectory(FileSystem* fs, const char* dirName) {
	Directory* dir = fs->currentDirectory;
	int i;
	for ( i = 0; i < dir->numChildren; i++) {
		Item* item = dir->children[i];
		if (strcmp(item->name, dirName) == 0 && item->parent == dir) {
			fs->currentDirectory = (Directory*)item;
			fs->currentDirectoryPath[fs->currentDirectoryPathSize] = fs->currentDirectory;
			fs->currentDirectoryPathSize++;
			break;
		}
	}
}
int checkFile(FileSystem* fs, const char* Name){
	int sonuc;
	Directory* dir = fs->currentDirectory;
	int i;
	Item* item;
	for ( i = 0; i < dir->numChildren; i++) {
		item = dir->children[i];
		if (strcmp(item->name, Name) == 0 && item->parent == dir) {
			if(item->type=='f'){
				int sonuc = deleteFile(fs,Name, dir, i);
				if(sonuc==0){
					dir->numChildren--;
				}
				free(item);
				return sonuc;
			}
			else{
				return 2;//is not a file
			}
		}
		else{
			sonuc = 3;//no such file
		}
	}
	return sonuc;
}
int deleteFile(FileSystem* fs, const char* Name, Directory* dir, int i){
	char path[MAX_NAME_LENGTH];
	sprintf(path, "%s/", getcwd(NULL, 0));
	int j=0;
	for(j=2;j<fs->currentDirectoryPathSize;j++){
		strcat(path,fs->currentDirectoryPath[j]->item.name);
		strcat(path,"/");
	}
	strcat(path,Name);
	if (unlink(path) == 0) {
        dir->children[i] = NULL;
        int k;
        for(k=i;k<dir->numChildren;k++){
        	dir->children[k] = dir->children[k+1];
		}
		return 0;
    } else {
        return 1;//couldn't delete
    }
}
int checkDirectory(FileSystem* fs, const char* Name){
	int sonuc;
	Directory* dir = fs->currentDirectory;
	int i;
	Item* item;
	for ( i = 0; i < dir->numChildren; i++) {
		item = dir->children[i];
		if (strcmp(item->name, Name) == 0 && item->parent == dir) {
			if(item->type=='f'){
				return 2;//is not a directory
			}
			else{
				Directory* dirdir = (Directory*)item;
				openDirectory(fs, dirdir->item.name);
				getDirectoriesofDir(fs,dirdir->item.name);
				goBack(fs);
				if (dirdir->numChildren == 0) {
					sonuc = deleteDirectory(fs,Name, dir,i);
					if(sonuc==0){
        				dir->numChildren--;
					}
					free(item);
					return sonuc;
				} else {
					return 4;//directory is not empty
				}
			}
		}
		else{
			sonuc = 3;//no such directory
		}
	}
	return sonuc;
}
int deleteDirectory(FileSystem* fs, const char* Name, Directory* dir, int i){
	char path[MAX_NAME_LENGTH];
	sprintf(path, "%s/", getcwd(NULL, 0));
	int j=0;
	for(j=2;j<fs->currentDirectoryPathSize;j++){
		strcat(path,fs->currentDirectoryPath[j]->item.name);
		strcat(path,"/");
	}
	strcat(path,Name);
	if (rmdir(path) == 0) {
        dir->children[i] = NULL;
        int k;
        for(k=i;k<dir->numChildren;k++){
        	dir->children[k] = dir->children[k+1];
		}
        return 0;
    } else {
        return 1;//couldn't delete
    }	
}
int deleteFileorDirectory(FileSystem* fs, const char* Name) {
	int sonuc;
	Directory* dir = fs->currentDirectory;
	int i;
	Item* item;
	for ( i = 0; i < dir->numChildren; i++) {
		item = dir->children[i];
		if (strcmp(item->name, Name) == 0 && item->parent == dir) {
			if(item->type=='f'){
				sonuc = deleteFile(fs,Name,dir,i);
				free(item);
				return sonuc;
			}
			else if(item->type=='d'){
				Directory* dirdir = (Directory*)item;
				openDirectory(fs, dirdir->item.name);
				getDirectoriesofDir(fs,dirdir->item.name);
				if (dirdir->numChildren == 0) {
					goBack(fs);
					sonuc = deleteDirectory(fs,Name,dir,i);
					free(item);
					return sonuc;
				} else {
					while(dirdir->numChildren!=0){
						Item* item = dirdir->children[0];
						if(item->type=='f' && item!=NULL){
							sonuc = deleteFile(fs,item->name,dirdir,0);
							free(item);
						}
						else if(item->type=='d' && item!=NULL){
							sonuc = deleteFileorDirectory(fs,dirdir->children[0]->name);
						}
						if(sonuc==0){
	        				dirdir->numChildren--;
						}
					}
					goBack(fs);
					sonuc = deleteDirectory(fs,Name,dir,i);
					free(item);
					return sonuc;
				}
				goBack(fs);
			}
			break;
		}
	}
}
void goBack(FileSystem* fs) {
	if (fs->currentDirectoryPathSize <= 1) {
		return;
	}
	fs->currentDirectoryPathSize--;
	fs->currentDirectory = fs->currentDirectoryPath[fs->currentDirectoryPathSize - 1];
}
void writePath(FileSystem* fs){
	int i;
	for(i=0; i<fs->currentDirectoryPathSize; i++)
	{
		printf("%s ->> ",fs->currentDirectoryPath[i]);
	}
}
void writeContent(FileSystem* fs, const char* fileName){
	Directory* dir = fs->currentDirectory;
	int i;
	for ( i = 0; i < dir->numChildren; i++) {
		Item* item = dir->children[i];
		if (strcmp(item->name, fileName) == 0 && item->parent == dir) {
			File* file= (File*)item;
			printf("%s\n",file->textContent);
		}
	}
	writePath(fs); 
}

void createActualFile(const char* filePath, const char* content) {
	FILE* file = fopen(filePath, "w");
	if (file != NULL) {
		fprintf(file, "%s", content);
		fclose(file);
	}
}

void createActualDirectory(const char* dirPath) {
	mkdir(dirPath, 0777);
}
void getMachineDirectories(FileSystem* fs){
	Directory* dir;
	DIR *directory;
    struct dirent *entry;
    unsigned char type;
    char typeChar[1024];
    directory = opendir(".");
    if (directory == NULL) {
        printf("Failed to open the current directory.\n");
        return 1;
    }
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        char *directoryName = basename(cwd);
        dir = createDirectory(directoryName);
		insertItem(fs->currentDirectory, (Item*)dir);
		openDirectory(fs, directoryName);
    }
    int i=0;
    while ((entry = readdir(directory)) != NULL) {
    	type = entry->d_type;
    	if(type==4 && strcmp(entry->d_name,".")!=0 && strcmp(entry->d_name,"..")!=0){
	    	Directory* dir2 = createDirectory(entry->d_name);
			insertItem(fs->currentDirectory, (Item*)dir2);
			i++;
		}else if(type==8 && strcmp(entry->d_name,".")!=0 && strcmp(entry->d_name,"..")!=0){
			FILE* file2 = fopen(entry->d_name, "r");
            if (file2 == NULL) {
                printf("Failed to open file: %s\n", entry->d_name);
                continue;
            }
            char content[MAX_NAME_LENGTH];
            size_t bytesRead = fread(content, sizeof(char), MAX_NAME_LENGTH - 1, file2);
            content[bytesRead] = '\0';
            fclose(file2);
			File* file = createFile(entry->d_name,content);
			insertItem(fs->currentDirectory, (Item*)file);
			i++;
		}
    }
    dir->numChildren=i;
    closedir(directory);
}
void getDirectoriesofDir(FileSystem* fs,const char* dirName){
	Directory* dir;
	DIR *directory;
	char fopenPath[MAX_NAME_LENGTH]="";
    struct dirent *entry;
    int j=0;
    char current[1024]=".";
    unsigned char type;
    char typeChar[1024];
	for(j=2;j<fs->currentDirectoryPathSize;j++){
		strcat(current,"/");
		strcat(current,fs->currentDirectoryPath[j]->item.name);
	}
    directory = opendir(current);
    if (directory == NULL) {
        printf("\nFailed to open the current directory.\n");
        return;
    }
    int i=0;
    while ((entry = readdir(directory)) != NULL) {
    	type = entry->d_type;
    	if(type==4 && strcmp(entry->d_name,".")!=0 && strcmp(entry->d_name,"..")!=0){
	    	Directory* dir2 = createDirectory(entry->d_name);
			insertItem(fs->currentDirectory, (Item*)dir2);
			i++;
		}else if(type==8 && strcmp(entry->d_name,".")!=0 && strcmp(entry->d_name,"..")!=0){
			strcpy(fopenPath,current);
			strcat(fopenPath,"/");
			strcat(fopenPath,entry->d_name);
			FILE* file2 = fopen(fopenPath, "r");
            if (file2 == NULL) {
                printf("Failed to open file: %s\n", entry->d_name);
            }
            else{
	            char content[MAX_NAME_LENGTH] = {0};
	            size_t bytesRead = fread(content, sizeof(char), MAX_NAME_LENGTH-1, file2);
	            content[bytesRead] = '\0';
	            fclose(file2);
				File* file = createFile(entry->d_name,content);
				insertItem(fs->currentDirectory, (Item*)file);
				i++;
			}
		}
    }
    fs->currentDirectory->numChildren=i;
    closedir(directory);
}
void copyRecurFileOrDirectory(const char* source, const char* dest) {
    pid_t pid = fork();
    if (pid == 0) {
        execlp("cp", "cp", "-r", source, dest, NULL);
        exit(0);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            int exitStatus = WEXITSTATUS(status);
            if (exitStatus == 0) {
            } else {
                printf("\n%s copy to %s failed. With exit status: %d\n", source, dest, exitStatus);
            }
        }
    } else {
        printf("\n%s copy to %s failed.\n", source, dest);
    }
}
void copyFileOrDirectory(const char* source, const char* dest) {
    pid_t pid = fork();
    if (pid == 0) {
        execlp("cp", "cp", source, dest, NULL);
        exit(0);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    } else {
        printf("\n%s copy to %s failed.\n", source, dest);
    }
}
void relocate(FileSystem* fs, char* source, char* dest){
	char sourcename[MAX_NAME_LENGTH];
	char srcName[MAX_NAME_LENGTH];
	strcpy(sourcename,source);
	Item* sourceitem;
	char destname[MAX_NAME_LENGTH];
	strcpy(destname,dest);
	Item* destitem;
	int i =0;
	char sourceresult = strchr(source, '/');
	if(sourceresult!=NULL){
		char *tokensource = strtok(source, "/");
		while (tokensource != NULL) {
			strcpy(sourcename, tokensource);
			openDirectory(fs,sourcename);
			getDirectoriesofDir(fs, sourcename);
			i++;
			tokensource = strtok(NULL, "/");
	        if (tokensource == NULL) {
	            tokensource = strtok(NULL, "\0");
	        }
	        if (tokensource == NULL) {
	            tokensource = strtok(NULL, "\n");
	        }
	        if (tokensource == NULL) {
	            break;
	        }
	        strcpy(srcName, tokensource);
		}
		sourceitem = (Item*)&(fs->currentDirectory->item);
		int n;
		for(n=0;n<i;n++){
			goBack(fs);
		}
	}
	else{
		strcpy(srcName, source);
		sourceitem = (Item*)&(fs->currentDirectory->item);
	}
	char destresult = strchr(dest, '/');
	if(destresult!=NULL){
		char *tokendest = strtok(dest, "/");
		while (tokendest != NULL) {
			strcpy(destname, tokendest);
			openDirectory(fs,destname);
			Item itemOpen = fs->currentDirectory->item;
			if(itemOpen.type=='d'){
				getDirectoriesofDir(fs, destname);
			}
			
			i++;
			tokendest = strtok(NULL, "/");
	        if (tokendest == NULL) {
	            tokendest = strtok(NULL, "\0");
	        }
	        if (tokendest == NULL) {
	            tokendest = strtok(NULL, "\n");
	        }
	        if (tokendest == NULL) {
	            break;
	        }
		}
		destitem = (Item*)(fs->currentDirectory->item.parent);
		int n;
		for(n=0;n<i;n++){
			goBack(fs);
		}
	}
	else{
		openDirectory(fs,destname);
		getDirectoriesofDir(fs, destname);
		destitem = (Item*)(fs->currentDirectory->item.parent);
		goBack(fs);
	}
	Directory* dir = (Directory*)sourceitem;
	int w;
	for ( w = 0; w < dir->numChildren; w++) {
		Item* item = dir->children[w];
		if (strcmp(item->name, srcName) == 0 && item->parent == dir) {
			break;
		}
	}
	removeFile((Directory*)sourceitem,w);
	sourceitem=destitem;
}
int removeFile(Directory* dir, int i){
    dir->children[i] = NULL;
    int k;
    for(k=i;k<dir->numChildren;k++){
    	dir->children[k] = dir->children[k+1];
	}
	dir->numChildren--;
	return 0;
}
void moveFileorDirectory(FileSystem* fs, char* source, char* dest){
	char *path = malloc(PATH_MAX);
	path[0] = '\0';
	sprintf(path, "%s/", getcwd(NULL, 0));
	int j=0;
	for(j=2;j<fs->currentDirectoryPathSize;j++){
		strcat(path,fs->currentDirectoryPath[j]->item.name);
		strcat(path,"/");
	}
	char *source_path = malloc(PATH_MAX);
	source_path[0] = '\0';
	strcat(source_path,path);
	strcat(source_path,source);
	int len = strlen(dest);
	if(dest[len-1]=='/'){
		strcat(dest,source);
	}
    char *destination_path = malloc(PATH_MAX);
    destination_path[0] = '\0';
    strcat(destination_path,path);
	strcat(destination_path,dest);
    if (rename(source_path, destination_path) == 0) {
    } else {
        printf("Cannot move %s to %s\n",source, dest);
    }
    relocate(fs,source,dest);
    free(path);
    free(source_path);
    free(destination_path);
}
int main() {
	char komut[MAX_NAME_LENGTH];
	FileSystem* fs = createFileSystem();
	char c1[30], c2[30], c3[MAX_NAME_LENGTH];
	getMachineDirectories(fs);
	writePath(fs);
	do{
		fgets(komut, MAX_NAME_LENGTH, stdin);
		komut[strcspn(komut, "\n")] = '\0';
    	char *token = strtok(komut, " ");
		if (token != NULL) {
	       strcpy(c1, token);
	       
	        token = strtok(NULL, " ");
	        if (token) {
	            strcpy(c2, token);
	            token = strtok(NULL, "\n");
	            if (token) {
	                strcpy(c3, token);
	            }
	        }
	    }

		if(strcmp(c1,"touch")==0){
			char result = strchr(c2, ',');
			if(result != NULL){
				char *token2 = strtok(c2, ",");
				while (token2 != NULL) {
					char path[MAX_NAME_LENGTH];
					char filename[MAX_NAME_LENGTH];
					strcpy(filename, token2);
					File* file = createFile(filename,c3);
					insertItem(fs->currentDirectory, (Item*)file);
					if(strcmp(fs->currentDirectory->item.name,"root")!=0|| strcmp(fs->currentDirectory->item.name,"Desktop")!=0){
						sprintf(path, "%s/", getcwd(NULL, 0));
						int j=0;
						for(j=2;j<fs->currentDirectoryPathSize;j++){
							strcat(path,fs->currentDirectoryPath[j]->item.name);
							strcat(path,"/");
						}
						strcat(path,filename);
					}
					else{
						sprintf(path, "%s/%s", getcwd(NULL, 0), filename);
					}
			  		createActualFile(path, c3);
			        token2 = strtok(NULL, ",");
			        if (token2 == NULL) {
			            token2 = strtok(NULL, "\0");
			        }
			        if (token2 == NULL) {
			            token2 = strtok(NULL, "\n");
			        }
			    }
			    printCurrentDirectory(fs);
			}
			else{
				char path[MAX_NAME_LENGTH];
				File* file = createFile(c2,c3);
				insertItem(fs->currentDirectory, (Item*)file);
				if(strcmp(fs->currentDirectory->item.name,"root")!=0|| strcmp(fs->currentDirectory->item.name,"Desktop")!=0){
					sprintf(path, "%s/", getcwd(NULL, 0));
					int j=0;
					for(j=2;j<fs->currentDirectoryPathSize;j++){
						strcat(path,fs->currentDirectoryPath[j]->item.name);
						strcat(path,"/");
					}
					strcat(path,c2);
				}
				else{
					sprintf(path, "%s/%s", getcwd(NULL, 0), c2);
				}
		  		createActualFile(path, c3);
			  	printCurrentDirectory(fs);
		  	}
		}else if(strcmp(c1,"mkdir")==0){	
			char result = strchr(c2, ',');
			if(result != NULL){
				char *token2 = strtok(c2, ",");
				while (token2 != NULL) {
					char path[MAX_NAME_LENGTH];
					char foldername[MAX_NAME_LENGTH];
					strcpy(foldername, token2);
					Directory* dir = createDirectory(foldername);
					insertItem(fs->currentDirectory, (Item*)dir);
					if(strcmp(fs->currentDirectory->item.name,"root")!=0|| strcmp(fs->currentDirectory->item.name,"Desktop")!=0){
						sprintf(path, "%s/", getcwd(NULL, 0));
						int j=0;
						for(j=2;j<fs->currentDirectoryPathSize;j++){
							strcat(path,fs->currentDirectoryPath[j]->item.name);
							strcat(path,"/");
						}
						strcat(path,foldername);
					}
					else{
						sprintf(path, "%s/%s", getcwd(NULL, 0), foldername);
					}
			 		createActualDirectory(path);
			        token2 = strtok(NULL, ",");
			        if (token2 == NULL) {
			            break;
			        }
			    }
			    printCurrentDirectory(fs);
			}
			else{
				char path[MAX_NAME_LENGTH];
				Directory* dir = createDirectory(c2);
				insertItem(fs->currentDirectory, (Item*)dir);
				if(strcmp(fs->currentDirectory->item.name,"root")!=0|| strcmp(fs->currentDirectory->item.name,"Desktop")!=0){
					sprintf(path, "%s/", getcwd(NULL, 0));
					int j=0;
					for(j=2;j<fs->currentDirectoryPathSize;j++){
						strcat(path,fs->currentDirectoryPath[j]->item.name);
						strcat(path,"/");
					}
					strcat(path,c2);
				}
				else{
					sprintf(path, "%s/%s", getcwd(NULL, 0), c2);
				}
		 		createActualDirectory(path);
				printCurrentDirectory(fs);	
			}
		}else if(strcmp(c1,"ls")==0){
			printCurrentDirectory(fs);
		}else if(strcmp(c1,"cat")==0){
			writeContent(fs,c2);
		}else if(strcmp(c1,"rm")==0){
			if(strcmp(c2,"-r")==0){
				char result = strchr(c3, ',');
				if(result != NULL){
					char *token2 = strtok(c3, ",");
					while (token2 != NULL) {
						char name[MAX_NAME_LENGTH];
						strcpy(name, token2);
						int sonuc = deleteFileorDirectory(fs,name);
						if(sonuc==0){
							fs->currentDirectory->numChildren--;
						}else if(sonuc==3){
							printf("\n%s No such file or directory\n",name);
						}else if(sonuc==1){
							printf("\nUnable to delete the file or directory: %s\n",name);
						}
						token2 = strtok(NULL, ",");
				        if (token2 == NULL) {
				            break;
				        }
					}
					printCurrentDirectory(fs);
				}
				else{
					int sonuc = deleteFileorDirectory(fs,c3);
					if(sonuc==0){
						fs->currentDirectory->numChildren--;
					}else if(sonuc==3){
						printf("\n%s No such file or directory\n",c3);
					}else if(sonuc==1){
						printf("\nUnable to delete the file or directory: %s\n",c3);
					}
					printCurrentDirectory(fs);
				}
			}
			else{
				char result = strchr(c2, ',');
				if(result != NULL){
					char *token2 = strtok(c2, ",");
					while (token2 != NULL) {
						char name[MAX_NAME_LENGTH];
						strcpy(name, token2);
						int sonuc = checkFile(fs,name);
						if(sonuc==2){
							printf("\n%s Is not a file\n",name);
						}else if(sonuc==3){
							printf("\n%s No such file or directory\n",name);
						}else if(sonuc==1){
							printf("\nUnable to delete the file: %s\n",name);
						}
						token2 = strtok(NULL, ",");
				        if (token2 == NULL) {
				            break;
				        }
					}
					printCurrentDirectory(fs);
				}
				else{
					int sonuc = checkFile(fs,c2);
					if(sonuc==2){
						printf("\n%s Is not a file\n",c2);
					}else if(sonuc==3){
						printf("\n%s No such file or directory\n",c2);
					}else if(sonuc==1){
						printf("\nUnable to delete the file: %s\n",c2);
					}
					printCurrentDirectory(fs);
				}
			}
		}else if(strcmp(c1,"rmdir")==0){
			char result = strchr(c2, ',');
			if(result != NULL){
				char *token2 = strtok(c2, ",");
				while (token2 != NULL) {
					char name[MAX_NAME_LENGTH];
					strcpy(name, token2);
					int sonuc = checkDirectory(fs,name);
					if(sonuc==2){
						printf("\n%s Is not a directory\n",name);
					}else if(sonuc==3){
						printf("\n%s No such file or directory\n",name);
					}else if(sonuc==1){
						printf("\nUnable to delete the directory: %s\n",name);
					}else if(sonuc==4){
						printf("\n%s Directory is not empty\n",name);
					}
					token2 = strtok(NULL, ",");
			        if (token2 == NULL) {
			            break;
			        }
				}
				printCurrentDirectory(fs);
			}
			else{
				int sonuc = checkDirectory(fs,c2);
				if(sonuc==2){
					printf("\n%s Is not a directory\n",c2);
				}else if(sonuc==3){
					printf("\n%s No such file or directory\n",c2);
				}else if(sonuc==1){
					printf("\nUnable to delete the directory: %s\n",c2);
				}else if(sonuc==4){
					printf("\n%s Directory is not empty\n",c2);
				}
				printCurrentDirectory(fs);
			}
		}
		else if(strcmp(c1,"cd")==0){
			if(strcmp(c2,"..")==0)
			{
				goBack(fs);
				writePath(fs);
			}
			else{
				openDirectory(fs, c2);
				int numchild = fs->currentDirectory->numChildren;
				int o;
				for(o=0;o<numchild;o++){
					fs->currentDirectory->children[o]=NULL;
					fs->currentDirectory->numChildren--;
				}
				getDirectoriesofDir(fs, c2);
				writePath(fs);
			}
		}else if(strcmp(c1,"mv")==0){
			strcat(c3,"\0");
			moveFileorDirectory(fs,c2,c3);
			writePath(fs);
		}else if(strcmp(c1,"cp")==0){
			if(strcmp(c2,"-r")==0){
				char sourceCopy[MAX_NAME_LENGTH];
				char destCopy[MAX_NAME_LENGTH];
				char *tokenrec = strtok(c3, " ");
				if (tokenrec != NULL) {
			       strcpy(sourceCopy, tokenrec);
			    	tokenrec = strtok(NULL, "\n");
			        if (tokenrec) {
			            strcpy(destCopy, tokenrec);
			        }
			    }
				copyRecurFileOrDirectory(sourceCopy,destCopy);
				writePath(fs);
			}
			else{
				copyFileOrDirectory(c2,c3);
				writePath(fs);
			}
		}
		else if(strcmp(c1,"end")==0 || strcmp(komut,"end")==0){
			break;
		}
		else{
			printf("Boyle bir komut yok!\n");
			writePath(fs);
		}
	}while(strcmp(c1,"end")!=0);
  	free(fs->self);
  	free(fs);
	return 0;
}
