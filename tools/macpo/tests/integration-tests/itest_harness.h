
#ifndef TOOLS_MACPO_TESTS_INTEGRATION_TESTS_ITEST_HARNESS_H_
#define TOOLS_MACPO_TESTS_INTEGRATION_TESTS_ITEST_HARNESS_H_

#include <string>
#include <vector>

#include "minst.h"

typedef std::vector<std::string> string_list_t;

bool file_exists(const std::string& filename);
bool verify_output(std::string& filename, std::string& binary);

std::string get_src_directory();
std::string get_build_directory();
std::string get_tests_directory();
std::string instrument_and_link(std::string input_file,
        string_list_t* special_args, options_t& options);

static void print_args(const string_list_t& args);
static void populate_args(string_list_t& args, std::string& input_file,
        std::string& output_file, string_list_t* special_args);
static std::string get_process_stderr(std::string& binary_name);
static void split(const std::string& string, char delim, string_list_t& list);
static std::string get_file_contents(std::string& filename);

#endif  // TOOLS_MACPO_TESTS_INTEGRATION_TESTS_ITEST_HARNESS_H_
