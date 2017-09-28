#ifndef D_THREAD_H_
#define D_THREAD_H_

#include <windows.h>

namespace Lib
{
	// --------------------------------------------
	//  Thread class
	// --------------------------------------------
	template<class T, class P>
	class Thread
	{
	public:
		typedef void (T::*ThreadFunc)( P );
		
		Thread();
		~Thread();

		// Run - Start the Thread and run the method
		// pClass->(*pfFunc), passing p as an argument.
		// Returns true if the thread was created 
		// successfully, false otherwise
		bool Run( T* pClass, ThreadFunc pfFunc, P p );

		// Suspend - Suspends the thread (if one is active)
		void Suspend();

		// Resume - Resumes a previously suspended thread
		void Resume();

		// Terminate - Terminates the thread (if one is active).
		// Prefer another means of exiting the thread, as
		// calling Terminate() does not allow the thread to free
		// any resources it may hold
		void Terminate();

		// IsThreadActive - Called in the context of another
		// (external) thread to test whether the thread is
		// currently running
		bool IsThreadActive() const;

	protected:
		// Static Thread Proc - The ThreadProc called by the
		// Windows CreateThread() function.  The parameter is
		// a pointer to the thread instance that is being
		// started.  
		static DWORD WINAPI StartThread( void* pParam );

		// Handle to the created Thread
		HANDLE m_hThread;

		// ID of the created thread
		DWORD m_threadID;

		// ThreadFunc invoketion members
		T*			m_pInstance;
		ThreadFunc	m_pfFunc;
		P			m_param;
	};

	// ------------------------------------------------------
	template<class T, class P>
	Thread<T, P>::Thread()
		: m_hThread( NULL )
		, m_threadID( (DWORD)-1 )
		, m_pInstance( NULL )
		, m_pfFunc( NULL )
	{
	}

	// ------------------------------------------------------
	template<class T, class P>
	Thread<T, P>::~Thread()
	{
		if (IsThreadActive())
			Terminate();
	}
	
	// ------------------------------------------------------
	template<class T, class P>
	bool Thread<T, P>::Run( T* pClass, ThreadFunc pfFunc, const P param )
	{
		// Store the values in this class instance so
		// the static StartThread() function can call
		// the appropriate method on the object
		m_pInstance = pClass;
		m_pfFunc = pfFunc;
		m_param = param;

		m_hThread = ::CreateThread( NULL, 0, StartThread, this, 0, &m_threadID );
		return (m_hThread != NULL);
	}

	// ------------------------------------------------------
	template<class T, class P>
	void Thread<T, P>::Suspend()
	{
		::SuspendThread( m_hThread );
	}

	// ------------------------------------------------------
	template<class T, class P>
	void Thread<T, P>::Resume()
	{
		int resumeCount = ::ResumeThread( m_hThread );
		while (resumeCount > 1)
			resumeCount = ::ResumeThread( m_hThread );
	}

	// ------------------------------------------------------
	template<class T, class P>
	void Thread<T, P>::Terminate()
	{
        if (m_hThread)
        {
		    ::TerminateThread( m_hThread, 0 );
		    ::CloseHandle( m_hThread );
            m_hThread = NULL;
        }
    }

	// ------------------------------------------------------
	template<class T, class P>
	bool Thread<T, P>::IsThreadActive() const
	{
		return ((m_hThread != NULL) && (::WaitForSingleObject(m_hThread, 0) != WAIT_OBJECT_0));
	}

	// ------------------------------------------------------
	template<class T, class P>
	DWORD WINAPI Thread<T, P>::StartThread( void* pParam )
	{
		Thread* pInstance = reinterpret_cast<Thread*>( pParam );
		if ( !pInstance )
			return (DWORD)-1;

		// Get the invoketion variables so we don't have to
		// use even more funky syntax
		T* pClassInstance = pInstance->m_pInstance;
		ThreadFunc pfFunc = pInstance->m_pfFunc;
		P param = pInstance->m_param;

		// We have a valid instance of the Thread class, use
		// the thread's stored parameters to call the client
		// (worker) function.  This will continue to run in
		// the context of this (seperate) thread until finished
		((*pClassInstance).*pfFunc)( param );

		return 0;
	}
}


#endif