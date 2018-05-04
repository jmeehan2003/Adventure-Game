#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

char connList[7][10];
char pathToVictory[50][10];
char currentRoom[10];
char END[9];
char filepath[20];
char currentRoomFile[50];
char nextRoom[10];
int connCount = 0;
int numSteps = 0;
void locationMenu(char*);
void getConnections(char*);
void getRoomFile(char*);
void endGameCelebration();
int getUserInput();

void main()
{
	// timestamp of newest subdirectory examined. Initialized to -1 for error checking
	int newestDirTime = -1;
	
	// prefix we're looking for
	char prefix[32] = "meehajam.rooms.";

	// array to hold the name of the newest director that contains the prefix
	char newestDirectory[256];
	memset(newestDirectory, '\0', sizeof (newestDirectory));

	// holds starting directory
	DIR* dirToCheck;
	// holds current subdirectory of the starting directory
	struct dirent* fileInDir;
	// holds information we've gained about the subdirectory
	struct stat dirInfo;

	//open current directory
	dirToCheck = opendir(".");

	// make sure crrent directory opened properly
	if (dirToCheck > 0)
	{
		// check each entry in the curent directory
		while ((fileInDir = readdir(dirToCheck)) != NULL)
		{
			// if entry has the correct prefix
			if (strstr(fileInDir->d_name, prefix) != NULL)
			{
	//			printf("Found the prefix: %s\n", fileInDir->d_name);
				// get entry details
				stat(fileInDir->d_name, &dirInfo);

				// if this entry is more recent
				if ((int)dirInfo.st_mtime > newestDirTime)
				{
					newestDirTime = (int)dirInfo.st_mtime;
					memset(newestDirectory, '\0', sizeof (newestDirectory));
					strcpy(newestDirectory, fileInDir->d_name);
	//				printf("Newer subdir: %s, new time: %d\n", fileInDir->d_name, newestDirTime);

				}
			}
		}
	}
		
	// close directory
	closedir(dirToCheck);

	DIR* dirFiles;
	dirFiles = opendir(newestDirectory);
	// find start room
	char filename[30];
	fileInDir = readdir(dirFiles);
	while ((fileInDir = readdir(dirFiles)) != NULL)
	{
	
		// only look at files, skipe over symbolic links, directories, etc.
		// source: https://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program/4204758
		if (fileInDir->d_type == DT_REG) {
	//		printf("%s\n", fileInDir->d_name);
			memset(filename, '\0', sizeof (filename));
			strcpy(filename, newestDirectory);		
			strcat(filename, "/");
			// copy into filepath to use for opening files later
			strcpy(filepath, filename);
			strcat(filename, fileInDir->d_name);
	//		printf("filename: %s\n", filename);
			
			FILE* roomfile = fopen(filename, "r");
			//char myString[100];
			//fgets(myString, 50, roomfile);
			//printf("%s\n", myString); 
			fseek(roomfile, -8, SEEK_END);
			char letter = fgetc(roomfile);
			if (letter == 'R')
			{
				fseek(roomfile, 11, SEEK_SET);
				fgets(currentRoom, 10, roomfile);;
				int len = strlen(currentRoom);
				if (currentRoom[len-1]=='\n')
					currentRoom[len-1]='\0';
			
					
				printf("Start room: %s\n", currentRoom); 
				
			}
			else if (letter == 'N')
			{

				fseek(roomfile, 11, SEEK_SET);
				fgets(END, 10, roomfile);;
				int len = strlen(END);
				if (END[len-1]=='\n')
					END[len-1]='\0';

				printf("End room: %s\n", END); 

			}
			// close the file
			fclose(roomfile);		

		}
			
	}
	closedir(dirFiles);

	locationMenu(currentRoom);
	int res = getUserInput();
	while (res == 1 || res == 0)
	{
		printf("current: %s  next: %s \n", currentRoom, nextRoom);
		if (res) 
		{
			memset(currentRoom, '\0', sizeof (currentRoom));
			strcpy(currentRoom, nextRoom);
			strcpy(&pathToVictory[numSteps][0], nextRoom);
			numSteps++;
			printf("\n");
			locationMenu(nextRoom);
			res = getUserInput();					
		}
		else
		{
		//	printf("Next room: %s\n", nextRoom);
			printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
			locationMenu(currentRoom);
			res = getUserInput();
		}		
	}	
	
	endGameCelebration();

	printf("Newest entry found is: %s\n", newestDirectory);
}

