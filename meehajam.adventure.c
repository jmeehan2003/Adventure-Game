/*****************************************************************
** Author:  James Meehan
** Date: 05/08/2018
** Description:  This is the main program for the Adventure text-
** basesd dungeon crawler game.  The program reads information from 
** the room files from the most recently created room directory.
** The player proceeds to work their way through the maze of rooms
** until they reach the end room. They can also receive the current
** time at any point by typing in "time". 
*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

//global variables
char filepath[20]; // path to the room files directory
char currentRoomFile[50]; // full path with filename to access a specific room file
char pathToVictory[50][10]; // list of rooms user has visited
int numSteps = 0; // number of steps the user has taken
char currentRoom[10]; // room the user is currently in
char nextRoom[10];  // room user is attempting to visit next
char END[9]; // the end room - user wins when they reach this room
char connList[7][10];   // list of the room connections (updated each move)
int connCount = 0; // for looping through the connected rooms

void locationMenu(char*);
void getConnections(char*);
void getRoomFile(char*);
void endGameCelebration();
void getRoomsData();
int getUserInput();
void* getTime();
void playGame();		

// setup thread variable and mutex
pthread_mutex_t lock;
pthread_t threadID;

int main()
{
	// initiate lock and check that it was setup properly
	if (pthread_mutex_init(&lock, NULL) != 0) 
	{
		printf("Mutex init failed. Program exiting.\n");
		return 1;
	}
	
	// get room files directory/file path, start room, and end room
	getRoomsData();
	
	// start game loop
	playGame();
	
	return 0;
}

/*************************************************************************
** Function: playGame()
** Description: The playGame() functions enters a loop for the player to
** keep selecting connection rooms until they reach the end room.  If the
** user enters a string that does not match with a connected room they 
** are shown an error message and reprompted. playGame() continues until
** the user has reached the end room.
**************************************************************************/
void playGame() {
	//crate a file pointer and open the file to store the time requests from the user
	FILE *ftime;
	ftime = fopen("currentTime.txt", "w+");

	// promt the user with the game menu.  The currentRoom variable here is the starting 
	// room, which was populated from the getRoomsData() function.
	locationMenu(currentRoom);
	
	// get the value returned from getUserInput (either the next room, invalid entry, time,
	// or end room) 
	int res = getUserInput();
	
	// while the user has not reached the end room
	while (res >= 0 && res < 3  )
	{
		// main thread locks the mutex
		pthread_mutex_lock(&lock);

		// if user enters a valid connecting room
		if (res == 1) 
		{
			// flush currentRoom array and copy the user entered room into it
			memset(currentRoom, '\0', sizeof (currentRoom));
			strcpy(currentRoom, nextRoom);
			
			// add user entered room to their Path to Victory and increase their number of steps
			strcpy(&pathToVictory[numSteps][0], nextRoom);
			numSteps++;
			printf("\n");

			// reprompt the user with the game menu and get their response
			locationMenu(nextRoom);
			res = getUserInput();					
		}
		// if users enters "time"
		else if (res == 2)
		{
			// create a second thread and check it was created properly.  The second thread
			// will immediatedly try to lock the mutex but it will be blocked because they 
			// main thread has control of it.
			int threadResult = pthread_create(&threadID, NULL, getTime, NULL);
			if (threadResult != 0)
			{
				printf("Thread could not be created :[%s]", strerror(threadResult));
			}
			printf("\n");
			
			// unlock the main thread mutex.  Now the getTime() function called by the second
			// thread will be unblocked and can run
			pthread_mutex_unlock(&lock);
		
			// wait for the second thread to finish
			pthread_join(threadID, NULL);
			
			// lock the main thread again for file writing
			pthread_mutex_lock(&lock);

			// read the next line in the ftime file and store in a temporary buff array
			char buff[255];
			fgets(buff, 255, ftime);

			// eat the newline fgets stores
			// source: https://gsamaras.wordpress.com/code/read-file-line-by-line-in-c-and-c/
			buff[strlen(buff) - 1] = '\0';	
			
			// print out the time
			printf("%s\n", buff);
			printf("\n");

			// reprompt the user for a room and get their response
			printf("WHERE TO? >");
			res = getUserInput();
		}
		// the user entered a string that did not match up with a connecting room or "time"
		else
		{
			// print an error message, reprompt the user with the game menu, and get their response
			printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
			locationMenu(currentRoom);
			res = getUserInput();
		}	

		// unlock the mutex for the next iteration of the loop
		pthread_mutex_unlock(&lock);	
	}		
		
	// if the user has exited the loop, then they have escaped the dungeon by reaching the End Room.  		
	// close the time file an, destroy the mutex, and call the endGameCelebration() function.
	fclose(ftime);
	pthread_mutex_destroy(&lock);
	endGameCelebration();
}

