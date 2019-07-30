#include "tar.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>

/* Tar Header Block, from POSIX 1003.1-1990. */
struct posix_header
{                   /* Byte offset */
    char name[100];     /*   0 */
    char mode[8];       /* 100 */
    char uid[8];        /* 108 */
    char gid[8];        /* 116 */
    char size[12];      /* 124 */
    char mtime[12];     /* 136 */
    char chksum[8];     /* 148 */
    char typeflag;      /* 156 */
    char linkname[100]; /* 157 */
    char magic[6];      /* 257 */
    char version[2];    /* 263 */
    char uname[32];     /* 265 */
    char gname[32];     /* 297 */
    char devmajor[8];   /* 329 */
    char devminor[8];   /* 337 */
    char prefix[155];   /* 345 */
    /* 500 */
};

#define TMAGIC   "ustar" /* "ustar" and a null */
#define TMAGLEN  6
#define TVERSION "00"    /* "00" and no null */
#define TVERSLEN 2

/* Values used in typeflag field. */
#define REGTYPE  '0'   /* Regular file */
#define AREGTYPE '\0'  /* Regular file */
#define LNKTYPE  '1'   /* Link */
#define SYMTYPE  '2'   /* Reserved */
#define CHRTYPE  '3'   /* Character special */
#define BLKTYPE  '4'   /* Block special */
#define DIRTYPE  '5'   /* Directory */
#define FIFOTYPE '6'   /* FIFO special */
#define CONTTYPE '7'   /* Reserved */
#define XHDTYPE  'x'   /* Extended header referring to the next file in the archive */
#define XGLTYPE  'g'   /* Global extended header */

/* Bits used in the mode field, values in octal. */
#define TSUID    04000 /* Set UID on execution */
#define TSGID    02000 /* Set GID on execution */
#define TSVTX    01000 /* Reserved */

/* File permissions */
#define TUREAD   00400 /* Read by owner */
#define TUWRITE  00200 /* Write by owner */
#define TUEXEC   00100 /* Execute/search by owner */
#define TGREAD   00040 /* Read by group */
#define TGWRITE  00020 /* Write by group */
#define TGEXEC   00010 /* Execute/search by group */
#define TOREAD   00004 /* Read by other */
#define TOWRITE  00002 /* Write by other */
#define TOEXEC   00001 /* Execute/search by other */

/* Tar Header Block, GNU extensions. */

/* In GNU tar, SYMTYPE is for to symbolic links, and CONTTYPE is for
   contiguous files, so maybe disobeying the "reserved" comment in POSIX
   header description. I suspect these were meant to be used this way, and
   should not have really been "reserved" in the published standards. */

/* *BEWARE* *BEWARE* *BEWARE* that the following information is still
   boiling, and may change. Even if the OLDGNU format description should be
   accurate, the so-called GNU format is not yet fully decided. It is
   surely meant to use only extensions allowed by POSIX, but the sketch
   below repeats some ugliness from the OLDGNU format, which should rather
   go away. Sparse files should be saved in such a way that they do *not*
   require two passes at archive creation time. Huge files get some POSIX
   fields to overflow, alternate solutions have to be sought for this. */

/* Descriptor for a single file hole. */
struct sparse
{                  /* Byte offset */
    char offset[12];   /*  0 */
    char numbytes[12]; /* 12 */
    /*  24 */
};

/* Sparse files are not supported in POSIX ustar format. For sparse files
   with a POSIX header, a GNU extra header is provided which holds overall
   sparse information and a few sparse descriptors. When an old GNU header
   replaces both the POSIX header and the GNU extra header, it holds some
   sparse descriptors too. Whether POSIX or not, if more sparse descriptors
   are still needed, they are put into as many successive sparse headers as
   necessary. The following constants tell how many sparse descriptors fit
   in each kind of header able to hold them. */

#define SPARSES_IN_EXTRA_HEADER  16
#define SPARSES_IN_OLDGNU_HEADER 4
#define SPARSES_IN_SPARSE_HEADER 21

/* Extension header for sparse files, used immediately after the GNU extra
   header, and used only if all sparse information cannot fit into that
   extra header. There might even be many such extension headers, one after
   the other, until all sparse information has been recorded. */

