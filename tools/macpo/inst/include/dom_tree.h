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

#ifndef TOOLS_MACPO_INST_INCLUDE_DOM_TREE_H_
#define TOOLS_MACPO_INST_INCLUDE_DOM_TREE_H_

#include <rose.h>
#include <stdint.h>

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

class dom_tree_t {
 private:
    typedef std::set<SgGraphNode*> node_set_t;
    typedef std::vector<SgGraphNode*> node_list_t;

    typedef std::map<SgNode*, SgGraphNode*> ast_map_t;
    typedef std::map<SgGraphNode*, node_set_t*> dom_map_t;
    typedef std::map<SgGraphNode*, SgGraphNode*> idom_map_t;

    ast_map_t ast_map;
    StaticCFG::CFG* cfg;
    dom_map_t dominators;
    idom_map_t immediate_dominators;
    SgIncidenceDirectedGraph* graph;
    SgFunctionDefinition* function_definition;

 public:
    explicit dom_tree_t(SgFunctionDefinition* _function_definition) {
        cfg = NULL;
        graph = NULL;
        function_definition = _function_definition;
    }

    ~dom_tree_t() {
        _free_dom_tree();
        free_idom_tree();

        if (cfg != NULL) {
            delete cfg;
            cfg = NULL;
        }

        if (graph != NULL) {
            delete graph;
            graph = NULL;
        }
    }

    void print_idom_tree() {
        for (idom_map_t::iterator it = immediate_dominators.begin();
                it != immediate_dominators.end(); it++) {
            SgGraphNode* lhs_node = it->first;
            SgGraphNode* rhs_node = it->second;

            if (rhs_node != NULL) {
                SgNode* ast_lhs_node = lhs_node->get_SgNode();
                std::string lhs_str = getVariantName(ast_lhs_node->variantT());

                SgNode* ast_rhs_node = rhs_node->get_SgNode();
                std::string rhs_str = getVariantName(ast_rhs_node->variantT());

                std::cerr << mprefix << lhs_str << " is immediately dominated "
                    "by: " << rhs_str << std::endl;
            }
        }
    }

    void print_dom_tree() {
        for (dom_map_t::iterator it = dominators.begin();
                it != dominators.end(); it++) {
            SgGraphNode* lhs_node = it->first;
            node_set_t*& node_set = it->second;

            if (node_set != NULL && node_set->size() > 1) {
                SgNode* ast_lhs_node = lhs_node->get_SgNode();
                std::string lhs_str = getVariantName(ast_lhs_node->variantT());
                std::cerr << mprefix << lhs_str << " is dominated by: ";

                for (node_set_t::iterator it = node_set->begin();
                    it != node_set->end(); it++) {
                    SgGraphNode* rhs_node = *it;
                    SgNode* ast_rhs_node = rhs_node->get_SgNode();
                    VariantT variant = ast_rhs_node->variantT();
                    std::string rhs_str = getVariantName(variant);

                    std::cerr << rhs_str << " ";
                }

                std::cerr << std::endl;
            }
        }
    }

    void free_idom_tree() {
        immediate_dominators.clear();
    }

    bool immediately_dominates(SgNode* high, SgNode* low) {
        return _immediately_dominates(to_graph_node(high), to_graph_node(low));
    }

    bool strictly_dominates(SgNode* high, SgNode* low) {
        return _strictly_dominates(to_graph_node(high), to_graph_node(low));
    }

    bool dominates(SgNode* high, SgNode* low) {
        return _dominates(to_graph_node(high), to_graph_node(low));
    }

    int16_t build_idom_tree() {
        // First, construct the CFG.
        cfg = new StaticCFG::CFG(function_definition);
        cfg->buildFilteredCFG();

        // We will operate on a modified representation of the graph.
        graph = cfg->getGraph();
        if (graph == NULL || graph->numberOfGraphNodes() <= 0) {
            std::cerr << mprefix << "empty graph upon CFG construction."
                << std::endl;
            return -1;
        }

        assert(_initialize_doms(graph) == 0);

        int status = 0;
        if ((status = _build_ast_map()) != 0) {
            return status;
        }

        if ((status = _build_dom_tree()) != 0) {
            std::cerr << mprefix << "failed to build dominator tree."
                << std::endl;
            return status;
        }

        // Walk over the dominator tree and copy only the immediate dominators.
        for (dom_map_t::iterator it = dominators.begin();
                it != dominators.end(); it++) {
            SgGraphNode* lhs_node = it->first;
            node_set_t*& node_set = it->second;
            if (node_set != NULL) {
                // This is the set of all nodes that
                // strictly dominate the node under consideration.
                node_set_t sdom_set;
                for (node_set_t::iterator it = node_set->begin();
                        it != node_set->end(); it++) {
                    SgGraphNode* rhs_node = *it;
                    if (rhs_node != lhs_node) {
                        sdom_set.insert(rhs_node);
                    }
                }

                if (sdom_set.size() == 0) {
                    continue;
                }

                // For each node M that strictly dominates N, check that
                // every other node P that strictly dominates N, also
                // dominates M. If so, then M is the immediate dominator
                // of N.
                for (node_set_t::iterator it = sdom_set.begin();
                        it != sdom_set.end(); it++) {
                    SgGraphNode* M = *it;

                    bool sdom = true;
                    for (node_set_t::iterator it = sdom_set.begin();
                            it != sdom_set.end(); it++) {
                        SgGraphNode* P = *it;
                        if (_dominates(P, M) == false) {
                            sdom = false;
                            break;
                        }
                    }

                    if (sdom == true) {
                        // M is the immediate dominator of N.
                        assert(immediate_dominators[lhs_node] == NULL);
                        immediate_dominators[lhs_node] = M;
                    }
                }
            }
        }

        return 0;
    }

