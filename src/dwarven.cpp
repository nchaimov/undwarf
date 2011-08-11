#include "rose.h"

#include <cstdio>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/predicate.hpp>

int main ( int argc, char* argv[] ) {
	
	// Parses the input files and generates the AST
	SgProject* project = frontend(argc,argv);
	ROSE_ASSERT (project != NULL);

    // Make sure we have a valid AST 
    AstTests::runAllTests(project);
	
    Rose_STL_Container<SgNode*> interpLists = NodeQuery::querySubTree(project, V_SgAsmInterpretationList);
    BOOST_FOREACH(SgNode * l, interpLists) {
        SgAsmInterpretationList * interpList = isSgAsmInterpretationList(l);
        if(interpList == NULL) {
            std::cerr << "ERROR: Couldn't find an interpretation list in AST!" << std::endl;
            return -1;
        }
        const SgAsmInterpretationPtrList & interps = interpList->get_interpretations();
        BOOST_FOREACH(SgAsmInterpretation * interp, interps) {
            SgAsmDwarfCompilationUnitList * unitList = interp->get_dwarf_info();
            ROSE_ASSERT(unitList != NULL);
            const SgAsmDwarfCompilationUnitPtrList & units = unitList->get_cu_list();
            BOOST_FOREACH(SgAsmDwarfCompilationUnit * unit, units) {
                ROSE_ASSERT(unit != NULL);
                std::string producer = unit->get_producer();
                if(producer.empty()) {
                    producer = "<empty>";
                }
                std::cerr << std::setw(14) << "Producer: "   << std::setw(32) << producer                   << std::endl;
                std::cerr << std::setw(14) << "Language: "   << std::setw(32) << unit->get_language()       << std::endl;
                std::cerr << std::setw(14) << "Low PC: "     << std::setw(32) << unit->get_low_pc()         << std::endl;
                std::cerr << std::setw(14) << "Hi PC: "      << std::setw(32) << unit->get_hi_pc()          << std::endl;
                std::cerr << std::setw(14) << "Version: "    << std::setw(32) << unit->get_version_stamp()  << std::endl;
                std::cerr << std::setw(14) << "Abb offset: " << std::setw(32) << unit->get_abbrev_offset()  << std::endl;
                std::cerr << std::setw(14) << "Addr size: "  << std::setw(32) << unit->get_address_size()   << std::endl;
                std::cerr << std::setw(14) << "Offset len: " << std::setw(32) << unit->get_offset_length()  << std::endl;

                SgAsmDwarfConstructList * constructList = unit->get_language_constructs();
                const SgAsmDwarfConstructPtrList & constructs = constructList->get_list();
                BOOST_FOREACH(SgAsmDwarfConstruct * construct, constructs) {
                    ROSE_ASSERT(construct != NULL);
                    std::string name = construct->get_name();
                    if(name.empty()) {
                        name = "<empty>";
                    }
                    std::cerr << std::endl;
                    std::cerr << std::setw(14) << "CONSTRUCT: "  << std::setw(32) << construct->class_name() << std::endl;
                    std::cerr << std::setw(14) << "Name: "       << std::setw(32) << name   << std::endl;
                    std::cerr << std::setw(14) << "Offset: "     << std::setw(32) << construct->get_offset() << std::endl;
                    std::cerr << std::setw(14) << "Overall: "    << std::setw(32) << construct->get_overall_offset() << std::endl;
                    std::cerr << std::setw(14) << "Nest level: " << std::setw(32) << construct->get_nesting_level() << std::endl;
                }
            }
        }

    }

    return 0;
}                                  
