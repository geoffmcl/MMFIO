/*\
 * MMBS.cxx
 *
 * Copyright (c) 2014 - Geoff R. McLane
 * Licence: GNU GPL version 2
 *
\*/

#include <stdio.h>
#include <string>
#include <stdint.h>
#include <MMBS.hxx>
#include "utils.hxx"

#ifndef SPRTF
#define SPRTF printf
#endif

static const char *module = "MMBS";

#ifndef MY_DEF_ID
#define MY_DEF_ID 456789
#endif

#ifndef MY_DEF_VAL
#define MY_DEF_VAL 1234
#endif

static SPLITPATH sp;

typedef MmapBS<uint64_t> storage_mmap_t;

void show_help()
{
    SPRTF("%s [Options] [backstore_file]\n", module);
    SPRTF("Options:\n");
    SPRTF(" --help  (-h or -?) = This help and exit(0)\n");

    SPRTF("\n");
    SPRTF("If no 'backstore_file' name is given then will use a 'temporary' file,\n");
    SPRTF("which will be deleted on exit.\n");
    SPRTF("The file does not need to exist, and it will be overwritten if it does,\n");
    SPRTF("but any path given MUST exist. It will NOT be created!\n");

    uint64_t val = MY_DEF_VAL;
    uint64_t id  = MY_DEF_ID;
    uint64_t off = id * sizeof(uint64_t);
    SPRTF("\n");
    SPRTF("As the first test an id %s, with value %s, set at offset '%s'\n",
        get_I64u_Stg(id), get_I64u_Stg(val), get_I64u_Stg(off) );
    SPRTF("to test the file can 'grow' immediately to this size. In fact,\n");
    SPRTF("in anticipation of further settings the file size will be plus 4096.\n");
    SPRTF("Then increasing values will be placed at offsets back to 0, testing\n");
    SPRTF("the memory mapping works for all ranges.\n");
    SPRTF("Then the original value will be retrived from the store and compared.\n");
    SPRTF("\n");
    SPRTF("A larger 'id' can be set by -DMY_DEF_ID=987654321000, but be aware the\n");
    SPRTF("file will grow to 8 times this size, so you will need sufficient disk space.\n");
    off = 987654321000;
    off *= 8;
    off += 4096;
    SPRTF("That paticular value would require a whopping %s of disk space!\n", get_k_num_stg(off,1));
    SPRTF("But should work if you can supply that much space. Try it ;=))\n");

}


// main() OS entry
int main( int argc, char **argv )
{
    int iret;
    char *arg, *p;
    std::string file = "";
    bool do_del = true;
    for (iret = 1; iret < argc; iret++) {
        arg = argv[iret];
        if ((strcmp(arg,"-?") == 0)||
            (strcmp(arg,"-h") == 0)||
            (strcmp(arg,"--help") == 0)) {
            show_help();
            return 0;
        }
        p = 0;
        if (getAbsandSplit( arg, &sp ) &&
            checkDirectoryExists( &sp, &p )) {
            file = arg;
            do_del = false;
        } else {
            if (p)
                SPRTF("%s: Unable to find path '%s'\n", module, p );
            else
                SPRTF("%s: Error resolving path '%s'\n", module, arg );
            return 1;
        }
    }
    iret = 0;
    SPRTF("%s: Creating backing store mapping file '%s'\n", module,
        (file.length() ? file.c_str() : "temporary") );
    storage_mmap_t mmt;
    if (!mmt.Open(file,do_del)) {
        printf("%s: Failed to open memory mapped backing store!\n", module);
        return 1;
    }
    uint64_t val = MY_DEF_VAL;
    uint64_t id  = MY_DEF_ID;
    SPRTF("%s: Setting id %s, with value %s\n", module,
        get_I64u_Stg(id), get_I64u_Stg(val) );

    if (!mmt.set(id,val)) {
        printf("%s: Failed to set value in memory mapped backing store!\n", module);
        return 1;
    }
    SPRTF("%s: Setting %s ids, down to 0, with new values...\n", module, get_I64u_Stg(id));
    while (id > 0) {
        id--;
        val++;
        if (!mmt.set(id,val)) {
            printf("%s: Failed to set value in memory mapped backing store!\n", module);
            return 1;
        }
    }
    val = MY_DEF_VAL;
    id  = MY_DEF_ID;
    SPRTF("%s: Getting id %s value expect %s ", module, get_I64u_Stg(id), get_I64u_Stg(val) );
    uint64_t val2 = mmt[id];
    if (val2 == val) {
         SPRTF("got %s - SUCCESS.\n", get_I64u_Stg(val2));
    } else {
         SPRTF("got %s - FAILED!\n", get_I64u_Stg(val2));
    }


    return iret;
}


// eof = MMBS.cxx
