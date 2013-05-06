
#ifndef	SCOURER_H_
#define	SCOURER_H_

#include "generic_defs.h"

class scourer_t : public AstTopDownProcessing<attrib>
{
	bool globalSkip;

	public:
	std::string var_name;
	SgNode *currNode, *assignOp;

	scourer_t(SgNode* _currNode, std::string _var_name)
	{
		assignOp=NULL, globalSkip=false, currNode=_currNode, var_name=_var_name;
	}

	virtual attrib evaluateInheritedAttribute(SgNode* node, attrib attr);
};

#endif	/* SCOURER_H_ */
