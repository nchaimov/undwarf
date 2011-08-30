#include <stdint.h>
#include "DwarfROSEConverter.h"
#include "rose.h"
#include "typeTable.h"
#include "attributes.h"
#include "sageUtils.h"
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>

size_t DwarfROSE::unnamed_count = 0;

SgType * DwarfROSE::typeFromAttribute(OffsetAttribute * attr, SgScopeStatement * scope) {
    if(attr == NULL) {
        return SageBuilder::buildVoidType();
    } else {
        return DwarfROSE::convertType(attr->type, scope);
    }
}

SgType * DwarfROSE::convertType(SgAsmDwarfConstruct * c, SgScopeStatement * scope) {
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
                pointedTo = convertType(attr->type, scope);
            }
            return SageBuilder::buildPointerType(pointedTo);
        };

        // ENUMS
        case V_SgAsmDwarfEnumerationType: {
            OffsetAttribute * attr = OffsetAttribute::get(c);
            ROSE_ASSERT(attr != NULL);
            SgEnumDeclaration * enumDecl = isSgEnumDeclaration(attr->node);
            if(enumDecl == NULL) {
                enumDecl = convertEnum(isSgAsmDwarfEnumerationType(c), scope);
                ROSE_ASSERT(enumDecl != NULL);
                SageInterface::fixStatement(enumDecl, scope);
                SageInterface::insertStatementAfterLastDeclaration(enumDecl, scope);
            }
            return SageUtils::buildEnumType(enumDecl);
        };

        // TYPEDEFS
        case V_SgAsmDwarfTypedef: {
            OffsetAttribute * attr = OffsetAttribute::get(c);
            ROSE_ASSERT(attr != NULL);
            SgTypedefDeclaration * decl = isSgTypedefDeclaration(attr->node);
            if(decl == NULL) {
                if(SageInterface::getProject()->get_verbose() > 0) {
                    std::cerr << "No existing decl for typedef; will generate one." << std::endl;
                }
                SgStatement * parent = isSgStatement(scope->get_parent());
                SgScopeStatement * parentScope = parent == NULL ? scope : SageInterface::getScope(parent);
                decl = convertTypedef(isSgAsmDwarfTypedef(c), parentScope);
                decl->set_forward(true);
                SageInterface::fixStatement(decl, parentScope);
                if(parent == NULL) {
                    SageInterface::prependStatement(decl, parentScope);
                } else {
                    SageInterface::insertStatementBefore(parent, decl, parentScope);
                }
                attr->node = decl;
                return SageUtils::buildTypedefType(decl);
            } else {
                return SageUtils::buildTypedefType(decl);
            }
        };
        
        // CONST
        case V_SgAsmDwarfConstType: {
            OffsetAttribute * attr = OffsetAttribute::get(c);
            ROSE_ASSERT(attr != NULL);
            SgType * baseType = typeFromAttribute(attr, scope);
            return SageBuilder::buildConstType(baseType);
        };

        // VOLATILE
        case V_SgAsmDwarfVolatileType: {
            OffsetAttribute * attr = OffsetAttribute::get(c);
            ROSE_ASSERT(attr != NULL);
            SgType * baseType = typeFromAttribute(attr, scope);
            return SageBuilder::buildVolatileType(baseType);
        };

        // STRUCTS, UNIONS
        case V_SgAsmDwarfClassType:
        case V_SgAsmDwarfStructureType:
        case V_SgAsmDwarfUnionType: {
            OffsetAttribute * attr = OffsetAttribute::get(c);
            ROSE_ASSERT(attr != NULL);
            SgClassDeclaration * decl = isSgClassDeclaration(attr->node);
            if(decl == NULL) {
                if(SageInterface::getProject()->get_verbose() > 0) {
                    std::cerr << "Class/struct/union had no associated declaration." << std::endl;
                    std::cerr << "Node was: " << c->class_name() << " " << c << " " << c->get_name() << std::endl;
                    std::cerr << "Will build forward declaration." << std::endl;
                }
                SgStatement * parent = isSgStatement(scope->get_parent());
                SgScopeStatement * parentScope = parent == NULL ? scope : SageInterface::getScope(parent);
                SgClassDeclaration::class_types classType = SgClassDeclaration::e_class;
                SgClassDeclaration * newDecl = NULL;
                std::string name = c->get_name();
                if(c->variantT() == V_SgAsmDwarfStructureType) {
                    classType = SgClassDeclaration::e_struct;    
                    if(name.empty()) {
                        name = "_UNNAMED_STRUCT_" + boost::lexical_cast<std::string>(unnamed_count++);
                    }
                    newDecl = SageBuilder::buildStructDeclaration(name);
                    if(isSgAsmDwarfStructureType(c)->get_body() == NULL) {
                        newDecl->set_forward(true);
                    }
                } else if(c->variantT() == V_SgAsmDwarfUnionType) {
                    classType = SgClassDeclaration::e_union;
                    if(name.empty()) {
                        name = "_UNNAMED_UNION_" + boost::lexical_cast<std::string>(unnamed_count++);
                    }
                    newDecl = SageUtils::buildUnionDeclaration(name);
                    if(isSgAsmDwarfUnionType(c)->get_body() == NULL) {
                        newDecl->set_forward(true);
                    }
                } else if(name.empty()) {
                    name = "_UNNAMED_CLASS_" + boost::lexical_cast<std::string>(unnamed_count++);
                    newDecl = SageBuilder::buildClassDeclaration(name, parentScope);
                }
                SgClassDeclaration * forwardDecl = SageBuilder::buildNondefiningClassDeclaration_nfi(SgName(name),
                        classType, parentScope);
                SageInterface::setOneSourcePositionForTransformation(forwardDecl);
                SageInterface::fixStatement(forwardDecl, parentScope);
                SageInterface::setOneSourcePositionForTransformation(newDecl);
                SageInterface::fixStatement(newDecl, parentScope);
                if(parent == NULL) {
                    SageInterface::prependStatement(newDecl, parentScope);
                } else {
                    SageInterface::insertStatementBefore(parent, newDecl, parentScope);
                }
                attr->node = newDecl;


                SageInterface::prependStatement(forwardDecl, parentScope);
                return SageUtils::buildClassType(forwardDecl);
            } else {
                return SageUtils::buildClassType(decl);
            }
        };

        // ARRAYS
        case V_SgAsmDwarfArrayType: {
            SgAsmDwarfArrayType * arrayType = isSgAsmDwarfArrayType(c);
            OffsetAttribute * attr = OffsetAttribute::get(c);
            ROSE_ASSERT(attr != NULL);
            SgType * baseType = typeFromAttribute(attr, scope);
            Rose_STL_Container<SgNode*> subranges = NodeQuery::querySubTree(arrayType, V_SgAsmDwarfSubrangeType);
            SgAsmDwarfSubrangeType * subrange = isSgAsmDwarfSubrangeType(subranges[0]);
            ROSE_ASSERT(subrange != NULL);
            uint64_t upper = subrange->get_upper_bound() + 1;
            SgExpression * sizeExpr = SageBuilder::buildUnsignedLongLongIntVal(upper);
            ROSE_ASSERT(sizeExpr != NULL);
            return SageBuilder::buildArrayType(baseType, sizeExpr); 
        };

        // FUNCTION POINTERS
        case V_SgAsmDwarfSubroutineType: {
            std::string name = c->get_name();
            OffsetAttribute * attr = OffsetAttribute::get(c);
            SgType * retType = typeFromAttribute(attr, scope);

            SgFunctionParameterList * paramList = SageUtils::buildEmptyParameterList();

            Rose_STL_Container<SgNode*> formalParams = NodeQuery::querySubTree(c, V_SgAsmDwarfFormalParameter);
            BOOST_FOREACH(SgNode * n, formalParams) {
                SgAsmDwarfFormalParameter * formalParam = isSgAsmDwarfFormalParameter(n);
                std::string paramName = formalParam->get_name();
                OffsetAttribute * paramAttr = OffsetAttribute::get(formalParam);
                SgType * paramType = typeFromAttribute(paramAttr, scope);
                SgInitializedName * initName = SageBuilder::buildInitializedName(paramName, paramType);
                SageInterface::appendArg(paramList, initName);
            }

            SgType * t = SageBuilder::buildFunctionType(retType, paramList);
            if(name.empty()) {
                name = "_FUNCTION_POINTER_" + boost::lexical_cast<std::string>(unnamed_count++);
            }
            SgFunctionDeclaration * decl = SageBuilder::buildNondefiningFunctionDeclaration(SgName(name), retType, paramList);
            paramList->set_parent(decl);
            decl->set_parent(t);
            decl->set_scope(scope);
            return t;
        };

        // UPC RELAXED
        case V_SgAsmDwarfUpcRelaxedType: {
            OffsetAttribute * attr = OffsetAttribute::get(c);
            ROSE_ASSERT(attr != NULL);
            SgType * baseType = typeFromAttribute(attr, scope);
            return SageBuilder::buildUpcRelaxedType(baseType);
        };
        
        // UPC STRICT
        case V_SgAsmDwarfUpcStrictType: {
            OffsetAttribute * attr = OffsetAttribute::get(c);
            ROSE_ASSERT(attr != NULL);
            SgType * baseType = typeFromAttribute(attr, scope);
            return SageBuilder::buildUpcStrictType(baseType);
        };

        // UPC SHARED
        case V_SgAsmDwarfUpcSharedType: {
            OffsetAttribute * attr = OffsetAttribute::get(c);
            ROSE_ASSERT(attr != NULL);
            SgType * baseType = typeFromAttribute(attr, scope);
            return SageBuilder::buildUpcSharedType(baseType);
        };

        default: 
            std::cerr << "Unhandled type " << c->class_name() << " " << c->get_name() << std::endl;
    }    

    return SageBuilder::buildUnknownType();
}


