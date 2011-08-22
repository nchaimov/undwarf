#ifndef __TYPETABLE_H__
#define __TYPETABLE_H__

#include "rose.h"
#include <string>

class TypeTable {

    public:
        static TypeTable & getInstance() {
            static TypeTable instance;
            return instance;
        }

        std::string typeToName(VariantT t) ;

        VariantT nameToType(const std::string & name) ;

        SgType * createType(VariantT v);
        SgType * createType(const std::string & name);
        

    private:
        std::map<std::string, VariantT> nameMap;
        std::map<VariantT, std::string> typeMap;
        TypeTable();
        TypeTable(TypeTable const &);
        void operator=(TypeTable const &);

};

#endif
