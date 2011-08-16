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

typedef std::map<std::string, SgAsmDwarfConstruct*> offsetMapType;
TypeTable & typeTable = TypeTable::getInstance();

class OffsetAttribute : public AstAttribute {
    public:
        SgAsmDwarfConstruct * type;

        OffsetAttribute(SgAsmDwarfConstruct * t = NULL) : type(t) {};
        
        static const std::string OFFSET_ATTRIBUTE;

        static inline OffsetAttribute * get(SgNode * n) {
           AstAttribute * attr = n->getAttribute(OFFSET_ATTRIBUTE);        
           return dynamic_cast<OffsetAttribute*>(attr);
        };

        inline void add(SgNode * n) {
            n->addNewAttribute(OFFSET_ATTRIBUTE, this);
        }
};

const std::string OffsetAttribute::OFFSET_ATTRIBUTE = "OFFSET_ATTRIBUTE";

class InheritedAttribute {
    public:
        int depth;
        SgAsmDwarfSubprogram * parentSubprogram;

        InheritedAttribute(int d, SgAsmDwarfSubprogram * s) : depth(d), parentSubprogram(s) {};
        InheritedAttribute( const InheritedAttribute & X) : depth(X.depth), parentSubprogram(X.parentSubprogram) {};
};

class DwarfTraversal : public AstTopDownProcessing<InheritedAttribute> {
    public:
        virtual InheritedAttribute evaluateInheritedAttribute(SgNode * n, InheritedAttribute inheritedAttribute);
        
};

