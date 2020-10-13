#include "embedded.h"
#include "tar.h"

#ifdef __APPLE__
#include <mach-o/getsect.h>
#include <mach-o/ldsyms.h>
#include <dlfcn.h>
extern int _res_stub;
static size_t osxsectionsize(const char* sectname)
{
    size_t sz; _res_stub = 0;
    getsectiondata(&_mh_execute_header, SEG_DATA, sectname, &sz);
    return sz;
}
static void* osxsectiondata(const char* sectname)
{
    size_t sz; _res_stub = 0;
    void* data = getsectiondata(&_mh_execute_header, SEG_DATA, sectname, &sz);
    return data;
}
#define EXTLD(NAME) \
  extern const unsigned char _section$__DATA__ ## NAME [];
#define LDVAR(NAME) (osxsectiondata("__" #NAME))
#define LDLEN(NAME) (osxsectionsize("__" #NAME))
#else /* GNU/Linux ld */
#define EXTLD(NAME) \
  extern const unsigned char _binary_ ## NAME ## _start[]; \
  extern const unsigned char _binary_ ## NAME ## _end[];
#define LDVAR(NAME) \
  _binary_ ## NAME ## _start
#define LDLEN(NAME) \
  ((_binary_ ## NAME ## _end) - (_binary_ ## NAME ## _start))
#endif

/*-----------------------------------------------------------------
 * Embedded data
 *-----------------------------------------------------------------*/
EXTLD(res_dat)

int embedded_file(void** data, size_t* sz, const char* fpath)
{
    const void* tar_data = LDVAR(res_dat);
    const size_t tar_sz  = LDLEN(res_dat);
    return tar_read_file((void*)tar_data, tar_sz, data, sz, fpath);
}
