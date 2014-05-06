
#include "generic_defs.h"
#include "argparse.h"

#include "gtest/gtest.h"

TEST(ArgParse, InitOptions) {
    options_t options;

    EXPECT_EQ(options.no_compile, false);
    EXPECT_EQ(options.location_list.size(), 0);
    EXPECT_EQ(options.backup_filename, "");
}

TEST(ArgParse, Empty) {
    char argument[10] = {0};
    options_t options;

    EXPECT_EQ(argparse::parse_arguments(argument, options), -1);
}

TEST(ArgParse, ValidNoCompile) {
    char argument[128];
    options_t options;

    snprintf(argument, sizeof(argument), "--macpo:no-compile");
    EXPECT_EQ(argparse::parse_arguments(argument, options), 0);

    EXPECT_EQ(options.no_compile, true);
}

TEST(ArgParse, ValidInstrumentFunction) {
    char argument[128];
    options_t options;

    snprintf(argument, sizeof(argument), "--macpo:instrument=foo");
    EXPECT_EQ(argparse::parse_arguments(argument, options), 0);

    EXPECT_EQ(options.location_list.size(), 1);
    EXPECT_EQ(options.get_action("foo"), ACTION_INSTRUMENT);
}

TEST(ArgParse, ValidInstrumentLoop) {
    char argument[128];
    options_t options;

    snprintf(argument, sizeof(argument), "--macpo:instrument=foo:13");
    EXPECT_EQ(argparse::parse_arguments(argument, options), 0);

    EXPECT_EQ(options.location_list.size(), 1);
    EXPECT_EQ(options.get_action("foo", 13), ACTION_INSTRUMENT);
}

TEST(ArgParse, ValidAlignCheckFunction) {
    char argument[128];
    options_t options;

    snprintf(argument, sizeof(argument), "--macpo:check-alignment=foo");
    EXPECT_EQ(argparse::parse_arguments(argument, options), 0);

    EXPECT_EQ(options.location_list.size(), 1);
    EXPECT_EQ(options.get_action("foo"), ACTION_ALIGNCHECK);
}

TEST(ArgParse, ValidAlignCheckLoop) {
    char argument[128];
    options_t options;

    snprintf(argument, sizeof(argument), "--macpo:check-alignment=foo:15");
    EXPECT_EQ(argparse::parse_arguments(argument, options), 0);

    EXPECT_EQ(options.location_list.size(), 1);
    EXPECT_EQ(options.get_action("foo", 15), ACTION_ALIGNCHECK);
}

TEST(ArgParse, ValidMultipleAlignCheckLoop) {
    char argument[128];
    options_t options;

    snprintf(argument, sizeof(argument), "--macpo:check-alignment=foo:15");
    EXPECT_EQ(argparse::parse_arguments(argument, options), 0);

    snprintf(argument, sizeof(argument), "--macpo:check-alignment=bar:32");
    EXPECT_EQ(argparse::parse_arguments(argument, options), 0);

    snprintf(argument, sizeof(argument), "--macpo:check-alignment=foobar.c");
    EXPECT_EQ(argparse::parse_arguments(argument, options), 0);

    EXPECT_EQ(options.location_list.size(), 3);
    EXPECT_EQ(options.get_action("foo", 15), ACTION_ALIGNCHECK);
    EXPECT_EQ(options.get_action("bar", 32), ACTION_ALIGNCHECK);
    EXPECT_EQ(options.get_action("foobar.c"), ACTION_ALIGNCHECK);
}

TEST(ArgParse, ValidMultipleDifferentChecks) {
    char argument[128];
    options_t options;

    snprintf(argument, sizeof(argument), "--macpo:instrument=foo:15");
    EXPECT_EQ(argparse::parse_arguments(argument, options), 0);

    snprintf(argument, sizeof(argument), "--macpo:check-alignment=bar:32");
    EXPECT_EQ(argparse::parse_arguments(argument, options), 0);

    EXPECT_EQ(options.location_list.size(), 2);
    EXPECT_EQ(options.get_action("foo", 15), ACTION_INSTRUMENT);
    EXPECT_EQ(options.get_action("bar", 32), ACTION_ALIGNCHECK);
}

TEST(ArgParse, LocationCheckFunctions) {
    char argument[128];
    options_t options;

    snprintf(argument, sizeof(argument), "--macpo:check-alignment=foo");
    EXPECT_EQ(argparse::parse_arguments(argument, options), 0);

    snprintf(argument, sizeof(argument), "--macpo:check-alignment=bar.c:35");
    EXPECT_EQ(argparse::parse_arguments(argument, options), 0);

    EXPECT_EQ(options.location_list.size(), 2);

    location_t location_foo;
    location_foo.line_number = 0;
    location_foo.function_name = "foo";

    EXPECT_EQ(options.get_action("foo"), ACTION_ALIGNCHECK);
    EXPECT_EQ(options.get_action("foo", 0), ACTION_ALIGNCHECK);
    EXPECT_EQ(options.get_action(location_foo), ACTION_ALIGNCHECK);

    location_t location_bar;
    location_bar.line_number = 35;
    location_bar.function_name = "bar.c";
    EXPECT_EQ(options.get_action("bar.c"), ACTION_NONE);
    EXPECT_EQ(options.get_action("bar.c", 35), ACTION_ALIGNCHECK);
    EXPECT_EQ(options.get_action(location_bar), ACTION_ALIGNCHECK);
}

TEST(ArgParse, LocationAddFunctions) {
    options_t options;

    options.add_location(ACTION_NONE, "foo");
    EXPECT_EQ(options.get_action("foo"), ACTION_NONE);

    options.reset();
    options.add_location(ACTION_ALIGNCHECK, "foo", 34);
    EXPECT_EQ(options.get_action("foo", 34), ACTION_ALIGNCHECK);

    location_t location;
    location.line_number = 35;
    location.function_name = "bar.c";

    options.reset();
    options.add_location(ACTION_INSTRUMENT, location);
    EXPECT_EQ(options.get_action("bar.c", 35), ACTION_INSTRUMENT);
}

TEST(ArgParse, ValidBackup) {
    char argument[128];
    options_t options;

    snprintf(argument, sizeof(argument), "--macpo:backup-filename=foobar");
    EXPECT_EQ(argparse::parse_arguments(argument, options), 0);

    EXPECT_EQ(options.backup_filename, "foobar");
}

TEST(ArgParse, InvalidOption01) {
    char argument[128] = {0};
    options_t options;

    snprintf(argument, sizeof(argument), "--macpo");
    EXPECT_EQ(argparse::parse_arguments(argument, options), -1);
}

TEST(ArgParse, InvalidOption02) {
    char argument[128] = {0};
    options_t options;

    snprintf(argument, sizeof(argument), "--macpo:");
    EXPECT_EQ(argparse::parse_arguments(argument, options), -1);
}

TEST(ArgParse, NoLocation01) {
    char argument[128] = {0};
    options_t options;

    snprintf(argument, sizeof(argument), "--macpo:instrument");
    EXPECT_EQ(argparse::parse_arguments(argument, options), -1);
}

TEST(ArgParse, NoLocation02) {
    char argument[128] = {0};
    options_t options;

    snprintf(argument, sizeof(argument), "--macpo:instrument=");
    EXPECT_EQ(argparse::parse_arguments(argument, options), -1);
}
