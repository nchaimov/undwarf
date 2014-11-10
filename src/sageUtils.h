// This file contains functions that should logically be inside SageInterface
// or SageBuilder, but aren't.

#include <string>
#include "rose.h"

namespace SageUtils {
    SgFunctionParameterList * buildEmptyParameterList() ;
    SgEnumType * buildEnumType(SgEnumDeclaration * d) ;
    SgTypedefType * buildTypedefType(SgTypedefDeclaration * d) ;
    SgClassType * buildClassType(SgClassDeclaration * d) ;
    SgClassDeclaration * buildUnionDeclaration(const SgName & name, SgScopeStatement * scope = NULL) ;
    bool operatorIsCast(std::string name);
    void addComment(const std::string & comment, SgScopeStatement * scope);
}
