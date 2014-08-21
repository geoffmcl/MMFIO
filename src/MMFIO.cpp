/*\
 * MMFIO.cpp
 *
 * Licence: Code Project Open License (CPOL)
 *
 * 20140819: Substantial modification to not only work with TEMPORARY files,
 *           but also allows copying an existing file to a new location
 *           uisng memory mapped file I/O, but in no way intended to be a 
 *           'safe' file copying utility.
 *
 * It main purpose remains simply as an EXAMPLE use of the CWinMMFIO class to
 * copy data from one file to another using memory mapping.
 *
\*/
////////////////////////////////////////////////////////////////////////
// MMFIO.cpp : Defines the entry point for the console application.
//
#include <stdio.h>
#include <stdlib.h> // for strtoull(), ...
#include "MMFIO.h"
#include "MMFIOdef.h"
#include "utils.hxx"
#pragma warning( disable : 4996 )

#ifndef MY_BLOCK_SIZE
#define MY_BLOCK_SIZE 4096  // this is the statndard page size, thus minumu overlaps of view reads/write
//#define MY_BLOCK_SIZE 45678
#endif
#ifndef MY_ARB_SIZE
// #define MY_ARB_SIZE 92480851771    // maybe too much, too long to complete and uses LOTS of disk space - 1.35 TB
// #define MY_ARB_SIZE 2480851771    // maybe too much, too long to complete and uses LOTS of disk space - 36.97 GB
#define MY_ARB_SIZE 480851771
#endif

static const TCHAR *write_file = 0;
static const TCHAR *read_file  = 0;
static suint64 def_len = (suint64)((suint64)MY_ARB_SIZE * (suint64)sizeof(__int64)); // arbitrary length
static int block_size = MY_BLOCK_SIZE;  // default block size
static int tot_cycles = 0;  // (int)(def_len / block_size);
static int read_temp = 0;
static int write_temp = 0;

static SPLITPATH read;
static SPLITPATH write;

int get_input(const char *prompt)
{
    printf("%s",prompt);
    int ret = getch();
    return ret;
}

void show_info()
{
    uint64_t rlen = 0;
    uint64_t read_len = 0;
    uint64_t write_len = 0;
    uint64_t curr_rd_len = 0;
    uint64_t curr_wr_len = 0;
    read_temp = 0;
    write_temp = 0;
    // deal with the READ file - existing or NEW
    if (!read_file) {
        read_file = getTEMPFileName();
        read_temp = 1;
        read_len = def_len; // create and expand to this size
    } else {
        if (is_file_or_directory64((char *)read_file) == DT_FILE) {
            curr_rd_len = get_last_stat_file_size64();    // set default len = existing read file
            read_temp = 2;
            read_len = 0;       // no additional space for read
            def_len = curr_rd_len;
        } else {
            read_len = def_len; // create and expand to this size
        }
    }

    // always need output length
    if (!write_file) {
        write_file = getTEMPFileName();
        write_len = def_len;    // original, or adjusted length
        write_temp = 1;
    } else {
        if (is_file_or_directory64((char *)write_file) == DT_FILE) {
            curr_wr_len = get_last_stat_file_size64();
            write_len = def_len - curr_wr_len;
            write_temp = 2;
        } else {
            write_len = def_len;    // original, or adjusted length
        }
    }

    if (!getAbsandSplit( read_file, &read ) ||
        !getAbsandSplit( write_file, &write) ) {
        fprintf(stderr,"INTERNAL ERROR: Aborting...\n");
        get_input("Any key to exit : ");
        exit(1);
    }

    tot_cycles = (int)(def_len / block_size);
    if ( def_len % block_size )
        tot_cycles++;

    rlen = read_len + write_len; // aditional disk space required, but could be different dirves
    bool nsp_read = false;
    bool nsp_write = false;
    uint64_t fs_read = getFreeDiskSpace(read.drive);;
    uint64_t fs_write = getFreeDiskSpace(write.drive);
    if (strcmp(read.drive,write.drive)) {
        if (fs_read > read_len)
            nsp_read = false;
        else
            nsp_read = true;
        if (fs_write > write_len)
            nsp_write = false;
        else
            nsp_write = true;
    } else {
        // all one drive
        if (fs_read < rlen) {
            if (read_len)
                nsp_read = true;
            nsp_write = true;
        }
    }
    /////////////////////////////////////////////////////////////////////////////////////
    // got the information - show what will happen, and ask confirmation to proceed.
    ///////////////////////////////////////////////////////////////////////////////////

    printf("\n");
    if (read_temp < 2) {
        printf("This program creates a '%s' read file of %s bytes (%s)\n", read_file, get_nice_num_stg(def_len), 
                    get_k_num_stg(def_len));
    } else {
        printf("This program will read the '%s' read file of %s bytes (%s)\n", read_file, get_nice_num_stg(curr_rd_len), 
                    get_k_num_stg(curr_rd_len));
    }
    printf("then proceeds to write it to '%s'\n", write_file);
    if (write_temp == 2) {
        printf("WARNING: The existing file of '%s' bytes (%s) will be OVERWRITTEN!\n", get_nice_num_stg(curr_wr_len),
            get_k_num_stg(curr_wr_len));
    }
    printf("Both files are MEMORY MAPPED, and the IO is done in %d blocks\n", block_size);
    printf("This will require a total of %s read/write cycles.\n", get_nice_num_stg(tot_cycles));
    printf("\n");
    printf("This is a task requiring a total '%s' of disk space\n", get_k_num_stg(rlen));

    char *cp2;
    if (!checkDirectoryExists( &read, &cp2 )) {
        printf("\nThe read path '%s' does NOT exist!\n",cp2);
        printf("Change the read path!\n");
        get_input("Any key to exit : ");
        exit(1);
    }
    if (!checkDirectoryExists( &write, &cp2 )) {
        printf("\nThe write path '%s' does NOT exist!\n",cp2);
        printf("Change the write path!\n");
        get_input("Any key to exit : ");
        exit(1);
    }

    if (strcmp(read.drive,write.drive)) {
        printf("The read drive %s used has '%s' free.\n", read.drive, get_k_num_stg(fs_read));
        printf("The write drive %s used has '%s' free.\n", write.drive, get_k_num_stg(fs_write));
    } else {
        printf("The drive %s used has '%s' free.\n", write.drive, get_k_num_stg(fs_write));
    }

    if (nsp_read || nsp_write) {
        printf("\nCan NOT find sufficient space for the operation!\n");
        printf("Change the MY_ARB_SIZE size '%s' (%s) to a lower value\nor choose another drive with the space!\n", get_nice_num_stg(def_len),
            get_k_num_stg(def_len));
        get_input("Any key to exit : ");
        exit(1);
    }
    if (read_temp == 2) {
        printf("\n");
        printf("WARNING: This is NOT intended as a SAFE file copy utility!\n");
        printf("Its sole purpose is to provide an example usage of the MMFIO class\n");
        printf("to copy data from one file to another using memory mapping.\n");
        printf("      *** BE CLEARLY AND SEVERELY WARNED ***\n");
    }
    printf("\n");
    int ret = get_input("Enter 'y' to continue : ");
    if (ret != 'y') {
        printf(" no\n");
        exit(1);
    }

    printf(" yes\n");
    //exit(1);
}

