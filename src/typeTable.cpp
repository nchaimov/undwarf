#include "typeTable.h"
#include "rose.h"
#include <string>

SgType * TypeTable::createType(VariantT t) {
    switch(t) {  
        case V_SgTypeBool:
            return SageBuilder::buildBoolType();
        case V_SgTypeChar:
            return SageBuilder::buildCharType();
        case V_SgTypeDouble:
            return SageBuilder::buildDoubleType();
        case V_SgTypeFloat:
            return SageBuilder::buildFloatType();
        case V_SgTypeInt:
            return SageBuilder::buildIntType();
        case V_SgTypeLong:
            return SageBuilder::buildLongType();
        case V_SgTypeLongDouble:
            return SageBuilder::buildLongDoubleType();
        case V_SgTypeLongLong:
            return SageBuilder::buildLongLongType();
        case V_SgTypeShort:
            return SageBuilder::buildShortType();
        case V_SgTypeVoid:
            return SageBuilder::buildVoidType();
        case V_SgTypeWchar:
            return SageBuilder::buildWcharType();
        case V_SgTypeSignedChar:
            return SageBuilder::buildSignedCharType();
        case V_SgTypeSignedInt:
            return SageBuilder::buildSignedIntType();
        case V_SgTypeSignedLong:
            return SageBuilder::buildSignedLongType();
        case V_SgTypeSignedLongLong:
            return SageBuilder::buildSignedLongLongType();
        case V_SgTypeSignedShort:
            return SageBuilder::buildSignedShortType();
        case V_SgTypeUnsignedChar:
            return SageBuilder::buildUnsignedCharType();
        case V_SgTypeUnsignedInt:
            return SageBuilder::buildUnsignedIntType();
        case V_SgTypeUnsignedLong:
            return SageBuilder::buildUnsignedLongType();
        case V_SgTypeUnsignedLongLong:
            return SageBuilder::buildUnsignedLongLongType();
        case V_SgTypeUnsignedShort:
            return SageBuilder::buildUnsignedShortType();
        
        default:
            return SageBuilder::buildUnknownType();
    }
}

SgType * TypeTable::createType(const std::string & name) {
    return createType(nameToType(name));
}

TypeTable::TypeTable() {
    nameMap["bool"]                       =       V_SgTypeBool;
    nameMap["char"]                       =       V_SgTypeChar;
    nameMap["double"]                     =       V_SgTypeDouble;     
    nameMap["float"]                      =       V_SgTypeFloat;
    nameMap["int"]                        =       V_SgTypeInt;
    nameMap["long"]                       =       V_SgTypeLong;
    nameMap["long int"]                   =       V_SgTypeLong;
    nameMap["long double"]                =       V_SgTypeLongDouble;
    nameMap["long long"]                  =       V_SgTypeLongLong;
    nameMap["long long int"]              =       V_SgTypeLongLong;
    nameMap["short"]                      =       V_SgTypeShort;
    nameMap["short int"]                  =       V_SgTypeShort;
    nameMap["void"]                       =       V_SgTypeVoid;
    nameMap["wchar_t"]                    =       V_SgTypeWchar;
    nameMap["signed char"]                =       V_SgTypeSignedChar;
    nameMap["signed int"]                 =       V_SgTypeSignedInt;
    nameMap["signed long"]                =       V_SgTypeSignedLong;
    nameMap["signed long int"]            =       V_SgTypeSignedLong;
    nameMap["signed long long"]           =       V_SgTypeSignedLongLong;
    nameMap["signed long long int"]       =       V_SgTypeSignedLongLong;
    nameMap["signed short"]               =       V_SgTypeSignedShort;
    nameMap["signed short int"]           =       V_SgTypeSignedShort;
    nameMap["unsigned char"]              =       V_SgTypeUnsignedChar;
    nameMap["unsigned int"]               =       V_SgTypeUnsignedInt;
    nameMap["unsigned long"]              =       V_SgTypeUnsignedLong;
    nameMap["unsigned long int"]          =       V_SgTypeUnsignedLong;
    nameMap["unsigned long long"]         =       V_SgTypeUnsignedLongLong;
    nameMap["unsigned long long int"]     =       V_SgTypeUnsignedLongLong;
    nameMap["unsigned short"]             =       V_SgTypeUnsignedShort;
    nameMap["unsigned short int"]         =       V_SgTypeUnsignedShort;

    typeMap[V_SgTypeBool] = "bool";
    typeMap[V_SgTypeChar] = "char";
    typeMap[V_SgTypeDouble] = "double";     
    typeMap[V_SgTypeFloat] = "float";
    typeMap[V_SgTypeInt] = "int";
    typeMap[V_SgTypeLong] = "long";
    typeMap[V_SgTypeLong] = "long int";
    typeMap[V_SgTypeLongDouble] = "long double";
    typeMap[V_SgTypeLongLong] = "long long";
    typeMap[V_SgTypeLongLong] = "long long int";
    typeMap[V_SgTypeShort] = "short";
    typeMap[V_SgTypeShort] = "short int";
    typeMap[V_SgTypeVoid] = "void";
    typeMap[V_SgTypeWchar] = "wchar_t";
    typeMap[V_SgTypeSignedChar] = "signed char";
    typeMap[V_SgTypeSignedInt] = "signed int";
    typeMap[V_SgTypeSignedLong] = "signed long";
    typeMap[V_SgTypeSignedLong] = "signed long int";
    typeMap[V_SgTypeSignedLongLong] = "signed long long";
    typeMap[V_SgTypeSignedLongLong] = "signed long long int";
    typeMap[V_SgTypeSignedShort] = "signed short";
    typeMap[V_SgTypeSignedShort] = "signed short int";
    typeMap[V_SgTypeUnsignedChar] = "unsigned char";
    typeMap[V_SgTypeUnsignedInt] = "unsigned int";
    typeMap[V_SgTypeUnsignedLong] = "unsigned long";
    typeMap[V_SgTypeUnsignedLong] = "unsigned long int";
    typeMap[V_SgTypeUnsignedLongLong] = "unsigned long long";
    typeMap[V_SgTypeUnsignedLongLong] = "unsigned long long int";
    typeMap[V_SgTypeUnsignedShort] = "unsigned short";
    typeMap[V_SgTypeUnsignedShort] = "unsigned short int";
}
