#include "DwarfROSEConverter.h"
#include "rose.h"
#include "typeTable.h"
#include "attributes.h"
#include <boost/foreach.hpp>

SgType * DwarfROSE::typeFromAttribute(OffsetAttribute * attr) {
    if(attr == NULL) {
        return SageBuilder::buildVoidType();
    } else {
        return DwarfROSE::convertType(attr->type);
    }
}

SgType * DwarfROSE::convertType(SgAsmDwarfConstruct * c) {
    if(c == NULL) {
        return SageBuilder::buildVoidType();
    }

    switch(c->variantT()) {
        case V_SgAsmDwarfBaseType:
            return TypeTable::getInstance().createType(c->get_name());

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

        default: 
            std::cerr << "Unhandled type " << c->class_name() << std::endl;
    }    

    return NULL;
}

SgFunctionParameterList * buildEmptyParameterList() {
    SgFunctionParameterList *parameterList = new SgFunctionParameterList();
    ROSE_ASSERT (parameterList);
    parameterList->set_definingDeclaration (NULL);
    parameterList->set_firstNondefiningDeclaration (parameterList);
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
