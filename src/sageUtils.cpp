#include "sageUtils.h"
#include "rose.h"
#include "boost/algorithm/string/erase.hpp"

SgFunctionParameterList * SageUtils::buildEmptyParameterList() {
    SgFunctionParameterList *parameterList = new SgFunctionParameterList();
    ROSE_ASSERT (parameterList);
    parameterList->set_definingDeclaration (NULL);
    parameterList->set_firstNondefiningDeclaration(parameterList);
    SageInterface::setOneSourcePositionForTransformation(parameterList);
    return parameterList;
}

SgEnumType * SageUtils::buildEnumType(SgEnumDeclaration * d) {
    SgEnumType * type = new SgEnumType();
    type->set_declaration(d);
    return type;
}

SgTypedefType * SageUtils::buildTypedefType(SgTypedefDeclaration * d) {
    SgTypedefType * type = new SgTypedefType();
    type->set_declaration(d);
    return type;
}

SgClassType * SageUtils::buildClassType(SgClassDeclaration * d) {
    SgClassType * type = new SgClassType();
    type->set_declaration(d);
    return type;
}

SgClassDeclaration * SageUtils::buildUnionDeclaration(const SgName & name, SgScopeStatement * scope) {
    SgClassDeclaration* defdecl = SageBuilder::buildClassDeclaration_nfi(name,SgClassDeclaration::e_union,scope,NULL);
    SageInterface::setOneSourcePositionForTransformation(defdecl);
    ROSE_ASSERT(defdecl->get_firstNondefiningDeclaration() != NULL);
    SageInterface::setOneSourcePositionForTransformation(defdecl->get_firstNondefiningDeclaration());
    return defdecl; 
}

bool SageUtils::operatorIsCast(std::string name) {
    boost::erase_all(name, " ");
    if(    name == "operator,"
        || name == "operator="
        || name == "operator+="
        || name == "operator-="
        || name == "operator&="
        || name == "operator|="
        || name == "operator*="
        || name == "operator/="
        || name == "operator%="
        || name == "operator^="
        || name == "operator<<="
        || name == "operator>>="
        || name == "operator||"
        || name == "operator&&"
        || name == "operator|"
        || name == "operator^"
        || name == "operator&"
        || name == "operator=="
        || name == "operator!="
        || name == "operator<"
        || name == "operator>"
        || name == "operator<="
        || name == "opreator>="
        || name == "operator<<"
        || name == "operator>>"
        || name == "operator+"
        || name == "operator-"
        || name == "operator*"
        || name == "operator/"
        || name == "operator%"
        || name == "operator.*"
        || name == "operator->*"
        || name == "operator++"
        || name == "operator--"
        || name == "operator~"
        || name == "operator!"
        || name == "operator[]"
        || name == "operator->"
        || name == "operator." ) {
            return false;
        } else {
            return true;
        }
}
