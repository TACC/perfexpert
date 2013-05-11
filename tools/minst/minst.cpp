
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

attrib MINST::evaluateInheritedAttribute(SgNode* node, attrib attr)
{
	// If explicit instructions to skip this node, then just return
	if (attr.skip)
		return attr;

	// Add header file for indigo's record function
	if (isSgGlobal(node) && lang != LANG_FORTRAN)
		insertHeader("mrt.h", PreprocessingInfo::after, false, (SgGlobal*) node);

	// Check if this is the function that we are told to instrument
	if (isSgFunctionDefinition(node) && line_number == 0)
	{
		std::string demangled_name = demangle_function_name((SgFunctionDefinition*) node);
		if (!function_match(demangled_name, inst_func.c_str()))
		{
			attr.skip = TRUE;
			return attr;
		}

		std::cerr << "Operating on function " << demangled_name << std::endl;

		if (action == ACTION_INSTRUMENT)
		{
			instrumentor_t inst(lang);
			inst.traverse(node, attr);
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
				instrumentor_t inst(lang);
				inst.traverse(node, attr);
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
