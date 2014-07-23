/*
 * Copyright (c) 2011-2013  University of Texas at Austin. All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * This file is part of PerfExpert.
 *
 * PerfExpert is free software: you can redistribute it and/or modify it under
 * the terms of the The University of Texas at Austin Research License
 * 
 * PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.
 * 
 * Authors: Leonardo Fialho and Ashay Rane
 *
 * $HEADER$
 */

#ifndef TOOLS_MACPO_INST_INCLUDE_IR_METHODS_H_
#define TOOLS_MACPO_INST_INCLUDE_IR_METHODS_H_

#include <rose.h>

#include <map>
#include <set>
#include <string>
#include <vector>

#include "generic_defs.h"
#include "inst_defs.h"

class ir_methods {
 private:
    static std::set<std::string> intrinsic_list;
    static SgExpression* _strip_unary_operators(SgExpression* expr);
    static void _populate_intrinsic_list();

 public:
    static const int INVALID_LOOP = 1 << 0;
    static const int INVALID_FLOW = 1 << 1;
    static const int INVALID_IDXV = 1 << 2;
    static const int INVALID_INIT = 1 << 3;
    static const int INVALID_TEST = 1 << 4;
    static const int INVALID_INCR = 1 << 5;

    static const int INVALID_OP = 1 << 0;
    static const int OP_ADD     = 1 << 1;
    static const int OP_SUB     = 1 << 2;
    static const int OP_MUL     = 1 << 3;
    static const int OP_DIV     = 1 << 4;

    typedef std::map<std::string, node_list_t> def_map_t;

    // XXX: Don't change the order of the elements of this enum!
    // XXX: The order is important for subsequent arithmetic comparisons.
    enum { DEPENDENT = 0, DEP_UNKNOWN, NON_DEPENDENT };

    static bool is_top_level_loop(SgNode* node, SgNode* ref_node);

    static SgCastExp* get_function_address(SgScopeStatement* loop_stmt);

    static void construct_def_map(const du_table_t& def_table,
            def_map_t& def_map);

    static int16_t is_input_dep(const SgNode* node, def_map_t& def_map);

    static void match_end_of_constructs(SgNode* ref_node, SgNode* stmt);

    static bool create_spans(SgExpression* expr_init, SgExpression* expr_term,
            loop_info_t& loop_info);

    static SgExpression* strip_unary_operators(SgExpression* expr);

    static std::vector<SgStatement*> get_var_decls(SgForStatement* for_stmt);

    static SgExpression* get_terminal_expr(SgExpression* idxv,
            SgExpression* incr, int incr_op);

    static void place_alignment_checks(expr_list_t& expr_list,
            Sg_File_Info* fileInfo, SgScopeStatement* loop_stmt,
            statement_list_t& statement_list, const std::string& prefix,
            int16_t dependence_status);

    static void remove_duplicate_expressions(expr_list_t& expr_list);

    static bool is_ancestor(SgNode* lower_node, SgNode* upper_node);

    static SgExprStatement* create_long_assign_statement(
            Sg_File_Info* fileInfo, const std::string& name,
            SgIntVal* value);

    static SgExprStatement* create_long_incr_statement(
            Sg_File_Info* fileInfo, const std::string& name);

    static SgVariableDeclaration* create_long_variable(
            Sg_File_Info* fileInfo, const std::string& name,
            int64_t init_value);

    static SgVariableDeclaration* create_long_variable_with_init(
            Sg_File_Info* fileInfo, const std::string& name,
            SgExpression* initializer);

    static bool vectorizable(SgStatement*& stmt);

    static int get_while_loop_header_components(SgScopeStatement*&
            scope_stmt, SgExpression*& idxv_expr, SgExpression*& test_expr,
            SgExpression*& incr_expr, int& incr_op);

    static int get_for_loop_header_components(SgForStatement*& for_stmt,
            SgExpression*& idxv_expr, SgExpression*& init_expr,
            SgExpression*& test_expr, SgExpression*& incr_expr, int& incr_op,
            def_map_t& def_map);

    static int get_loop_header_components(SgScopeStatement*& scope_stmt,
            SgExpression*& idxv_expr, SgExpression*& init_expr,
            SgExpression*& test_expr, SgExpression*& incr_expr, int& incr_op,
            def_map_t& def_map);

    static SgExpression* get_expr_value(SgNode*& node, std::string var_name);

    static SgExprStatement* prepare_call_statement(SgBasicBlock* bb,
            const std::string& function_name,
            const std::vector<SgExpression*>& params,
            const SgNode* reference_statement);

    static int64_t get_reference_index(reference_list_t& reference_list,
            std::string& stream_name);

    static std::string strip_index_expr(const std::string& stream_name);

    static bool is_known(const SgExpression* expr);

    static bool contains_expr(SgExpression*& bin_op,
            SgExpression*& search_expr);

    static void replace_expr(SgExpression*& expr, SgExpression*& search_expr,
            SgExpression*& replace_expr);

    static bool is_linear_reference(const SgBinaryOp* reference,
            bool check_lhs_operand);

    static bool is_loop(SgNode* node);

    static bool is_function(SgNode* node);

    static bool in_write_set(SgStatement* statement, SgExpression* expr);
    static SgExpression* rhs_expression(SgStatement* statement);

    static void incr_components(Sg_File_Info*& fileInfo,
            SgExpression*& expr, SgExpression*& incr_expr, int& incr_op);

    static bool is_intrinsic_function(const std::string& name);
};   /* ir_methods */

#endif  // TOOLS_MACPO_INST_INCLUDE_IR_METHODS_H_
