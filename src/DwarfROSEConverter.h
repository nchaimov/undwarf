#ifndef __DWARF_ROSE_CONVERTER_H__
#define __DWARF_ROSE_CONVERTER_H__

#include "rose.h"

class OffsetAttribute;

namespace DwarfROSE {

    extern size_t unnamed_count;
    SgFunctionParameterList * buildEmptyParameterList();
    SgType * typeFromAttribute(OffsetAttribute * a, SgScopeStatement * s);
    SgType * convertType(SgAsmDwarfConstruct * c, SgScopeStatement * s);    
    SgFunctionDeclaration * convertSubprogram(SgAsmDwarfSubprogram * p, SgScopeStatement * s);
    SgEnumDeclaration * convertEnum(SgAsmDwarfEnumerationType * e, SgScopeStatement * s);
    SgTypedefDeclaration * convertTypedef(SgAsmDwarfTypedef * t, SgScopeStatement * s);
    SgClassDeclaration * convertStruct(SgAsmDwarfStructureType * t, SgScopeStatement * s);
    SgClassDeclaration * convertUnion(SgAsmDwarfUnionType * t, SgScopeStatement * s);
    SgClassDeclaration * convertClass(SgAsmDwarfClassType * t, SgScopeStatement * s);
    SgVariableDeclaration * convertMember(SgAsmDwarfMember * m, SgScopeStatement * s);
    SgBaseClass * convertInheritance(SgAsmDwarfInheritance * m, SgScopeStatement * s);
    SgNamespaceDeclarationStatement * convertNamespace(SgAsmDwarfNamespace * n, SgScopeStatement * s);
}

#endif
