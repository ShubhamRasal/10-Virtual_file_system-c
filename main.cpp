#define _CRT_SECURE_NO_WARNINGS//only works on window to hide warnings
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<io.h>
#include<Windows.h>
#include<conio.h> 
#include <direct.h>
#define MAXINODE 50

#define READ 1
#define WRITE 2

#define MAXFILESIZE 1024

#define REGULAR 1
#define SPECIAL 2

#define START 0
#define CURRENT 1
#define END 2

typedef struct superblock
{
	int TotalInodes;
	int FreeInodes;
}SUPERBLOCK, *PSUPERBLOCK;

typedef struct inode
{
	char FileName[50];
	int InodeNumber;
	int FileSize;
	int FileActualSize;
	int FileType;
	char *Buffer;
	int LinkCount;
	int ReferenceCount;
	int permission;
	struct inode* next;
}INODE, *PINODE, **PPINODE;

typedef struct filetable
{
	int readoffset;
	int writeoffset;
	int count;
	int mode;
	PINODE ptrnode;
}FILETABLE, *PFILETABLE;

typedef struct ufdt
{
	PFILETABLE ptrfiletable;
}UFDT;

UFDT UFDTArr[50];
SUPERBLOCK SUPERBLOCKobj;
PINODE head = NULL;
COORD coord = { 0,0 }; // this is global variable
									/*center of axis is set to the top left cornor of the screen*/
void gotoxy(int x, int y)
{
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void InitialiseSuperBlock()
{
	int i = 0;
	while (i < MAXINODE)
	{
		//UFDTArr[i].ptrfiletable = NULL;
		i++;
	}
	SUPERBLOCKobj.TotalInodes = MAXINODE;
	SUPERBLOCKobj.FreeInodes = MAXINODE;
	//printf("inside ISB!!!\n");
}
void CreateDILB()
{
	int i = 0;
	PINODE newn = NULL;
	PINODE temp = NULL;

	while (i <= MAXINODE)
	{
		newn = (PINODE)malloc(sizeof(INODE));

		newn->LinkCount = newn->ReferenceCount = 0;
		newn->FileType = newn->FileSize = 0;
		newn->Buffer = NULL;
		newn->next = NULL;
		newn->InodeNumber = i;
		if (temp == NULL)
		{
			head = newn;
			temp = head;
		}
		else
		{
			temp->next = newn;
			temp = temp->next;
		}
		i++;

	}
	//printf("DILB created successfully!!!");
}

void ls_file()
{
	int i = 0;
	PINODE temp = head;
	if (SUPERBLOCKobj.FreeInodes == MAXINODE)
	{
		printf("ERROR:There are no files!!\n");
		return;
	}
	printf("\nFile Name\tInode Number\tFile Size\tLink Count\n");
	printf("--------------------------------------------------------------\n");
	
	while (temp != NULL)
	{
		if (temp->FileType != 0)
		{
			printf("%s\t\t%d\t\t%d\t\t%d\n", temp->FileName, temp->InodeNumber, temp->FileActualSize, temp->LinkCount);
		}
		temp = temp->next;
	}
	printf("--------------------------------------------------------------\n");
}

PINODE Get_Inode(char *name)
{
	PINODE temp = head;
	int i = 0;
	if (name == NULL)
	{
		return NULL;
	}
	while (temp != NULL)
	{
		if (strcmp(name, temp->FileName) == 0)
		{
			break;
		}
		temp = temp->next;
	}
	return temp;
}

int CreateFilex(char *name, int permission)
{
	int i = 1;
	PINODE temp = head;
	if ((name == NULL) || (permission == 0) || (permission > 3))
	{
		return -1;
	}
	if (SUPERBLOCKobj.FreeInodes == 0)
	{
		return -2;
	}
	if (Get_Inode(name) != NULL)
	{
		return -3;//file name already exists
	}
	(SUPERBLOCKobj.FreeInodes)--;

	while (temp != NULL)
	{
		if (temp->FileType == 0)
		{
			break;
		}
		temp = temp->next;
	}
	while (i < 50)
	{
		if (UFDTArr[i].ptrfiletable == NULL)
		{
			break;
		}
		i++;
	}

	UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));
	if (UFDTArr[i].ptrfiletable == NULL)
	{
		return -4;
	}

	UFDTArr[i].ptrfiletable->count = 1;
	UFDTArr[i].ptrfiletable->mode = permission;
	UFDTArr[i].ptrfiletable->readoffset = 0;
	UFDTArr[i].ptrfiletable->writeoffset = 0;
	
	UFDTArr[i].ptrfiletable->ptrnode = temp;
	strcpy_s(UFDTArr[i].ptrfiletable->ptrnode->FileName, name);

	UFDTArr[i].ptrfiletable->ptrnode->FileType = REGULAR;
	UFDTArr[i].ptrfiletable->ptrnode->ReferenceCount = 1;
	UFDTArr[i].ptrfiletable->ptrnode->LinkCount = 1;
	UFDTArr[i].ptrfiletable->ptrnode->FileSize = MAXFILESIZE;
	UFDTArr[i].ptrfiletable->ptrnode->FileActualSize = 0;
	UFDTArr[i].ptrfiletable->ptrnode->permission = permission;
	UFDTArr[i].ptrfiletable->ptrnode->Buffer = (char*)malloc(MAXFILESIZE);
	memset(UFDTArr[i].ptrfiletable->ptrnode->Buffer, 0, MAXFILESIZE);
	
	return i;
}

