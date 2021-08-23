#pragma once

#ifndef TARS_LOG_TEST
#include "servant/TarsLogger.h"
#endif
#include <iostream>

#define LOG_BADGE(_NAME) "[" << (_NAME) << "]"
#define LOG_TYPE(_TYPE) (_TYPE) << "|"
#define LOG_DESC(_DESCRIPTION) (_DESCRIPTION)
#define LOG_KV(_K, _V) "," << (_K) << "=" << (_V)

#define TRACE LocalRollLogger::DEBUG_LOG
#define DEBUG LocalRollLogger::DEBUG_LOG
#define INFO LocalRollLogger::INFO_LOG
#define WARNING LocalRollLogger::WARN_LOG
#define ERROR LocalRollLogger::ERROR_LOG
#define FATAL LocalRollLogger::ERROR_LOG

template <class Logger>
class EndlHelper
{
public:
    EndlHelper(Logger&& logger) : m_logger(std::move(logger)) {}

    template <class Message>
    Logger& operator<<(const Message& message)
    {
        return (m_logger << message);
    }

    ~EndlHelper() { m_logger << std::endl; }

private:
    Logger m_logger;
};

#define BCOS_LOG(level)        \
    if (LOG->isNeedLog(level)) \
    EndlHelper<tars::LoggerStream>(LOG->log(level))
#define BCOS_STAT_LOG(level)   \
    if (LOG->isNeedLog(level)) \
    EndlHelper<tars::LoggerStream>(LOG->log(level))
