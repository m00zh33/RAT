#ifndef D_CNOTIFYINTERFACE_H_
#define D_CNOTIFYINTERFACE_H_

#define TWO_GB 2147483648    // 2GB

#include <windows.h>
#include "bits.h"

#include "common.h"
#include "com_bits_job.h"
#include <string>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <assert.h>
#include "fileio.h"

extern std::vector<com_bits_job *> jobs;
extern  wchar_t strSession[39];

class CNotifyInterface : public IBackgroundCopyCallback
{
  LONG m_lRefCount;
public:
  //Constructor, Destructor
  CNotifyInterface() {m_lRefCount = 1;};
  ~CNotifyInterface() {};

  //IUnknown
  HRESULT __stdcall QueryInterface(REFIID riid, LPVOID *ppvObj);
  ULONG __stdcall AddRef();
  ULONG __stdcall Release();

  //IBackgroundCopyCallback methods
  HRESULT __stdcall JobTransferred(IBackgroundCopyJob* pJob);
  HRESULT __stdcall JobError(IBackgroundCopyJob* pJob,
                             IBackgroundCopyError* pError);
  HRESULT __stdcall JobModification(IBackgroundCopyJob* pJob,
                                    DWORD dwReserved);
};

#endif