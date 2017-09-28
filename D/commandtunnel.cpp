#include "commandtunnel.h"


void CCommandTunnel::Run( std::wstring command ) {
  m_thread.Run(this, &CCommandTunnel::DoWork, command);
}

bool CCommandTunnel::IsThreadActive() {
  return m_thread.IsThreadActive();
}

void CCommandTunnel::DoWork ( std::wstring command ) {

  srand ( (unsigned int)GetTickCount() );

  HANDLE hHandleArray[5] = {0};
  HANDLE g_hChildStd_IN_Rd = hHandleArray[3]; //connect
  HANDLE g_hChildStd_IN_Wr = hHandleArray[2]; //create
  HANDLE g_hChildStd_OUT_Rd = hHandleArray[1]; //create
  HANDLE g_hChildStd_OUT_Wr = hHandleArray[4]; //connect

  OVERLAPPED overlapped_s;
  ZeroMemory(&overlapped_s, sizeof(overlapped_s));

  SECURITY_ATTRIBUTES saAttr;
  ZeroMemory( &saAttr, sizeof(saAttr) );
  saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
  saAttr.bInheritHandle = TRUE; 
  saAttr.lpSecurityDescriptor = NULL;
  try {
    const char *PipeNameOut= "\\\\.\\pipe\\atestseasa";
    const char *PipeNameIn= "\\\\.\\pipe\\testascxzc";

    g_hChildStd_OUT_Rd = CreateNamedPipe(PipeNameOut,
      PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
      PIPE_TYPE_BYTE,
      PIPE_UNLIMITED_INSTANCES, 4086, 4086,
      NMPWAIT_USE_DEFAULT_WAIT, &saAttr);
    if (g_hChildStd_OUT_Rd == INVALID_HANDLE_VALUE || !g_hChildStd_OUT_Rd)
      throw std::exception();

    g_hChildStd_OUT_Wr = CreateFileA(PipeNameOut, GENERIC_WRITE, 0,
      &saAttr, OPEN_EXISTING, 0, NULL);
    while (g_hChildStd_OUT_Wr == INVALID_HANDLE_VALUE
      && ::GetLastError() == ERROR_PIPE_BUSY) {
        WaitNamedPipeA(PipeNameOut, NMPWAIT_USE_DEFAULT_WAIT);
        g_hChildStd_OUT_Wr = CreateFileA(PipeNameOut, GENERIC_WRITE, 0,
      &saAttr, OPEN_EXISTING, 0, NULL);
    }

    g_hChildStd_IN_Wr = CreateNamedPipe(PipeNameIn,
      PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
      PIPE_TYPE_BYTE,
      PIPE_UNLIMITED_INSTANCES, 4086, 4086,
      NMPWAIT_USE_DEFAULT_WAIT, &saAttr);
    if (g_hChildStd_IN_Wr == INVALID_HANDLE_VALUE || !g_hChildStd_IN_Wr)
      throw std::exception();

    g_hChildStd_IN_Rd = CreateFileA(PipeNameIn, GENERIC_READ, 0,
      &saAttr, OPEN_EXISTING, 0, NULL);
    while (g_hChildStd_IN_Rd == INVALID_HANDLE_VALUE
      && ::GetLastError() == ERROR_PIPE_BUSY) {
        WaitNamedPipeA(PipeNameOut, NMPWAIT_USE_DEFAULT_WAIT);
        g_hChildStd_IN_Rd = CreateFileA(PipeNameIn, GENERIC_READ, 0,
      &saAttr, OPEN_EXISTING, 0, NULL);
    }

    PROCESS_INFORMATION piProcInfo; 
    STARTUPINFOW siStartInfo;
    BOOL bSuccess = FALSE; 
    ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );
    ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
    siStartInfo.cb = sizeof(STARTUPINFO); 
    siStartInfo.hStdError = g_hChildStd_OUT_Wr;
    siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
    siStartInfo.hStdInput = g_hChildStd_IN_Rd;
    siStartInfo.dwFlags = STARTF_USESTDHANDLES; // | STARTF_USESHOWWINDOW;
    siStartInfo.wShowWindow = SW_HIDE;
    wchar_t *tmp = new wchar_t[command.length()+1];
    wcscpy_s(tmp, command.length()+1, command.data());
    bSuccess = CreateProcessW(
      NULL, 
      tmp,           // command line 
      NULL,          // process security attributes 
      NULL,          // primary thread security attributes 
      TRUE,          // handles are inherited 
      0,             // creation flags 
      NULL,          // use parent's environment 
      NULL,          // use parent's current directory 
      &siStartInfo,  // STARTUPINFO pointer 
      &piProcInfo
    );  // receives PROCESS_INFORMATION 
    delete[] tmp;
    if ( ! bSuccess ) {
      throw std::exception();
    }

    hHandleArray[0] = piProcInfo.hProcess;
    DWORD dwDelta = 1000;

    char buff[4087];

    DWORD result;

    for (;;) {

      //result = WaitForSingleObjectEx(piProcInfo.hProcess, dwDelta, false);
      //if (result != WAIT_TIMEOUT) break;

      result = WaitForMultipleObjectsEx(2, hHandleArray, false, dwDelta,
        true);
      if (result == 0) break;
      if (result == WAIT_TIMEOUT) continue;

      ZeroMemory(buff, sizeof(buff));
      result = ReadFileEx(g_hChildStd_OUT_Rd, &buff, sizeof(buff) - 1, 
        &overlapped_s, NULL);
      int sizebuff;
      for (sizebuff = sizeof(buff); sizebuff > 0; sizebuff--) {
        if (buff[sizebuff-1]) break;
      }
      if (!sizebuff) continue;

    }

    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);
    throw std::exception();

  } catch (...) {
    CloseHandle(g_hChildStd_OUT_Rd);
    CloseHandle(g_hChildStd_OUT_Wr);
    CloseHandle(g_hChildStd_IN_Rd);
    CloseHandle(g_hChildStd_IN_Wr);
    return;
  }
}