SgFunctionDeclaration * DwarfROSE::convertSubprogram(SgAsmDwarfSubprogram * s, SgScopeStatement * scope) {
    if(s == NULL) {
        return NULL;
    }
    
    std::string name = s->get_name();
    if(name.empty()) {
        //name = "_UNNAMED_FUNCTION_" + boost::lexical_cast<std::string>(unnamed_count++) + "_";
        return NULL;
    }
    OffsetAttribute * attr = OffsetAttribute::get(s);
    SgType * retType = typeFromAttribute(attr, scope);

    SgFunctionParameterList * paramList = SageUtils::buildEmptyParameterList();

    // subroutines can contain inlined subroutines; we don't want to find their formal parameters
    //Rose_STL_Container<SgNode*> formalParams = NodeQuery::querySubTree(s, V_SgAsmDwarfFormalParameter);
    SgAsmDwarfConstructList * cList = s->get_body();
    if(cList != NULL) {
        const SgAsmDwarfConstructPtrList & children = cList->get_list();
        BOOST_FOREACH(SgAsmDwarfConstruct * n, children) {
            SgAsmDwarfFormalParameter * formalParam = isSgAsmDwarfFormalParameter(n);
            if(formalParam != NULL && !formalParam->get_artificiality()) {
                std::string paramName = formalParam->get_name();
                OffsetAttribute * paramAttr = OffsetAttribute::get(formalParam);
                SgType * paramType = typeFromAttribute(paramAttr, scope);
                SgInitializedName * initName = SageBuilder::buildInitializedName(paramName, paramType);
                SageInterface::appendArg(paramList, initName);
            }
        }
    }

    SgFunctionDeclaration * decl = SageBuilder::buildNondefiningFunctionDeclaration(SgName(name), retType, paramList);
    switch(s->get_accessibility()) {
        case 2:
            decl->get_declarationModifier().get_accessModifier().setProtected();
            break;
        case 3:
            decl->get_declarationModifier().get_accessModifier().setPrivate();
            break;
        default:
            decl->get_declarationModifier().get_accessModifier().setPublic();
    }

    return decl;
}

