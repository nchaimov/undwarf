
#include <signal.h>
#include "rose.h"

int verbose = 0;

// Build an inherited attribute for the tree traversal to test the rewrite mechanism
class InheritedAttribute
   {
     public:
      // Depth in AST
         int depth;

       // Specific constructors are required
          InheritedAttribute (int x) : depth(x) {};
          InheritedAttribute ( const InheritedAttribute & X ) : depth(X.depth) {};
   };

class visitorTraversal : public AstTopDownProcessing<InheritedAttribute>
   {
     public:
       // virtual function must be defined
          virtual InheritedAttribute evaluateInheritedAttribute(SgNode* n, InheritedAttribute inheritedAttribute);
   };

InheritedAttribute
visitorTraversal::evaluateInheritedAttribute(SgNode* n, InheritedAttribute inheritedAttribute)
   {
    Sg_File_Info* s = n->get_startOfConstruct();
    Sg_File_Info* e = n->get_endOfConstruct();
    Sg_File_Info* f = n->get_file_info();
    for(int x=0; x < inheritedAttribute.depth; ++x) {
        printf(" ");
    }
    if(s != NULL && e != NULL && !isSgLabelStatement(n)) { 
        printf ("%s (%d, %d, %d)->(%d, %d): %s",n->sage_class_name(),s->get_file_id()+1,s->get_raw_line(),s->get_raw_col(),e->get_raw_line(),e->get_raw_col(),  verbose ? n->unparseToString().c_str() : "" );
        if(isSgAsmDwarfConstruct(n)) {
            printf(" [DWARF construct name: %s]", isSgAsmDwarfConstruct(n)->get_name().c_str());
        }
        SgExprStatement * exprStmt = isSgExprStatement(n);
        if(exprStmt != NULL) {
            printf(" [expr type: %s]", exprStmt->get_expression()->sage_class_name());           
            SgFunctionCallExp * fcall = isSgFunctionCallExp(exprStmt->get_expression());
            if(fcall != NULL) {
               SgExpression * funcExpr = fcall->get_function();
               if(funcExpr != NULL) {
                    printf(" [function expr: %s]", funcExpr->class_name().c_str());
               }
               SgFunctionDeclaration * fdecl = fcall->getAssociatedFunctionDeclaration();
               if(fdecl != NULL) {
                    printf(" [called function: %s]", fdecl->get_name().str());
               }
            }
        }
        if(isSgFunctionDeclaration(n)) {
            printf(" [declares function: %s]", isSgFunctionDeclaration(n)->get_name().str());
        }
        SgStatement * sgStmt = isSgStatement(n);
        if(sgStmt != NULL) {
            printf(" [scope: %s, %p]", sgStmt->get_scope()->sage_class_name(), sgStmt->get_scope());
        }
        //SgLabelStatement * lblStmt = isSgLabelStatement(n);
        //if(lblStmt != NULL) {
        //    SgStatement * lblStmt2 = lblStmt->get_statement();
        //}
    } else if (f != NULL) {
		SgInitializedName * iname = isSgInitializedName(n);
		if(iname != NULL) {
            SgType* inameType = iname->get_type();
			printf("%s (%d, %d, %d): %s [type: %s", n->sage_class_name(),f->get_file_id()+1,f->get_raw_line(),f->get_raw_col(),n->unparseToString().c_str(),inameType->class_name().c_str());
			SgDeclarationStatement * ds = isSgDeclarationStatement(iname->get_parent());
			if(ds != NULL) {
				if(ds->get_declarationModifier().get_storageModifier().isStatic()) {
					printf(" static");
				}
			}
			
			SgArrayType * art = isSgArrayType(iname->get_type());
			if(art != NULL) {
				printf(" %d", art->get_rank());
			}
			
			printf("]");
            if(isSgAsmDwarfConstruct(n)) {
                printf(" [DWARF construct name: %s]", isSgAsmDwarfConstruct(n)->get_name().c_str());
            }
            } else {
        	printf("%s (%d, %d, %d): %s", n->sage_class_name(),f->get_file_id()+1,f->get_raw_line(),f->get_raw_col(), verbose ? n->unparseToString().c_str() : "");
		}
    } else {
        printf("%s : %s", n->sage_class_name(), verbose ? n->unparseToString().c_str() : "");
        if(isSgAsmDwarfConstruct(n)) {
            printf(" [DWARF construct name: %s]", isSgAsmDwarfConstruct(n)->get_name().c_str());
        }
    }
    printf(" succ# %lu", n->get_numberOfTraversalSuccessors());
	printf("\n");
     return InheritedAttribute(inheritedAttribute.depth+1);
   }

int
main ( int argc, char* argv[] )
   {
	Rose_STL_Container<std::string> l = CommandlineProcessing::generateArgListFromArgcArgv(argc,argv);
	 if ( CommandlineProcessing::isOptionWithParameter(l, "-taurose:","(v|verbose)",verbose ,false)) {
		std::cerr << "Verbose logging enabled" << std::endl;
		verbose = 1;
		std::cerr << "Verbose: " << verbose << std::endl;
	}
     SgProject* project = frontend(l);
     ROSE_ASSERT (project != NULL);

     SgFile & localFile = project->get_file(0);
     localFile.get_file_info()->display("localFile information");

     InheritedAttribute inheritedAttribute(0);

     visitorTraversal exampleTraversal;

     exampleTraversal.traverse(project,inheritedAttribute);

     return 0;
   }