void endGameCelebration()
{
	strcpy (&pathToVictory[numSteps][0], nextRoom);
	numSteps++;
	printf("\nYOU HAVE REACHED THE END ROOM. CONGRATULATIONS!\n");
	printf("YOU TOOK %d STEPS.  YOUR PATH TO VICTORY WAS:\n", numSteps);
	int a;
	for (a = 0; a < numSteps; a++)
	{
		printf("%s\n", pathToVictory[a]);
	}
}


int getUserInput()
{
	memset(nextRoom, '\0', sizeof (nextRoom));
	char* buffer = NULL;

	size_t len;
	int read = getline(&buffer, &len, stdin);
	int blength = strlen(buffer);

	strcpy (nextRoom, buffer);

	// remove newline that getline may add to the end
	int slength= strlen(nextRoom);
	if (nextRoom[slength-1]=='\n')
		nextRoom[slength-1]='\0';
//	for (z = 0; z < 10; z++)
//		printf("%c %d\n", nextRoom[z], nextRoom[z]);
	if (numSteps >= 50) {
		printf("ALAS, YOU'VE TAKEN 50 STEPS AND YOU'RE MASSIVELY OUT OF SHAPE.  YOU HAVE PERISHED FROM EXHAUSTION. GAME OVER\n");
		exit(-1);
	}

	if (-1 != read)
	{	
		if (strcmp (nextRoom, END) == 0)
			return 2;
		
		int i;
		for (i = 0; i < connCount; i++)
		{
			int res = strcmp(nextRoom, connList[i]);

	
			int slen1 = strlen(nextRoom);
			int slen2 = strlen(connList[i]);
	
			if (strcmp (nextRoom, connList[i]) == 0) {
				printf("MATCH!  %s\n", connList[i]);
				free(buffer);	
				return 1;			
			}	
		}
		
		free(buffer);	
		buffer = NULL;
		return 0;		
	}
	else 
		printf("Error reading line\n");
	
	free(buffer);
}

void locationMenu(char* room) {
	printf("CURRENT LOCATION: %s\n", room);
	printf("POSSIBLE CONNECTIONS: ");
	getConnections(room);
	printf("WHERE TO? >");
	
}

void getConnections(char* room)
{	
//	printf("inside getConn");
	memset(connList, '\0', sizeof (connList));
	connCount = 0;

	getRoomFile(room);
	FILE* roomfile = fopen(currentRoomFile, "r");
	if (roomfile == NULL) {
		printf(" File DID NOT OPEN");
		exit(-1);
	}
	ssize_t buffer_size = 0;
	char * buffer = 0;
	//char *buffer = malloc(buffer_size * sizeof(char));
	//if (buffer == NULL) {
	//	perror("Unable to allocate buffer");
	//	exit(1);
	//}
	
	getline(&buffer, &buffer_size, roomfile);
	//printf("linelen: %d\n", linelen);
	getline(&buffer, &buffer_size, roomfile);
	//printf("linelen: %d\n", linelen);
	
	char letterCheck;
	do {
		//
		strtok(buffer, ": ");
		//
		strtok (NULL, "\n\r ");
		//
		char *connection = strtok (NULL, "\n\r ");	
		strcpy (&connList[connCount][0], connection);
		connCount++;
		//	

		getline(&buffer, &buffer_size, roomfile);
		letterCheck = buffer[0];
	} while (letterCheck == 'C');

	int i;
	printf("%s", connList[0]);
	for (i = 1; i < connCount; i++)
		printf(", %s", connList[i]);
	printf(".\n");	

	fclose(roomfile);
	free(buffer);
	buffer = NULL;
//	printf("exiting getCoon");
}
	
void getRoomFile(char* room)
{
	memset(currentRoomFile, '\0', sizeof (currentRoomFile));
	strcpy(currentRoomFile, filepath);
	strcat(currentRoomFile, room);
	strcat(currentRoomFile, "_room");
}


