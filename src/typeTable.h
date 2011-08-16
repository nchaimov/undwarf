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

        inline std::string typeToName(VariantT t) {
            if(typeMap.count(t) > 0) {
                return typeMap[t];
            } else {
                return "V_SgTypeUnknown";
            }
        }


        inline VariantT nameToType(const std::string & name) {
            if(nameMap.count(name) > 0) {
                return nameMap[name];
            } else {
                return V_SgTypeUnknown;
            }
        }


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
