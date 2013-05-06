
#include <set>
#include <rose.h>

#include "stream_counter.h"

using namespace SageBuilder;
using namespace SageInterface;

attrib stream_counter_t::evaluateInheritedAttribute(SgNode* node, attrib attr)
{
	// If explicit instructions to skip this node, then just return
	if (attr.skip == TRUE)
		return attr;

	SgNode* parent = node->get_parent();

	// Decide whether read / write depending on the operand of AssignOp
	if (parent && isSgAssignOp(parent))
		attr.read = parent->getChildIndex(node) == 0 ? TYPE_WRITE : TYPE_READ;

	// LHS operand of PntrArrRefExp is always skipped
	if (parent && isSgPntrArrRefExp(parent) && parent->getChildIndex(node) == 0)
	{
		attr.skip=TRUE;
		return attr;
	}

	// RHS operand of PntrArrRefExp is always read and never written
	if (parent && isSgPntrArrRefExp(parent) && parent->getChildIndex(node) > 0)
		attr.read = true;

	SgFortranDo* doLoop = getEnclosingNode<SgFortranDo>(node);
	if (!doLoop)
		return attr;

	if (isSgPntrArrRefExp(node)	// Is this an array reference statement?
		&& parent
		&& (!isSgPntrArrRefExp(parent)|| (isSgPntrArrRefExp(parent) && parent->getChildIndex(node) > 0)))	// Is this node representing the top-level dimension of an array or if this array expression is used to read from another array?
	{
		std::string stream_name = node->unparseToString();
		unsigned index = stream_name.find_first_of('[');
		stream_name = stream_name.substr(0, index);

		// For fortran array notation
		index = stream_name.find_first_of('(');
		stream_name = stream_name.substr(0, index);

		discovered_streams.insert(stream_name);
		attr.skip = true;
	}

	return attr;
}
