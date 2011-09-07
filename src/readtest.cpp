#include "rose.h"

int main ( int argc, char* argv[] ) {
	SgProject* project = frontend(argc,argv);
	ROSE_ASSERT (project != NULL);
    return 0;
}                                  