int OpenFile(char *name, int mode)
{
	int i = 0;
	PINODE temp = NULL;
	if (name == NULL || mode <= 0)
	{
		return -1;
	}
	temp = Get_Inode(name);
	if(temp==NULL)
	{
		return -2;
	}
	if (temp->permission < mode)
	{
		return -3;
	}
	while (i < 50)
	{
		if (UFDTArr[i].ptrfiletable == NULL)
		{
			break;
		}
		i++;
	}
	UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));

	if (UFDTArr[i].ptrfiletable == NULL)
	{
		return -1;
	}
	UFDTArr[i].ptrfiletable->count = 1;
	UFDTArr[i].ptrfiletable->mode = mode;
	if (mode == READ + WRITE)
	{
		UFDTArr[i].ptrfiletable->readoffset = 0;
		UFDTArr[i].ptrfiletable->writeoffset = 0;
	}
	else if (mode == READ)
	{
		UFDTArr[i].ptrfiletable->readoffset = 0;;
	}
	else if (mode == WRITE)
	{
		UFDTArr[i].ptrfiletable->writeoffset = 0;
	}
	UFDTArr[i].ptrfiletable->ptrnode = temp;
	(UFDTArr[i].ptrfiletable->ptrnode->ReferenceCount)++;
	
	printf("inside open_file\n");
	return i;

}


int ReadFile(int fd, char* arr, int isize)
{
	int read_size = 0;
	if (UFDTArr[fd].ptrfiletable == NULL)
	{
		printf("hjkd");
		return -1;
	}
	if ((UFDTArr[fd].ptrfiletable->mode != READ )&&( UFDTArr[fd].ptrfiletable->mode != READ + WRITE))
	{
		return -2;// incorrect parameter
	}
	if (UFDTArr[fd].ptrfiletable->ptrnode->permission != READ && UFDTArr[fd].ptrfiletable->ptrnode->permission != READ + WRITE)
	{
		return -2;// permission denied
	}
	if (UFDTArr[fd].ptrfiletable->readoffset == UFDTArr[fd].ptrfiletable->ptrnode->FileActualSize)
	{
		return -3;// reached at end of file
	}
	if (UFDTArr[fd].ptrfiletable->ptrnode->FileType != REGULAR)
	{
		return-4;
	}
	read_size = (UFDTArr[fd].ptrfiletable->ptrnode->FileActualSize) - (UFDTArr[fd].ptrfiletable->readoffset);
	if (read_size < isize)
	{
		strncpy(arr, (UFDTArr[fd].ptrfiletable->ptrnode->Buffer) + (UFDTArr[fd].ptrfiletable->readoffset), read_size);
		UFDTArr[fd].ptrfiletable->readoffset = UFDTArr[fd].ptrfiletable->readoffset + read_size;
	}
	else
	{
		
		strncpy(arr, (UFDTArr[fd].ptrfiletable->ptrnode->Buffer) + (UFDTArr[fd].ptrfiletable->readoffset),isize);
		UFDTArr[fd].ptrfiletable->readoffset = UFDTArr[fd].ptrfiletable->readoffset + isize;
	}

	return isize;
}

