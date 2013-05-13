
#include <rose.h>

#include "minst.h"
#include "expander.h"
#include "splitter.h"
#include "instrumentor.h"

using namespace SageBuilder;
using namespace SageInterface;

std::string demangle_function_name(SgFunctionDefinition* def)
{
	if (!def)	return "";

	return StringUtility::demangledName(def->get_mangled_name().getString());
}

bool function_match(const std::string demangled_name, const char* szSearchString)
{
	const char* szfunction_name = demangled_name.c_str();
	return strstr(szfunction_name, szSearchString) == szfunction_name;
}

void MINST::insert_map_function(SgNode* node)
{
	SgFunctionDeclaration *func_decl, *decl;
	SgProcedureHeaderStatement* header;
	const char* func_name = "indigo__create_map";

	if (SageInterface::is_Fortran_language())
	{
		header = SageBuilder::buildProcedureHeaderStatement(func_name, buildVoidType(), buildFunctionParameterList(), SgProcedureHeaderStatement::e_subroutine_subprogram_kind, global_node);
		func_decl = isSgFunctionDeclaration(header);
	}
	else
	{
		func_decl = SageBuilder::buildDefiningFunctionDeclaration(func_name, buildVoidType(), buildFunctionParameterList(), global_node);
		fdecl = SageBuilder::buildNondefiningFunctionDeclaration(func_decl, global_node);
	}

	appendStatement(func_decl, global_node);
}

void MINST::atTraversalStart()
{
	global_node=NULL, fdecl=NULL, bb=NULL;
}

void MINST::atTraversalEnd()
{
	if (global_node && fdecl)
		global_node->prepend_declaration(fdecl);
}

attrib MINST::evaluateInheritedAttribute(SgNode* node, attrib attr)
{
	std::vector<std::string> stream_list;

	// If explicit instructions to skip this node, then just return
	if (attr.skip)
		return attr;

	// Add header file for indigo's record function
	if (isSgGlobal(node))
	{
		global_node = static_cast<SgGlobal*>(node);
		if (lang != LANG_FORTRAN && action == ACTION_INSTRUMENT)
			insertHeader("mrt.h", PreprocessingInfo::after, false, global_node);
	}

	// Check if this is the function that we are told to instrument
	if (isSgFunctionDefinition(node))
	{
		std::string demangled_name = demangle_function_name((SgFunctionDefinition*) node);
		if (function_match(demangled_name, "main") && action == ACTION_INSTRUMENT)
		{
			// Found main, now insert calls to indigo__init() and indigo__create_map()
			SgBasicBlock* body = ((SgFunctionDefinition*) node)->get_body();

			// Skip over the initial variable declarations and `IMPLICIT' statements
			SgStatement *statement=NULL;
			SgStatementPtrList& stmts = body->get_statements();
			for (SgStatementPtrList::iterator it=stmts.begin(); it!=stmts.end(); it++)
			{
				statement=*it;

				if (!isSgImplicitStatement(statement) && !isSgVariableDeclaration(statement))
					break;
			}

			ROSE_ASSERT(statement!=NULL);

			std::string indigo__init = lang!=LANG_FORTRAN ? "indigo__init_" : "indigo__init";
			std::string indigo__create_map = "indigo__create_map";
			SgExprStatement* init_call = buildFunctionCallStmt(SgName(indigo__init), buildVoidType(), NULL, body);
			SgExprStatement* map_call  = buildFunctionCallStmt(SgName(indigo__create_map), buildVoidType(), NULL, body);
			insertStatementBefore(statement, init_call);
			insertStatementAfter (init_call, map_call);
		}

		if (line_number == 0)
		{
			if (!function_match(demangled_name, inst_func.c_str()))
			{
				attr.skip = TRUE;
				return attr;
			}

			std::cerr << "Operating on function " << demangled_name << std::endl;

			if (action == ACTION_INSTRUMENT)
			{
				// We found the function that we wanted to instrument, now insert the indigo__create_map_() function in this file
				insert_map_function(node);

				instrumentor_t inst(lang);
				inst.traverse(node, attr);
				stream_list = inst.get_stream_list();
			}
		}
	}
	else if (line_number != 0 && isSgLocatedNode(node))	// We have to instrument some loops
	{
		int _line_number = ((SgLocatedNode *) node)->get_file_info()->get_line();
		if (_line_number == line_number && (isSgFortranDo(node) || isSgForStatement(node) || isSgWhileStmt(node) || isSgDoWhileStmt(node)))
		{
			SgFunctionDefinition* def = getEnclosingNode<SgFunctionDefinition>(node);
			std::cerr << (action == ACTION_INSTRUMENT ? "Instrumenting" : "Splitting") << " loop in function " << demangle_function_name(def) << " at line " << _line_number << std::endl;

			if (action == ACTION_INSTRUMENT)
			{
				// We found the loop that we wanted to instrument, now insert the indigo__create_map_() function in this file
				insert_map_function(node);

				instrumentor_t inst(lang);
				inst.traverse(node, attr);
				stream_list = inst.get_stream_list();
			}
			else if (action == ACTION_SPLIT_LOOP)
			{
				expander_t expander;
				expander.traverse(node, attr);

				splitter_t splitter;
				splitter.traverse(node, attr);
			}
		}
	}
	else if (getEnclosingNode<SgFunctionDefinition>(node) != NULL)	// We are inside some other function
		attr.skip = true;

	return attr;
}