void give_help( TCHAR *name, bool full )
{
    printf("\n");
    printf("%s [Options] [in_file [out_file]]\n", get_filename_only(name));
    printf(" Options:\n");
    printf(" --help    (-h or -?) = This help, and exit(2)\n");
    printf(" --block <size>  (-b) = Set a new block size for each read/write. (def=%s)\n", 
        get_nice_num_stg(block_size));
    printf(" --random <size> (-r) = Set the random file size when generating an in_file. (def=%s)\n",
        get_nice_num_stg(def_len));

    printf("\n");
    printf("This application is ONLY an example of using CWinMMFIO class.\n");
    printf("Here the class is used to open the in_file, and copy its entire contents to\n");
    printf("the out_file using memory mapped I/O for both the read and writes.\n");
    printf("\n");
    printf("That is both files are memory mapped using the CreateFileMapping/MapViewOfFile\n");
    printf("Windows API, and the in file data is copied in blocks from the map-view to a buffer,\n");
    printf("and then copied from that buffer to the out_file map-view.\n");
    printf("While this type of memory mapped I/O MAY or MAY NOT be faster than conventional read/write\n");
    printf("of file data, this is NOT the intention of this exercise.\n");
    printf("\n");
    printf("If no in_file (or out_file) given then temporary files will be created, which are deleted at exit.\n");

    printf("\n");
    if (full) {
        printf("The CWinMMFIO class offers many of the same features as conventional file I/O. That is -\n");
        printf("Construnction:\n");
        printf("bool Open(const sstring& strfile, OPENFLAGS oflags, bool truncate = false, suint64 len = 1);\n");
        printf("bool Close();\n");
        printf("I/O:\n");
        printf("int Read(void* pBuf, quint nCount);\n");
        printf("int Write(void* pBuf, quint nCount);\n");
        printf("Position:\n");
        printf("bool Seek(sint64 lOffset, SEEKPOS eseekpos, bool truncate = false);\n");
	    printf("suint64 GetPosition();\n");
        printf("Length:\n");
        printf("bool GetLength( suint64 & nLength );\n");
        printf("bool SetLength(const sint64& nLength);\n");
        printf("error:\n");
        printf("void GetMMFLastError(sstring& strErr) { strErr = m_strErrMsg; }\n");
        printf("DWORD GetMMFLastErrorValue() { return m_dwLastErr; }\n");
    }
}

