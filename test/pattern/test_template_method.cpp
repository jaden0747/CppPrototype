#include <gtest/gtest.h>
#include "pattern/template_method.hpp"

using namespace pattern;

// ---------------------------------------------------------------------------
// CsvMiner
// ---------------------------------------------------------------------------
TEST(TemplateMethod, CsvMinerParsesRows)
{
    CsvMiner miner;
    miner.mine("a;b;c");
    EXPECT_EQ(3u, miner.parsedLines().size());
}

TEST(TemplateMethod, CsvMinerParsedLinesHavePrefix)
{
    CsvMiner miner;
    miner.mine("hello;world");
    EXPECT_EQ("csv:hello", miner.parsedLines()[0]);
    EXPECT_EQ("csv:world", miner.parsedLines()[1]);
}

TEST(TemplateMethod, CsvMinerReportContainsRowCount)
{
    CsvMiner miner;
    std::string report = miner.mine("a;b;c");
    EXPECT_NE(std::string::npos, report.find("3"));
}

TEST(TemplateMethod, CsvMinerReportContainsAnalysisNote)
{
    CsvMiner miner;
    std::string report = miner.mine("x;y");
    EXPECT_NE(std::string::npos, report.find("CSV analysis"));
}

TEST(TemplateMethod, CsvMinerSingleLine)
{
    CsvMiner miner;
    miner.mine("only");
    EXPECT_EQ(1u, miner.parsedLines().size());
    EXPECT_EQ("csv:only", miner.parsedLines()[0]);
}

// ---------------------------------------------------------------------------
// JsonMiner
// ---------------------------------------------------------------------------
TEST(TemplateMethod, JsonMinerParsesOneRecord)
{
    JsonMiner miner;
    miner.mine("{\"key\":\"value\"}");
    EXPECT_EQ(1u, miner.parsedLines().size());
}

TEST(TemplateMethod, JsonMinerParsedLineHasPrefix)
{
    JsonMiner miner;
    miner.mine("doc");
    EXPECT_EQ("json:doc", miner.parsedLines()[0]);
}

TEST(TemplateMethod, JsonMinerReportUsesDefaultAnalysis)
{
    JsonMiner miner;
    std::string report = miner.mine("{}");
    EXPECT_NE(std::string::npos, report.find("default analysis"));
}

// ---------------------------------------------------------------------------
// Template method is final — the skeleton is fixed
// ---------------------------------------------------------------------------
TEST(TemplateMethod, TemplateMethodAlwaysReturnsReport)
{
    CsvMiner  csv;
    JsonMiner json;
    EXPECT_FALSE(csv.mine("a").empty());
    EXPECT_FALSE(json.mine("b").empty());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