int GetFDFromName(char* name)
{
	int i = 0;
	while (i < 50)
	{
		if (UFDTArr[i].ptrfiletable != NULL)
		{
			if (_stricmp((UFDTArr[i].ptrfiletable->ptrnode->FileName), name) == 0)
			{
				break;
			}
		}
		i++;
	}

	if (i == 50)
	{
		return -1;
	}
	else
	{
		return i;
	}

}

int WriteFile(int fd, char *arr, int isize)
{
//	printf("inside write\n");
	if ((UFDTArr[fd].ptrfiletable->mode != WRITE) && (UFDTArr[fd].ptrfiletable->mode != WRITE + READ))
	{
		return-1;
	}
	if ((UFDTArr[fd].ptrfiletable->ptrnode->permission) != WRITE && (UFDTArr[fd].ptrfiletable->ptrnode->permission) != WRITE + READ)
	{
		return -1;
	}
	if ((UFDTArr[fd].ptrfiletable->writeoffset) == MAXFILESIZE)
	{
		return -2;// write pointer is at the end of file  (no memory )
	}
	if ((UFDTArr[fd].ptrfiletable->ptrnode->FileType) != REGULAR)
	{
		return -3;
	}
	strncpy((UFDTArr[fd].ptrfiletable->ptrnode->Buffer) + (UFDTArr[fd].ptrfiletable->writeoffset), arr, isize);
	(UFDTArr[fd].ptrfiletable->writeoffset) == ((UFDTArr[fd].ptrfiletable->writeoffset) + isize);
	(UFDTArr[fd].ptrfiletable->ptrnode->FileActualSize) = ((UFDTArr[fd].ptrfiletable->ptrnode->FileActualSize) + isize);

	
	return isize;

}


int LseekFile(int fd, int size, int from)
{
	if ((fd < 0) || (from > 2))
	{
		return -2;
	}
	if (UFDTArr[fd].ptrfiletable == NULL)
	{
		return -1;
	}
	if ((UFDTArr[fd].ptrfiletable->mode == READ) && (UFDTArr[fd].ptrfiletable->mode == READ + WRITE))
	{
		if (from == CURRENT)
		{
			if (((UFDTArr[fd].ptrfiletable->readoffset) + size) > (UFDTArr[fd].ptrfiletable->ptrnode->FileActualSize))
			{
				return -1;
			}
			if (((UFDTArr[fd].ptrfiletable->readoffset) + size) > 0)
			{
				return -1;
			}

			UFDTArr[fd].ptrfiletable->readoffset = (UFDTArr[fd].ptrfiletable->readoffset) + size;
		}//current ends
		else if (from == START)
		{
			if (size > (UFDTArr[fd].ptrfiletable->ptrnode->FileActualSize))
			{
				return -1;
			}
			if (size < 0)
			{
				return -1;
			}
			(UFDTArr[fd].ptrfiletable->readoffset) = size;
		}
		else if (from == END)
		{
			if ((UFDTArr[fd].ptrfiletable->ptrnode->FileActualSize) + size > MAXFILESIZE)
			{
				return -1;
			}
			if (((UFDTArr[fd].ptrfiletable->readoffset) + size) < 0)
			{
				return -1;
			}
			(UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->ptrnode->FileActualSize) + size;
		}
	}//outer if
	else if(UFDTArr[fd].ptrfiletable->mode==WRITE)
		{
			if (from == CURRENT)
			{
				if (((UFDTArr[fd].ptrfiletable->writeoffset) + size) > MAXFILESIZE)
				{
					return -1;
				}
				if (((UFDTArr[fd].ptrfiletable->writeoffset) + size) < 0)
				{
					return -1;
				}
				if (((UFDTArr[fd].ptrfiletable->writeoffset) + size) > (UFDTArr[fd].ptrfiletable->ptrnode->FileActualSize))
				{
					(UFDTArr[fd].ptrfiletable->ptrnode->FileActualSize) = (UFDTArr[fd].ptrfiletable->writeoffset) + size;
				}
				(UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset) + size;
			}
			else if(from==START)
			{
				if (size < MAXFILESIZE)
				{
					return -1;
				}
				if (size < 0)
				{
					return -1;
				}
				if (size > (UFDTArr[fd].ptrfiletable->ptrnode->FileActualSize))
				{
					(UFDTArr[fd].ptrfiletable->ptrnode->FileActualSize) = size;
				}
				(UFDTArr[fd].ptrfiletable->writeoffset) = size;
			}
			else if (from == END)
			{
				if ((UFDTArr[fd].ptrfiletable->ptrnode->FileActualSize) + size > MAXFILESIZE)
				{
					return-1;
				}
				if (((UFDTArr[fd].ptrfiletable->writeoffset) + size) < 0)
				{
					return -1;
				}
				(UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->ptrnode->FileActualSize) + size;
			}
	}
}

