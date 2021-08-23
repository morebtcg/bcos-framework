#include "libutilities/TarsLog.h"
#include <boost/test/unit_test.hpp>

enum TarsRollLogger
{
    DEBUG_LOG,
    INFO_LOG,
    WARN_LOG,
    ERROR_LOG
};

namespace bcos::test
{
class Log
{
public:
    bool IsNeedLog(TarsRollLogger level)
    {
        (void)level;
        return true;
    }

    std::ostream& log(TarsRollLogger level)
    {
        (void)level;
        return std::cout;
    }
};
std::shared_ptr<Log> LOG;

struct LogTestFixture
{
    LogTestFixture() { LOG = std::make_shared<Log>(); }

    ~LogTestFixture() {}
};

BOOST_FIXTURE_TEST_SUITE(LogTest, LogTestFixture)
BOOST_AUTO_TEST_CASE(testLogOut)
{
    BCOS_LOG(DEBUG) << "hello world!" << " fisco bcos!";
}

BOOST_AUTO_TEST_SUITE_END()
}  // namespace bcos::test