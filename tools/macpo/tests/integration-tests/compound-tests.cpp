
#include <rose.h>

#include <string>

#include "gtest/gtest.h"
#include "itest_harness.h"

TEST(CompoundTests, AlignCheckTwice) {
    options_t options;

    std::string tests_dir = get_tests_directory();
    std::string input_file = tests_dir + "/file_008.c";

    options.add_location(ACTION_ALIGNCHECK, "main");
    options.add_location(ACTION_ALIGNCHECK, "compute");

    std::string binary_file = instrument_and_link(input_file, NULL, options);
    ASSERT_TRUE(file_exists(binary_file));
    ASSERT_TRUE(verify_output(input_file, binary_file));

    remove(binary_file.c_str());
}

TEST(CompoundTests, AlignCheckAndOverlapCheck) {
    options_t options;

    std::string tests_dir = get_tests_directory();
    std::string input_file = tests_dir + "/file_009.c";

    options.add_location(ACTION_ALIGNCHECK, "main");
    options.add_location(ACTION_OVERLAPCHECK, "compute");

    std::string binary_file = instrument_and_link(input_file, NULL, options);
    ASSERT_TRUE(file_exists(binary_file));
    ASSERT_TRUE(verify_output(input_file, binary_file));

    remove(binary_file.c_str());
}
