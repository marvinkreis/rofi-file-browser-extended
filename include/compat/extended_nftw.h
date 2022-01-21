#ifndef FILE_BROWSER_EXTENDED_NFTW_H
#define FILE_BROWSER_EXTENDED_NFTW_H

#include <ftw.h>

#if defined(FTW_ACTIONRETVAL)  /* glibc */

#define extended_nftw nftw

#else  /* non-glibc */

#define FTW_ACTIONRETVAL 0x10
#define FTW_CONTINUE 0
#define FTW_STOP 1
#define FTW_SKIP_SUBTREE 2
#define FTW_SKIP_SIBLINGS 3

int extended_nftw(const char *path, int (*fn)(const char *, const struct stat *, int, struct FTW *), int fd_limit, int flags);
#endif

#endif