/*********************************************************************************************************
** Function: endGameCelebration()
** Description: The endGameCelecration() function runs when the user has reached the End Room.  A 
** congratulatory message is displayed which includes the number of steps it took them and their Path to
** Victory (rooms visited).
**********************************************************************************************************/
void endGameCelebration()
{
	// add End Room to Path to Victory array and increase the number of steps
	strcpy (&pathToVictory[numSteps][0], nextRoom);
	numSteps++;

	// print out the congratulatory message followed by the list of rooms visited.
	printf("\nYOU HAVE REACHED THE END ROOM. CONGRATULATIONS!\n");
	printf("YOU TOOK %d STEPS.  YOUR PATH TO VICTORY WAS:\n", numSteps);
	int a;
	for (a = 0; a < numSteps; a++)
	{
		printf("%s\n", pathToVictory[a]);
	}
}

/***********************************************************************************************************
** Function: getUserInput()
** Description: getUserInput() receives a string from the user.  It checks to see if the string matches the
** End Room, another connecting room, "time", or an invalid response and returns the corresponding integer
** to represent that.  
***********************************************************************************************************/
int getUserInput()
{
	// set a limt on the number of steps the user can take.  If somehow the user has reached 50 steps, the game is over
	if (numSteps >= 50) {
		printf("ALAS, YOU'VE TAKEN 50 STEPS AND YOU'RE MASSIVELY OUT OF SHAPE.  YOU HAVE PERISHED FROM EXHAUSTION. GAME OVER\n");
		return 0;
	}

	// clear the nextRoom array and create a temporary buffer array to store new user input
	memset(nextRoom, '\0', sizeof (nextRoom));
	char* buffer = NULL;

	// get user input
	size_t len;
	int read = getline(&buffer, &len, stdin);

	// copy user input into the nextRoom array
	strcpy (nextRoom, buffer);

	// remove newline that getline may add to the end
	int slength= strlen(nextRoom);
	if (nextRoom[slength-1]=='\n')
		nextRoom[slength-1]='\0';
	
	// free the buffer and set it to NULL to prevent memory leaks
	free(buffer);
	buffer = NULL;

	// if there are no errors reading the line from the user
	if (-1 != read)
	{	
		// check if user entered "time"
		if (strcmp (nextRoom, "time") == 0) {	
			return 2;
		}

		// else loop through the connList array to see if the
		// user input matches a connecting room
		int i;
		for (i = 0; i < connCount; i++)
		{
			// if there is a match 
			if (strcmp (nextRoom, connList[i]) == 0)
			{ 
				// check if it is the End Room
				if (strcmp (nextRoom, END) == 0)
					return 3;
				// else it's just a boring old connecting room
				else
					return 1;			
			}
		}
		
		// otherwise user entered a string that does not match a valid option
		return 0;		
	}
	// print an error message if there was an error reading the line from the user
	else 
	{
		printf("Error reading line\n");
	}
}

