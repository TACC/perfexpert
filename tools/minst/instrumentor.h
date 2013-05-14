
#ifndef	INSTRUMENTOR_H_
#define	INSTRUMENTOR_H_

#include "generic_defs.h"

typedef struct
{
	SgBasicBlock* bb;
	SgExprStatement* exprStmt;
	std::vector<SgExpression*> params;
} inst_info_t;

class instrumentor_t : public AstTopDownProcessing<attrib>
{
	private:
		std::vector<std::string> stream_list;
		std::vector<inst_info_t> inst_info_list;

	public:
		std::vector<std::string>& get_stream_list();

		virtual attrib evaluateInheritedAttribute(SgNode* node, attrib attr);
		virtual void atTraversalStart();
		virtual void atTraversalEnd();
};

#endif	/* INSTRUMENTOR_H_ */
