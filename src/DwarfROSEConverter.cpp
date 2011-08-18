#include <stdint.h>
#include "DwarfROSEConverter.h"
#include "rose.h"
#include "typeTable.h"
#include "attributes.h"
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

size_t DwarfROSE::unnamed_count = 0;

SgType * DwarfROSE::typeFromAttribute(OffsetAttribute * attr) {
    if(attr == NULL) {
        return SageBuilder::buildVoidType();
    } else {
        return DwarfROSE::convertType(attr->type);
    }
}

SgEnumType * buildEnumType(SgEnumDeclaration * d) {
    SgEnumType * type = new SgEnumType();
    type->set_declaration(d);
    return type;
}

SgTypedefType * buildTypedefType(SgTypedefDeclaration * d) {
    SgTypedefType * type = new SgTypedefType();
    type->set_declaration(d);
    return type;
}

SgClassType * buildClassType(SgClassDeclaration * d) {
    SgClassType * type = new SgClassType();
    type->set_declaration(d);
    return type;
}

SgClassDeclaration * buildUnionDeclaration(const SgName & name, SgScopeStatement * scope = NULL) {
    SgClassDeclaration* defdecl = SageBuilder::buildClassDeclaration_nfi(name,SgClassDeclaration::e_union,scope,NULL);
    SageInterface::setOneSourcePositionForTransformation(defdecl);
    ROSE_ASSERT(defdecl->get_firstNondefiningDeclaration() != NULL);
    SageInterface::setOneSourcePositionForTransformation(defdecl->get_firstNondefiningDeclaration());
    return defdecl; 
}

SgType * DwarfROSE::convertType(SgAsmDwarfConstruct * c) {
    if(c == NULL) {
        return SageBuilder::buildVoidType();
    }

    switch(c->variantT()) {
        // BASE TYPES
        case V_SgAsmDwarfBaseType:
            return TypeTable::getInstance().createType(c->get_name());

        // POINTERS
        case V_SgAsmDwarfPointerType: {
            OffsetAttribute * attr = OffsetAttribute::get(c);
            SgType * pointedTo = NULL;
            if(attr == NULL) {
                pointedTo = SageBuilder::buildVoidType();
            } else {
                pointedTo = convertType(attr->type);
            }
            return SageBuilder::buildPointerType(pointedTo);
        };

        // ENUMS
        case V_SgAsmDwarfEnumerationType: {
            OffsetAttribute * attr = OffsetAttribute::get(c);
            ROSE_ASSERT(attr != NULL);
            SgEnumDeclaration * enumDecl = isSgEnumDeclaration(attr->node);
            if(enumDecl == NULL) {
                std::cerr << "ERROR: Enumeration type had no associated enumeration declaration." << std::endl;
            } else {
                return buildEnumType(enumDecl);
            }
        };

        // TYPEDEFS
        case V_SgAsmDwarfTypedef: {
            OffsetAttribute * attr = OffsetAttribute::get(c);
            ROSE_ASSERT(attr != NULL);
            SgTypedefDeclaration * decl = isSgTypedefDeclaration(attr->node);
            if(decl == NULL) {
                std::cerr << "ERROR: Typedef had no associated typedef declaration." << std::endl;
            } else {
                return buildTypedefType(decl);
            }
        };
        
        // CONST
        case V_SgAsmDwarfConstType: {
            OffsetAttribute * attr = OffsetAttribute::get(c);
            ROSE_ASSERT(attr != NULL);
            SgType * baseType = typeFromAttribute(attr);
            return SageBuilder::buildConstType(baseType);
        };

        // VOLATILE
        case V_SgAsmDwarfVolatileType: {
            OffsetAttribute * attr = OffsetAttribute::get(c);
            ROSE_ASSERT(attr != NULL);
            SgType * baseType = typeFromAttribute(attr);
            return SageBuilder::buildVolatileType(baseType);
        };

        // STRUCTS
        case V_SgAsmDwarfStructureType: {
            OffsetAttribute * attr = OffsetAttribute::get(c);
            ROSE_ASSERT(attr != NULL);
            SgClassDeclaration * decl = isSgClassDeclaration(attr->node);
            if(decl == NULL) {
                std::cerr << "ERROR: Class/struct/union had no associated declaration." << std::endl;
            } else {
                return buildClassType(decl);
            }
        };
        

        default: 
            std::cerr << "Unhandled type " << c->class_name() << std::endl;
    }    

    return NULL;
}

SgFunctionParameterList * buildEmptyParameterList() {
    SgFunctionParameterList *parameterList = new SgFunctionParameterList();
    ROSE_ASSERT (parameterList);
    parameterList->set_definingDeclaration (NULL);
    parameterList->set_firstNondefiningDeclaration(parameterList);
    SageInterface::setOneSourcePositionForTransformation(parameterList);
    return parameterList;
}

