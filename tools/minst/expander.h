
#ifndef	EXPANDER_H_
#define	EXPANDER_H_

#include "generic_defs.h"

class expander_t : public AstTopDownProcessing<attrib>
{
	public:
		virtual attrib evaluateInheritedAttribute(SgNode* node, attrib attr);
};

#endif	/* EXPANDER_H_ */