struct sparse_header
{                                           /* Byte offset */
    struct sparse sp[SPARSES_IN_SPARSE_HEADER]; /*   0 */
    char isextended;                            /* 504 */
    /* 505 */
};

/* The old GNU format header conflicts with POSIX format in such a way that
   POSIX archives may fool old GNU tar's, and POSIX tar's might well be
   fooled by old GNU tar archives. An old GNU format header uses the space
   used by the prefix field in a POSIX header, and cumulates information
   normally found in a GNU extra header. With an old GNU tar header, we
   never see any POSIX header nor GNU extra header. Supplementary sparse
   headers are allowed, however. */

struct oldgnu_header
{                                           /* Byte offset */
    char unused_pad1[345];                      /*   0 */
    char atime[12];                             /* 345 Incr. archive: atime of the file */
    char ctime[12];                             /* 357 Incr. archive: ctime of the file */
    char offset[12];                            /* 369 Multivolume archive: the offset of the start of this volume */
    char longnames[4];                          /* 381 Not used */
    char unused_pad2;                           /* 385 */
    struct sparse sp[SPARSES_IN_OLDGNU_HEADER]; /* 386 */
    char isextended;                            /* 482 Sparse file: Extension sparse header follows */
    char realsize[12];                          /* 483 Sparse file: Real size*/
    /* 495 */
};

/* OLDGNU_MAGIC uses both magic and version fields, which are contiguous.
   Found in an archive, it indicates an old GNU header format, which will be
   hopefully become obsolescent. With OLDGNU_MAGIC, uname and gname are
   valid, though the header is not truly POSIX conforming. */
#define OLDGNU_MAGIC "ustar  "  /* 7 chars and a null */

/* The standards committee allows only capital A through capital Z for
   user-defined expansion. Other letters in use include:

   'A' Solaris Access Control List
   'E' Solaris Extended Attribute File
   'I' Inode only, as in 'star'
   'N' Obsolete GNU tar, for file names that do not fit into the main header.
   'X' POSIX 1003.1-2001 eXtended (VU version)  */

/* This is a dir entry that contains the names of files that were in the
   dir at the time the dump was made. */
#define GNUTYPE_DUMPDIR 'D'

/* Identifies the *next* file on the tape as having a long linkname. */
#define GNUTYPE_LONGLINK 'K'

/* Identifies the *next* file on the tape as having a long name. */
#define GNUTYPE_LONGNAME 'L'

/* This is the continuation of a file that began on another volume. */
#define GNUTYPE_MULTIVOL 'M'

/* This is for sparse files. */
#define GNUTYPE_SPARSE 'S'

/* This file is a tape/volume header. Ignore it on extraction. */
#define GNUTYPE_VOLHDR 'V'

/* Solaris extended header */
#define SOLARIS_XHDTYPE 'X'

/* Jörg Schilling star header */
struct star_header
{                   /* Byte offset */
    char name[100];     /*   0 */
    char mode[8];       /* 100 */
    char uid[8];        /* 108 */
    char gid[8];        /* 116 */
    char size[12];      /* 124 */
    char mtime[12];     /* 136 */
    char chksum[8];     /* 148 */
    char typeflag;      /* 156 */
    char linkname[100]; /* 157 */
    char magic[6];      /* 257 */
    char version[2];    /* 263 */
    char uname[32];     /* 265 */
    char gname[32];     /* 297 */
    char devmajor[8];   /* 329 */
    char devminor[8];   /* 337 */
    char prefix[131];   /* 345 */
    char atime[12];     /* 476 */
    char ctime[12];     /* 488 */
    /* 500 */
};

#define SPARSES_IN_STAR_HEADER      4
#define SPARSES_IN_STAR_EXT_HEADER  21

struct star_in_header
{
    char fill[345];                           /*   0  Everything that is before t_prefix */
    char prefix[1];                           /* 345  t_name prefix */
    char fill2;                               /* 346  */
    char fill3[8];                            /* 347  */
    char isextended;                          /* 355  */
    struct sparse sp[SPARSES_IN_STAR_HEADER]; /* 356  */
    char realsize[12];                        /* 452  Actual size of the file */
    char offset[12];                          /* 464  Offset of multivolume contents */
    char atime[12];                           /* 476  */
    char ctime[12];                           /* 488  */
    char mfill[8];                            /* 500  */
    char xmagic[4];                           /* 508  "tar" */
};

