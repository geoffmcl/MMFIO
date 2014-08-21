/*\
 * utils.cxx
 *
 * Copyright (c) 2014 - Geoff R. McLane
 * Licence: GNU GPL version 2
 *
\*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utime.h>
#include <sys/timeb.h>
#include <WinSock2.h>   // NOTE: This includes <Windows.h>
#include <stdio.h>
#include <stdlib.h> // for strtoull(), ...
#include <stdint.h>
#include <tchar.h>
#include "utils.hxx"

#pragma warning( disable : 4996 )

static const char *module = "utils";

#ifdef _MSC_VER
#define M_IS_DIR _S_IFDIR
#else // !_MSC_VER
#define M_IS_DIR S_IFDIR
#endif

static struct _stat64 buf64;
uint64_t get_last_stat_file_size64() { return buf64.st_size; }
DiskType is_file_or_directory64 ( const char * path )
{
    if (!path)
        return DT_NONE;
	if (_stat64(path,&buf64) == 0)
	{
		if (buf64.st_mode & M_IS_DIR)
			return DT_DIR;
		else
			return DT_FILE;
	}
	return DT_NONE;
}

int gettimeofday(struct timeval *tp, void *tzp) // from crossfeed source
{
    struct _timeb timebuffer;
    _ftime(&timebuffer);
    tp->tv_sec = (long)timebuffer.time;
    tp->tv_usec = timebuffer.millitm * 1000;
    return 0;
}

double get_seconds()
{
    struct timeval tv;
    gettimeofday(&tv,0);
    double t1 = (double)(tv.tv_sec+((double)tv.tv_usec/1000000.0));
    return t1;
}
char *get_elapsed_stg( double start )
{
    double now = get_seconds();
    return double_to_stg( now - start );
}

/* -------------------------------------------------------------------
   Assumes the buffer length is LARGER than the string length + 1
   ------------------------------------------------------------------- */
char *increment_string_in_buffer2(char *s)
{
	int i, begin, tail, len;
	int neg = (*s == '-');
	char tgt = neg ? '0' : '9';
 
	/* special case: "-1" */
	if (!strcmp(s, "-1")) {
		s[0] = '0', s[1] = '\0';
		return s;
	}
 
	len = (int)strlen(s);
	begin = (*s == '-' || *s == '+') ? 1 : 0;
 
	/* find out how many digits need to be changed */
	for (tail = len - 1; tail >= begin && s[tail] == tgt; tail--);
 
	if (tail < begin && !neg) {
		/* special case: all 9s, string will grow */
        /* ASSUME BUFFER IS LARGE ENOUGH
           *****************************
		if (!begin) {
            s = (char *)realloc(s, len + 2);
            if (!s) {
                printf("realloc(%d) FAILED!\n", len + 2 );
                exit(1);
            }
        }
          ============================= */
		s[0] = '1';
		for (i = 1; i <= len - begin; i++) s[i] = '0';
		s[len + 1] = '\0';
	} else if (tail == begin && neg && s[1] == '1') {
		/* special case: -1000..., so string will shrink */
		for (i = 1; i < len - begin; i++) s[i] = '9';
		s[len - 1] = '\0';
	} else { /* normal case; change tail to all 0 or 9, change prev digit by 1*/
		for (i = len - 1; i > tail; i--)
			s[i] = neg ? '9' : '0';
		s[tail] += neg ? -1 : 1;
	}
 
	return s;
}

/* ------------------------------------------------------------------
   Hmmm, maybe NOT every case well covered, but looks good
   ----------------------------------------------------------------- */
