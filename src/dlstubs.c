// from http://gcc.gnu.org/ml/gcc-help/2001-10/msg00303.html
// workaround for absence of libdl.a

#include <sys/types.h>
#include <dlfcn.h>

typedef long int Lmid_t;

typedef struct
{
  __const char *dli_fname;	/* File name of defining object.  */
  void *dli_fbase;		/* Load address of that object.  */
  __const char *dli_sname;	/* Name of nearest symbol.  */
  void *dli_saddr;		/* Exact value of nearest symbol.  */
} Dl_info;


	/* dl*() stub routines for static compilation.  Prepared from
	   /usr/include/dlfcn.h by Hal Pomeranz <hal@deer-run.com> */

	void *dlopen(const char *str, int x) {}
	void *dlsym(void *ptr, const char *str) {}
	int dlclose(void *ptr) {}
	char *dlerror() {}
	void *dlmopen(Lmid_t a, const char *str, int x) {}
	int dladdr(void *ptr1, Dl_info *ptr2) {}
	int dldump(const char *str1, const char *str2, int x) {}
	int dlinfo(void *ptr1, int x, void *ptr2) {}

	void *_dlopen(const char *str, int x) {}
	void *_dlsym(void *ptr, const char *str) {}
	int _dlclose(void *ptr) {}
	char *_dlerror() {}
	void *_dlmopen(Lmid_t a, const char *str, int x) {}
	int _dladdr(void *ptr1, Dl_info *ptr2) {}
	int _dldump(const char *str1, const char *str2, int x) {}
	int _dlinfo(void *ptr1, int x, void *ptr2) {}
