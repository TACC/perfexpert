
#ifndef IR_METHODS_H_
#define IR_METHODS_H_

#include "rose.h"
#include "generic_defs.h"
#include "VariableRenaming.h"

class ir_methods {
    public:
        static const int INVALID_LOOP = 1 << 0;
        static const int INVALID_FLOW = 1 << 1;
        static const int INVALID_INIT = 1 << 2;
        static const int INVALID_TEST = 1 << 3;
        static const int INVALID_INCR = 1 << 4;

        typedef VariableRenaming::NumNodeRenameEntry::iterator entry_iterator;
        typedef std::map<std::string, VariableRenaming::NumNodeRenameEntry>
            def_map_t;

        static bool vectorizable(SgStatement*& stmt);

        static int get_loop_header_components(VariableRenaming*& var_renaming,
                SgForStatement*& for_stmt, def_map_t& def_map, SgExpression*&
                idxv_expr, SgExpression*& init_expr, SgExpression*& test_expr,
                SgExpression*& incr_expr);

        static void construct_def_map(VariableRenaming::NumNodeRenameTable&
                rename_table, std::map<std::string,
                VariableRenaming::NumNodeRenameEntry>& def_map);

        static SgExpression* get_expr_value(SgNode*& node, std::string
                var_name);

        static SgExprStatement* insert_instrumentation_call(inst_info_t&
                inst_info);
        static long get_reference_index(reference_list_t& reference_list,
                std::string& stream_name);

        static std::string strip_index_expr(const std::string& stream_name);

        static bool is_known(const SgExpression* expr);

        static void replace_expr(SgBinaryOp*& bin_op,
                SgExpression*& search_expr,
                SgExpression*& replace_expr);

        static bool is_linear_reference(const SgBinaryOp* reference,
                bool check_lhs_operand);
};   /* ir_methods */

#endif  /* IR_METHODS_H_ */