struct star_ext_header
{
    struct sparse sp[SPARSES_IN_STAR_EXT_HEADER];
    char isextended;
};

/*
 * Validate number field
 *
 * This has to be pretty lenient in order to accommodate the enormous
 * variety of tar writers in the world:
 *  = POSIX (IEEE Std 1003.1-1988) ustar requires octal values with leading
 *    zeros and allows fields to be terminated with space or null characters
 *  = Many writers use different termination (in particular, libarchive
 *    omits terminator bytes to squeeze one or two more digits)
 *  = Many writers pad with space and omit leading zeros
 *  = GNU tar and star write base-256 values if numbers are too
 *    big to be represented in octal
 *
 *  Examples of specific tar headers that we should support:
 *  = Perl Archive::Tar terminates uid, gid, devminor and devmajor with two
 *    null bytes, pads size with spaces and other numeric fields with zeroes
 *  = plexus-archiver prior to 2.6.3 (before switching to commons-compress)
 *    may have uid and gid fields filled with spaces without any octal digits
 *    at all and pads all numeric fields with spaces
 *
 * This should tolerate all variants in use. It will reject a field
 * where the writer just left garbage after a trailing NUL.
 */
static int validate_number_field(const char* p_field, size_t i_size)
{
    unsigned char marker = (unsigned char)p_field[0];
    if (marker == 128 || marker == 255 || marker == 0) {
        /* Base-256 marker, there's nothing we can check. */
        return 1;
    } else {
        /* Must be octal */
        size_t i = 0;
        /* Skip any leading spaces */
        while (i < i_size && p_field[i] == ' ') {
            ++i;
        }
        /* Skip octal digits. */
        while (i < i_size && p_field[i] >= '0' && p_field[i] <= '7') {
            ++i;
        }
        /* Any remaining characters must be space or NUL padding. */
        while (i < i_size) {
            if (p_field[i] != ' ' && p_field[i] != 0) {
                return 0;
            }
            ++i;
        }
        return 1;
    }
}

/*
 * Note that this implementation does not (and should not!) obey
 * locale settings; you cannot simply substitute strtol here, since
 * it does obey locale.
 */
static int64_t tar_atol_base_n(const char *p, size_t char_cnt, int base)
{
    int64_t l, maxval, limit, last_digit_limit;
    int digit, sign;

    maxval = INT64_MAX;
    limit = INT64_MAX / base;
    last_digit_limit = INT64_MAX % base;

    /* The pointer will not be dereferenced if char_cnt is zero
     * due to the way the && operator is evaluated. */
    while (char_cnt != 0 && (*p == ' ' || *p == '\t')) {
        p++;
        char_cnt--;
    }

    sign = 1;
    if (char_cnt != 0 && *p == '-') {
        sign = -1;
        p++;
        char_cnt--;

        maxval = INT64_MIN;
        limit = -(INT64_MIN / base);
        last_digit_limit = INT64_MIN % base;
    }

    l = 0;
    if (char_cnt != 0) {
        digit = *p - '0';
        while (digit >= 0 && digit < base  && char_cnt != 0) {
            if (l>limit || (l == limit && digit > last_digit_limit)) {
                return maxval; /* Truncate on overflow. */
            }
            l = (l * base) + digit;
            digit = *++p - '0';
            char_cnt--;
        }
    }
    return (sign < 0) ? -l : l;
}

static int64_t tar_atol8(const char *p, size_t char_cnt)
{
    return tar_atol_base_n(p, char_cnt, 8);
}

/*
 * Parse a base-256 integer. This is just a variable-length
 * twos-complement signed binary value in big-endian order, except
 * that the high-order bit is ignored. The values here can be up to
 * 12 bytes, so we need to be careful about overflowing 64-bit
 * (8-byte) integers.
 *
 * This code unashamedly assumes that the local machine uses 8-bit
 * bytes and twos-complement arithmetic.
 */
