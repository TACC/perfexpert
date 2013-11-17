
#include "generic_defs.h"
#include "ir_methods.h"

#include "gtest/gtest.h"

TEST(ReferenceIndex, EmptyStream) {
    reference_list_t reference_list;
    std::string stream_name = "";

    EXPECT_LT(ir_methods::get_reference_index(reference_list, stream_name), 0);
}

TEST(ReferenceIndex, NonExistedStream) {
    reference_list_t reference_list;
    std::string stream_name = "abc";

    EXPECT_EQ(ir_methods::get_reference_index(reference_list, stream_name), 0);
}

TEST(ReferenceIndex, ExistingStream) {
    reference_list_t reference_list;
    reference_info_t reference_info[2];

    reference_info[0].name = "abc";
    reference_info[1].name = "def";

    reference_list.push_back(reference_info[0]);
    reference_list.push_back(reference_info[1]);

    std::string stream_name = "def";

    EXPECT_EQ(ir_methods::get_reference_index(reference_list, stream_name), 1);
}

TEST(StripIndexExpr, CppArray) {
    std::string stream = "abc[1234]";
    EXPECT_EQ(ir_methods::strip_index_expr(stream), "abc");
}

TEST(StripIndexExpr, CppStruct) {
    std::string stream = "abc.def";
    EXPECT_EQ(ir_methods::strip_index_expr(stream), "abc.def");
}

TEST(StripIndexExpr, CppArrayOfStruct) {
    // If it does not end in an array, leave the index expr as it is.
    std::string stream = "abc[j].def";
    EXPECT_EQ(ir_methods::strip_index_expr(stream), "abc[j].def");
}

TEST(StripIndexExpr, CppStructOfArray) {
    std::string stream = "abc.def[k]";
    EXPECT_EQ(ir_methods::strip_index_expr(stream), "abc.def");
}

TEST(StripIndexExpr, FortranArray) {
    std::string stream = "abc(1234)";
    EXPECT_EQ(ir_methods::strip_index_expr(stream), "abc");
}
