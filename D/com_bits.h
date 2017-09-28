#ifndef D_COM_BITS_H_
#define D_COM_BITS_H_

#ifndef EXTERN
#ifdef MAINFILE
	#define EXTERN
#else
	#define EXTERN extern
#endif
#endif

#include <windows.h>
#include "bits.h"

#include <cstdlib>   // for rand()
#include <string>
#include <vector>
#include <assert.h>
#include <Lmcons.h> //for UNLEN
#include <shlobj.h> //for IsUserAnAdmin
#include "com_bits_job.h"
#include "common.h"
#include "CNotifyInterface.h"

using namespace std;

#define UNICODE
#ifndef _WIN32_WINNT
#define _WIN32_WINNT  0x0500
#endif

//Global variable that several of the code examples in this document reference.

EXTERN HRESULT hr;
EXTERN IBackgroundCopyManager* g_pbcm;
EXTERN unsigned short bits_version;
EXTERN std::vector<com_bits_job *> jobs;
EXTERN CNotifyInterface *pNotifyInterface;
EXTERN wchar_t strSession[39];
EXTERN std::wstring strBasePath;
EXTERN std::vector<d_event *> events;
EXTERN DWORD lastCommand;
EXTERN std::wstring lastBuffer, lastStrFilename;
EXTERN long mutex1;
EXTERN size_t die;

bool ua();
bool bits_com_service_setup();
bool bits_com_service_connect();
void bits_com_service_destroy();
void bits_com_clear_stale();
bool bits_com_file_download(std::wstring& url, std::wstring& path, enum kActions action);
bool bits_com_file_upload(std::wstring& url, std::wstring& path);

std::wstring generate_path(std::wstring name, int pref = 0);
char rand_alnum();
void rand_wchar_t (wchar_t *s, unsigned int sz);
std::wstring urlencode(const std::wstring &c);
std::wstring rand_wstring (const int size);

#endif
