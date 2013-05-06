
#ifndef	MINST_H_
#define	MINST_H_

#include "generic_defs.h"

class MINST : public AstTopDownProcessing<attrib>
{
	short action;
	int line_number;
	std::string inst_func;
	public:
		MINST(short _action, int _line_number, std::string _inst_func)
		{
			action=_action, line_number=_line_number, inst_func=_inst_func;
		}

		virtual attrib evaluateInheritedAttribute(SgNode* node, attrib attr);
};

#endif	/* MINST_H_ */
