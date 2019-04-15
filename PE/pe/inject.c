#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>

int parsePE(char *fileName);
DWORD getCave(DWORD rawAddSec, DWORD rawSizeSec, LPDWORD rawAddCode);

HANDLE file,fileMap;
LPVOID fileBase;

PIMAGE_DOS_HEADER			dosHeader;
PIMAGE_NT_HEADERS			peHeader;
PIMAGE_FILE_HEADER			fileHeader;
PIMAGE_OPTIONAL_HEADER32	optionalHeader;
PIMAGE_DATA_DIRECTORY		dataDirec;
PIMAGE_SECTION_HEADER		sectionHeader;
PIMAGE_IMPORT_DESCRIPTOR	iid;

int main()
{
	parsePE("BASECALC.EXE");
	parsePE("reverseMe.exe");
	parsePE("SetupReg.EXE");
	parsePE("sigsegv.exe");
	parsePE("UnPnP.exe");

	printf("Enter to Exit!\n");
	getch();
	
	return 0;
}

int parsePE(char *fileName)
{
	printf("----------------------------------------------------\n");
	printf("- Input file: %s\n\n",fileName);

	// Open file
	file = CreateFileA(
		fileName,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);
	if (file == INVALID_HANDLE_VALUE)
	{
		printf("Error: open file\n");
		return 1;
	}

	// File Mapping
	fileMap = CreateFileMapping(
		file,
		NULL,
		PAGE_READWRITE,
		0,
		0,
		NULL
		);
	if (fileMap == INVALID_HANDLE_VALUE)
	{
		printf("Error: CreateFileMapping\n");
		CloseHandle(file);
		return 1;
	}

	fileBase = MapViewOfFile(
		fileMap,
		FILE_MAP_WRITE,
		0,
		0,
		0
		);
	if (fileBase == NULL)
	{
		printf("Error: MapViewOfFile\n");
		CloseHandle(fileMap);
		CloseHandle(file);
	}

	// DOS header
	dosHeader = (PIMAGE_DOS_HEADER)fileBase;

	// PE header
	peHeader = (PIMAGE_NT_HEADERS)(fileBase + dosHeader->e_lfanew);

	// File header
	fileHeader = (PIMAGE_FILE_HEADER)&peHeader->FileHeader;

	// Option header
	optionalHeader = (PIMAGE_OPTIONAL_HEADER32)&peHeader->OptionalHeader;

	// Data directory
	dataDirec = (PIMAGE_DATA_DIRECTORY)&optionalHeader->DataDirectory;

	// Check 'MZ', 'PE'
	if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
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

	// *((LPDWORD)fileBase) = 0xAA;

	/************** Declaring variables ***************/
	// Variable of PE file
	DWORD oep = 0, imageBase = 0, fileAligment = 0, sectionAligment = 0;

	// Variable of .idata section
	DWORD rawOfSecImp = 0, rvaOfSecImp = 0, virSizeImp = 0, rawSizeImp = 0;
	DWORD numOfDll = 0, rvaOfFunc = 0;
	LPDWORD currThunk = NULL, firstTh = NULL, pSize = NULL;

	// Variable of .code section
	DWORD rawOfSecExe = 0, rvaOfSecExe = 0, virSizeExe = 0, rawSizeExe = 0;

	// Other Variables
	int i = 0;
	BOOL check = FALSE;
	PBYTE point = NULL, point1 = NULL;	// pointer to wirte
	DWORD rawAddCode = 0, numByteCave = 0;

	/*************************** Info PE file ******************************/

	oep = optionalHeader->AddressOfEntryPoint;
	fileAligment = optionalHeader->FileAlignment;
	sectionAligment = optionalHeader->SectionAlignment;
	imageBase = optionalHeader->ImageBase;

	printf("- File Base: %#X\n", fileBase);
	printf("- AddressOfEntryPoint: %#X\n", oep);
	printf("- FileAlignment: %#X\n", fileAligment);
	printf("- sectionAligment: %#X\n", sectionAligment);
	printf("- ImageBase: %#X\n", imageBase);

	/*************************** Import function ******************************/

	sectionHeader = IMAGE_FIRST_SECTION(peHeader);

	if (dataDirec[1].Size > 0)
	{
		// printf("RVA Import: %#X\n\n",dataDirec[1].VirtualAddress);

		// Find base of address of .idata section and find base of address of .code section
		for (i=0; i<fileHeader->NumberOfSections; ++i)
		{
			if (dataDirec[1].VirtualAddress >= sectionHeader->VirtualAddress)
			{
				rvaOfSecImp = sectionHeader->VirtualAddress;
				rawOfSecImp = sectionHeader->PointerToRawData;
				rawSizeImp = sectionHeader->SizeOfRawData;
				virSizeImp = sectionHeader->Misc.VirtualSize;
				pSize = &sectionHeader->Misc.VirtualSize;
			}

			if (sectionHeader->Characteristics & 0x00000020 || oep >= sectionHeader->VirtualAddress)
			{
				rvaOfSecExe = sectionHeader->VirtualAddress;
				rawOfSecExe = sectionHeader->PointerToRawData;
				rawSizeExe = sectionHeader->SizeOfRawData;
				virSizeExe = sectionHeader->Misc.VirtualSize;
			}

			++sectionHeader;
		}

		// Read Import DLL
		iid = (PIMAGE_IMPORT_DESCRIPTOR)((PBYTE)fileBase + dataDirec[1].VirtualAddress - rvaOfSecImp + rawOfSecImp);

		while (iid->OriginalFirstThunk != 0 || iid->TimeDateStamp != 0 || 
			iid->ForwarderChain != 0 || iid->Name != 0 || iid->FirstThunk !=0)
		{
			if (iid->OriginalFirstThunk != 0)
			{
				currThunk = (LPDWORD)((PBYTE)fileBase + iid->OriginalFirstThunk - rvaOfSecImp + rawOfSecImp);
			}
			else
			{
				currThunk = (LPDWORD)((PBYTE)fileBase + iid->FirstThunk - rvaOfSecImp + rawOfSecImp);
			}

			firstTh = (LPDWORD)(iid->FirstThunk);
			numOfDll++;

			while(*currThunk)
			{
				if (*currThunk & IMAGE_ORDINAL_FLAG32)
				{
					// printf("\tOrdinal: %#X\n",*currThunk^IMAGE_ORDINAL_FLAG32);
				}
				else
				{
					PIMAGE_IMPORT_BY_NAME ibn = (PIMAGE_IMPORT_BY_NAME)((PBYTE)fileBase + *currThunk - rvaOfSecImp + rawOfSecImp);
					// printf("\tFunction: %s\n",ibn->Name);
					if (strcmp(ibn->Name,"MessageBoxA") == 0 && check == FALSE)
					{
						check = TRUE;
						rvaOfFunc = (DWORD)firstTh;
						printf("- File have a MessageBoxA\n");
					}
				}

				++firstTh;
				++currThunk;
			}

			++iid;
		}
	}

	/*************************** Add IMAGE_IMPORT_DESCRIPTOR into file ******************************/
	if (check == FALSE)
	{
		printf("- Add MessageBoxA into file\n");

		// Get address of new IID
		if (virSizeImp < rawSizeImp)
		{
			rawAddCode = dataDirec[1].VirtualAddress - rvaOfSecImp + rawOfSecImp + virSizeImp;
			numByteCave = rawSizeImp - virSizeImp;
			// Change virtual size of import
			*((LPDWORD)pSize) = rawSizeImp;
		}
		else
		{
			numByteCave = getCave(rawOfSecImp, rawSizeImp, &rawAddCode);
		}

		// printf("rawAddCode: %#X\n", rawAddCode);
		// printf("numByteCave: %#d\n", numByteCave);

		if (numByteCave >= (numOfDll*0x14+0x49))
		{
			// Write new IID to file
			point = ((PBYTE)fileBase + rawAddCode);
			point1 = ((PBYTE)fileBase + dataDirec[1].VirtualAddress - rvaOfSecImp + rawOfSecImp);

			for (i=0; i<(numOfDll*0x14); ++i)
			{
				*(point++) = *(point1 + i);
			}
			point += 0x0C;

			char func[] = "MessageBoxA";
			char dll[] = "user32.dll";
			DWORD rvaFirstThunk = (DWORD)((DWORD)point - (DWORD)fileBase - rawOfSecImp + rvaOfSecImp + 8 + 0x14);
			DWORD rvaFunc = (DWORD)((DWORD)point - (DWORD)fileBase - rawOfSecImp + rvaOfSecImp + 0x10 + 0x14);
			DWORD rvaDll = (DWORD)((DWORD)point - (DWORD)fileBase - rawOfSecImp + rvaOfSecImp + 0x1E + 0x14);

			// printf("rvaFirstThunk %#X\n", rvaFirstThunk);
			// printf("rvaFunc %#X\n", rvaFunc);
			// printf("rvaDll %#X\n", rvaDll);

			// Write name Dll
			*((LPDWORD)point) = rvaDll;
			point += 4;
			// Write RVA FirstThunk
			*((LPDWORD)point) = rvaFirstThunk;
			point += 0x18;
			// Write Firthunk
			*((LPDWORD)point) = rvaFunc;
			point += 8;
			// Write name function
			*((LPWORD)point) = 0;
			point += 2;
			for (i=0; i< strlen(func); ++i)
			{
				*(point++) = func[i];
			}
			*(point++) = 0;
			// Write name dll
			for (i=0; i< strlen(dll); ++i)
			{
				*(point++) = dll[i];
			}
			*(point++) = 0;

			// RVA of MessageBoxA
			rvaOfFunc = rvaFirstThunk;

			// Change IID in data directory
			printf("- Change old Rva IID: %#X\n", dataDirec[1].VirtualAddress);
			dataDirec[1].VirtualAddress = rawAddCode - rawOfSecImp + rvaOfSecImp;
			printf("- New Rva IID: %#X\n", dataDirec[1].VirtualAddress);
		}
		else
		{
			printf(".idata not enought cave\n");
			goto _EXIT;
		}
	}

	// printf("RVA of MessageBoxA: %#X\n", rvaOfFunc);

	/*************************** Inject code into file ******************************/
	/*
		push 0
		push text_caption
		push message
		push 0
		call MessageBoxA
	*/
	// printf("rawOfSecExe: %#X\n", rawOfSecExe);
	// printf("virSizeExe: %#X\n", virSizeExe);
	// printf("rawOfSecImp: %#X\n", rawOfSecImp);
	// printf("virSizeImp: %#X\n", virSizeImp);

	if (virSizeExe < rawSizeExe)
	{
		rawAddCode = rawOfSecExe + virSizeExe;
		numByteCave = rawSizeExe - virSizeExe;
	}
	else
	{
		printf("> Get Cave Sucess!\n");
		numByteCave = getCave(rawOfSecExe, rawSizeExe, &rawAddCode);
	}

	// Check inject code
	if (*((PBYTE)fileBase + rawAddCode) != 0)
	{
		printf("\n-----> [File has injected code!] <-----\n\n");
		goto _EXIT;
	} 

	char msg[] = "INJECTED CODE";
	char title[] = "TITLE MSG";

	if (numByteCave < (0x18 + strlen(msg) + strlen(title) + 2))
	{
		printf("execute code not enought cave\n");
		goto _EXIT;
	}

	point = ((PBYTE)fileBase + rawAddCode);

	// Inject msg
	for (i=0; i<strlen(msg); ++i)
	{
		*(point++) = msg[i];
	}
	*(point++) = 0;

	// Inject title
	for (i=0; i<strlen(title); ++i)
	{
		*(point++) = title[i];
	}
	*(point++) = 0;

	// Inject call funcion MessageBoxA
	// push 0
	*((PWORD)point) = 0x006A;
	point += 2;
	// push text caption
	*(point++) = 0x68;
	*((LPDWORD)point) = imageBase + rawAddCode + strlen(msg) + 1 - rawOfSecExe + rvaOfSecExe;
	point += 4;
	// push text caption
	*(point++) = 0x68;
	*((LPDWORD)point) = imageBase + rawAddCode - rawOfSecExe + rvaOfSecExe;
	point += 4;
	// push 0
	*((PWORD)point) = 0x006A;
	point += 2;
	// Call MessageBoxA
	*((PWORD)point) = 0x15FF;
	point += 2;
	*((LPDWORD)point) = rvaOfFunc + imageBase;
	point += 4;
	// change OEP
	*(point++) = 0xB8;
	*((LPDWORD)point) = oep + imageBase;
	point += 4;
	*((PWORD)point) = 0xE0FF;
	point += 2;

	// Change OEP
	optionalHeader->AddressOfEntryPoint = rawAddCode - rawOfSecExe + rvaOfSecExe + strlen(msg) + strlen(title) + 2;
	printf("- Change old OEP: %#X\n", oep);
	printf("- To new OEP: %#X\n", optionalHeader->AddressOfEntryPoint);
	printf("\n-----> [Inject code sucess!] <-----\n\n");

	_EXIT:
	UnmapViewOfFile(fileBase);
	CloseHandle(fileMap);	
	CloseHandle(file);
}

DWORD getCave(DWORD rawAddSec, DWORD rawSizeSec, LPDWORD rawAddCode)
{
	int i = 0;
	PBYTE point = ((PBYTE)fileBase + rawAddSec + rawSizeSec - 1);
	while (*(point--) == 0) ++i;
	*rawAddCode = (DWORD)(point - (PBYTE)fileBase + 3);
	return i - 1;
}
