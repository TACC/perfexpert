
#ifndef	INSTRUMENTOR_H_
#define	INSTRUMENTOR_H_

#include "generic_defs.h"

class instrumentor_t : public AstTopDownProcessing<attrib>
{
	public:
		virtual attrib evaluateInheritedAttribute(SgNode* node, attrib attr);
};

#endif	/* INSTRUMENTOR_H_ */
