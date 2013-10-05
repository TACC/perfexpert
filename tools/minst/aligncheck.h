
#ifndef	ALIGNCHEK_H_
#define	ALIGNCHEK_H_

#include "generic_defs.h"

class aligncheck_t : public AstTopDownProcessing<attrib>
{
	public:
		virtual void atTraversalEnd();
		virtual void atTraversalStart();

		virtual attrib evaluateInheritedAttribute(SgNode* node, attrib attr);
};

#endif	/* ALIGNCHEK_H_ */
