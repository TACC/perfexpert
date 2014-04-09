
#include <rose.h>

#include <string>

#include "gtest/gtest.h"
#include "itest_harness.h"

TEST(BasicTests, NoActions) {
    options_t options = {0};
    std::string tests_dir = get_tests_directory();
    std::string input_file = tests_dir + "/file_001.c";

    std::string binary_file = instrument_and_link(input_file, NULL, options);
    ASSERT_TRUE(file_exists(binary_file));
    ASSERT_TRUE(verify_output(input_file, binary_file));

    remove(binary_file.c_str());
}

TEST(BasicTests, BasicInstrumentation) {
    options_t options = {0};
    options.action = ACTION_INSTRUMENT;
    options.function_name = "main";
    std::string tests_dir = get_tests_directory();
    std::string input_file = tests_dir + "/file_002.c";

    std::string binary_file = instrument_and_link(input_file, NULL, options);
    ASSERT_TRUE(file_exists(binary_file));
    ASSERT_TRUE(verify_output(input_file, binary_file));

    remove(binary_file.c_str());
}

TEST(BasicTests, SmallMatmult) {
    options_t options = {0};
    options.action = ACTION_INSTRUMENT;
    options.function_name = "compute";
    std::string tests_dir = get_tests_directory();
    std::string input_file = tests_dir + "/file_003.c";

    std::string binary_file = instrument_and_link(input_file, NULL, options);
    ASSERT_TRUE(file_exists(binary_file));
    ASSERT_TRUE(verify_output(input_file, binary_file));

    remove(binary_file.c_str());
}

TEST(BasicTests, OverlapCheck) {
    options_t options = {0};
    options.action = ACTION_OVERLAPCHECK;
    options.function_name = "compute";
    std::string tests_dir = get_tests_directory();
    std::string input_file = tests_dir + "/file_004.c";

    std::string binary_file = instrument_and_link(input_file, NULL, options);
    ASSERT_TRUE(file_exists(binary_file));
    ASSERT_TRUE(verify_output(input_file, binary_file));

    remove(binary_file.c_str());
}

TEST(BasicTests, StrideCheck) {
    options_t options = {0};
    options.action = ACTION_STRIDECHECK;
    options.function_name = "compute";
    std::string tests_dir = get_tests_directory();
    std::string input_file = tests_dir + "/file_005.c";

    std::string binary_file = instrument_and_link(input_file, NULL, options);
    ASSERT_TRUE(file_exists(binary_file));
    ASSERT_TRUE(verify_output(input_file, binary_file));

    remove(binary_file.c_str());
}

TEST(BasicTests, CompoundCheck) {
    options_t options = {0};
    options.action = ACTION_OVERLAPCHECK | ACTION_STRIDECHECK;
    options.function_name = "compute";
    std::string tests_dir = get_tests_directory();
    std::string input_file = tests_dir + "/file_006.c";

    std::string binary_file = instrument_and_link(input_file, NULL, options);
    ASSERT_TRUE(file_exists(binary_file));
    ASSERT_TRUE(verify_output(input_file, binary_file));

    remove(binary_file.c_str());
}
