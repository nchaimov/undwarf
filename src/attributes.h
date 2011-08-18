#ifndef __ATTRIBUTES_H__
#define __ATTRIBUTES_H__

#include "rose.h"


class OffsetAttribute : public AstAttribute {
    public:
        SgAsmDwarfConstruct * type;
        SgNode * node;

        OffsetAttribute(SgAsmDwarfConstruct * t = NULL,
                SgNode * n = NULL) : type(t), node(n) {};
        
        static const std::string OFFSET_ATTRIBUTE;

        static inline OffsetAttribute * get(SgNode * n) {
           AstAttribute * attr = n->getAttribute(OFFSET_ATTRIBUTE);        
           return dynamic_cast<OffsetAttribute*>(attr);
        };

        inline void add(SgNode * n) {
            n->addNewAttribute(OFFSET_ATTRIBUTE, this);
        }
};

typedef std::map<std::string, SgAsmDwarfConstruct*> offsetMapType;

class InheritedAttribute {
    public:
        SgScopeStatement * parentScope;

        InheritedAttribute(SgScopeStatement * p = NULL) : parentScope(p) {};
};                                                                     


#endif
