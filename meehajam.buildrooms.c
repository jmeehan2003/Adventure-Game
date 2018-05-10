/*******************************************************************************
** Author: James Meehan
** Date: 05/08/2018
** Description:  This program builds the rooms for the adventure program. 7 of 10 
** hardcoded room names are randomly selected and files are created in a separate 
** directory with their ROOM NAME, CONNECTING ROOMS, and ROOM TYPE.
********************************************************************************/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

enum roomtype {START_ROOM, MID_ROOM, END_ROOM};

// hardcoded names of 10 rooms
char* realnames[10] = {"Prison", "Dungeon", "Hellpit", "Crypt", "Hall", "Castle", "Chamber", "Sewer", "Room101", "Yard" };
int selectedrooms[7];  // array to hold the seen randomly selected rooms
int selectedroomtype[7]; // array to hold randomly selected room types
int Connections[7][7]; // array to keep track of all the connections between rooms
int numConnections[7]; // array to keep track of the number of connection of a room
char Directory[20]; // array to hold name of the directory to create the room files in

void CreateRooms();
void printRooms(char*);
void AddRandomConnection();
bool IsGraphFull();
int GetRandomRoom();
void InitializeArrays();
bool CanAddConnectionFrom(int);
void ConnectRoom(int, int);
bool ConnectionAlreadyExists(int, int);
bool IsSameRoom(int, int);
void MakeDirectory();
void CreateGraph();

int main() 
{
	// seed srand for proper pseudorandom functions
	srand(time(NULL));

	InitializeArrays();

	MakeDirectory();

	// populate the selectedrooms and selectedroomtypes arrays
	CreateRooms();

	// create the connections between the rooms
	CreateGraph();

	printRooms(Directory);

	return 0;
}

/****************************************************************************
** Function: MakeDirectory()
** Description: MakeDirectory() creates the directory to hold the room files.
****************************************************************************/
void MakeDirectory() 
{	
	// get process id and store in pid
	int pid = getpid();
	
	// hardcode directory name except for pid
	strcpy(Directory, "meehajam.rooms.");

	// concantenate Directory and process id
	sprintf(Directory, "%s%d", Directory, pid);
	
	// create the directory
	int result = mkdir(Directory, 0755);

	// check that directory was created successfully
	if (result != 0) {
		printf("Making directory failed! Exiting program\n");
		exit(1);
	}
}

/****************************************************************************
** Function: CreateGraph()
** Description: CreateGraph() continues to add random connections until the 
** graph is full (each room has at least 3, but not more than 6, oubound
** connections.
****************************************************************************/
void CreateGraph() 
{
	while (IsGraphFull() == false)
	{
		AddRandomConnection();
	}

}

/****************************************************************************
** Function: IsGraphFull()
** Description: This function checks that all rooms have at least 3, but no
** more than 6, outbound connections and returns true if so and false otherwise.  
****************************************************************************/
bool IsGraphFull()
{
	int i;
	for (i = 0; i < 7; i++)
	{
		if (numConnections[i] < 3 || numConnections[i] > 7)
			return false;
	}

	return true;
}

/****************************************************************************
** Function: InitializeArrays()
** Description: Initialize the arrays with appropriate values
****************************************************************************/
void InitializeArrays() 
{
	// initialize room and connection arrays to have all elements be -1
	memset(selectedrooms, -1, sizeof (selectedrooms));
	memset(selectedroomtype, -1, sizeof (selectedroomtype));
	memset(Connections, -1, sizeof (Connections));
	
	// initialize numConnections array to all 0's
	memset(numConnections, 0, sizeof (numConnections));
	
	// initialize directory to have all null terminators
	memset(Directory, '\0', sizeof (Directory));
}


/****************************************************************************
** Function: printRooms()
** Description: printRoomS() creates the room file names and places in the 
** previously created room files directory.  It then writes the room information
** (room name, connections, and room type) to each room file.
****************************************************************************/
void printRooms(char* Directory) 
{
	int i, j;
	// for each of the seven rooms
	for (i = 0; i < 7; i++)
	{
		// Create filename
		// Put directory of file into string filePath
		char filePath[30];
		memset(filePath, '\0', sizeof(filePath));
		strcpy(filePath, Directory);
		strcat(filePath, "/");
	
		// create file name and concatenate with filePath
		char fileName[15];
		memset(fileName, '\0', sizeof(fileName));
		// each index in selectedrooms was provided a unique random between 0 and 9
		// which in turn references a name value from the realnames array
		strcpy(fileName, realnames[selectedrooms[i]]);
		// append "_room" to the filename
		strcat(fileName, "_room");
		strcat(filePath, fileName);
		
		//open file for writing
		FILE* roomfile = fopen(filePath, "w");

		// print room name to file
		fprintf(roomfile, "ROOM NAME: %s\n", realnames[selectedrooms[i]]);

		// initialize count variable for numbering connections
		int count = 1;	
		
		// every time an integer in a column besides -1 is encountered, there is a connection and
		// this is added to the file
		for (j = 0; j < 7; j++) 
		{
			if (Connections[i][j] != -1) 
			{
				fprintf(roomfile, "CONNECTION %d: %s\n", count, realnames[selectedrooms[j]]);
				count++;	

			}

		}

		// get the room type associated with this room and write it to the file.  The START and END 
		// ROOM values of 0 and 1 were randomly assigned to the selectedroomtype array earlier
		// (using enums) and then the remainder of the rooms were made MID ROOMS.  
		int type = selectedroomtype[i];
		if (type == 0)	
			fprintf(roomfile, "ROOM TYPE: START_ROOM\n");
		else if (type == 1) 
			fprintf(roomfile, "ROOM TYPE: MID_ROOM\n");
		else if (type == 2) 
			fprintf(roomfile, "ROOM TYPE: END_ROOM\n");
		else 
			fprintf(roomfile, "ROOM TYPE: SUPER_BONUS_ROOM (aka big error in getting room type\n");
		
		// close the file
		fclose(roomfile);
	}

}