void CloseAll()
{
	int i = 0;
	while (i < 50)
	{
		if (UFDTArr[i].ptrfiletable != NULL)
		{
			UFDTArr[i].ptrfiletable->readoffset = 0;
			UFDTArr[i].ptrfiletable->writeoffset = 0;
			(UFDTArr[i].ptrfiletable->ptrnode->ReferenceCount)--;
			break;
		}
		i++;
	}
}

int stat_file(char *name)
{
	PINODE temp = head;
	int i = 0;
	if (name == NULL)
	{
		return -1;
	}

	while (temp != NULL)
	{
		if (strcmp(name, temp->FileName) == 0)
		{
			break;
		}
		temp = temp->next;
	}
	if (temp == NULL)
	{
		return -2;
	}
	printf("\n--------Statistical Information about file--------\n");
	printf("FILE NAME:\t\t%s\n", temp->FileName);
	printf("INODE NUMBER:\t\t%d\n", temp->InodeNumber);
	printf("FILE SIZE:\t\t%d\n", temp->FileSize);
	printf("ACTUAL SIZE:\t\t%d\n", temp->FileActualSize);
	printf("LINK COUNT:\t\t%d\n", temp->LinkCount);
	printf("REFERENCE COUNT:\t\t%d\n", temp->ReferenceCount);
	if (temp->permission == 1)
	{
		printf("FILE PERMISSION:\tREAD ONLY\n");
	}
	else if (temp->permission == 2)
	{
		printf("FILE PERMISSION:\tWRITE\n");
	}
	else if (temp->permission == 3)
	{
		printf("FILE PERMISSION:\tREAD & WRITE\n");
	}
	printf("\n-----------------------------------------------------\n\n");
	return 0;
}

void CloseByName(int fd)
{
	UFDTArr[fd].ptrfiletable->readoffset = 0;
	UFDTArr[fd].ptrfiletable->writeoffset = 0;
	(UFDTArr[fd].ptrfiletable->ptrnode->ReferenceCount)--;
}

int CloseByName(char *name)
{
	int i = 0;
	i = GetFDFromName(name);
	if (i == -1)
	{
		return -1;
	}
	UFDTArr[i].ptrfiletable->readoffset = 0;
	UFDTArr[i].ptrfiletable->writeoffset = 0;//commnet if you want to write at 
	(UFDTArr[i].ptrfiletable->ptrnode->ReferenceCount)--;//the end always
	//printf("inside close by name\n");
	return 0;
}

int rm_File(char* name)
{
	int fd = 0;
	fd = GetFDFromName(name);
	if (fd == -1)
	{
		return -1;
	}
	(UFDTArr[fd].ptrfiletable->ptrnode->LinkCount)--;
		if ((UFDTArr[fd].ptrfiletable->ptrnode->LinkCount) == 0)
		{
			UFDTArr[fd].ptrfiletable->ptrnode->FileType =0 ;
			UFDTArr[fd].ptrfiletable->ptrnode->FileName[0] ='\0';
			free(UFDTArr[fd].ptrfiletable);
		}
		UFDTArr[fd].ptrfiletable = NULL;
		(SUPERBLOCKobj.FreeInodes)++;
}

