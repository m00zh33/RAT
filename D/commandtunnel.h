#ifndef D_COMMANDTUNNEL_H_
#define D_COMMANDTUNNEL_H_

#include "thread.h"
#include "com_bits.h"
#include "common.h"
#include <string>
#include <iostream>
#include <sstream>

class CCommandTunnel
{
public:
  CCommandTunnel() { }
  void Run( std::wstring );
	bool IsThreadActive();

private:
	Lib::Thread<CCommandTunnel, std::wstring> m_thread;

	void DoWork( std::wstring command );
};


#endif