SgFunctionDeclaration * DwarfROSE::convertSubprogram(SgAsmDwarfSubprogram * s) {
    if(s == NULL) {
        return NULL;
    }
    
    std::string name = s->get_name();
    OffsetAttribute * attr = OffsetAttribute::get(s);
    SgType * retType = typeFromAttribute(attr);

    SgFunctionParameterList * paramList = buildEmptyParameterList();

    Rose_STL_Container<SgNode*> formalParams = NodeQuery::querySubTree(s, V_SgAsmDwarfFormalParameter);
    BOOST_FOREACH(SgNode * n, formalParams) {
        SgAsmDwarfFormalParameter * formalParam = isSgAsmDwarfFormalParameter(n);
        std::string paramName = formalParam->get_name();
        OffsetAttribute * paramAttr = OffsetAttribute::get(formalParam);
        SgType * paramType = typeFromAttribute(paramAttr);
        SgInitializedName * initName = SageBuilder::buildInitializedName(paramName, paramType);
        SageInterface::appendArg(paramList, initName);
    }

    SgFunctionDeclaration * decl = SageBuilder::buildNondefiningFunctionDeclaration(SgName(name), retType, paramList);
    
    return decl;
}

SgEnumDeclaration * DwarfROSE::convertEnum(SgAsmDwarfEnumerationType * e) {
    if(e == NULL) {
        return NULL;
    }
    
    std::string name = e->get_name();
    SgEnumDeclaration * decl = SageBuilder::buildEnumDeclaration(SgName(name));
    Rose_STL_Container<SgNode*> enumerators = NodeQuery::querySubTree(e, V_SgAsmDwarfEnumerator);
    BOOST_FOREACH(SgNode * n, enumerators) {
        SgAsmDwarfEnumerator * enumerator = isSgAsmDwarfEnumerator(n);
        uint64_t val = enumerator->get_const_val();
        std::string valName = enumerator->get_name();
        SgAssignInitializer * assign = SageBuilder::buildAssignInitializer(SageBuilder::buildIntVal(val), SageBuilder::buildIntType());
        SgInitializedName * init = SageBuilder::buildInitializedName(SgName(valName), SageBuilder::buildIntType(), assign);
        decl->append_enumerator(init);
        init->set_parent(decl);
    }
    OffsetAttribute::get(e)->node = decl;
    return decl;
}

SgTypedefDeclaration * DwarfROSE::convertTypedef(SgAsmDwarfTypedef * t, SgScopeStatement * s) {
    if(t == NULL) {
        return NULL;
    }               
    OffsetAttribute * attr = OffsetAttribute::get(t);
    ROSE_ASSERT(attr != NULL);
    SgType * baseType = typeFromAttribute(attr);
    std::string name = t->get_name();
    // ROSE insists that this have a scope when built; I don't know why.
    SgTypedefDeclaration * decl = SageBuilder::buildTypedefDeclaration(SgName(name), baseType, s);
    attr->node = decl;
    return decl;
}

SgClassDeclaration * DwarfROSE::convertStruct(SgAsmDwarfStructureType * s) {
    if(s == NULL) {
        return NULL;
    }

    OffsetAttribute * attr = OffsetAttribute::get(s);
    ROSE_ASSERT(attr != NULL);
    std::string name = s->get_name();
    if(name.empty()) {
        name = "_UNNAMED_STRUCT_" + boost::lexical_cast<std::string>(unnamed_count++) + "_";
    }
    SgClassDeclaration * decl = SageBuilder::buildStructDeclaration(SgName(name));
    ROSE_ASSERT(decl != NULL);
    attr->node = decl;
    return decl;
}

SgClassDeclaration * DwarfROSE::convertUnion(SgAsmDwarfUnionType * s) {
    if(s == NULL) {
        return NULL;
    }

    OffsetAttribute * attr = OffsetAttribute::get(s);
    ROSE_ASSERT(attr != NULL);
    std::string name = s->get_name();
    if(name.empty()) {
        name = "_UNNAMED_UNION_" + boost::lexical_cast<std::string>(unnamed_count++) + "_";
    }
    SgClassDeclaration * decl = buildUnionDeclaration(SgName(name));
    ROSE_ASSERT(decl != NULL);
    attr->node = decl;
    return decl;
}

SgVariableDeclaration * DwarfROSE::convertMember(SgAsmDwarfMember * m) {
    if(m == NULL) {
        return NULL;
    }

    OffsetAttribute * attr = OffsetAttribute::get(m);
    ROSE_ASSERT(attr != NULL);
    std::string name = m->get_name();
    SgType * type = typeFromAttribute(attr);
    SgVariableDeclaration * decl = SageBuilder::buildVariableDeclaration(SgName(name), type);
    attr->node = decl;
    return decl;

}