int fstat_File(int fd)
{

	PINODE temp = head;
	int i = 0;
	if (fd<0)
	{
		return -1;
	}


	if (UFDTArr[fd].ptrfiletable == NULL)
	{
		return -2;
	}
	
	temp = UFDTArr[fd].ptrfiletable->ptrnode;//going to correct file from File table
	printf("\n--------Statistical Information about file--------\n");
	printf("FILE NAME:\t\t%s\n", temp->FileName);
	printf("INODE NUMBER:\t\t%d\n", temp->InodeNumber);
	printf("FILE SIZE:\t\t%d\n", temp->FileSize);
	printf("ACTUAL SIZE:\t\t%d\n", temp->FileActualSize);
	printf("LINK COUNT:\t\t%d\n", temp->LinkCount);
	printf("REFERENCE COUNT:\t\t%d\n", temp->ReferenceCount);
	if (temp->permission == 1)
	{
		printf("FILE PERMISSION:\tREAD ONLY\n");
	}
	else if (temp->permission == 2)
	{
		printf("FILE PERMISSION:\tWRITE\n");
	}
	else if (temp->permission == 3)
	{
		printf("FILE PERMISSION:\tREAD & WRITE\n");
	}
	printf("\n-----------------------------------------------------\n\n");
	return 0;

}

int truncate_File(char *name)
{
	int fd = GetFDFromName(name);
	if (fd == -1)
	{
		return -1;
	}
	memset(UFDTArr[fd].ptrfiletable->ptrnode->Buffer, 0, 1024);
	UFDTArr[fd].ptrfiletable->readoffset = 0;
	UFDTArr[fd].ptrfiletable->writeoffset = 0;
	UFDTArr[fd].ptrfiletable->ptrnode->FileActualSize = 0;
}

int Backup()
{
	FILE *fp = NULL;
	PINODE temp = head;
	if (temp == NULL)
	{
		return -1;
	}
	char *ctemp = NULL;
	char* buffer=NULL;
	if (system("mkdir BACKUP") == -1)
	{
		printf("");
	}
	
	if (_chdir("BACKUP") == 0)
	{
		if( (buffer = _getcwd(NULL, 0)) != 0)
		{
			printf("PATH: %s\t",buffer);
		}
	}
	else
	{
		printf("ERROR:UNABLE TO LOCATE PATH !\n");
	}
	while (temp->FileType != 0)
	{
		if (temp->FileName[0] == '\0')
		{
			break;
		}
	
		if ((fp = fopen(temp->FileName, "w")) == NULL)
		{
			return -1;
		} 
		ctemp = temp->Buffer;
		while (*ctemp != '\0')
		{
			fwrite(ctemp, sizeof(char), 1, fp);
			//printf("%d\n", ++i);
			ctemp++;
		}
		temp = temp->next;
	}
	fclose(fp);

	return 0;
}

int restore()
{

	HANDLE hFind;
	WIN32_FIND_DATA FindFileData;
	FILE* fp = NULL;
	int ret = 0;
	char ch;
	int i = 0;
	char* buffer;
	char arr[1024];
	_chdir("BACKUP");
	if ((buffer = _getcwd(NULL, 0)) != 0)
	{
		printf("PATH: %s\t", buffer);
	}
	strcat(buffer, "\\*.txt");
	
	if ((hFind = FindFirstFile(buffer, &FindFileData)) != INVALID_HANDLE_VALUE) {
		do {
			printf("%s\n", FindFileData.cFileName);
			ret = CreateFilex(FindFileData.cFileName, 3);
			fp = fopen(FindFileData.cFileName,"r");
			//ctemp = UFDTArr[ret].ptrfiletable->ptrnode->Buffer;
			if (fp == NULL)
			{
				return -1;
			}
			while (1)
			{
				ch = fgetc(fp);
				if (ch == EOF)
				{
					arr[i] = '\0';
					break;
				}
				arr[i] = ch;
				//printf("%c", ch);
				i++;
			}
			WriteFile(ret,arr,strlen(arr));
		
			i = 0;
		} while (FindNextFile(hFind, &FindFileData));
		FindClose(hFind);
	}
	else 
	{
		return -1;
	}
	return 0;
}




