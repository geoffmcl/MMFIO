/*\
 * MMFIO.h
 *
 * Licence: Code Project Open License (CPOL)
 *
\*/
#ifndef MMFIO_H_
#define MMFIO_H_
// 20140818 - A Windows compile using CMmakelists.txt generator
// Note: This was originally compile under MFC/afx - all removed
// Note: The original used precompiled header - now removed

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utime.h>
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <stdio.h>
#include <stdlib.h> // for strtoull(), ...
#include <tchar.h>
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#include <WinSock2.h>   // NOTE: This includes <Windows.h>
#include <sys/timeb.h>
#include <conio.h>
#include <iostream>
#include <stdint.h>

#endif // #ifndef MMFIO_H_
// eof
