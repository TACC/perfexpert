
#ifndef	STREAM_COUNTER_H_
#define	STREAM_COUNTER_H_

#include "generic_defs.h"

class stream_counter_t : public AstTopDownProcessing<attrib>
{
	public:
		std::set<std::string> discovered_streams;
		virtual attrib evaluateInheritedAttribute(SgNode* node, attrib attr);
};

#endif	/* STREAM_COUNTER_H_ */
