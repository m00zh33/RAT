#ifndef D_DIRDUMP_H_
#define D_DIRDUMP_H_

#include "thread.h"
#include "com_bits.h"
#include "common.h"
#include <string>
#include <iostream>
#include <sstream>

class CDirDump
{
public:
  CDirDump() { }
  void Run( std::wstring );
	bool IsThreadActive();

private:
	Lib::Thread<CDirDump, std::wstring> m_thread;

	void DoWork( std::wstring root );
  void DoDir ( HANDLE g_hChildStd_IN_Wr, std::wstring &root, std::wstring &buffer );
  void DoDump( HANDLE g_hChildStd_IN_Wr, std::wstring &buffer );
  void Fail ( int code = 0 );
};

EXTERN CDirDump dirdump;


#endif