int parse_args( int argc, TCHAR *argv[] )
{
    int iret = 0;
    TCHAR *arg, *sarg;
    int i, i2, c;
    for (i = 1; i < argc; i++)  {
        arg = argv[i];
        c = *arg;

        if (c == '-') {
            i2 = i + 1;
            sarg = &arg[1];
            while (*sarg == '-') {
                sarg++;
            }
            c = *sarg;
            switch (c)  {
            case 'h':
            case '?':
                give_help(argv[0],
                    (strcmp(arg,"--help") ? false : true) );
                return 2;
            case 'b':
                if (i2 < argc) {
                    i++;
                    sarg = argv[i];
                    block_size = atoi(sarg);
                    if (block_size <= 0) {
                        fprintf(stderr,"Invalid block_size %s (%d)! Must be a positive integer...\n", sarg, block_size);
                        return 1;
                    }
                } else {
                    fprintf(stderr,"Expected an integer block size to follow '%s'! Aborting...\n", arg);
                    return 1;
                }
                break;
            case 'r':
                if (i2 < argc) {
                    i++;
                    sarg = argv[i];
                    char *tmp;
                    def_len = strtoul(sarg,&tmp,10);
                    if (def_len <= 0) {
                        fprintf(stderr,"Invalid random size %s (%s)! Must be a positive integer...\n", sarg,
                            get_nice_num_stg(def_len));
                        return 1;
                    }
                } else {
                    fprintf(stderr,"Expected an integr block size to follow '%s'! Aborting...\n", arg);
                    return 1;
                }
                break;
            default:
                fprintf(stderr,"Unknown arg '%s'! Try -? for help. Aborting...\n", arg);
                return 1;
            }
        } else {
            if (read_file == 0) {
                read_file = strdup(arg);
            } else if (write_file == 0) {
                write_file = strdup(arg);
            } else {
                fprintf(stderr,"Already have read file '%s', and wirte file '%s'. What is this '%s'?\n",
                    read_file, write_file, arg );
                iret = 1;
                break;
            }
        }
    }
    return iret;
}

