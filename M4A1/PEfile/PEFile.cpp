#include <stdio.h>
#include <conio.h>
#include <Windows.h>

int main(){
	char loc[20];
	HANDLE fi, mapOb;
	LPVOID point;
	PIMAGE_DOS_HEADER dosHeader;
	PIMAGE_NT_HEADERS peHeader;
	PIMAGE_FILE_HEADER fHeader;
	PIMAGE_OPTIONAL_HEADER32 opHeader;
	PIMAGE_SECTION_HEADER secHeader;
	PIMAGE_DATA_DIRECTORY dataDic;
	PIMAGE_IMPORT_DESCRIPTOR imp;
	PIMAGE_THUNK_DATA pThunkData;
	PIMAGE_EXPORT_DIRECTORY exp;
	DWORD *currThunk;
	int size;
	
	// input
	printf("Input file (Ex: test.dll or E:\\test.dll): ");
	gets(loc);
	
	// open file
	fi = CreateFile(loc, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fi == INVALID_HANDLE_VALUE){
		printf("\n==> Error: Can't open file!\n");
	}else{
		
		// maping file to memory
		mapOb = CreateFileMapping(fi, NULL, PAGE_READONLY, 0, 0, NULL);
		point = MapViewOfFile(mapOb, FILE_MAP_READ, 0, 0, 0);
		
		// DOS Header
		dosHeader = (PIMAGE_DOS_HEADER)point;
		
		// PE Header
		peHeader = (PIMAGE_NT_HEADERS)((BYTE*)dosHeader + dosHeader->e_lfanew);
		
		// File Header
		fHeader = (PIMAGE_FILE_HEADER)&peHeader->FileHeader;
		
		// Optional Header
		opHeader = (PIMAGE_OPTIONAL_HEADER32)&peHeader->OptionalHeader;
		
		// check 'MZ' , 'PE'
		if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
			printf("==> Error: Dos Header failse!\n");
		else if(peHeader->Signature != IMAGE_NT_SIGNATURE)
			printf("==> Error: PE Header false!\n");
		else{
			
			// write PE Infomation
			printf("[*] PE Infomation:\n");
			printf("- PointerToEntryPoint: %#x\n", dosHeader);
			printf("- CheckSum: %#x\n", dosHeader->e_csum);
			printf("- ImageBase: %#x\n", opHeader->ImageBase);
			printf("- FileAlignment: %#x\n", opHeader->FileAlignment);
			printf("- SizeOfImage: %#x\n", opHeader->SizeOfImage);
			
			// write sections
			printf("---------------------------------\n");
			printf("[*] Sections:\n");
			
			// pointer to first section header
			secHeader = IMAGE_FIRST_SECTION(peHeader);
			
		//	printf("%#x\n", peHeader);
		//	printf("%#x\n", fHeader);
			//printf("%#x\n", opHeader);
			//printf("%d\n", fHeader->NumberOfSections);
			
			for(int i=0; i<fHeader->NumberOfSections; i++){
				if( i>0)
					secHeader++;
					
				printf("- Section %d/%d.\n", i+1, peHeader->FileHeader.NumberOfSections);
				printf("- Name: %s\n", secHeader->Name);
				printf("- Characteristics: %#x\n", secHeader->Characteristics);
				printf("- RawAddress: %#x\n", secHeader->Misc.PhysicalAddress);
				printf("- RawSize: %#x\n", secHeader->SizeOfRawData);
				printf("- VirtualAddress: %#x\n", secHeader->VirtualAddress);
				printf("- VirtualSize: %#x\n", secHeader->Misc.VirtualSize);
				printf("\n");
				
			}
			
			// get Data Directory
			dataDic = opHeader->DataDirectory;
			//for(int i=0; i<IMAGE_NUMBEROF_DIRECTORY_ENTRIES; i++){
			//	printf("RVA: %#x ; Size: %#x\n", dataDic[i].VirtualAddress, dataDic[i].Size);
			//}
									
			// load point dll
			HMODULE fMap = LoadLibrary(TEXT(loc));	
						
			// write file export
			if (dataDic[0].Size > 0){
				printf("---------------------------------\n");
				printf("[*] Export Data Section:\n");
				
				exp = (PIMAGE_EXPORT_DIRECTORY)((DWORD_PTR)fMap + dataDic[0].VirtualAddress);
				
				printf("- DLL name: %s\n", (DWORD_PTR)fMap +exp->Name);
				
				DWORD *eat = (DWORD*)((DWORD_PTR)fMap + exp->AddressOfFunctions);
				WORD *eot = (WORD*)((DWORD_PTR)fMap + exp->AddressOfNameOrdinals);
				DWORD *ent = (DWORD*)((DWORD_PTR)fMap + exp->AddressOfNames);
				
				for( int i=0; i<exp->NumberOfFunctions; i++){
					printf("\tFuntion RVAs: %#x", eat[i]);
					
					for(int j=0; j<exp->NumberOfNames; j++){
						
						if (i == eot[j]){
							printf("\tName: %s", ((DWORD_PTR)fMap + ent[j]));
							break;
						}
					}
					
					printf ("\n");
				}
			}
			
			// write file import
			if (dataDic[1].Size > 0){

				printf("---------------------------------\n");
				printf("[*] Import Data Section:\n");
				
				imp = (PIMAGE_IMPORT_DESCRIPTOR)( (DWORD_PTR)fMap + dataDic[1].VirtualAddress);
				
				while (imp->Characteristics != 0  || imp->FirstThunk != 0 || imp->ForwarderChain != 0 || imp->Name != 0 || imp->OriginalFirstThunk != 0 || imp->TimeDateStamp != 0){
			
					printf("- DLL name: %s\n", (DWORD_PTR)fMap + imp->Name);
					
					// check OFT
					if (imp->OriginalFirstThunk != 0){
						currThunk = (DWORD*)((DWORD_PTR)fMap + imp->OriginalFirstThunk);
					}else{
						currThunk = (DWORD*)((DWORD_PTR)fMap + imp->FirstThunk);
					}

					while( *currThunk != 0){
						
						DWORD imgTrnkData = *currThunk;

						// check ordinal
						if(imgTrnkData & IMAGE_ORDINAL_FLAG32){
							printf("\tOrdinal: %#x\n", imgTrnkData^IMAGE_ORDINAL_FLAG32);
						}else{
							IMAGE_IMPORT_BY_NAME *iibn = (IMAGE_IMPORT_BY_NAME*)((DWORD_PTR)fMap + imgTrnkData);
							if( (DWORD)iibn->Name < 0x80000000){
								printf("\tFuntion: %s\n", iibn->Name);
							}else{
								printf("\tFuntion: %#x\n", iibn->Name);
							}
						}
						
						currThunk++;
					}
					imp++;
					
				}
			}
			
		}
	}
	
	// close file
	UnmapViewOfFile(point);
	CloseHandle(mapOb);
	CloseHandle(fi);
	
	printf("==> Press any key to exit!");
	getchar();
	return 0;
}
