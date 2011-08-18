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
        OffsetAttribute * attr = new OffsetAttribute();
        attr->add(construct);
        if(map.count(construct->get_type_ref()) > 0) {
            attr->type = map[construct->get_type_ref()];
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

SgSourceFile * newFileInProject(SgProject * project) {
    SgSourceFile * newFile = new SgSourceFile();
    newFile->set_parent(project);
    newFile->set_Cxx_only(true);
    SgGlobal * global = new SgGlobal();
    newFile->set_globalScope(global);
    global->set_parent(newFile);
    SageInterface::setSourcePositionForTransformation(newFile);
    newFile->set_startOfConstruct(Sg_File_Info::generateDefaultFileInfoForTransformationNode());
    newFile->get_startOfConstruct()->set_parent(newFile);
    return newFile;
}

class UndwarfTraversal : public AstTopDownProcessing<InheritedAttribute> {
    private:
        SgGlobal * global;

    public:
        virtual InheritedAttribute evaluateInheritedAttribute(SgNode * n, InheritedAttribute a);
        UndwarfTraversal(SgGlobal * g) : global(g) {};

};

void fixEnumDeclaration(SgEnumDeclaration * e, SgScopeStatement * scope) {
    SgInitializedNamePtrList & initNames = e->get_enumerators();
    BOOST_FOREACH(SgInitializedName * initName, initNames) {
        initName->set_scope(scope);
    }
    e->set_parent(scope);
    e->set_scope(scope);
}

InheritedAttribute UndwarfTraversal::evaluateInheritedAttribute(SgNode * n, InheritedAttribute a) {
    SgNode * parentSubprogram = a.parentSubprogram;

    SgDeclarationStatement * newDecl = NULL;
    SgScopeStatement * scope = global;

    switch(n->variantT()) {
        case V_SgAsmDwarfEnumerationType: {
            SgEnumDeclaration * enumDecl = DwarfROSE::convertEnum(isSgAsmDwarfEnumerationType(n));
            fixEnumDeclaration(enumDecl, scope);
            newDecl = enumDecl;
            break;
        };

        case V_SgAsmDwarfSubprogram: {
            SgFunctionDeclaration * funcDecl = DwarfROSE::convertSubprogram(isSgAsmDwarfSubprogram(n));
            parentSubprogram = n;
            newDecl = funcDecl;
            break;
        };

        case V_SgAsmDwarfTypedef: {
            SgTypedefDeclaration * typedefDecl = DwarfROSE::convertTypedef(isSgAsmDwarfTypedef(n), scope);
            newDecl = typedefDecl;
            break;
        };

        default: ; // Do nothing
    }

    if(newDecl != NULL)  {
        SageInterface::fixStatement(newDecl, scope);
        SageInterface::insertStatementAfterLastDeclaration(newDecl, scope);
    }

    return InheritedAttribute(parentSubprogram);
}

int main ( int argc, char* argv[] ) {
	
	// Parses the input files and generates the AST
	SgProject* project = frontend(argc,argv);
	ROSE_ASSERT (project != NULL);

    // Make sure we have a valid AST 
    AstTests::runAllTests(project);

    offsetMapType & offsets = constructOffsetMap(project);
    annotateDwarfConstructs(project, offsets);

    SgSourceFile * newFile = newFileInProject(project);
    SgGlobal * global = newFile->get_globalScope();

    InheritedAttribute attr(NULL);
    UndwarfTraversal traversal(global);

    Rose_STL_Container<SgNode*> interps = NodeQuery::querySubTree(project, V_SgAsmInterpretationList);
    BOOST_FOREACH(SgNode * n, interps) {
        traversal.traverse(n, attr);
    }

    std::cout << global->unparseToCompleteString() << std::endl;

    delete global;

    return 0;
}                                  