static int64_t tar_atol256(const char *_p, size_t char_cnt)
{
    uint64_t l;
    const unsigned char *p = (const unsigned char *)_p;
    unsigned char c, neg;

    /* Extend 7-bit 2s-comp to 8-bit 2s-comp, decide sign. */
    c = *p;
    if (c & 0x40) {
        neg = 0xff;
        c |= 0x80;
        l = ~0ull;
    } else {
        neg = 0;
        c &= 0x7f;
        l = 0;
    }

    /* If more than 8 bytes, check that we can ignore
     * high-order bits without overflow. */
    while (char_cnt > sizeof(int64_t)) {
        --char_cnt;
        if (c != neg)
            return neg ? INT64_MIN : INT64_MAX;
        c = *++p;
    }

    /* c is first byte that fits; if sign mismatch, return overflow */
    if ((c ^ neg) & 0x80) {
        return neg ? INT64_MIN : INT64_MAX;
    }

    /* Accumulate remaining bytes. */
    while (--char_cnt > 0) {
        l = (l << 8) | c;
        c = *++p;
    }
    l = (l << 8) | c;
    /* Return signed twos-complement value. */
    return (int64_t)l;
}

/*-
 * Convert text->integer.
 *
 * Traditional tar formats (including POSIX) specify base-8 for
 * all of the standard numeric fields. This is a significant limitation
 * in practice:
 *   = file size is limited to 8GB
 *   = rdevmajor and rdevminor are limited to 21 bits
 *   = uid/gid are limited to 21 bits
 *
 * There are two workarounds for this:
 *   = pax extended headers, which use variable-length string fields
 *   = GNU tar and STAR both allow either base-8 or base-256 in
 *      most fields. The high bit is set to indicate base-256.
 *
 * On read, this implementation supports both extensions.
 */
static int64_t tar_atol(const char *p, size_t char_cnt)
{
    /*
     * Technically, GNU tar considers a field to be in base-256
     * only if the first byte is 0xff or 0x80.
     */
    if (*p & 0x80)
        return tar_atol256(p, char_cnt);
    return tar_atol8(p, char_cnt);
}

static const char* normalized_path(const char* path)
{
    const size_t len = strlen(path) + 1;
    char* buf = calloc(len, 1);
    const char* in_ptr = path;
    char* out_ptr = buf;
    while (*in_ptr != 0) {
        if (*in_ptr == '/') {
            char c1 = in_ptr[1];
            if (c1 == '.') {
                char c2 = in_ptr[2];
                if (c2 == '/') {
                    in_ptr += 2;
                    continue;
                } else if (c2 == '.' && (in_ptr[3] == '/' || in_ptr[3] == 0)) {
                    in_ptr += 3;
                    while (out_ptr > buf && *--out_ptr != '/') {
                    }
                    if (in_ptr[0] == 0) {
                        /* retain '/' */
                        out_ptr++;
                    }
                    continue;
                }
            } else if (c1 == '/') {
                ++in_ptr;
                continue;
            }
        }
        *out_ptr++ = *in_ptr++;
    }
    *out_ptr = 0;
    return buf;
}

int tar_read_file(void* tar_data, size_t tar_sz, void** fdata, size_t* fsize, const char* fpath)
{
    const char* npath = normalized_path(fpath);
    int r = 0;

    void* cur = tar_data;
    while((size_t)(cur - tar_data) < tar_sz) {
        struct posix_header* h = cur;
        cur += 512;
        assert(validate_number_field(h->size, sizeof(h->size)) == 1);
        long sz = (long) tar_atol(h->size, sizeof(h->size));
        switch(h->typeflag) {
            case REGTYPE:
            case AREGTYPE: {
                void* contents = cur;
                if (strncmp(h->name, npath, strlen(npath)) == 0) {
                    *fdata = contents;
                    *fsize = sz;
                    r = 1;
                    goto cleanup;
                }
                cur += sz + (512 - (sz % 512));
                break;
            }
            case DIRTYPE:
                break;
            default:
                continue;
        }
    }

cleanup:
    free((void*)npath);
    return r;
}
