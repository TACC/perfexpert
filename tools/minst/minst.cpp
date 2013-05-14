
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

void MINST::insert_map_prototype(SgNode* node)
{
	const char* func_name = "indigo__create_map";

	if (!SageInterface::is_Fortran_language())
	{
		SgFunctionDeclaration *decl;
		decl = SageBuilder::buildDefiningFunctionDeclaration(func_name, buildVoidType(), buildFunctionParameterList(), global_node);
		non_def_decl = SageBuilder::buildNondefiningFunctionDeclaration(decl, global_node);
	}
}

void MINST::insert_map_function(SgNode* node)
{
	SgProcedureHeaderStatement* header;
	const char* func_name = "indigo__create_map";

	if (SageInterface::is_Fortran_language())
	{
		header = SageBuilder::buildProcedureHeaderStatement(func_name, buildVoidType(), buildFunctionParameterList(), SgProcedureHeaderStatement::e_subroutine_subprogram_kind, global_node);
		def_decl = isSgFunctionDeclaration(header);
	}
	else
		def_decl = SageBuilder::buildDefiningFunctionDeclaration(func_name, buildVoidType(), buildFunctionParameterList(), global_node);

	appendStatement(def_decl, global_node);
}

void MINST::atTraversalStart()
{
	stream_list.clear();
	global_node=NULL, non_def_decl=NULL, def_decl=NULL, file_info=NULL;
}

void MINST::atTraversalEnd()
{
	if (global_node && non_def_decl)
		global_node->prepend_declaration(non_def_decl);

	if (def_decl)
	{
		SgBasicBlock* bb = def_decl->get_definition()->get_body();
		if (bb && stream_list.size() > 0)
		{
			int idx=0;
			std::string indigo__write_idx = SageInterface::is_Fortran_language() ? "indigo__write_idx" : "indigo__write_idx_";
			for (std::vector<std::string>::iterator it=stream_list.begin(); it!=stream_list.end(); it++,idx++)
			{
				// Add a call to indigo__write_idx() and place it in the basic block bb
				std::string stream_name = *it;

				SgIntVal* param_idx = new SgIntVal(file_info, idx);
				SgStringVal* param_stream_name = new SgStringVal(file_info, stream_name);

				std::vector<SgExpression*> expr_vector;
				expr_vector.push_back(param_idx);
				expr_vector.push_back(param_stream_name);

				SgExprStatement* write_idx_call = buildFunctionCallStmt(SgName(indigo__write_idx), buildVoidType(), buildExprListExp(expr_vector), bb);
				bb->append_statement(write_idx_call);
			}
		}
	}
}

attrib MINST::evaluateInheritedAttribute(SgNode* node, attrib attr)
{
	// If explicit instructions to skip this node, then just return
	if (attr.skip)
		return attr;

	// Add header file for indigo's record function
	if (isSgGlobal(node))
	{
		global_node = static_cast<SgGlobal*>(node);
		file_info = Sg_File_Info::generateFileInfoForTransformationNode(
				((SgLocatedNode*) node)->get_file_info()->get_filenameString());

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

				if (!isSgImplicitStatement(statement) && !isSgDeclarationStatement(statement))
					break;
			}

			ROSE_ASSERT(statement!=NULL);

			std::string indigo__init = lang!=LANG_FORTRAN ? "indigo__init_" : "indigo__init";
			std::string indigo__create_map = "indigo__create_map";
			SgExprStatement* init_call = buildFunctionCallStmt(SgName(indigo__init), buildVoidType(), NULL, body);
			SgExprStatement* map_call  = buildFunctionCallStmt(SgName(indigo__create_map), buildVoidType(), NULL, body);
			insertStatementBefore(statement, init_call);
			insertStatementAfter (init_call, map_call);

			insert_map_prototype(node);
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
