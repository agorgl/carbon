#include "embedded.h"
#include "tar.h"

#ifdef __APPLE__
#include <mach-o/getsect.h>
#define EXTLD(NAME) \
  extern const unsigned char _section$__DATA__ ## NAME [];
#define LDVAR(NAME) _section$__DATA__ ## NAME
#define LDLEN(NAME) (getsectbyname("__DATA", "__" #NAME)->size)
#elif (defined __WIN32__) /* MinGW */
#define EXTLD(NAME) \
  extern const unsigned char binary_ ## NAME ## _start[]; \
  extern const unsigned char binary_ ## NAME ## _end[];
#define LDVAR(NAME) \
  binary_ ## NAME ## _start
#define LDLEN(NAME) \
  ((binary_ ## NAME ## _end) - (binary_ ## NAME ## _start))
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
