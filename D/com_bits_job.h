#ifndef D_COM_BITS_JOB_H_
#define D_COM_BITS_JOB_H_

#include <string>

enum kActions {
  kActionNone,
  kActionRun,
  kActionDownload,
  kActionSession,
  kActionUpload,
  kActionReport,
  kActionNoop,
  kActionFailsafe,
  kActionDelete,
};

class com_bits_job {
  public:
    unsigned int state;
    GUID id;
    std::wstring name;
    std::wstring url;
    std::wstring path;
    enum kActions action;
};

class d_event {
  public:
    DWORD time;
    enum kActions action;
};

#endif