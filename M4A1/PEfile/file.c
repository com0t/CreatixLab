#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include "geterr.h"
#include <string.h>
#include <dirent.h>

int listFile();
int parsePE(char *fielName);

HANDLE file, fileMapping;
LPVOID fileBase;

char directory[20][50];

int main()
{
	int numberFile;
	int i;

	// Read all file in directory and return number of file
	numberFile = listFile();

	// parse PE file
	for (i=0;i< numberFile; ++i)
	{
		if ((strstr(directory[i],".exe") != NULL || strstr(directory[i],".EXE") != NULL) &&
			strstr(directory[i],"pe.exe") == NULL && strstr(directory[i],"file.exe") == NULL)
			parsePE(directory[i]);
	}

	// parsePE("sigsegv.exe");

	printf("press key to exit!");
	getch();
	return 0;
}

int listFile()
{
	DIR *d;
	struct dirent *dir;
	int i,j;

	d = opendir(".");
	i = 0;
	j = 0;

	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
//			 printf("%s\n", dir->d_name);
			for (j=0; j<strlen(dir->d_name); ++j) directory[i][j] = dir->d_name[j];
				directory[i++][j] = 0;
		}
		closedir(d);
	}
	return i;
}

int parsePE(char *fileName)
{
	
	PIMAGE_DOS_HEADER			dosHeader;
	PIMAGE_NT_HEADERS			peHeader;
	PIMAGE_FILE_HEADER			fileHeader;
	PIMAGE_OPTIONAL_HEADER32	optionalHeader;
	PIMAGE_DATA_DIRECTORY		dataDirectory;
	PIMAGE_SECTION_HEADER		sectionHeader;
	PIMAGE_IMPORT_DESCRIPTOR	importDescriptor;

	LPDWORD currThunk = NULL;
	BOOL check = FALSE;
	int i = 0;

	// Variables of code section
	DWORD rawAddress = 0, virtualSize = 0,rawSize = 0, virtualAddres = 0;

	// Variables of .idata section
	DWORD rvaImp = 0, virSizeImp = 0, virOfImpSection = 0, numOfDLL = 0, rawOfImpSection = 0,rawSizeImp = 0;
	LPDWORD virSize = NULL;

	// info file
	DWORD entryPoint = 0, imageBase = 0;

	// Variables of function MessageBoxA
	DWORD rva = 0;
	
	printf("---------------------------------------\n");
	printf("- Input file name: %s\n",fileName);

	// Open file
	file = CreateFileA(
		fileName, 
		GENERIC_READ, 
		FILE_SHARE_READ, 
		NULL, 
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL
		);

	if (file == INVALID_HANDLE_VALUE)
	{
		printf("==> Error: open file\n");
		ErrorExit(TEXT("CreateFileA"));
		return 1;
	}
	
	// Mapping file
	fileMapping = CreateFileMapping(
		file,
		NULL,
		PAGE_READONLY,
		0,
		0,
		NULL
		);

	if (fileMapping == INVALID_HANDLE_VALUE)
	{
		printf("==> Error: CreateFileMapping");
		ErrorExit(TEXT("CreateFileMapping"));
		CloseHandle(file);
		return 1;
	}

	fileBase = MapViewOfFile(
		fileMapping,
		FILE_MAP_READ,
		0,
		0,
		0
		);

	if (fileBase == NULL)
	{
		printf("==> Error: MapViewOfFile");
		ErrorExit(TEXT("MapViewOfFile"));
		CloseHandle(fileMapping);
		CloseHandle(file);
		return 1;
	}

	// Read attribute file
	// DOS Header
	dosHeader = (PIMAGE_DOS_HEADER)(PBYTE)fileBase;

	// PE Header
	peHeader = (PIMAGE_NT_HEADERS)((PBYTE)fileBase + dosHeader->e_lfanew);

	// File Header
	fileHeader = (PIMAGE_FILE_HEADER)&peHeader->FileHeader;

	// Optional Header
	optionalHeader = (PIMAGE_OPTIONAL_HEADER32)&peHeader->OptionalHeader;

	// Data Directory
	dataDirectory = (PIMAGE_DATA_DIRECTORY)&optionalHeader->DataDirectory;

	// Check 'MZ', 'PE'
	if (peHeader->Signature != IMAGE_NT_SIGNATURE && dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		perror("===> Error: DOS Header invalid!\n");
		perror("===> Error: PE Header invalid!\n");
		goto _EXIT;
	}
	else if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		perror("===> Error: DOS Header invalid!\n");
		goto _EXIT;
	}
	else if (peHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		perror("===> Error: PE Header invalid!\n");
		goto _EXIT;	
	}

	// Check PE32
	if (optionalHeader->Magic != 0x010B)
	{
		printf("===> Error: file isn't PE32!\n");
		goto _EXIT;
	}

	// PE infomation
	printf("******* PE Infomation *******\n");
	printf("- %20s: %#p\n","FileBase",dosHeader);
	printf("- %20s: %#p\n","AddressOfEntryPoint",optionalHeader->AddressOfEntryPoint);
	printf("- %20s: %#X\n","CheckSum",dosHeader->e_csum);
	printf("- %20s: %#p\n","ImageBase",optionalHeader->ImageBase);
	printf("- %20s: %#X\n","FileAlignment",optionalHeader->FileAlignment);
	printf("- %20s: %#X\n","SectionAlignment",optionalHeader->SectionAlignment);
	printf("- %20s: %#X\n","SizeOfImage",optionalHeader->SizeOfImage);
	
	imageBase = optionalHeader->ImageBase;
	entryPoint = optionalHeader->AddressOfEntryPoint;
	rvaImp = dataDirectory[1].VirtualAddress;
	virSizeImp = dataDirectory[1].Size;

	// Sections Header
	sectionHeader = IMAGE_FIRST_SECTION(peHeader);
	printf("- %20s: %#d\n","NumberOfSections",fileHeader->NumberOfSections);

	printf("----------------------------------------------------------------------------------------\n");
	printf("|%7s|%10s|%15s|%10s|%10s|%14s|%14s|\n",
		"Section","Name","Characteristics","RawAddress",
		"RawSize","VirtualAddress","VirtualSize");

	for (i=0;i<fileHeader->NumberOfSections;++i)
	{
		if (i>0) ++sectionHeader;

		printf("|%7d|%10s|%#15X|%#10X|%#10X|%#14X|%#14X|\n",
			(i+1),sectionHeader->Name,sectionHeader->Characteristics,
			sectionHeader->PointerToRawData,sectionHeader->SizeOfRawData,
			sectionHeader->VirtualAddress,sectionHeader->Misc.VirtualSize);

		// Find section contains execute code
		if (sectionHeader->Characteristics & 0x20)
		{
			rawAddress = sectionHeader->PointerToRawData;
			virtualAddres = sectionHeader->VirtualAddress;
			virtualSize = sectionHeader->Misc.VirtualSize;
			rawSize = sectionHeader->SizeOfRawData;
		}

		// Find RVA of .idata section
		if (rvaImp >= sectionHeader->VirtualAddress) 
		{
			virOfImpSection = sectionHeader->VirtualAddress;
			rawOfImpSection = sectionHeader->PointerToRawData;
			virSize = (LPDWORD)&sectionHeader->Misc.VirtualSize;
			rawSizeImp = sectionHeader->SizeOfRawData;
		}
	}
	printf("----------------------------------------------------------------------------------------\n");

	// printf("Infomation of code section\n");
	// printf("RawAddress: %#p\n",rawAddress);
	// printf("VirtualSize: %#X\n",virtualSize);
	// printf("RawSize: %#X\n", rawSize);
	
	// Load Library
	HMODULE dll = LoadLibrary(TEXT(fileName));

	if (dll == NULL)
	{
		printf("===> Error load library\n");
		ErrorExit(TEXT("LoadLibrary"));
		goto _EXIT;
	}

	// printf("\nBase of DLL: %#X\n",dll);

	// File Import
	if (dataDirectory[1].Size > 0)
	{
		printf("\n******* Import Section *******\n");

		check = FALSE;
		importDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD_PTR)dll + dataDirectory[1].VirtualAddress);

		while(importDescriptor->Characteristics != 0  || importDescriptor->FirstThunk != 0 || importDescriptor->ForwarderChain != 0 || 
			importDescriptor->Name != 0 || importDescriptor->OriginalFirstThunk != 0 || importDescriptor->TimeDateStamp != 0)
		{
			// printf("- %s\n", (DWORD_PTR)dll + importDescriptor->Name);

			++numOfDLL;

			// Check OriginalFirstThunk
			if (importDescriptor->OriginalFirstThunk != 0)
			{
				currThunk = (LPDWORD)((DWORD_PTR)dll + importDescriptor->OriginalFirstThunk);
			}
			else
			{
				currThunk = (LPDWORD)((DWORD_PTR)dll + importDescriptor->FirstThunk);
			}

			LPDWORD currFirstTh = (LPDWORD)importDescriptor->FirstThunk;

			// Read function in DLL
			while(*currThunk)
			{
				// Check ordinal
				if (*currThunk & IMAGE_ORDINAL_FLAG32)
				{
					// printf("\tOrdinal: %#X\n",*currThunk^IMAGE_ORDINAL_FLAG32);
				}
				else
				{
					PIMAGE_IMPORT_BY_NAME iibn = (PIMAGE_IMPORT_BY_NAME)((DWORD_PTR)dll + *currThunk);
					
					if ((DWORD)iibn->Name < 0x80000000)
					{
						printf("\tFunction: %s\n",iibn->Name);

						if (strcmp(iibn->Name,"MessageBoxA") == 0 && check == FALSE)
						{
							printf("==> file have a function MessageBoxA\n");
							check = TRUE;
							rva = (DWORD)currFirstTh + optionalHeader->ImageBase;
							printf("currThunk: %#X\n",currThunk);
							printf("currFirstTh: %#X\n",currFirstTh);
							printf("FirstThunk: %#X\n", importDescriptor->FirstThunk);
							printf("OriginalFirstThunk: %#X\n", importDescriptor->OriginalFirstThunk);
							printf("RVA: %#X\n", rva);
						}
					}
					else
					{
						// printf("\tFunction: %#X\n", iibn->Name);
					}
				}

				++currFirstTh;
				++currThunk;
			}
			
			printf("\n");
			++importDescriptor;
		}
	}

	UnmapViewOfFile(fileBase);
	CloseHandle(fileMapping);	
	CloseHandle(file);
	FreeLibrary(dll);
	

	/************************  Inject code to PE file  *******************************/
	
	// Open file
	file = CreateFileA(
		fileName,			// File name
		GENERIC_READ | GENERIC_WRITE,		// Access mode
		FILE_SHARE_READ,	// Share mode
		NULL,				// Security attribute
		OPEN_EXISTING,		// action on file
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);
	if (file == INVALID_HANDLE_VALUE)
	{
		printf("==> Error: open file\n");
		ErrorExit(TEXT("CreateFileA"));
		return 1;
	}
	
	// Mapping file
	fileMapping = CreateFileMapping(
		file,
		NULL,
		PAGE_READWRITE,
		0,
		0,
		NULL
		);

	if (fileMapping == INVALID_HANDLE_VALUE)
	{
		printf("==> Error: CreateFileMapping");
		ErrorExit(TEXT("CreateFileMapping"));
		CloseHandle(file);
		return 1;
	}

	fileBase = MapViewOfFile(
		fileMapping,
		FILE_MAP_WRITE,
		0,
		0,
		0
		);

	if (fileBase == NULL)
	{
		printf("==> Error: MapViewOfFile");
		ErrorExit(TEXT("MapViewOfFile"));
		CloseHandle(fileMapping);
		CloseHandle(file);
		return 1;
	}

	/************************  Create new IMAGE_IMPORT_DESCRIPTOR  *******************************/
	if (check == FALSE)		// file don't have a function MessageBoxA
	{
		DWORD rawOfIID = 0;
		PBYTE point = NULL;

		printf("file don't have a MessageBoxA function\n");
		// printf("RVA of .idata section: %#X\n",virOfImpSection);
		// printf("VirtuaSize: %#X\n",virSizeImp);
		// printf("Number of DLL: %d\n",numOfDLL);
		// printf("RVA of .idata: %#X\n",rvaImp);
		// printf("Raw of .idata section: %#X\n",rawOfImpSection);
		
		rawOfIID = rvaImp - virOfImpSection + rawOfImpSection + virSizeImp;
		rawOfIID = (rawOfIID/0x10)*0x10 + 0x10;
		point = ((PBYTE)fileBase + rawOfIID);

		printf("Raw of IID: %#X\n", rawOfIID);

		// Check cave for new IID
		for (i=0; i<(numOfDLL*0x14+0x28+0x21); ++i)
		{
			if (*(point + i) != 0x00)
			{
				printf("File don't have a cave");
				break;
			}
		}

		// Copies old IID to new IID
		for (i=0; i<(numOfDLL*0x14); ++i)
		{
			*(point++) = *((PBYTE)fileBase + i + rawOfImpSection);
		}

		// Write OriginalFirstThunk, TimeDateStamp, ForwaderChain
		for (i=0; i<3;++i)
		{
			*((LPDWORD)point) = 0;
			point += 4;
		}

		DWORD rvaOfThunk = (DWORD)(point - (PBYTE)fileBase + 8 + 0X14 - rawOfImpSection + virOfImpSection);
		DWORD rvaOfFunction = (DWORD)(rvaOfThunk + 8);
		DWORD rvaOfName = (DWORD)(rvaOfFunction + 0x0e);
		// printf("%#p\n",rvaOfThunk);
		// printf("%#p\n",rvaOfFunction);
		// printf("%#p\n",rvaOfName);
		
		// Write name DLL
		*((LPDWORD)point) = rvaOfName;
		point += 4;
		// Write RVA FirstThunk
		*((LPDWORD)point) = rvaOfThunk;
		point += (4 + 0x14);
		// Write RVA of function
		*((LPDWORD)point) = rvaOfFunction;
		point += 8;

		// Write string of funcion MessageBox
		char func[] = "MessageBoxA";

		*((PWORD)point) = 0x0000;
		point += 2;
		
		for (i=0; i<strlen(func); ++i)
		{
			*(point++) = func[i];
		}
		*(point++) = 0x00;

		// Write string of DLL
		char nameDLL[] = "user32.dll";

		for (i=0; i<strlen(nameDLL); ++i)
		{
			*(point++) = nameDLL[i];
		}
		*(point++) = 0x00;

		dataDirectory[1].VirtualAddress = rawOfIID - rawOfImpSection + virOfImpSection;
		*virSize = rawSizeImp;
		dataDirectory[1].Size = rawSizeImp;

		rva = rvaOfThunk + imageBase;
		printf("RVA: %#X\n",rva);
	}


	/************************  Inject code to PE file  *******************************/
	printf("------------- Inject code ------------\n");
	
	char msg[] = "INJECTED CODE";
	char title[] = "TITLE MSG";
	DWORD rawOfCode = 0;
	PBYTE currPoint = NULL;	// pointer to code inject

	// Find cave for code
	if (rawSize < virtualSize)
	{
		rawOfCode = rawAddress + rawSize - 0x33;
		currPoint = (PBYTE)fileBase + rawOfCode;
		for (i=0; i<33; ++i)
		{
			if (*currPoint != 0x00)
			{
				printf("Code section do not have space\n");
				goto _EXIT;
			}
		}
	}
	else
	{
		rawOfCode = rawAddress + virtualSize;
		if ((rawSize - virtualSize) < 0x33 || *((PBYTE)fileBase + rawOfCode) != 0)
		{
			printf("Code section do not have space\n");
			goto _EXIT;
		}
	}
	
	currPoint = (PBYTE)fileBase + rawOfCode;

	// Inject msg content
	for (i=0; i<strlen(msg);++i)
	{
		*(currPoint + i) = msg[i];		
	}
	*(currPoint + i++) = 0x00;
	currPoint += i;
	
	// Inject title
	for (i=0; i<strlen(title); ++i)
	{
		*(currPoint + i) = title[i];
	}
	*(currPoint + i++) = 0x00;
	currPoint += i;

	// Inject code call MessageBoxA
	// push 0	; MB_OK
	*(PWORD)(currPoint) = 0x006A;
	currPoint += 2;
	// push text title
	*(currPoint++) = 0x68;
	*((LPDWORD)currPoint) = (DWORD)rawOfCode + strlen(msg) + 1 - rawAddress + imageBase + virtualAddres;
	currPoint += 4;
	// push text msg
	*(currPoint++) = 0x68;
	*((LPDWORD)currPoint) = (DWORD)rawOfCode - rawAddress + imageBase + virtualAddres;
	currPoint += 4;
	// push 0 ; 
	*(PWORD)(currPoint) = (WORD)0x006A;
	currPoint += 2;
	// call MessageBoxA
	*((PWORD)currPoint) = 0x15FF;
	currPoint += 2;
	*((LPDWORD)currPoint) = rva;
	currPoint += 4;

	// Make EntryPoint
	*(currPoint++) = 0xB8;
	*((LPDWORD)currPoint) = entryPoint + imageBase;
	currPoint += 4;
	*((PWORD)currPoint) = 0xE0FF;
	currPoint += 2;

	// Change EntryPoint
	optionalHeader->AddressOfEntryPoint = (DWORD)(rawOfCode + strlen(msg) + strlen(title) + 2 - rawAddress + virtualAddres);
	
	// printf("RawAddress: %#X\n",rawAddress);
	// printf("ImageBase: %#X\n",imageBase);
	// printf("VirtualAddress: %#X\n",virtualAddres);
	printf("RawOfCode: %#X\n",rawOfCode);
	printf("EntryPoint: %#X\n", entryPoint);
	printf("Change EntryPoint: %#X\n",optionalHeader->AddressOfEntryPoint);

	printf("------------- Inject OK ------------\n\n");
	
	_EXIT:
	UnmapViewOfFile(fileBase);
	CloseHandle(fileMapping);	
	CloseHandle(file);
}
