#pragma once

#include "../../libutilities/Common.h"
#include "../../libutilities/ConcurrentCOW.h"
#include "../../libutilities/Error.h"
#include "../protocol/ProtocolTypeDef.h"
#include "Common.h"
#include <boost/exception/diagnostic_information.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/throw_exception.hpp>
#include <exception>

namespace bcos::storage
{
class Entry
{
public:
    enum Status
    {
        NORMAL = 0,
        DELETED = 1
    };

    Entry() : m_data({std::vector<std::string>(), 0}) {}

    explicit Entry(TableInfo::ConstPtr tableInfo, protocol::BlockNumber _num = 0)
      : m_data({std::vector<std::string>(), 0}), m_tableInfo(std::move(tableInfo)), m_num(_num)
    {}

    Entry(const Entry&) = default;
    Entry(Entry&&) = default;
    bcos::storage::Entry& operator=(const Entry&) = default;
    bcos::storage::Entry& operator=(Entry&&) = default;

    virtual ~Entry() {}

    std::string_view getField(size_t index) const
    {
        auto& fields = m_data.get()->fields;
        if (index >= fields.size())
        {
            BOOST_THROW_EXCEPTION(
                BCOS_ERROR(-1, "Get field index: " + boost::lexical_cast<std::string>(index) +
                                   " failed, index out of range: " +
                                   boost::lexical_cast<std::string>(fields.size())));
        }

        return fields[index];
    }

    std::string_view getField(const std::string_view& field) const
    {
        if (!m_tableInfo)
        {
            BOOST_THROW_EXCEPTION(
                BCOS_ERROR(-1, "Get field: " + std::string(field) + " error, tableInfo is null"));
        }

        auto index = m_tableInfo->fieldIndex(field);
        return getField(index);
    }

    void setField(size_t index, std::string value)
    {
        if (index >= m_data.get()->fields.size())
        {
            BOOST_THROW_EXCEPTION(
                BCOS_ERROR(-1, "Set field index: " + boost::lexical_cast<std::string>(index) +
                                   " failed, index out of range: " +
                                   boost::lexical_cast<std::string>(m_data.get()->fields.size())));
        }

        auto data = m_data.mutableGet();
        auto& field = (data->fields)[index];

        ssize_t updatedCapacity = value.size() - field.size();

        data->fields[index] = std::move(value);
        data->capacityOfHashField += updatedCapacity;
        m_dirty = true;
    }

    void setField(const std::string_view& field, std::string value)
    {
        if (!m_tableInfo)
        {
            BOOST_THROW_EXCEPTION(
                BCOS_ERROR(-1, "Set field: " + std::string(field) + " error, tableInfo is null"));
        }

        auto index = m_tableInfo->fieldIndex(field);

        auto data = m_data.mutableGet();
        if (data->fields.size() < m_tableInfo->fields().size())
        {
            data->fields.resize(m_tableInfo->fields().size());
        }

        auto& fieldValue = (data->fields)[index];

        ssize_t updatedCapacity = value.size() - fieldValue.size();
        fieldValue = std::move(value);
        data->capacityOfHashField += updatedCapacity;
        m_dirty = true;
    }

    virtual std::vector<std::string>::const_iterator begin() const noexcept
    {
        return m_data.get()->fields.cbegin();
    }
    virtual std::vector<std::string>::const_iterator end() const noexcept
    {
        return m_data.get()->fields.cend();
    }

    bool rollbacked() const noexcept { return m_rollbacked; }
    void setRollbacked(bool _rollbacked) noexcept { m_rollbacked = _rollbacked; }
    Status status() const noexcept { return m_status; }

    void setStatus(Status status) noexcept
    {
        m_status = status;
        m_dirty = true;
    }

    protocol::BlockNumber num() const noexcept { return m_num; }
    void setNum(protocol::BlockNumber num) noexcept { m_num = num; }

    bool dirty() const noexcept { return m_dirty; }
    void setDirty(bool dirty) noexcept { m_dirty = dirty; }

    ssize_t capacityOfHashField() const noexcept
    {  // the capacity is used to calculate gas, must return the same value in different DB
        return m_data.get()->capacityOfHashField;
    }

    size_t version() const noexcept { return m_version; }
    void setVersion(size_t version) noexcept { m_version = version; }

    ssize_t refCount() const noexcept { return m_data.refCount(); }

    const std::vector<std::string>& fields() const noexcept { return m_data.get()->fields; }

    void importFields(std::vector<std::string> input) noexcept
    {
        auto data = m_data.mutableGet();
        data->capacityOfHashField = 0;
        for (auto& value : input)
        {
            data->capacityOfHashField += value.size();
        }

        data->fields = std::move(input);
        m_dirty = true;
    }

    std::vector<std::string>&& exportFields() noexcept
    {
        auto data = m_data.mutableGet();
        data->capacityOfHashField = 0;
        return std::move(data->fields);
    }

    TableInfo::ConstPtr tableInfo() const { return m_tableInfo; }
    void setTableInfo(TableInfo::ConstPtr tableInfo) { m_tableInfo = std::move(tableInfo); }

    bool valid() const noexcept { return ((m_status != Status::DELETED) && (!m_rollbacked)); }

private:
    struct Data
    {
        std::vector<std::string> fields;
        ssize_t capacityOfHashField;
    };

    // should serialization
    Status m_status = Status::NORMAL;
    bcos::ConcurrentCOW<Data> m_data;

    // no need to serialization
    TableInfo::ConstPtr m_tableInfo;
    protocol::BlockNumber m_num = 0;
    size_t m_version = 0;
    bool m_dirty = false;
    bool m_rollbacked = false;
};
}  // namespace bcos::storage