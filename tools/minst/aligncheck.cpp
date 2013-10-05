
#include <rose.h>

#include "rose.h"
#include "aligncheck.h"

using namespace SageBuilder;
using namespace SageInterface;

void aligncheck_t::atTraversalStart() {
}

void aligncheck_t::atTraversalEnd() {
}

attrib aligncheck_t::evaluateInheritedAttribute(SgNode* node, attrib attr) {
    return attr;
}