int _tmain(int argc, TCHAR* argv[])
{
	int nRetCode = 0;

    nRetCode = parse_args( argc, argv );
    if (nRetCode)
        return nRetCode;

    show_info();

	//Create test read file some big size, say 18 GB
	HANDLE hFile = CreateFile(read_file, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (hFile ==  INVALID_HANDLE_VALUE) {
        printf("Failed to open/create file '%s'\n", read_file);
        return 1;
    }

    suint64 nLength = def_len;
    LONG nLengthHigh;
    DWORD dwPtrLow;
    LONGLONG tactime, tmodtime;
    struct _utimbuf ut;
    bool set_time = false;
    if (read_temp == 2) {
        printf("Opened read '%s', of length %s bytes (%s)...\n", read_file, get_nice_num_stg(nLength),
            get_k_num_stg(nLength));
        memset(&ut,0,sizeof(ut));
        if (GetFileTime(hFile, NULL, (FILETIME*)&tactime, (FILETIME*)&tmodtime) ) {
           ut.actime = (time_t)( ( tactime - 116444736000000000) / 10000000 );
           ut.modtime = (time_t)( ( tmodtime - 116444736000000000) / 10000000 );
           set_time = true;
        }
    } else {
        printf("Created/Truncated a read file '%s'\n", read_file);

	    nLengthHigh = (nLength >> 32);
	    dwPtrLow = SetFilePointer(hFile, (LONG) (nLength & 0xFFFFFFFF),
		    &nLengthHigh, FILE_BEGIN);
        if ((dwPtrLow ==  INVALID_SET_FILE_POINTER) && (GetLastError() != NO_ERROR)) {
            printf("Failed SetFilePointer\n");
            return 1;
        }
	    if (!SetEndOfFile(hFile)) {
            printf("Failed SetEndOfFile\n");
            return 1;
        }
        printf("Increased its length to %s bytes (%s)...\n", get_nice_num_stg(nLength),
            get_k_num_stg(nLength));
    }
	CloseHandle(hFile);

	//Create a 1 byte write file, we copy the contents of readfile.txt into this
	hFile = CreateFile(write_file, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (hFile ==  INVALID_HANDLE_VALUE) {
        printf("Failed to create file '%s'\n", write_file);
        return 1;
    }
    printf("Created write file '%s'\n", write_file);

	nLength = 1;

	nLengthHigh = (nLength >> 32);
	dwPtrLow = SetFilePointer(hFile, (LONG) (nLength & 0xFFFFFFFF),
		&nLengthHigh, FILE_BEGIN);
    if ((dwPtrLow ==  INVALID_SET_FILE_POINTER) && (GetLastError() != NO_ERROR)) {
        printf("Failed SetFilePointer 2\n");
        return 1;
    }
	if (!SetEndOfFile(hFile)) {
        printf("Failed SetEndOfFile 2\n");
        return 1;
    }
	CloseHandle(hFile);
    printf("Set its length to %I64d bytes...\n", nLength);

	/*
		The following piece of code reads the contents of 'readfile.txt' ???KB at
		a time and writes it into 'writefile.txt', the loop runs until all bytes
		from 'readfile' has been written to 'writefile'
	*/

	CWinMMFIO mmf,mmfwrite;
	bool bRet = mmf.Open(read_file, OPN_READWRITE);
	if(!bRet) {
        printf("Failed to open read file '%s]\n", read_file);
        return 1;
    }

	bRet = mmfwrite.Open(write_file, OPN_READWRITE);
	if(!bRet) {
        printf("Failed to open write file '%s]\n", write_file);
        return 1;
    }

	char* pBuf = new char[block_size];

	printf("Copying readfile to writefile using MMFIO objects, using %d byte buffer...\n", block_size);
	int ncount, nwrite;
    double start = get_seconds();
    int cycles = 0;
    uint64_t total = 0;
	while(1)
	{
		ncount = mmf.Read(pBuf, 4096);
		if(ncount)
		{
			nwrite = mmfwrite.Write(pBuf, ncount);
            if (nwrite != ncount) {
                printf("Write failed! Requested %d, got %d!\n", ncount, nwrite );
                break;
            }
		} else
			break;
        cycles++;
        total += ncount;
        if ((cycles % 20000) == 0) {
            //printf("%d cycles. total read/written %I64u bytes, elapsed %s secs...\n", cycles, total, get_elapsed_stg(start) );
            double done = ((double)cycles / (double)tot_cycles) * 100.0;
            int ipct1000 = (int)(done * 10.0);
            double pct = (double)ipct1000 / (double)10.0;
            double secs = get_seconds() - start;
            double erem = (secs / done) * (100.0 - pct);
            printf("%.1lf%%: transferred %s, in %s, est %s...\n",
                pct,
                get_k_num_stg(total),
               seconds_to_hhmmss_stg(secs),
               seconds_to_hhmmss_stg(erem));
        }
	}

	suint64 nWriteFileLength;
    if ( !mmfwrite.GetLength(nWriteFileLength) ) {
        nWriteFileLength = total;
    }

    /////////////////////////////////////////////////////////////////////////////////////
    // show final results

    printf("\n");

    printf("%d cycles. total read/written %s bytes, elapsed %s secs...\n", cycles, 
        get_nice_num_stg(total), get_elapsed_stg(start) );

    if (nWriteFileLength == def_len) {
        double diff = get_seconds() - start;
        uint64_t bps = (uint64_t)(((double)nWriteFileLength / diff) + 0.5);
        printf("Appears a successful read/write operation... at rate %s/sec\n", get_k_num_stg(bps));
    } else {
        std::string err;
        printf("Warning: Write length is now %s bytes (%s)...\n", get_nice_num_stg(nWriteFileLength),
        get_k_num_stg(nWriteFileLength));
        mmf.GetMMFLastError(err);
        if (err.length()) {
            printf("Read error '%s'\n", err.c_str());
        }
        err = "";
        mmfwrite.GetMMFLastError(err);
        if (err.length()) {
            printf("Write error '%s'\n", err.c_str());
        }

    }

	mmf.Close();
	mmfwrite.Close();

	delete[] pBuf;

    if (read_temp == 1) {
        if (_unlink(read_file)) {
            printf("Attempt to delete '%s' failed! ", read_file);
            if (errno == EACCES)
                printf("No access error (EACCES)!");
            else if (errno ==  ENOENT) 
                printf("File not found (ENOENT)!");
            else
                printf("errno value %d!", errno);
            printf("\n");
        } else {
            printf("Deleted temporaty file '%s'.\n", read_file);
        }
    }
    if (set_time) {
        _utime( write_file, &ut );
    } else if (write_temp == 1) {
        if (_unlink(write_file)) {
            printf("Attempt to delete '%s' failed! ", write_file);
            if (errno == EACCES)
                printf("No access error (EACCES)!");
            else if (errno ==  ENOENT) 
                printf("File not found (ENOENT)!");
            else
                printf("errno value %d!", errno);
            printf("\n");
        } else {
            printf("Deleted temporary file '%s'.\n", write_file);
        }
    }

	return nRetCode;
}

/* eof */