InheritedAttribute DwarfTraversal::evaluateInheritedAttribute(SgNode * n, InheritedAttribute inheritedAttribute) {
    int depth = inheritedAttribute.depth;
    SgAsmDwarfSubprogram * parentSubprogram = inheritedAttribute.parentSubprogram;
    
    switch(n->variantT()) {
        case V_SgAsmDwarfSubprogram: {
            parentSubprogram = isSgAsmDwarfSubprogram(n);                        
        };
        break;

        default: ;
    }

    if(parentSubprogram != NULL) {
        depth++;
    }

    SgAsmDwarfConstruct * dc = isSgAsmDwarfConstruct(n);
    if(dc != NULL) {
        for(int i = 0; i < depth; ++i) {
             std::cout << " ";
        }
        std::cout << n->class_name() << "    offset: " << dc->get_offset() << " name: " << dc->get_name();
        
        OffsetAttribute * attr = OffsetAttribute::get(dc);

        if(attr != NULL) { 
            std::cout << " type: " << attr->type->get_name();
        }
        std::cout << std::endl;
    }

    return InheritedAttribute(depth, parentSubprogram);

}
    
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

    SgGlobal * global = new SgGlobal();
    SageInterface::setSourcePositionForTransformation(global);
    global->set_parent(project);

    Rose_STL_Container<SgNode*> subprograms = NodeQuery::querySubTree(project, V_SgAsmDwarfSubprogram);
    BOOST_FOREACH(SgNode * n, subprograms) {
        SgAsmDwarfSubprogram * subprogram = isSgAsmDwarfSubprogram(n);
        const std::string & name = subprogram->get_name();
        OffsetAttribute * attr = OffsetAttribute::get(subprogram);
        std::string retType = "";
        if(attr != NULL) {
            SgAsmDwarfConstruct * type = attr->type;
            if(type != NULL) {
                retType = typeToName(type);
            } else {
                std::cerr << "Subprogram " << name << " had no type in its offset attribute." << std::endl;
            }
        } else {
            retType = "void";
        } 
        std::vector<std::pair<std::string, std::string> > args;
        Rose_STL_Container<SgNode*> formalParameters = NodeQuery::querySubTree(subprogram, V_SgAsmDwarfFormalParameter);
        BOOST_FOREACH(SgNode * n2, formalParameters) {
            SgAsmDwarfFormalParameter * formalParameter = isSgAsmDwarfFormalParameter(n2);
            OffsetAttribute * attr = OffsetAttribute::get(formalParameter);
            if(attr != NULL) {
                SgAsmDwarfConstruct * type = attr->type;
                if(type != NULL) {
                    args.push_back(std::pair<std::string, std::string>( formalParameter->get_name(), typeToName(type) ));
                } else {
                    std::cerr << "Formal parameter " << formalParameter->get_name() << " had no type in its offset attribute." << std::endl;
                }
            } else {
                std::cerr << "Formal parameter " << formalParameter->get_name() << " had no attached offset attribute." << std::endl;
            }
        }

        std::cout << retType << " " << name;
        std::cout << "(";
        for(size_t i = 0; i < args.size(); ++i) {
            std::cout << (args[i].second) << " " << (args[i].first);
            if(i != args.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << ")" << std::endl;
    }

    
    AstTests::runAllTests(project);

    std::cout << global->unparseToCompleteString() << std::endl;

    delete global;

//    InheritedAttribute inheritedAttribute(0, NULL);
//    DwarfTraversal traversal;
//    traversal.traverse(project, inheritedAttribute);


//    Rose_STL_Container<SgNode*> interpLists = NodeQuery::querySubTree(project, V_SgAsmInterpretationList);
//    BOOST_FOREACH(SgNode * l, interpLists) {
//        SgAsmInterpretationList * interpList = isSgAsmInterpretationList(l);
//        if(interpList == NULL) {
//            std::cerr << "ERROR: Couldn't find an interpretation list in AST!" << std::endl;
//            return -1;
//        }
//        const SgAsmInterpretationPtrList & interps = interpList->get_interpretations();
//        BOOST_FOREACH(SgAsmInterpretation * interp, interps) {
//            SgAsmDwarfCompilationUnitList * unitList = interp->get_dwarf_info();
//            ROSE_ASSERT(unitList != NULL);
//            const SgAsmDwarfCompilationUnitPtrList & units = unitList->get_cu_list();
//            BOOST_FOREACH(SgAsmDwarfCompilationUnit * unit, units) {
//                ROSE_ASSERT(unit != NULL);
//                std::string producer = unit->get_producer();
//                if(producer.empty()) {
//                    producer = "<empty>";
//                }
//                std::cerr << std::setw(14) << "Producer: "   << std::setw(32) << producer                   << std::endl;
//                std::cerr << std::setw(14) << "Language: "   << std::setw(32) << unit->get_language()       << std::endl;
//                std::cerr << std::setw(14) << "Low PC: "     << std::setw(32) << unit->get_low_pc()         << std::endl;
//                std::cerr << std::setw(14) << "Hi PC: "      << std::setw(32) << unit->get_hi_pc()          << std::endl;
//                std::cerr << std::setw(14) << "Version: "    << std::setw(32) << unit->get_version_stamp()  << std::endl;
//                std::cerr << std::setw(14) << "Abb offset: " << std::setw(32) << unit->get_abbrev_offset()  << std::endl;
//                std::cerr << std::setw(14) << "Addr size: "  << std::setw(32) << unit->get_address_size()   << std::endl;
//                std::cerr << std::setw(14) << "Offset len: " << std::setw(32) << unit->get_offset_length()  << std::endl;
//
//                SgAsmDwarfConstructList * constructList = unit->get_language_constructs();
//                const SgAsmDwarfConstructPtrList & constructs = constructList->get_list();
//                BOOST_FOREACH(SgAsmDwarfConstruct * construct, constructs) {
//                    ROSE_ASSERT(construct != NULL);
//                    std::string name = construct->get_name();
//                    if(name.empty()) {
//                        name = "<empty>";
//                    }
//                    std::cerr << std::endl;
//                    std::cerr << std::setw(14) << "CONSTRUCT: "  << std::setw(32) << construct->class_name() << std::endl;
//                    std::cerr << std::setw(14) << "Name: "       << std::setw(32) << name   << std::endl;
//                    std::cerr << std::setw(14) << "Offset: "     << std::setw(32) << construct->get_offset() << std::endl;
//                    std::cerr << std::setw(14) << "Overall: "    << std::setw(32) << construct->get_overall_offset() << std::endl;
//                    std::cerr << std::setw(14) << "Nest level: " << std::setw(32) << construct->get_nesting_level() << std::endl;
//                }
//            }
//        }
//
//    }

    return 0;
}                                  