 private:
    void _free_dom_tree() {
        for (dom_map_t::iterator it = dominators.begin();
                it != dominators.end(); it++) {
            node_set_t*& node_set = it->second;
            if (node_set != NULL) {
                node_set->clear();
                delete node_set;
                node_set = NULL;
            }
        }

        dominators.clear();
    }

    bool _immediately_dominates(SgGraphNode* high, SgGraphNode* low) {
        if (low == NULL || high == NULL) {
            return false;
        }

        if (immediate_dominators.find(low) == immediate_dominators.end()) {
            return false;
        }

        if (immediate_dominators[low] == high) {
            return true;
        }

        return false;
    }

    bool _strictly_dominates(SgGraphNode* high, SgGraphNode* low) {
        // Only if high dominates low and high is not the same as low.
        return _dominates(high, low) && high != low;
    }

    bool _dominates(SgGraphNode* high, SgGraphNode* low) {
        if (high == NULL || low == NULL) {
            return false;
        }

        // Cursory checks to find if the input nodes are valid.
        if (dominators.find(low) == dominators.end()) {
            return false;
        }

        node_set_t*& node_set = dominators[low];
        if (node_set == NULL) {
            return false;
        }

        // Does the high node exist among the set
        // of dominating nodes of the low node?
        if (std::find(node_set->begin(), node_set->end(), high)
                != node_set->end()) {
            return true;
        }

        return false;
    }

    SgGraphNode* to_graph_node(SgNode* ast_node) {
        if (ast_map.find(ast_node) == ast_map.end()) {
            return NULL;
        }

        return ast_map[ast_node];
    }

    int16_t _build_ast_map() {
        node_set_t node_set = graph->computeNodeSet();
        for (node_set_t::iterator it = node_set.begin(); it != node_set.end();
                it++) {
            SgGraphNode* graph_node = *it;
            if (graph_node != NULL) {
                SgNode* ast_node = graph_node->get_SgNode();
                ast_map[ast_node] = graph_node;
            }
        }

        return 0;
    }

    int16_t _build_dom_tree() {
        bool changed;
        do {
            changed = false;
            node_set_t node_set = graph->computeNodeSet();

            for (node_set_t::iterator it = node_set.begin();
                    it != node_set.end(); it++) {
                SgGraphNode* node = *it;

                node_list_t predecessor_list;
                graph->getPredecessors(node, predecessor_list);

                // Skip the entry node.
                if (predecessor_list.size() != 0) {
                    if (_process_node(node, predecessor_list) == true) {
                        changed = true;
                    }
                }
            }
        } while (changed == true);

        return 0;
    }

    void _intersect(node_set_t*& intersection, node_set_t*& dom_list) {
        // Check if there is anything to intersect with.
        if (dom_list == NULL) {
            return;
        }

        // If we are initializing intersection, then copy the dominator list.
        if (intersection == NULL) {
            intersection = new node_set_t();
            *intersection = *dom_list;
            return;
        }

        // Otherwise, we have to do the hard work.
        // Which, in fact, is made easy by the STL routines.

        node_set_t result;
        set_intersection(dom_list->begin(), dom_list->end(),
                intersection->begin(), intersection->end(),
                std::inserter(result, result.begin()));

        *intersection = result;
    }

    int16_t _process_node(SgGraphNode* node,
            const node_list_t& predecessor_list) {
        node_set_t* intersection = NULL;

        // Iterate over all predecessors, finding the intersection.
        for (node_list_t::const_iterator it = predecessor_list.begin();
                it != predecessor_list.end(); it++) {
            SgGraphNode* pred = *it;
            node_set_t* dom_list = dominators[pred];
            _intersect(intersection, dom_list);
        }

        if (intersection == NULL) {
            if (dominators[node] == NULL) {
                return false;
            } else {
                // Something seems to have gone wrong.
                assert(0 && "Empty dominator set for non-empty predecessors!");
            }
        } else {
            // Add itself to the list of dominators.
            if (std::find(intersection->begin(), intersection->end(), node)
                    == intersection->end()) {
                intersection->insert(node);
            }

            // Dominators may have changed.
            // Should we allocate memory?
            if (dominators[node] == NULL) {
                dominators[node] = new node_set_t();
            } else {
                // Have the dominators really changed?
                if (*intersection == *(dominators[node])) {
                    return false;
                }
            }

            *(dominators[node]) = *intersection;

            intersection->clear();
            delete intersection;

            return true;
        }

        // Should not fall through here.
        assert(0 && "Unreachable code executed!");
    }

    int8_t _initialize_doms(SgIncidenceDirectedGraph* graph) {
        node_set_t node_set = graph->computeNodeSet();
        size_t node_count = graph->numberOfGraphNodes();

        for (node_set_t::iterator it = node_set.begin(); it != node_set.end();
                it++) {
            SgGraphNode* node = *it;
            node_set_t* node_list = NULL;

            node_list_t predecessor_list;
            graph->getPredecessors(node, predecessor_list);

            if (predecessor_list.size() == 0) {
                // The entry node is special,
                // add itself to the dominator list.
                node_list = new node_set_t();
                if (node_list == NULL) {
                    return -1;
                }

                node_list->insert(node);
            } else {
                // Let the dominator list be empty,
                // signifying all nodes in CFG.
            }

            dominators[node] = node_list;
            immediate_dominators[node] = NULL;
        }

        return 0;
    }
};

#endif  // TOOLS_MACPO_INST_INCLUDE_DOM_TREE_H_
