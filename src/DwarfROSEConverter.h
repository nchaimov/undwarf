#ifndef __DWARF_ROSE_CONVERTER_H__
#define __DWARF_ROSE_CONVERTER_H__

#include "rose.h"

class OffsetAttribute;

namespace DwarfROSE {

    extern size_t unnamed_count;

    SgType * typeFromAttribute(OffsetAttribute * a);
    SgType * convertType(SgAsmDwarfConstruct * c);    
    SgFunctionDeclaration * convertSubprogram(SgAsmDwarfSubprogram * s);
    SgEnumDeclaration * convertEnum(SgAsmDwarfEnumerationType * e);
    SgTypedefDeclaration * convertTypedef(SgAsmDwarfTypedef * t, SgScopeStatement * s);
    SgClassDeclaration * convertStruct(SgAsmDwarfStructureType * s);
    SgClassDeclaration * convertUnion(SgAsmDwarfUnionType * s);
    SgVariableDeclaration * convertMember(SgAsmDwarfMember * m);
}

#endif
