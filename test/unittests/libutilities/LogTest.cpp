#define TARS_LOG_TEST

#include "libutilities/TarsLog.h"
#include <boost/test/unit_test.hpp>
#include <iostream>

enum LocalRollLogger
{
    DEBUG_LOG,
    INFO_LOG,
    WARN_LOG,
    ERROR_LOG
};

namespace tars
{
class LoggerStreamFactory
{
public:
    bool isNeedLog(LocalRollLogger level)
    {
        (void)level;
        return true;
    }

    std::stringstream log(LocalRollLogger level)
    {
        (void)level;
        std::stringstream ss;
        return ss;
    }
};

using LoggerStream = std::ostream;
}  // namespace tars


namespace bcos::test
{
std::shared_ptr<tars::LoggerStreamFactory> LOG;

struct LogTestFixture
{
    LogTestFixture() { LOG = std::make_shared<tars::LoggerStreamFactory>(); }

    ~LogTestFixture() {}
};


BOOST_FIXTURE_TEST_SUITE(LogTest, LogTestFixture)
BOOST_AUTO_TEST_CASE(testLogOut)
{
    /*
    BCOS_LOG(DEBUG) << "hello world!"
                    << " fisco bcos!";
                    */
}

BOOST_AUTO_TEST_SUITE_END()
}  // namespace bcos::test