void trim_secs_buffer( char *cp )
{
    size_t ii, len = strlen(cp);
    while (len) {
        len--;
        if (cp[len] == '.') {
            strcat(cp,"0");
            break;
        } else if (cp[len] > '0') {
            break;
        }
        cp[len] = 0;
    }
    if (len) {
        int c;
        size_t zi;
        for (ii = 0; ii < len; ii++) {
            if (cp[ii] == '.') {
                zi = ii;
                ii++;
                int digs = 0;
                int zeros = 0;
                for (; ii < len; ii++) {
                    c = cp[ii];
                    if (c == '0')
                        zeros++;
                    else
                        digs++;
                    if (digs > 3) {
                        c = cp[ii+1];
                        cp[ii+1] = 0;
                        if (c == '0') {
                            cp[ii] = 0;
                            ii--;
                            while (ii > zi) {
                                if (cp[ii] > '0')
                                    break;
                                cp[ii] = 0;
                                ii--;
                            }
                        } else if (c >= '5') {
                            while (ii > zi) {
                                c = cp[ii];
                                if (c == '9') {
                                    cp[ii] = 0;
                                } else {
                                    c++;
                                    cp[ii] = c;
                                    c = 0;
                                    break;
                                }
                                ii--;
                            }
                            if ((c == '9') && (ii == zi)) {
                                // must increment the value before the decimal
                                cp[ii] = 0; // lose the decimal point
                                increment_string_in_buffer2(cp); // and bump the number in buffer
                            }
                        }
                        ii = len;
                        break;
                    } else if (digs && (zeros >= 3) && (c == '0')) {
                        // had some digits after decimal, and 3 or more zeros, and this is one of them
                        cp[ii] = 0; // stop it here
                        ii--;
                        while (ii > zi) {
                            if (cp[ii] > '0')
                                break;
                            cp[ii] = 0;
                            ii--;
                        }
                        break;
                    }
                }
                break;
            }
        }
    }
}

char *seconds_to_hhmmss_stg(double secs)
{
    char *cp = GetNxtBuf();
    int i;
    if (secs < 1.0) {
        // got for near max, but clean it up a bit
        sprintf(cp,"%.15lf", secs );
        trim_secs_buffer(cp);
        strcat(cp," secs");
        return cp;
    }
    if (secs < 60.0) {
        // just show 2 decimal places
        i = (int)((secs + 0.005) * 100.0);
        sprintf(cp,"%.2lf secs", ((double)i / 100.0));
        return cp;
    }
    // now have minutes, no decimal on seconds
    int mins = (int)(secs / 60.0);
    secs -= (double)(mins * 60);
    i = (int)(secs + 0.5);
    if (mins < 60.0) {
        secs -= (mins * 60.0);
        sprintf(cp, "%d:%02d mm:ss", mins, i);
        return cp;
    }
    int hour = (int)(mins / 60.0);
    mins -= hour * 60;
    sprintf(cp,"%d:%02d:%02d", hour, mins, i);
    return cp;
}

char *get_elapsed_hhmmss_stg(double start)
{
    double elap = get_seconds() - start;
    return seconds_to_hhmmss_stg(elap);
}

static uint64_t ds_fs;
static ULARGE_INTEGER ds_free, ds_total, ds_tbfree;
uint64_t getlastFreeDiskSpace() { return ds_fs; }
uint64_t getFreeDiskSpace(  LPCTSTR dir )
{
    if ( GetDiskFreeSpaceEx(
        dir,            // _In_opt_   LPCTSTR lpDirectoryName,
        &ds_free,       // _Out_opt_  PULARGE_INTEGER lpFreeBytesAvailable,
        &ds_total,      // _Out_opt_  PULARGE_INTEGER lpTotalNumberOfBytes,
        &ds_tbfree )) { // _Out_opt_  PULARGE_INTEGER lpTotalNumberOfFreeBytes
        ds_fs = ds_free.QuadPart;
    } else {
        ds_fs = -1;
    }
    return ds_fs;
}

char *getFreeDiskSpace_stg( LPCTSTR dir )
{
    char *cp = 0;
    if (getFreeDiskSpace(dir) != -1) {
        cp = get_k_num_stg(ds_fs);
    }
    return cp;
}

static char _s_strbufs[MX_ONE_BUF * MX_BUFFERS];
static int iNextBuf = 0;

char *GetNxtBuf()
{
   iNextBuf++;
   if(iNextBuf >= MX_BUFFERS)
      iNextBuf = 0;
   return &_s_strbufs[MX_ONE_BUF * iNextBuf];
}

#ifndef SPRINTF
#define SPRINTF sprintf
#endif
#ifndef STRLEN
#define STRLEN strlen
#endif
#ifndef STRCAT
#define STRCAT strcat
#endif

