
#ifndef	INSTRUMENTOR_H_
#define	INSTRUMENTOR_H_

#include "generic_defs.h"

class instrumentor_t : public AstTopDownProcessing<attrib>
{
	private:
		short lang;
		std::vector<std::string> stream_list;

	public:
		instrumentor_t (short _lang);
		virtual attrib evaluateInheritedAttribute(SgNode* node, attrib attr);
		virtual void atTraversalStart();
		std::vector<std::string>& get_stream_list();
};

#endif	/* INSTRUMENTOR_H_ */
