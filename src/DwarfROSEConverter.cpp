#include "DwarfROSEConverter.h"
#include "rose.h"
#include "typeTable.h"

SgType * DwarfROSE::dwarfTypeToSgType(SgAsmDwarfConstruct * c) {
    if(c == NULL) {
        return NULL;
    }

    switch(c->variantT()) {
        case V_SgAsmDwarfBaseType:
            return TypeTable::getInstance().createType(c->get_name());

        default: 
            std::cerr << "Unhandled type " << c->class_name() << std::endl;
    }    

    return NULL;
}
