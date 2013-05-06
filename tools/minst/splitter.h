
#ifndef	SPLITTER_H_
#define	SPLITTER_H_

#include "generic_defs.h"

typedef struct
{
	bool neg;
	SgNode* lhs;
	SgNode* rhs;
} statement_t;

class splitter_t : public AstTopDownProcessing<attrib>
{
	public:
	SgFortranDo* clonedLoop;
	SgNode *assignOp, *initOp, *termOp;
	std::vector<statement_t> partial_statements;

	splitter_t()
	{
		initOp = NULL;
		termOp = NULL;
		assignOp = NULL;
		clonedLoop = NULL;
	}

	virtual attrib evaluateInheritedAttribute(SgNode* node, attrib attr);
	virtual void atTraversalStart();
	virtual void atTraversalEnd();
};

#endif	/* SPLITTER_H_ */
