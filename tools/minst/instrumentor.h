
#ifndef	INSTRUMENTOR_H_
#define	INSTRUMENTOR_H_

#include "generic_defs.h"

class instrumentor_t : public AstTopDownProcessing<attrib>
{
	private:
		std::vector<std::string> stream_list;

	public:
		virtual attrib evaluateInheritedAttribute(SgNode* node, attrib attr);
		virtual void atTraversalStart();

		std::vector<std::string>& getStreamList();
};

#endif	/* INSTRUMENTOR_H_ */
