
#include "generic_defs.h"
#include "histogram.h"

#include "gtest/gtest.h"

TEST(libmrt, HistogramSort) {
    histogram_t<int64_t, int64_t> hist;

    hist.increment(13, 20);
    hist.increment(3, 15);
    hist.increment(13, 10);
    hist.increment(3, 20);

    histogram_t<int64_t, int64_t>::pair_list_t pair_list = hist.sort();
    EXPECT_EQ(pair_list.size(), 2);

    histogram_t<int64_t, int64_t>::pair_t pair = pair_list[0];
    EXPECT_EQ(pair.first, 3);
    EXPECT_EQ(pair.second, 35);

    pair = pair_list[1];
    EXPECT_EQ(pair.first, 13);
    EXPECT_EQ(pair.second, 30);
}
