
#ifndef	MINST_H_
#define	MINST_H_

#include "generic_defs.h"

class MINST : public AstTopDownProcessing<attrib> {
    short action;
    int line_number;
    std::string inst_func;
    std::vector<std::string> stream_list;

    SgGlobal* global_node;
    Sg_File_Info* file_info;
    SgFunctionDeclaration *def_decl, *non_def_decl;

    public:
    MINST(short _action, int _line_number, std::string _inst_func);

    void insert_map_function(SgNode* node);
    void insert_map_prototype(SgNode* node);

    virtual void atTraversalEnd();
    virtual void atTraversalStart();

    virtual attrib evaluateInheritedAttribute(SgNode* node, attrib attr);
};

#endif	/* MINST_H_ */
