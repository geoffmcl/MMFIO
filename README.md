CWinMMFIO class Project
=======================

This is an exercise in using the CWinMMFIO class.

_MMFIO.exe_ - demostrates copying data from one file to another in this case using memory mapping.

_MMBS.exe_ - demonstrates using memory mapping in a backing store file.

Since this is all about the Windows memory mapping of files, it will only compile in Windows.

Files: CMakeLists.txt
---------------------

See the BUILD.txt file for how to build this project using cmake. There is a convenient
build-me.bat in the 'build' folder.

Files: MMFIO.cpp MMFIO.h
------------------------

MMFIO.exe:  Defines the entry point for the console application, setup and use of CWinMMFIO
class.

The main purpose is simply as an EXAMPLE use of the CWinMMFIO class to
copy data from one file to another using memory mapping.

##### Simplest Usage:

&gt; MMFIO &lt;no command&gt;

Will create TWO temporary files, in the folder pointed to by the 'TMP', 'TEMP', ...
environment variables.

The first 'temporary' in_file will be expanded to the (MY_ARB_SIZE * sizeof(__int64) size), with what ever 
values are in memory at the time (or maybe all zero) - this is done by the OS, and not sure
what it puts in that memory.

The second temporary out_file will be expanded to 1 byte, since memory mapping can NOT be 
done on files of zero length.

Then the class CWinMMFIO will be used to Open both files, and the data will be Read from the in_file,
in blocks of BLOCK_SIZE, and Write to the out_file.

##### Optional Useage:

&gt; MMFIO [Options] [in_file [out_file]]

Options:  
--help    (-h or -?) = This help, and exit(2)  
--block &lt;size&gt;  (-b) = Set a new block size for each read/write. (def=4,096)  
--random &lt;size&gt; (-r) = Set the random file size when generating an in_file. (def=3,846,814,168)  

As indicated above if no in_file (or out_file) are given then temporary files will be created, 
which are deleted at exit.

If the in_file exists, then the 'random size' will be set to the size of that existing file,
and its entire content will be copied to the out_file, and if the out_file is NOT a temporary
file, then it will be given the same date/time of the input file.

BE WARNED: This is test example application and is NOT intended as a SAFE file copy utility!  
                   ** BE CLEARLY AND SEVERELY WARNED **

Files: MMFIODef.cpp MMFIODef.h
------------------------------

Built into a static library, MMFIODef.lib, for use by other application

#### Class definition:

Public Interface of the CWinMMFIO class :-

/* Construction */  
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

#### Interface description:

##### bool Open(const sstring& strfile, OPENFLAGS oflags, bool extend = false, suint64 len = 1);

Opens the file strfile in the access mode defined in the parameter oflags; the OPENFLAGS enumerator 
has two constants, OPN_READ and OPN_READWRITE. A zero byte file cannot be opened either for reading 
or writing, hence the extend and length optional parameters.

If extend is set true, then the file size will be extended (or truncated) to the given lenght.

##### bool Close();

Unmaps the view of the file, and closes open HANDLEs to the file mapping object and the opened file.

##### int Read(void* pBuf, quint nCount);

Reads nCount number of bytes into the buffer pBuf; if nCount number of bytes are not available, 
then Read reads as many bytes available from the current position to the end of the file.

The caller must ensure that the buffer pBuf is at least nCount number of bytes wide. The function 
returns the actual number of bytes read. If the current file pointer position is at the end of the 
file and nCount is greater than zero, then Read returns zero.

##### int Write(void* pBuf, quint nCount);

Writes nCount number of bytes from pBuf to the file starting from the current file pointer position. 
The file size is extended by a quantum of 64 KB, determined by the GetSystemInfo(), dwAllocationGranularity,
whenever an attempt is made to write past the end of file. This length is defined by a LONG internal 
variable m_lExtendOnWriteLength. 

The file will be restored to its actual length when necessary.

##### bool Seek(sint64 lOffset, SEEKPOS eseekpos, bool extend = false);