void trim_float_buf( char *pb )
{
   size_t len = STRLEN(pb);
   size_t i, dot, zcnt;
   for( i = 0; i < len; i++ )
   {
      if( pb[i] == '.' )
         break;
   }
   dot = i + 1; // keep position of decimal point (a DOT)
   zcnt = 0;
   for( i = dot; i < len; i++ )
   {
      if( pb[i] != '0' )
      {
         i++;  // skip past first
         if( i < len )  // if there are more chars
         {
            i++;  // skip past second char
            if( i < len )
            {
               size_t i2 = i + 1;
               if( i2 < len )
               {
                  if ( pb[i2] >= '5' )
                  {
                     if( pb[i-1] < '9' )
                     {
                        pb[i-1]++;
                     }
                  }
               }
            }
         }
         pb[i] = 0;
         break;
      }
      zcnt++;     // count ZEROS after DOT
   }
   if( zcnt == (len - dot) )
   {
      // it was ALL zeros
      pb[dot - 1] = 0;
   }
}


char *double_to_stg( double d )
{
    char *cp = GetNxtBuf();
    sprintf(cp,"%lf",d);
    trim_float_buf(cp);
    return cp;
}

char *get_I64_Stg( long long val )
{
    char *cp = GetNxtBuf();
#ifdef _MSC_VER
    sprintf(cp,"%I64d",val);
#else
    sprintf(cp,"%lld",val);
#endif
    return cp;
}

char *get_I64u_Stg( unsigned long long val )
{
    char *cp = GetNxtBuf();
#ifdef _MSC_VER
    sprintf(cp,"%I64u",val);
#else
    sprintf(cp,"%llu",val);
#endif
    return cp;
}

char *get_k_num_stg( uint64_t i64, int type, int dotrim )
{
   char *pb = GetNxtBuf();
   const char *form = " bytes";
   unsigned long long byts = i64;
   double res;
   const char*ffm = "%0.20f";  // get 20 digits
   if( byts < 1024 ) {
      strcpy(pb, get_I64_Stg(byts));
      dotrim = 0;
   } else if( byts < 1024*1024 ) {
      res = ((double)byts / 1024.0);
      form = (type ? " KiloBypes" : " KB");
      SPRINTF(pb, ffm, res);
   } else if( byts < 1024*1024*1024 ) {
      res = ((double)byts / (1024.0*1024.0));
      form = (type ? " MegaBypes" : " MB");
      SPRINTF(pb, ffm, res);
   } else { // if( byts <  (1024*1024*1024*1024)){
      double val = (double)byts;
      double db = (1024.0*1024.0*1024.0);   // x3
      double db2 = (1024.0*1024.0*1024.0*1024.0);   // x4
      if( val < db2 )
      {
         res = val / db;
         form = (type ? " GigaBypes" : " GB");
         SPRINTF(pb, ffm, res);
      }
      else
      {
         db *= 1024.0;  // x4
         db2 *= 1024.0; // x5
         if( val < db2 )
         {
            res = val / db;
            form = (type ? " TeraBypes" : " TB");
            SPRINTF(pb, ffm, res);
         }
         else
         {
            db *= 1024.0;  // x5
            db2 *= 1024.0; // x6
            if( val < db2 )
            {
               res = val / db;
               form = (type ? " PetaBypes" : " PB");
               SPRINTF(pb, ffm, res);
            }
            else
            {
               db *= 1024.0;  // x6
               db2 *= 1024.0; // x7
               if( val < db2 )
               {
                  res = val / db;
                  form = (type ? " ExaBypes" : " EB");
                  SPRINTF(pb, ffm, res);
               }
               else
               {
                  db *= 1024.0;  // x7
                  res = val / db;
                  form = (type ? " ZettaBypes" : " ZB");
                  SPRINTF(pb, ffm, res);
               }
            }
         }
      }
   }
   if( dotrim > 0 )
      trim_float_buf(pb);

   STRCAT(pb, form);

   //if( ascii > 0 )
   //   Convert_2_ASCII(pb);

   return pb;
}

/* ======================================================================
   nice_num_to_buf = get nice number, with commas
   given a destination buffer,
   and a source buffer of ascii
   NO CHECK OF LENGTH DONE!!! assumed destination is large enough
   and assumed it is a NUMBER ONLY IN THE SOURCE
   ====================================================================== */