/***********************************************************************************************************
** Function: getTime()
** Description: The getTime() function is called by the second.  It first tries to lock the mutex but will 
** be blocked because the main thread will be controlling it.  When the main thread unlocks the mutex, the
** second thread will take control and write the current time time to the currentTime.txt file. Right before
** exiting, the mutex will be unlocked so the main thread can regain control to read from the time file.
***********************************************************************************************************/
void* getTime() {
	// the second thread tries to lock the thread here but will be blocked since the main thread controls
	// it. When the main thread unlocks the mutex, the rest of the function will run		
	pthread_mutex_lock(&lock);
	
	// get the current time and store it in the tempory buffer array
	// source: www.cplusplus.com/reference/ctime/localtime 
	char buffer[80];
	time_t rawtime;
	struct tm* timeinfo;
	time (&rawtime);
	timeinfo = localtime(&rawtime);

	// use strftime for formatting
	strftime(buffer,80, "%l:%M%P, %A, %B%e, %Y",timeinfo);

	// write the time to currentTime.txt
	FILE* ftimeWrite;
	ftimeWrite = fopen("currentTime.txt", "a");
	fputs(buffer, ftimeWrite);
	fputs("\n", ftimeWrite);
	// close the file
	fclose(ftimeWrite);

	// unlcok the mutex so the main thread can gain control to read from the time file
	pthread_mutex_unlock(&lock);
}

/***********************************************************************************************************
** Function: locationMenu()
** Description: locatoinMenu() prints out the users current location, the rooms that are connected to their
** current room, and prompts them for input regarding which room they would like to move to
***********************************************************************************************************/
void locationMenu(char* room) {
	// show user their currrent location
	printf("CURRENT LOCATION: %s\n", room);
	
	// get list of connecting rooms from the getConnections() function
	printf("POSSIBLE CONNECTIONS: ");
	getConnections(room);

	// prompt the user for input
	printf("WHERE TO? >");
	
}

/***********************************************************************************************************
** Function: getConnections()
** Description: getConnetions() retrieves the list of connecting rooms from the corresponding room file
***********************************************************************************************************/
void getConnections(char* room)
{	
	// flush the connList array to store the new room's connection list
	memset(connList, '\0', sizeof (connList));
	// set number of connections for the room to 0
	connCount = 0;

	// get the complete filepath for the room
	getRoomFile(room);

	// open the file for reading
	FILE* roomfile = fopen(currentRoomFile, "r");
	if (roomfile == NULL) {
		printf(" File DID NOT OPEN\n");
		exit(-1);
	}

	// create tempory buffer to store room file data
	ssize_t buffer_size = 0;
	char * buffer = 0;	
	// get the first line and disregard it as that only contains the current room's name
	getline(&buffer, &buffer_size, roomfile);
	// get the second line,which does contain a room connection
	getline(&buffer, &buffer_size, roomfile);

	// create letterCheck variable to store values to check for do-while loop termination
	char letterCheck;
	
	/*  this do-while loop extracts the next connection room from the room file list until
	    it no longer finds a 'C' at the beginning of the line.  This works because the room
	    files have a specfic format where the first letter of the first will be 'R' (ROOM NAME),
	    the next 3+ lines will start with 'C' (CONNECTION X) and after all the connections
	    have been listed the final row will start 'R' (ROOM TYPE) again,
	*/
	do {
		// use strtok to break up the line by the semicolon or space
		strtok(buffer, ": ");
		// use it again to get the next delimited value
		strtok (NULL, "\n\r ");
		// the next use of strtok will get the connecting room name so tore it in a char array
		char* connection = strtok (NULL, "\n\r ");	
		// copy the room name into the next spot in the connList array
		strcpy (&connList[connCount][0], connection);
		// increase the count of connecting rooms
		connCount++;
		// get the next line for the next iteration
		getline(&buffer, &buffer_size, roomfile);
		// get the first letter of the line to check for loop termination
		letterCheck = buffer[0];
	} while (letterCheck == 'C');

	// print out the list of connecting rooms now stored in the connList array
	int i;
	printf("%s", connList[0]);
	for (i = 1; i < connCount; i++)
		printf(", %s", connList[i]);
	printf(".\n");	

	// close the file and free buffer to prevent memory leaks
	fclose(roomfile);
	free(buffer);
	buffer = NULL;
}

