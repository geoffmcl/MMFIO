/*\
 * MMFIODef.h
 *
 * Licence: Code Project Open License (CPOL)
 *
 * 20140819: Quite a number of changes, and hopefully improvements - geoff
 *
\*/
////////////////////////////////////////////////////////////////////////////////////
// MMFIODef.h : Class to handle open, read, write, close a file using memory mapping
//
#ifndef _MMFIODEF_H
#define _MMFIODEF_H

#include <Windows.h>
#include <string>
#include <valarray>
#include <vector>
#include <assert.h>
#include <tchar.h>

using namespace std;

typedef int qint;
typedef unsigned int quint;
typedef int qint;
typedef __int64 sint64;
typedef unsigned __int64 suint64;
#define sstring string
#define SysAssert assert

namespace NmsMMF
{
	enum SEEKPOS
	{
		SP_BEGIN = 0,
		SP_CUR,
		SP_END,
	};

	enum OPENFLAGS
	{
		OPN_READ = 0, //Opens the file for reading only
		OPN_READWRITE //Opens the file for reading and writing
	};
};


using namespace NmsMMF;

class CWinMMFIO
{
private:
	BYTE m_cRefCount;
	OPENFLAGS m_eOpenflags;
	HANDLE m_hFileMapping;
	PBYTE m_pbFile;
	DWORD m_dwBytesInView;
	sint64 m_qwFileSize;
	sint64 m_nViewBegin;//from begining of file
	sint64 m_nCurPos;//from begining of file
	HANDLE m_hFile;
	DWORD m_dwAllocGranularity;
	LONG m_lExtendOnWriteLength;
	DWORD m_dwflagsFileMapping;
	DWORD m_dwflagsView;
	bool m_bFileExtended;
	sstring m_strErrMsg;
    DWORD m_dwLastErr;
    DWORD m_dwPageSize;

	void _flush();
	bool _checkFileExtended();
	bool _seek(sint64 lOffset, SEEKPOS eseekpos, bool extend = false);
	void _close();

public:
	CWinMMFIO();
	virtual ~CWinMMFIO();

	/* Construction */
	// bool Truncate(const sstring& strfile, suint64 lOffset);
	bool Open(const sstring& strfile, OPENFLAGS oflags, bool extend = false, suint64 len = 1);
	bool Close();

	/* I/O */
	int Read(void* pBuf, quint nCount);
	int Write(void* pBuf, quint nCount);

	/* Position */
	bool Seek(sint64 lOffset, SEEKPOS eseekpos, bool extend = false);
	suint64 GetPosition();

	/* Length */
	bool GetLength( suint64 & nLength );
	bool SetLength(const sint64& nLength);

	/*error*/
	void GetMMFLastError(sstring& strErr) { strErr = m_strErrMsg; }
    DWORD GetMMFLastErrorValue() { return m_dwLastErr; }
};

#endif // #ifndef _MMFIODEF_H
// eof
