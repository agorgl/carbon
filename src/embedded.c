#include "embedded.h"
#include "tar.h"

/*-----------------------------------------------------------------
 * Embedded data
 *-----------------------------------------------------------------*/
extern char _binary_res_dat_start[];
extern char _binary_res_dat_end[];

int embedded_file(void** data, size_t* sz, const char* fpath)
{
    void* tar_data = _binary_res_dat_start;
    size_t tar_sz  = _binary_res_dat_end - _binary_res_dat_start;
    return tar_read_file(tar_data, tar_sz, data, sz, fpath);
}