/******************************************************************************************************************
** Function: AddRandomConnection()
** Description: This functions adds a valid outbound connection from one room to another to the graph.
******************************************************************************************************************/
void AddRandomConnection()
{
	//ints to represent the two rooms
	int A;
	int B;
	
	// loop until broken
	while(true)
	{
		// assign a random room to A
		A = GetRandomRoom();
		// break the loop if A can add another connection, otherwise loop again and get another random room
		if (CanAddConnectionFrom(A) == true)
			break;

	}
	
	/* with a valid room looking for a connection assigned to A, find a valid room for B that can connect to A:
	   B is assigned a random rom and if that room can add another connection, is not the same room as A, and their 
           is currently not connection between the two rooms, then it is a valid connection.  Otherwise, loop again and 
           get a new random room for B.
        */

	do
	{
		// assign random room to B
		B = GetRandomRoom();;
	}
	while(CanAddConnectionFrom(B) == false || IsSameRoom(A, B) == true || ConnectionAlreadyExists(A, B) == true);

	// now that we have validated the rooms can be connected, connect them.	
	ConnectRoom(A, B);

}

/****************************************************************************
** Function: GetRandomRoom()
** Description: GetRandomRoom() returns a random room (an integer that corresponds
** with the index value of the realnames array).  It does not validate if the
** connection can bde added  
****************************************************************************/
int GetRandomRoom()
{
	// get a number between 0 and 6 to get a random room in the selectedrooms array
	int room = rand() % 7;
	return room;
}

/****************************************************************************
** Function: CanAddConnectionFrom()
** Description: This function returns true if a connection can be added from
** from room x (if room x currently has less than 6 outbound connections) and
** false otherwise.
****************************************************************************/
bool CanAddConnectionFrom(int x){
	int count = 0;
	int i;
	for (i = 0; i < 7; i++) 
	{
		// -1 represent an available location in the Connections array
		if (Connections[x][i] == -1)
			count++;

		// break loop early if a connection is found
		if (count > 1)
			return true;	
	}
	
	// return false if no connection found
	return false;
}

/****************************************************************************
** Function: ConnectRoom()
** Description: ConnectRoom() connects rooms x and y together but does not 
** check if the connection is valid.
****************************************************************************/
void ConnectRoom(int x, int y)
{
	/* Connect rooms x and y.  Any value other than -1 represents a connection
	   in the Connections array and I though I'd spice it up and use 3 instead of 1
	   for a connection.  The connection is made both ways because if A connects to
	   B, then B must have a connection back to A.
	*/
	Connections[x][y] = 3;
	Connections[y][x] = 3;

	// increase the number of Connections for each room in the numConnections array
	numConnections[x]++;
	numConnections[y]++;

}

/****************************************************************************
** Function: CreateRooms()
** Description: CreateRooms() populates the selectedrooms and selectedroomtype
** arrays with random values
****************************************************************************/
void CreateRooms() 
{
	int room, i, j;
	int start, end, valid;
	// set starting and end room values to -1
	start = end = -1;
	// go through 7 iterations to get the 7 room names
	for (i = 0; i < 7; i++)
	{
		// loop until a number not already in selectedrooms array can be placed in
		while (selectedrooms[i] == -1) 
		{
			// valid acts as bool here with 1 for true (valid room name) and 0 for false
			valid = 1;
			// get a random number between 0 and 9 
			room = rand() % 10;
			// loops through number already in selectedrooms to see if it's a duplicate
			for (j = 0; j < i; j++) 
			{
				// if it's a duplicate, set valid bool to false to run the loop again
				if (room == selectedrooms[j])
					valid = 0;			
			}	
			
			// if it's not a duplicate, place the value in the array
			if (valid)
				selectedrooms[i] = room;
		}
		
	}

	/* Now the room names have been randomly selected.  The values in selectedrooms at each index match represent
	   index in realnames where the name of the room is (i.e. if selectedrooms[0] = 7, then the first room name is
	   realnames[7], which equals "Sewer".
	*/

	/* randomly place START ROOM:
	  get a random number between 0 and 6.  Whatever number is provided reprsents the index in selectedrooms that 
	  corresponds to the START ROOM.  For example, if the random number is 5, then selectedrooms[5] is the START ROOM.
	*/
	start = rand() % 7;
	selectedroomtype[start] = START_ROOM;
	
	/* randomly place END ROOM:
 	   follow the same procedure as setting the START ROOM but also check that the value isn't the same as the already
           randomly selected START ROOM. 
	*/
	do {
		end = rand() % 7;
	}
	while (end == start); 
	selectedroomtype[end] = END_ROOM;

	// Now that the START and END rooms have been randomly assigned, the remaining rooms must be mid rooms
	for (i = 0; i < 7; i++)
	{
		if (selectedroomtype[i] == -1)
			selectedroomtype[i] = MID_ROOM;
	}	
}

/****************************************************************************
** Function: ConnectionAlreadyExists()
** Description: This function checks and returns true is a connenction already
** exists from x to y and false otherwise.
****************************************************************************/
bool ConnectionAlreadyExists(int x, int y)
{
	// 3 represents a valid connection from x to y in Connections
	return (Connections[x][y] == 3);
}

/****************************************************************************
** Function: IsSameRoom()
** Description: IsSameRoom() returns true if rooms x and y are the same and 
** falst otherwise.
****************************************************************************/
bool IsSameRoom(int x, int y)
{
	// if x equals y then they are the same room
	return (x == y);
}
