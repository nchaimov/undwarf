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

static offsetMapType & constructOffsetMap(SgNode * top) {
    offsetMapType * offsetMap = new offsetMapType();

    Rose_STL_Container<SgNode*> constructs = NodeQuery::querySubTree(top, V_SgAsmDwarfConstruct);
    BOOST_FOREACH(SgNode * n, constructs) {
        SgAsmDwarfConstruct * construct = isSgAsmDwarfConstruct(n);
        ROSE_ASSERT(construct != NULL);
        std::string offsetStr = "<" + boost::lexical_cast<std::string>(construct->get_offset()) + ">";
        if(SageInterface::getProject()->get_verbose() > 0) {
            std::string name = construct->get_name();
            if(name.empty()) {
                name = "<unnamed>";
            }
            std::cerr << construct->class_name() << " " << name << " is " << offsetStr << std::endl;
        }
        (*offsetMap)[offsetStr] = construct;
    }          

    return *offsetMap;
}

static void annotateDwarfConstructs(SgNode * top, offsetMapType & map) {
    Rose_STL_Container<SgNode*> constructs = NodeQuery::querySubTree(top, V_SgAsmDwarfConstruct);
    BOOST_FOREACH(SgNode * n, constructs) {
        SgAsmDwarfConstruct * construct = isSgAsmDwarfConstruct(n);
        OffsetAttribute * attr = new OffsetAttribute();
        attr->add(construct);
        if(map.count(construct->get_type_ref()) > 0) {
            attr->type = map[construct->get_type_ref()];
        } else if(SageInterface::getProject()->get_verbose() > 0 ) {
            std::cerr << "Skipping annotation of " << construct->class_name() << " \"" <<  construct->get_name() << "\" because it has no entry in the offset map." << std::endl;
        } 

        if(map.count(construct->get_spec_ref()) > 0) {
            attr->spec = map[construct->get_spec_ref()];
        }   
    }
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

static void fixEnumDeclaration(SgEnumDeclaration * e, SgScopeStatement * scope) {
    SgInitializedNamePtrList & initNames = e->get_enumerators();
    BOOST_FOREACH(SgInitializedName * initName, initNames) {
        initName->set_scope(scope);
    }
    e->set_parent(scope);
    e->set_scope(scope);
    SgEnumSymbol * sym = new SgEnumSymbol(e);
    scope->insert_symbol(e->get_name(), sym);
    
}                                   

InheritedAttribute UndwarfTraversal::evaluateInheritedAttribute(SgNode * n, InheritedAttribute a) {
    SgScopeStatement * parentScope = a.parentScope;

    SgDeclarationStatement * newDecl = NULL;
    SgScopeStatement * scope = (parentScope == NULL) ? global : parentScope;

    if(SageInterface::getProject()->get_verbose() > 0 && isSgAsmDwarfConstruct(n)) {
        SgAsmDwarfConstruct * dc = isSgAsmDwarfConstruct(n);
        std::cerr << "Processing " << dc->class_name() << " " << dc << " " << dc->get_name() << std::endl; 
    }
                             
    switch(n->variantT()) {
        case V_SgAsmDwarfEnumerationType: {
            SgEnumDeclaration * enumDecl = DwarfROSE::convertEnum(isSgAsmDwarfEnumerationType(n), scope);
            fixEnumDeclaration(enumDecl, scope);
            newDecl = enumDecl;
            break;
        };

        case V_SgAsmDwarfSubprogram: {
            SgAsmDwarfSubprogram * sub = isSgAsmDwarfSubprogram(n);
            // Compiler-generated functions are marked artificial
            if(!sub->get_artificiality()) {
                SgFunctionDeclaration * funcDecl = DwarfROSE::convertSubprogram(sub, scope);
                newDecl = funcDecl;
            }
            break;
        };

        case V_SgAsmDwarfTypedef: {
            SgTypedefDeclaration * typedefDecl = DwarfROSE::convertTypedef(isSgAsmDwarfTypedef(n), scope);
            newDecl = typedefDecl;
            break;
        };

        case V_SgAsmDwarfStructureType: {
            OffsetAttribute * attr = OffsetAttribute::get(n);
            if(attr != NULL && attr->node != NULL && isSgClassDeclaration(attr->node)) {
                parentScope = isSgClassDeclaration(attr->node)->get_definition();
            } else {
                SgClassDeclaration * structDecl = DwarfROSE::convertStruct(isSgAsmDwarfStructureType(n), scope);
                parentScope = structDecl->get_definition();
                newDecl = structDecl;
            }
            break;
        };

        case V_SgAsmDwarfUnionType: {
            OffsetAttribute * attr = OffsetAttribute::get(n);
            if(attr != NULL && attr->node != NULL && isSgClassDeclaration(attr->node)) {
                parentScope = isSgClassDeclaration(attr->node)->get_definition();
            } else {
                SgClassDeclaration * unionDecl = DwarfROSE::convertUnion(isSgAsmDwarfUnionType(n), scope);
                parentScope = unionDecl->get_definition();
                newDecl = unionDecl;
            }
            break;
        };

        case V_SgAsmDwarfClassType: {
            OffsetAttribute * attr = OffsetAttribute::get(n);
            if(attr != NULL && attr->node != NULL && isSgClassDeclaration(attr->node)) {
                parentScope = isSgClassDeclaration(attr->node)->get_definition();
            } else {
                SgClassDeclaration * classDecl = DwarfROSE::convertClass(isSgAsmDwarfClassType(n), scope);
                if(classDecl != NULL) {
                    parentScope = classDecl->get_definition();
                    newDecl = classDecl;
                }
            }
            break;
        };                       


        case V_SgAsmDwarfNamespace: {
            OffsetAttribute * attr = OffsetAttribute::get(n);
            SgAsmDwarfNamespace * ns = isSgAsmDwarfNamespace(n);
            std::string spec = ns->get_spec_ref();
            if(attr != NULL && attr->node != NULL && isSgNamespaceDeclarationStatement(attr->node)) {
                parentScope = isSgNamespaceDeclarationStatement(attr->node)->get_definition();
            } else {
                SgNamespaceDeclarationStatement * nsDecl = DwarfROSE::convertNamespace(isSgAsmDwarfNamespace(n), scope);
                parentScope = nsDecl->get_definition();
                newDecl = nsDecl;
            }
            break;                            
        };

        case V_SgAsmDwarfMember: {
            SgAsmDwarfMember * mem = isSgAsmDwarfMember(n);
            if(!mem->get_artificiality()) {
                SgVariableDeclaration * varDecl = DwarfROSE::convertMember(mem, scope);
                newDecl = varDecl;
            }
            break;
        };

        case V_SgAsmDwarfInheritance: {
            SgBaseClass * baseClass = DwarfROSE::convertInheritance(isSgAsmDwarfInheritance(n), scope);
            SgClassDefinition * classDefn = isSgClassDefinition(scope);
            if(baseClass != NULL && classDefn != NULL) {
                classDefn->get_inheritances().push_back(baseClass);
            }
            break;                              
        };

        default: ; // Do nothing
    }

    if(newDecl != NULL)  {
        SageInterface::fixStatement(newDecl, scope);
        SageInterface::insertStatementAfterLastDeclaration(newDecl, scope);
    }

    return InheritedAttribute(parentScope);
}

int main ( int argc, char* argv[] ) {
	// Parses the input files and generates the AST
	SgProject* project = frontend(argc,argv);
	ROSE_ASSERT (project != NULL);

    // Make sure we have a valid AST 
    AstTests::runAllTests(project);

    Rose_STL_Container<SgNode*> units = NodeQuery::querySubTree(project, V_SgAsmDwarfCompilationUnit);
    BOOST_FOREACH(SgNode * n, units) {
        SgAsmDwarfCompilationUnit * unit = isSgAsmDwarfCompilationUnit(n);
        offsetMapType & offsets = constructOffsetMap(unit);
        annotateDwarfConstructs(unit, offsets);

        SgSourceFile * newFile = newFileInProject(project);
        SgGlobal * global = newFile->get_globalScope();
        // Make sure the global scope is marked as a transformation
        // or it won't unparse correctly.
        SageInterface::setSourcePositionForTransformation(global);
        global->set_startOfConstruct(Sg_File_Info::generateDefaultFileInfoForTransformationNode());
        global->set_endOfConstruct(Sg_File_Info::generateDefaultFileInfoForTransformationNode());

        // Note in the output what the source of the code was.
        SageInterface::attachComment(global, std::string("BEGIN COMPILATION UNIT ") + unit->get_name());

        // Generate the header
        InheritedAttribute attr(NULL);
        UndwarfTraversal traversal(global);
        traversal.traverse(unit, attr);


        // Print the generated header.
        std::cout << global->unparseToCompleteString() << std::endl << std::endl;
    }

    return 0;
}                                  
