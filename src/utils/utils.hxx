/*\
 * utils.hxx
 *
 * Copyright (c) 2014 - Geoff R. McLane
 * Licence: GNU GPL version 2
 *
\*/

#ifndef _MMFIOUTILS_HXX_
#define _MMFIOUTILS_HXX_

#ifdef MAX_PATH
#define MY_FULLPATH_SZ MAX_PATH
#else
#define MY_FULLPATH_SZ 260
#endif
typedef struct tagSPLITPATH {
    TCHAR fullpath[MY_FULLPATH_SZ+2];
    TCHAR drive[_MAX_DRIVE];
    TCHAR path[_MAX_DIR];
    TCHAR fname[_MAX_FNAME];
    TCHAR ext[_MAX_EXT];
}SPLITPATH, *PSPLITPATH;

extern bool getAbsandSplit( const TCHAR *file, PSPLITPATH psp );
extern bool checkDirectoryExists( PSPLITPATH psp, TCHAR **ppc );

extern TCHAR *get_filename_only( TCHAR *name );

enum DiskType {
    DT_NONE,
    DT_FILE,
    DT_DIR
};

extern DiskType is_file_or_directory64 ( const char * path );
extern uint64_t get_last_stat_file_size64();
extern double get_seconds();
extern char *get_elapsed_stg( double start );
extern uint64_t getFreeDiskSpace(  LPCTSTR dir ); // return -1 if function fails
extern char *getFreeDiskSpace_stg( LPCTSTR dir ); // return NULL if function fails
extern uint64_t getlastFreeDiskSpace(); // only valid after one of the above calls
extern char *get_elapsed_hhmmss_stg(double start);
extern char *seconds_to_hhmmss_stg(double secs);

extern char *get_k_num_stg( uint64_t i64, int type = 0, int dotrim = 1 );
extern TCHAR *ULI_to_k_num_stg( ULARGE_INTEGER uli );

extern char *get_nice_num_stg( uint64_t len );

extern char *double_to_stg( double d );
extern void trim_float_buf( char *pb );

extern DWORD GetLastErrorMsg( LPTSTR lpm, DWORD dwLen, DWORD dwErr );
extern char *getTEMPFileName();


#ifndef MX_ONE_BUF
#define MX_ONE_BUF 1024
#endif
#ifndef MX_BUFFERS
#define MX_BUFFERS 1024
#endif
extern char *GetNxtBuf();

extern char *get_I64u_Stg( unsigned long long val );

#endif // #ifndef _MMFIOUTILS_HXX_
// eof - MMFIOUtils.hxx