/***********************************************************************************************************
** Function: getRoomFile()
** Description: getRoomFile() takes a string representing a room name as a parameter, creates a complete 
** filepath to that room, and stores the filepath in the currentRoomFile global array.
***********************************************************************************************************/	
void getRoomFile(char* room)
{
	memset(currentRoomFile, '\0', sizeof (currentRoomFile));
	strcpy(currentRoomFile, filepath);
	strcat(currentRoomFile, room);
	strcat(currentRoomFile, "_room");
}

/***********************************************************************************************************
** Function: getRoomsData()
** Description: getRoomsData gets the directory containing the room files.  If there are multiple room file
** directories, it retrieves the directory most recently created. The function also populates the currentRoom
** global array with the Start Room (so the first call to locationMenu is correct) and populates END with
** the ending room to easily check if the user has achieved victory.
***********************************************************************************************************/
void getRoomsData() {
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
				// get entry details
				stat(fileInDir->d_name, &dirInfo);

				// if this entry is more recent
				if ((int)dirInfo.st_mtime > newestDirTime)
				{
					newestDirTime = (int)dirInfo.st_mtime;
					memset(newestDirectory, '\0', sizeof (newestDirectory));
					strcpy(newestDirectory, fileInDir->d_name);
				}
			}
		}
	}
		
	// close directory
	closedir(dirToCheck);
	
	// open the rooms directory with the end goal of storing the Start Room, End Room, and filepath to room files in global arrays
	DIR* dirFiles;
	dirFiles = opendir(newestDirectory);
	char filename[30];
	fileInDir = readdir(dirFiles);
	while ((fileInDir = readdir(dirFiles)) != NULL)
	{
		// loop through the room files, searching for the Start and End rooms
		// only look at files, skips over symbolic links, directories, etc.
		// source: https://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program/4204758
		if (fileInDir->d_type == DT_REG) {
			// flush the filename array and copy the room directory path into it
			memset(filename, '\0', sizeof (filename));
			strcpy(filename, newestDirectory);		
			strcat(filename, "/");
			// copy into filepath to use for opening files later
			strcpy(filepath, filename);
			// get full filepath to a room file 
			strcat(filename, fileInDir->d_name);

			// open the room files
			FILE* roomfile = fopen(filename, "r"); 
			// move file pointer to 8 bytes from the end of the file. This will put the file pointer in the last line of the 
			// room file, which is where ROOM TYPE is designated.
			fseek(roomfile, -8, SEEK_END);
			/* get the letter at this position and check to see if it is 'R'.  Room types can only be "START ROOM", "MID ROOM",
			   and "END ROOM".  The only room with an 'R' at this postion is "START ROOM".  Similarly, the only room with an 
			   'N' at this position is "END ROOM"
			*/
			char letter = fgetc(roomfile);
			// if START ROOM found
			if (letter == 'R')
			{
				// use fseek and SEEK_SET to set the file pointer 11 bytes in from the start of the file.  This is where the
				// name of the room starts
				fseek(roomfile, 11, SEEK_SET);
				// get the rest of the first line, which will be the full room name and store in currentRoom global array
				fgets(currentRoom, 10, roomfile);;
				// eat the newline character added by fgets
				int len = strlen(currentRoom);
				if (currentRoom[len-1]=='\n')
					currentRoom[len-1]='\0'; 
			}
			// if END ROOM found
			else if (letter == 'N')
			{
				// move file pointer to start of actual room name
				fseek(roomfile, 11, SEEK_SET);
				// store the full room name in the END global array
				fgets(END, 10, roomfile);;
				// eat the newline character added by gets
				int len = strlen(END);
				if (END[len-1]=='\n')
					END[len-1]='\0';
			}
			// close the file
			fclose(roomfile);		
		}
			
	}
	// close the directory
	closedir(dirFiles);

//	printf("Newest entry found is: %s\n", newestDirectory);
}
