#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

enum roomtype {START_ROOM, MID_ROOM, END_ROOM};

struct Room {
	int id;
	char* name;
	char* type;
	int numOutboundConnections;
	struct room* outboundConnections[6];
};

struct Room rooms[7];

char* realnames[10] = {"Prison", "Dungeon", "Hellpit", "Crypt", "Hall", "Castle", "Chamber", "Sewer", "Room101", "Yard" };
//int selectedrooms[7] = {-1, -1, -1, -1, -1, -1, -1};
//int selectedroomtype[7] = {-1, -1, -1, -1, -1, -1, -1};
int selectedrooms[7];
int selectedroomtype[7];
int Connections[7][7];
int numConnections[7];
char Directory[20];

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
	srand(time(NULL));

	InitializeArrays();

	MakeDirectory();

	CreateRooms();

	CreateGraph();

	printRooms(Directory);

	return 0;
}

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

void CreateGraph() 
{
	while (IsGraphFull() == false)
	{
		AddRandomConnection();
	}

}


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


void printRooms(char* Directory) 
{
	int i, j;
	for (i = 0; i < 7; i++)
	{
		// Create filename
		// Put directory of file into string filePath
		char filePath[30];
		memset(filePath, '\0', sizeof(filePath));
		strcpy(filePath, Directory);
		strcat(filePath, "/");
	
		// create file name and concatenate with filePAth
		char fileName[15];
		memset(fileName, '\0', sizeof(fileName));
		strcpy(fileName, realnames[selectedrooms[i]]);
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

		// get the room type associated with this room and write it to the file
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

void AddRandomConnection()
{
	int A;
	int B;
	
	while(true)
	{
		A = GetRandomRoom();
		if (CanAddConnectionFrom(A) == true)
			break;

	}

	do
	{
		int a, b;
		B = GetRandomRoom();;
	}
	while(CanAddConnectionFrom(B) == false || IsSameRoom(A, B) == true || ConnectionAlreadyExists(A, B) == true);

	ConnectRoom(A, B);

}

int GetRandomRoom()
{
	// get a number between 0 and 6 to get a random room in the selectedrooms array
	int room = rand() % 7;
	return room;
}

bool CanAddConnectionFrom(int x){
	int count = 0;
	int i;
	for (i = 0; i < 7; i++) 
	{
		if (Connections[x][i] == -1)
			count++;

		if (count > 1)
			return true;	
	}
	
	return false;
}

void ConnectRoom(int x, int y)
{
	Connections[x][y] = 3;
	Connections[y][x] = 3;

	// increase the number of Connections for each room in the numConnections array
	numConnections[x]++;
	numConnections[y]++;

}

void CreateRooms() 
{
	
	int room, i, j;
	int start, end, valid;
	start = end = -1;
	for (i = 0; i < 7; i++)
	{
		while (selectedrooms[i] == -1) 
		{
			valid = 1;
			room = rand() % 10;
			for (j = 0; j < i; j++) 
			{
				if (room == selectedrooms[j])
					valid = 0;			
			}	
			
			if (valid)
				selectedrooms[i] = room;

		}
		
	}

	// randomly place start room
	start = rand() % 7;
	selectedroomtype[start] = START_ROOM;
	
	// radnomly place end room
	do {
		end = rand() % 7;
	}
	while (end == start); 
	selectedroomtype[end] = END_ROOM;

	// make the remaining rooms mid rooms
	for (i = 0; i < 7; i++)
	{
		if (selectedroomtype[i] == -1)
			selectedroomtype[i] = MID_ROOM;
	}	

}

bool ConnectionAlreadyExists(int x, int y)
{
	return (Connections[x][y] == 3);
		
}

bool IsSameRoom(int x, int y)
{
	// if x equals y then they are the same room
	return (x == y);

}
