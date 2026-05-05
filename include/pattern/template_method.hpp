#pragma once
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Template Method Pattern
//
// Intent: Define the skeleton of an algorithm in a base class, deferring
// some steps to subclasses. Template Method lets subclasses redefine
// certain steps of an algorithm without changing the algorithm's structure.
//
// Real-world analogy: A data-mining report pipeline. The steps are always:
// open file → extract data → parse data → analyse data → send report.
// Concrete miners (CSV, PDF, XML) override only the file-format-specific
// steps; the pipeline structure never changes.
//
// Key C++ mechanics used:
//  - Non-virtual public interface (NVI): the "template method" is public and
//    non-virtual; the customisation hooks are protected and virtual.
//  - `final` on the template method prevents accidental overrides.
//  - Optional hooks have a default (empty) implementation so subclasses can
//    selectively override only what they need.
//
// When to use:
//  - You want to implement the invariant parts of an algorithm once and leave
//    it up to subclasses to fill in the behaviour that can vary.
//  - Several classes contain nearly identical algorithms with minor differences.
// ---------------------------------------------------------------------------

namespace pattern
{

// ---------- Abstract class with template method --------------------------

class DataMiner
{
public:
    virtual ~DataMiner() = default;

    // Template method — defines the algorithm skeleton
    virtual std::string mine(const std::string& path) final
    {
        openFile(path);
        extractData();
        parseData();
        analyseData();
        std::string report = buildReport();
        closeFile();
        return report;
    }

    // Accessors for test inspection
    const std::vector<std::string>& rawLines() const
    {
        return rawLines_;
    }
    const std::vector<std::string>& parsedLines() const
    {
        return parsedLines_;
    }

protected:
    // Primitive operations (must override)
    virtual void openFile(const std::string& path) = 0;
    virtual void extractData()                     = 0;
    virtual void parseData()                       = 0;

    // Hook — concrete behaviour; subclasses may override
    virtual void analyseData()
    {
        analysisNote_ = "(default analysis)";
    }

    virtual void closeFile()
    {
    } // optional hook, default no-op

    virtual std::string buildReport() const
    {
        return "Report[" + analysisNote_ + "]: " + std::to_string(parsedLines_.size()) + " records";
    }

    std::vector<std::string> rawLines_;
    std::vector<std::string> parsedLines_;
    std::string              analysisNote_;
};

// ---------- Concrete: CSV miner ------------------------------------------

class CsvMiner : public DataMiner
{
protected:
    void openFile(const std::string& path) override
    {
        // Simulate reading a CSV: split "path" by semicolons for testing
        rawLines_.clear();
        std::string token;
        for (char c : path)
        {
            if (c == ';')
            {
                rawLines_.push_back(token);
                token.clear();
            }
            else
                token += c;
        }
        if (!token.empty())
            rawLines_.push_back(token);
    }

    void extractData() override
    {
    } // raw data already in rawLines_

    void parseData() override
    {
        parsedLines_.clear();
        for (const auto& line : rawLines_)
            parsedLines_.push_back("csv:" + line);
    }

    void analyseData() override
    {
        analysisNote_ = "CSV analysis (" + std::to_string(parsedLines_.size()) + " rows)";
    }
};

// ---------- Concrete: JSON miner -----------------------------------------

class JsonMiner : public DataMiner
{
protected:
    void openFile(const std::string& path) override
    {
        rawLines_ = {path}; // treat entire string as one "document"
    }

    void extractData() override
    {
    }

    void parseData() override
    {
        parsedLines_.clear();
        parsedLines_.push_back("json:" + rawLines_[0]);
    }
    // Inherits default analyseData() and closeFile()
};

} // namespace pattern
