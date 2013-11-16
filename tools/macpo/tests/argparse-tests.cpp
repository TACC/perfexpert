
#include "generic_defs.h"
#include "argparse.h"

#include "gtest/gtest.h"

// Tests argument processing
TEST(ArgParse, Empty) {
    char argument[10] = {0};
    options_t options;

    EXPECT_EQ(argparse::parse_arguments(argument, options), -1);
}

TEST(ArgParse, Valid) {
    char argument[128] = {0};
    options_t options;

    snprintf(argument, 128, "--macpo:no-compile");
    EXPECT_EQ(argparse::parse_arguments(argument, options), 0);
    EXPECT_EQ(options.no_compile, true);
    EXPECT_EQ(options.action, ACTION_NONE);
    EXPECT_EQ(options.function_name, "");
    EXPECT_EQ(options.line_number, -1);

    snprintf(argument, 128, "--macpo:instrument=foo");
    EXPECT_EQ(argparse::parse_arguments(argument, options), 0);
    EXPECT_EQ(options.no_compile, false);
    EXPECT_EQ(options.action, ACTION_INSTRUMENT);
    EXPECT_EQ(options.function_name, "foo");
    EXPECT_EQ(options.line_number, -1);

    snprintf(argument, 128, "--macpo:instrument=foo#13");
    EXPECT_EQ(argparse::parse_arguments(argument, options), 0);
    EXPECT_EQ(options.no_compile, false);
    EXPECT_EQ(options.action, ACTION_INSTRUMENT);
    EXPECT_EQ(options.function_name, "foo");
    EXPECT_EQ(options.line_number, 13);

    snprintf(argument, 128, "--macpo:check-alignment=foo");
    EXPECT_EQ(argparse::parse_arguments(argument, options), 0);
    EXPECT_EQ(options.no_compile, false);
    EXPECT_EQ(options.action, ACTION_ALIGNCHECK);
    EXPECT_EQ(options.function_name, "foo");
    EXPECT_EQ(options.line_number, -1);

    snprintf(argument, 128, "--macpo:check-alignment=foo#15");
    EXPECT_EQ(argparse::parse_arguments(argument, options), 0);
    EXPECT_EQ(options.no_compile, false);
    EXPECT_EQ(options.action, ACTION_ALIGNCHECK);
    EXPECT_EQ(options.function_name, "foo");
    EXPECT_EQ(options.line_number, 15);
}

TEST(ArgParse, Invalid) {
    char argument[128] = {0};
    options_t options;

    snprintf(argument, 128, "--macpo");
    EXPECT_EQ(argparse::parse_arguments(argument, options), -1);

    snprintf(argument, 128, "--macpo:");
    EXPECT_EQ(argparse::parse_arguments(argument, options), -1);

    snprintf(argument, 128, "--macpo:instrument");
    EXPECT_EQ(argparse::parse_arguments(argument, options), -1);

    snprintf(argument, 128, "--macpo:instrument=");
    EXPECT_EQ(argparse::parse_arguments(argument, options), -1);

    snprintf(argument, 128, "--macpo:check-alignment");
    EXPECT_EQ(argparse::parse_arguments(argument, options), -1);

    snprintf(argument, 128, "--macpo:check-alignment=");
    EXPECT_EQ(argparse::parse_arguments(argument, options), -1);

    snprintf(argument, 128, "--:instrument");
    EXPECT_EQ(argparse::parse_arguments(argument, options), -1);

    snprintf(argument, 128, "--");
    EXPECT_EQ(argparse::parse_arguments(argument, options), -1);

    snprintf(argument, 128, "--:");
    EXPECT_EQ(argparse::parse_arguments(argument, options), -1);
}