Sets the current file pointer to the location specified by lOffset relative to SEEKPOS eseekpos. 
The SEEKPOS enumerator has three constants SP_BEGIN, SP_CUR, and SP_END. SP_BEGIN specifies that 
the seek is relative to file beginning. SP_CUR specifies that the seek is relative to the current 
file pointer. SP_END specifies that the seek is relative to the end of file.

The function will fail is an attempt is made to seek beyond the current enf of file unless 
extend is set true. When extend is true, the file size will be increased to the seek position 
plus the value in the m_dwPageSize, 4 KB.

The file will be restored to its actual length when necessary.

##### suint64 GetPosition();

Returns the current file pointer position.

##### bool GetLength( suint64 & nLength );

Returns the actual length of the file in bytes if true returned.

This call will attempt to set the length of the file to its actual length, thus 
can fail in which case no length will be returned.

Call GetMMFLastError and/or GetMMFLastErrorValue to get the error message and value.
In most cases the GetMMFLastErrorValue will return a windows DWORD error value, which 
can be passed to the system function FormatMessage to get an error string.


##### bool SetLength(const sint64& nLength);

Sets the length of the file to nLength bytes. If the length cannot be set, the return value will 
be false. 

Call GetMMFLastError and/or GetMMFLastErrorValue to get the error message and value.
In most cases the GetMMFLastErrorValue will return a windows DWORD error value, which 
can be passed to the system function FormatMessage to get an error string.

##### void GetMMFLastError(sstring& strErr);

Call GetMMFLastError to get the error message in the form of a string whenever a function fails.

##### DWORD GetMMFLastErrorValue();

Call GetMMFLastErrorValue to get the error message in the form of a string whenever a function fails.

In most cases the GetMMFLastErrorValue will return a windows DWORD error value, which 
can be passed to the system function FormatMessage to get an error string.

Files: utils.cxx utils.hxx
--------------------------

Built into a static library, genutil.lib, for use in the applications.

Just a big bunch of utility functions...

Files: MMBS.cxx MMBS.hxx
------------------------

MMBS.exe: An example application using the CWinMMFIO class to create a 'temporary' or 'permanent 
backing store file using memory mapping to set values in the file, and retrieve that 
value later.

##### Class Template

template &lt;typename TValue&gt;  
class MmapBS {

Allows the user to define the nature of the storage, like -

typedef MmapBS&lt;uint64_t&gt; storage_mmap_t;

to store uint64_t values in the file. The file offset for the storeage will be at an id times 
the size of the TValue.

##### Class Definition

###### bool Open(const std::string& infile="", bool remove=true)

Open the backing store either using the supplied infile name, or a 'temporary' file name if 
none given. If an infile given, it will be created if not existing, or overwritting any 
existing file. Any path to this file must already exist. Paths will not be created.

Will return true if successful.

######  bool set(const uint64_t id, const TValue value) {

Store the TValue value at the 'id' location, for later accessing. The 'id' can be beyond the 
current store size. It will be expanded to store the value.

###### TValue operator[](uint64_t id) {

Access the value for an 'id' from the backing store. The 'id' must be within the current 
store size. Will throw a std::bad_alloc() if the accessing fails.

Directories
-----------

build - for building 32-bit libraries and executables

build.x64 - if available, for building 64-bit libraries and executables

See BUILD.txt for more build details.

Licence
-------

The original MMFIO_CP is under a Code Project Open License (CPOL)

New pieces added are under a GNU GPL v2 licence - see LICENSE.txt

Files: org-zip/MMFIO_CP.zip
---------------------------

Original 2009 zip file.

The original source site is at <a target="_blank" 
href="http://www.codeproject.com/Articles/37201/Simple-File-I-O-Using-Windows-Memory-Mapped-Files">CodeProject</a>

 
Have fun.

Geoff.  
email: reports _AT_ geoffair _DOT_ info  
20140820

###### eof