int main()
{
	char *ptr = NULL;
	int i = 0;
	int ret = 0, fd = 0, count = 0;
	char command[4][80], str[80], arr[1024];
	char user[20], pass1[8], pass[8] = "1234",user1[7]="root";

	while (1)
	{
		printf("Enter User Name-\n");
		scanf("%s", user);
		printf("Enter password-\n");
		scanf("%s", pass1);
		fflush(stdin);
		
		if (strcmp(user1, user) != 0)
		{
			printf("Incorrect User Id \n");
			continue;
		}
		if (strcmp(pass, pass1) != 0) 
		{
			printf("Incorrect Password !\n");
			continue;
		}
		else
		{
			fflush(stdin);
			system("cls");
			gotoxy(20, 10);
			printf("LOADING !");
			for (i = 0; i < 25; i++)
			{
				printf("!");
				Sleep(200);
			}
			system("cls");
			break;
		}
	} 
	
	InitialiseSuperBlock();
	CreateDILB();
	fflush(stdin);
	i = 0;
	while (1)
	{
		fflush(stdin);
		strcpy_s(str,"");
		printf("\nMarvellous VFS:->");
		fgets(str, 80, stdin);//80 coz strength of commandline is 80
		if ((count == 0)&&(i==0))
		{
			system("cls");
			i++;
			continue;
		}
		count = sscanf(str, "%s%s%s%s", command[0], command[1], command[2], command[3]);
		
		if (count == 1)
		{
			if (_stricmp(command[0], "ls") == 0)
			{
				ls_file();
			}
			else if (_stricmp(command[0], "closeall") == 0)
			{
				CloseAll();
				printf("All files are Closed Successfully!!!");
			}
			else if (_stricmp(command[0], "clear") == 0)
			{
				system("cls");
				//fflush(stdin);
				continue;
			}
			else if (_stricmp(command[0], "exit") == 0)
			{
				printf("Terminating the Marvellous Virtual File System\n");
				for (i = 0; i < 10; i++)
				{
					printf("o   ");
					Sleep(100);

				}
				break;
			}
			else if (_stricmp(command[0], "help") == 0)
			{
				//DisplayHelp();
				continue;
			}
			else if (_stricmp(command[0], "backup") == 0)
			{
				ret= Backup();
				if (ret == -1)
				{
					printf("ERROR:Unable to Backup !\n");
					continue;
				}
				else {
					printf("Successfully Backuped !\n");
				}
			}
			else if (_stricmp(command[0], "restore") == 0)
			{
				ret = restore();
				if (ret == -1)
				{
					printf("ERROR:Unable to restore !\n");
					continue;
				}
				else {
					printf("Restored.\n");
				}
			}
			else
			{
				printf("ERROR:Command not Found!!!\n");
				continue;
			}
		}
		else if (count == 2)
			{
				if (_stricmp(command[0], "write") == 0)
				{
					fd = GetFDFromName(command[1]);
					if (fd == -1)
					{
						printf("ERROR:Incorrect Parameter\n");
						continue;
					}
					printf("Enter Data:\n");
					scanf("%[^\n]", arr);
			
					ret = strlen(arr);
					if (ret == 0)
					{
						printf("ERROR:Incorrect Paramter\n");
								continue;
					}
					ret = WriteFile(fd, arr, ret);
					if (ret == -1)
					{
						printf("ERROR:Permission Denied!!");
					}
					if (ret == -2)
					{
						printf("ERROR:No sufficient memory to write\n");
					}
					if (ret == -3)
					{
						printf("ERROR:It is not Regular file\n");
					}
				}//write if ends 
				else if (_stricmp(command[0], "stat") == 0)
				{
					ret = stat_file(command[1]);
					if (ret == -1)
					{
						printf("ERROR:INCORRECT PARAMETERS\n");
					}
					if (ret == -2)
					{
						printf("ERROR:THERE IS NO SUCH FILE!!\n");
					}
					continue;
				}
				else if (_stricmp(command[0], "close") == 0)
				{
					ret = CloseByName(command[1]);
					if (ret == -1)
					{
						printf("ERROR:There is no such File\n");
					}
					else
					{
						printf("closed:%s", command[1]);
					}
					continue;
				}//closebyName ends
				else if (_stricmp(command[0], "rm") == 0)
				{
					ret = rm_File(command[1]);
					if (ret == -1)
					{
						printf("ERROR:NO FILE FOUND!!");
						continue;
					}
					else
					{
						printf("removed!\n");
					}

				}//rm ends
				else if (_stricmp(command[0], "fstat") == 0)
				{
					ret = stat_file(command[1]);
					if (ret == -1)
					{
						printf("ERROR:INCORRECT PARAMETERS\n");
					}
					if (ret == -2)
					{
						printf("ERROR:THERE IS NO SUCH FILE!!\n");
					}
					continue;
				}
				else if (_stricmp(command[0], "truncate") == 0)
				{
					ret = truncate_File(command[1]);
					if (ret == -1)
					{
						printf("ERROR:Incorrect Paramter !\n");
					}
				}
				else
				{
					printf("\nERROR:Command not found !!\n");
					continue;
				}
			}
			else if (count == 3)
			{
				if (_stricmp(command[0], "create") == 0)
				{
					ret = CreateFilex(command[1], atoi(command[2]));
					if (ret >= 0)
					{
						printf("File is successfully  created with file descriptor:%d\n", ret);
					}
					if (ret == -1)
					{
						printf("Incorrect parameters\n");
					}
					if (ret == -2)
					{
						printf("ERROR:There is no inodes\n");
					}
					if (ret == -3)
					{
						printf("ERROR:File already exists\n");
					}
					if (ret == -4)
					{
						printf("ERROR:Memory allocation failure\n");
					}
					continue;
				}//create file if ends
				else if (_stricmp(command[0], "open") == 0)//open**
				{
					ret = OpenFile(command[1], atoi(command[2]));
					if (ret >= 0)
					{
						printf("File is successfully opened with file descriptor:%d\n", ret);
					}
					if (ret == -1)
					{
						printf("Incorrect parameters\n");
					}
					if (ret == -2)
					{
						printf("ERROR:File is not present\n");
					}
					if (ret == -3)
					{
						printf("ERROR:permission Denied!\n");
					}
					continue;
				}//end of open file else if
				else if (_stricmp(command[0], "read") == 0)
				{
					fd = GetFDFromName(command[1]);
					if (fd == 1)
					{
						printf("ERROR:Incorrect Parameter\n");
					}
					ptr = (char*)malloc(sizeof(atoi(command[2])) + 1);
					if (ptr == NULL)
					{
						printf("ERROR:Memory allocation failure\n");
					}
					ret = ReadFile(fd, ptr, atoi(command[2]));
					if (ret == -1)
					{
						printf("ERROR:File not exists\n");
					}
					if (ret == -2)
					{
						printf("EROOR:Permission Denied!\n");
					}
					if (ret == -3)
					{
						printf("ERROR:Reached at the end of file\n");
					}
					if (ret == -4)
					{
						printf("ERROR:It is not Regular File\n");
					}
					if (ret == 0)
					{
						printf("ERROR:FILE EMPTY");
					}
					if (ret > 0)
					{
						_write(2, ptr, ret);
					}

					continue;

				}//read else complete
				else
				{
					printf("\nERROR:Command not Found!!!\n");
				}

			}//if of count ==3
			else if (count == 4)
			{
				if (_stricmp(command[0], "lseek") == 0)
				{
				fd = GetFDFromName(command[1]);
					if (fd == -1)
					{
						printf("ERROR:Incorrect  Parameter\n");
						continue;
					}
				ret = LseekFile(fd, atoi(command[2]), atoi(command[3]));
					if (ret == -1)
					{
						printf("ERROR:Unable to Perform lseek   !!\n");
					}
				}
				else
				{
					printf("ERROR:Command not found  !!\n");
				}

			}
			else
			{
				printf("ERROR:Command not found  !!\n");
			}
 
		}//while ends
		
		
		return 0;
	}
	
