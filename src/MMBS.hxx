/*\
 * MMBS.hxx
 *
 * Copyright (c) 2014 - Geoff R. McLane
 * Licence: GNU GPL version 2
 *
\*/

#ifndef _MMBS_HXX_
#define _MMBS_HXX_
#include <string>
#include <stdint.h>
#include <MMFIODef.h>
#include "utils.hxx"

template <typename TValue>
class MmapBS {

public:
    MmapBS() : m_size(4096), m_delete(false) {  }
    ~MmapBS()
    { 
        mmf.Close();
        if (m_delete)
            _unlink( file.c_str() );
    }
    bool Open(const std::string& infile="", bool remove=true)
    {
        bool bRet = false;
        file = infile;
        m_delete = remove;
        if (file == "") {
            file = getTEMPFileName();
        }
        bRet = mmf.Open(file.c_str(), OPN_READWRITE, true, m_size);
        if (!bRet) {
            std::string err;
            mmf.GetMMFLastError(err);
            fprintf(stderr, "Failed to open file '%s' - %s\n", file.c_str(), err.c_str());
        }
        return bRet;
    }
    bool set(const uint64_t id, const TValue value) {
        bool bRet = false;
        quint sz = sizeof(TValue);
        uint64_t offset =  id * sz;
        // note: offset MAY be beyond present file size, so add extend
        if ( !mmf.Seek( offset, SP_BEGIN, true ) ) {
            return bRet;
        }
        if( mmf.Write( (void *)&value, sz ) != sz ) {
            return bRet;
        }
        bRet = true;
        if (id > m_size)
            m_size = id;
        return bRet;
    }

    TValue operator[](uint64_t id) {
        quint sz = sizeof(TValue);
        uint64_t offset =  id * sz;
        char buf[26];
        char *cp = buf;
        // note: offset MUST be within current file size, so NO extend
        if ( !mmf.Seek( offset, SP_BEGIN, false ) ) {
            throw std::bad_alloc();
        }
        if (mmf.Read((void *)cp,sz) != sz) {
            throw std::bad_alloc();
        }
        TValue *ptv = (TValue *)cp;
        return *ptv;
    }

private:

    CWinMMFIO mmf;

    std::string file;

    uint64_t m_size;

    bool m_delete;
};


#endif // #ifndef _MMBS_HXX_
// eof - MMBS.hxx
