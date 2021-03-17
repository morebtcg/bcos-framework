/*
 *  Copyright (C) 2021 FISCO BCOS.
 *  SPDX-License-Identifier: Apache-2.0
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 * @file: TestPromptFixture.h
 * fixture class for test case prompt
 */

#pragma once
#include <bcos-framework/libutilities/Common.h>
#include <boost/filesystem.hpp>

namespace bcos
{
namespace test
{
class TestPrompt
{
public:
    static TestPrompt& get()
    {
        static TestPrompt instance;
        return instance;
    }
    TestPrompt(TestPrompt const&) = delete;
    void operator=(TestPrompt const&) = delete;
    /**
     * @brief : init every test-suite by printting the name of cases
     * @param _maxTests
     */
    void initTest(size_t _maxTests = 1);

    /**
     * @brief : release resources when test-suite finished
     */
    void finishTest();

    void setCurrentTestFile(boost::filesystem::path const& _name) { m_currentTestFileName = _name; }
    void setCurrentTestName(std::string const& _name) { m_currentTestName = _name; }
    std::string const& testName() { return m_currentTestName; }
    std::string const& caseName() { return m_currentTestCaseName; }
    boost::filesystem::path const& testFile() { return m_currentTestFileName; }

private:
    TestPrompt() {}
    size_t m_currTest;
    size_t m_maxTests;
    std::string m_currentTestName;
    std::string m_currentTestCaseName;
    boost::filesystem::path m_currentTestFileName;
    typedef std::pair<double, std::string> execTimeName;
    std::vector<execTimeName> m_execTimeResults;
    int64_t m_startTime;
};

class TestPromptFixture
{
public:
    // init test-suite fixture
    TestPromptFixture() { TestPrompt::get().initTest(); }
    // release test-suite fixture
    ~TestPromptFixture() { TestPrompt::get().finishTest(); }
};
}  // namespace test
}  // namespace bcos