void nice_num_to_buf( char * dst, char * src ) // get nice number, with commas
{
   size_t i;
   size_t len = strlen(src);
   size_t rem = len % 3;
   size_t cnt = 0;
   for( i = 0; i < len; i++ )
   {
      if( rem ) {
         *dst++ = src[i];
         rem--;
         if( ( rem == 0 ) && ( (i + 1) < len ) )
            *dst++ = ',';
      } else {
         *dst++ = src[i];
         cnt++;
         if( ( cnt == 3 ) && ( (i + 1) < len ) ) {
            *dst++ = ',';
            cnt = 0;
         }
      }
   }
   *dst = 0;
}

char *get_nice_num_stg( uint64_t len )
{
    char *src = get_I64u_Stg( len );
    char *dst = GetNxtBuf();
    nice_num_to_buf(dst,src);
    return dst;
}

TCHAR *ULI_to_k_num_stg( ULARGE_INTEGER uli )
{
    uint64_t val = uli.QuadPart;
    return get_k_num_stg(val);
}

DWORD GetLastErrorMsg( LPTSTR lpm, DWORD dwLen, DWORD dwErr )
{
   PVOID lpMsgBuf = 0;
   DWORD    dwr;

   dwr = FormatMessage( 
      FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwErr,   //	GetLastError(),
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
      (LPTSTR) &lpMsgBuf,
		0,
		NULL );
   
   //dwr = strlen(lpMsgBuf);
   if( ( dwr == 0 ) || ( dwr >= dwLen ) )
      dwr = (DWORD)-1;
   else
      strcat(lpm, (LPTSTR)lpMsgBuf);

   //	printf("%s:%s\n",lpm,(LPCTSTR)lpMsgBuf);
   // Free the buffer.
   if( lpMsgBuf )
      LocalFree( lpMsgBuf );

   return dwr;
}

static void showlasterr(const char *msg)
{
    TCHAR emsg[264];
    DWORD res, err = GetLastError();
    LPTSTR lps = emsg;
    *lps = 0;
    res = GetLastErrorMsg( lps, 256, err ); 
    if (res != -1) {
        fprintf(stderr,"Error: %s (%d) - %s\n", msg, err, lps );
    } else {
        fprintf(stderr,"Error: %s (%d)\n", msg, err );
    }
}


char *getTEMPFileName()
{
    char szTmpFile[256];
    char *cp = GetNxtBuf();
    /* Create temporary file for mapping. */
    if (!GetTempPath (256, szTmpFile)) {
        showlasterr("GetTempPath FAILED!");
        //throw std::bad_alloc();
        //exit(1);
    }
    if (!GetTempFileName (szTmpFile,"MM",0,cp )) {
        showlasterr("GetTempFileName FAILED!");
        //throw std::bad_alloc();
        exit(1);
    }
    return cp;
}

bool getAbsandSplit( const TCHAR *file, PSPLITPATH psp )
{
    bool bret = false;
    LPSTR lps = 0;
    DWORD dwd = GetFullPathName(
        file,
        MY_FULLPATH_SZ,
        psp->fullpath,
        &lps );
    if (dwd && (dwd <= MY_FULLPATH_SZ)) {
        _splitpath( psp->fullpath,
            psp->drive, psp->path, psp->fname, psp->ext );
        bret = true;
    }
    return bret;
}

bool checkDirectoryExists( PSPLITPATH psp, TCHAR **ppc )
{
    bool bret = false;
    char *cp2 = GetNxtBuf();
    strcpy(cp2,psp->drive);
    strcat(cp2,psp->path);
    size_t len = strlen(cp2);
    if (len) {
        len--;
        int c = cp2[len];
        if ((c == '\\') || (c == '/'))
            cp2[len] = 0;   // drop trailing slash
        if (is_file_or_directory64( cp2 ) == DT_DIR ) {
            bret = true;
        }
    }
    *ppc = cp2;
    return bret;
}

TCHAR *get_filename_only( TCHAR *name )
{
    TCHAR *fn = name;
    size_t ii, len = strlen(name);
    int c;
    for (ii = 0; ii < len; ii++) {
        c = name[ii];
        if ( ((c == '\\') || (c == '/')) && ((ii + 1) < len) ) {
            fn = &name[ii + 1];
        }
    }
    return fn;
}


// eof = MMFIOUtils.cxx
