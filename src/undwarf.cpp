#include "rose.h"

#include <cstdio>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <utility>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "typeTable.h"
#include "DwarfROSEConverter.h"
#include "attributes.h"
    
static TypeTable & typeTable = TypeTable::getInstance();

offsetMapType & constructOffsetMap(SgNode * top) {
    offsetMapType * offsetMap = new offsetMapType();

    Rose_STL_Container<SgNode*> constructs = NodeQuery::querySubTree(top, V_SgAsmDwarfConstruct);
    BOOST_FOREACH(SgNode * n, constructs) {
        SgAsmDwarfConstruct * construct = isSgAsmDwarfConstruct(n);
        ROSE_ASSERT(construct != NULL);
        std::string offsetStr = "<" + boost::lexical_cast<std::string>(construct->get_offset()) + ">";
        std::string name = construct->get_name();
        if(name.empty()) {
            name = "<unnamed>";
        }
#ifdef DEBUG
        std::cerr << construct->class_name() << " " << name << " is " << offsetStr << std::endl;
#endif
        (*offsetMap)[offsetStr] = construct;
    }          

    return *offsetMap;
}



void annotateDwarfConstructs(SgNode * top, offsetMapType & map) {
    Rose_STL_Container<SgNode*> constructs = NodeQuery::querySubTree(top, V_SgAsmDwarfConstruct);
    BOOST_FOREACH(SgNode * n, constructs) {
        SgAsmDwarfConstruct * construct = isSgAsmDwarfConstruct(n);
        if(map.count(construct->get_type_ref()) > 0) {
            OffsetAttribute * attr = new OffsetAttribute(map[construct->get_type_ref()]);
            attr->add(construct);
        }
#ifdef DEBUG
        else {
            std::cerr << "Skipping annotation of " << construct->class_name() << " \"" <<  construct->get_name() << "\" because it has no entry in the offset map." << std::endl;
        } 
#endif
    }
}

std::string typeToName(SgAsmDwarfConstruct * d) {
    std::stringstream s;

    switch(d->variantT()) {
        case V_SgAsmDwarfBaseType: {
            s << d->get_name();                           
        };
        break;

        case V_SgAsmDwarfPointerType: {
            OffsetAttribute * attr = OffsetAttribute::get(d);
            if(attr != NULL) {
                SgAsmDwarfConstruct * type = attr->type;
                if(type != NULL) { 
                    s << typeToName(type) << "*";
                } else {
                    std::cerr << "Pointer type " << d->get_name() << " had no type in its offset attribute." << std::endl;
                }
            } else {
                std::cerr << "Pointer type " << d->get_name() << " had no attached offset attribute." << std::endl;
            }
        };
        break;
    
        default:
            std::cerr << "Unhandled type " << d->class_name() << std::endl;
    }
    return s.str();
}

int main ( int argc, char* argv[] ) {
	
	// Parses the input files and generates the AST
	SgProject* project = frontend(argc,argv);
	ROSE_ASSERT (project != NULL);

    // Make sure we have a valid AST 
    AstTests::runAllTests(project);

    offsetMapType & offsets = constructOffsetMap(project);
    annotateDwarfConstructs(project, offsets);

    SgSourceFile * newFile = new SgSourceFile();
    newFile->set_parent(project);
    newFile->set_Cxx_only(true);
    SgGlobal * global = new SgGlobal();
    newFile->set_globalScope(global);
    global->set_parent(newFile);
    SageInterface::setSourcePositionForTransformation(newFile);
    newFile->set_startOfConstruct(Sg_File_Info::generateDefaultFileInfoForTransformationNode());

    Rose_STL_Container<SgNode*> subprograms = NodeQuery::querySubTree(project, V_SgAsmDwarfSubprogram);
    BOOST_FOREACH(SgNode * n, subprograms) {
       SgFunctionDeclaration * decl = DwarfROSE::convertSubprogram(isSgAsmDwarfSubprogram(n));
       SageInterface::fixStatement(decl, global);
       SageInterface::insertStatementAfterLastDeclaration(decl, global);
    }

    
    AstTests::runAllTests(project);

    std::cout << global->unparseToCompleteString() << std::endl;

    delete global;

    return 0;
}                                  