SgEnumDeclaration * DwarfROSE::convertEnum(SgAsmDwarfEnumerationType * e, SgScopeStatement * scope) {
    if(e == NULL) {
        return NULL;
    }
    std::string name = e->get_name();
    if(name.empty()) {
        name = "_UNNAMED_ENUM_" + boost::lexical_cast<std::string>(unnamed_count++) + "_";
    }
    SgEnumDeclaration * decl = SageBuilder::buildEnumDeclaration(SgName(name), scope);
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

SgTypedefDeclaration * DwarfROSE::convertTypedef(SgAsmDwarfTypedef * t, SgScopeStatement * scope) {
    if(t == NULL) {
        return NULL;
    }               
    OffsetAttribute * attr = OffsetAttribute::get(t);
    ROSE_ASSERT(attr != NULL);
    SgType * baseType = typeFromAttribute(attr, scope);
    std::string name = t->get_name();
    // ROSE insists that this have a scope when built; I don't know why.
    SgTypedefDeclaration * decl = SageBuilder::buildTypedefDeclaration(SgName(name), baseType, scope);
    attr->node = decl;
    return decl;
}

SgClassDeclaration * DwarfROSE::convertStruct(SgAsmDwarfStructureType * s, SgScopeStatement * scope) {
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
    if(s->get_body() == NULL || s->get_body()->get_list().empty()) {
        decl->set_forward(true);
    }
    return decl;
}

SgClassDeclaration * DwarfROSE::convertUnion(SgAsmDwarfUnionType * s, SgScopeStatement * scope) {
    if(s == NULL) {
        return NULL;
    }

    OffsetAttribute * attr = OffsetAttribute::get(s);
    ROSE_ASSERT(attr != NULL);
    std::string name = s->get_name();
    if(name.empty()) {
        name = "_UNNAMED_UNION_" + boost::lexical_cast<std::string>(unnamed_count++) + "_";
    }
    SgClassDeclaration * decl = SageUtils::buildUnionDeclaration(SgName(name));
    ROSE_ASSERT(decl != NULL);
    attr->node = decl;
    if(s->get_body() == NULL) {
        decl->set_forward(true);
    }
    return decl;
}

SgVariableDeclaration * DwarfROSE::convertMember(SgAsmDwarfMember * m, SgScopeStatement * scope) {
    if(m == NULL) {
        return NULL;
    }

    OffsetAttribute * attr = OffsetAttribute::get(m);
    ROSE_ASSERT(attr != NULL);
    std::string name = m->get_name();
    SgType * type = typeFromAttribute(attr, scope);
    SgVariableDeclaration * decl = SageBuilder::buildVariableDeclaration(SgName(name), type);
    uint64_t bitfield = m->get_bit_size();
    if(bitfield > 0) {
        decl->set_bitfield(SageBuilder::buildUnsignedLongVal(bitfield));
    }
    switch(m->get_accessibility()) {
        case 2:
            decl->get_declarationModifier().get_accessModifier().setProtected();
            break;
        case 3:
            decl->get_declarationModifier().get_accessModifier().setPrivate();
            break;
        default:
            decl->get_declarationModifier().get_accessModifier().setPublic();
    }
    attr->node = decl;
    return decl;

}

SgClassDeclaration * DwarfROSE::convertClass(SgAsmDwarfClassType * s, SgScopeStatement * scope) {
    if(s == NULL) {
        return NULL;
    }

    OffsetAttribute * attr = OffsetAttribute::get(s);
    ROSE_ASSERT(attr != NULL);
    SgClassDeclaration * decl = NULL;
    if(attr->spec != NULL) {
        OffsetAttribute * specAttr = OffsetAttribute::get(attr->spec);
        if(specAttr != NULL && specAttr->node != NULL && isSgClassDeclaration(specAttr->node)) {
            SgClassDeclaration * nondefDecl = isSgClassDeclaration(specAttr->node);
            std::string qName = nondefDecl->get_qualified_name().getString();
            if(boost::starts_with(qName, "::")) {
                qName = qName.substr(2, std::string::npos);
            } 
            decl = SageBuilder::buildClassDeclaration_nfi(qName, SgClassDeclaration::e_class, scope, nondefDecl);
        } else {
            std::cerr << "WARNING: Specification offset entry found but couldn't find specification." << std::endl;
        }
    } else {
        std::string name = s->get_name();
        if(name.empty()) {
            name = "_UNNAMED_CLASS_" + boost::lexical_cast<std::string>(unnamed_count++) + "_";
        }
        decl = SageBuilder::buildClassDeclaration(SgName(name), scope);
        ROSE_ASSERT(decl != NULL);
        attr->node = decl;
        if(s->get_body() == NULL || s->get_body()->get_list().empty()) {
            decl->set_forward(true);
        }
        SageBuilder::buildClassDefinition(decl);
    }
    return decl;
}

SgNamespaceDeclarationStatement * DwarfROSE::convertNamespace(SgAsmDwarfNamespace * s, SgScopeStatement * scope) {
    if(s == NULL) {
        return NULL;
    }

    OffsetAttribute * attr = OffsetAttribute::get(s);
    ROSE_ASSERT(attr != NULL);
    std::string name = s->get_name();
    bool unnamed = false;
    if(name.empty()) {
        name = "_UNNAMED_NAMESPACE_" + boost::lexical_cast<std::string>(unnamed_count++) + "_";
        unnamed = true;
    }
    SgNamespaceDeclarationStatement * decl = SageBuilder::buildNamespaceDeclaration_nfi(SgName(name), unnamed, scope);
    ROSE_ASSERT(decl != NULL);                                                                                        
    SageInterface::setOneSourcePositionForTransformation(decl->get_firstNondefiningDeclaration());
    attr->node = decl;
    if(s->get_body() == NULL || s->get_body()->get_list().empty()) {
        decl->set_forward(true);
    }
    SageBuilder::buildNamespaceDefinition(decl);
    return decl;
}
