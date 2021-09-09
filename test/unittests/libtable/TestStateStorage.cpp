/**
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
 * @brief Unit tests for the Table
 * @file Table.cpp
 */

#include "../../../testutils/TestPromptFixture.h"
#include "../libstorage/StateStorage.h"
#include "Hash.h"
#include "interfaces/storage/StorageInterface.h"
#include "libutilities/ThreadPool.h"
#include <tbb/concurrent_vector.h>
#include <boost/lexical_cast.hpp>
#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <future>
#include <iostream>
#include <optional>
#include <string>

using namespace std;
using namespace bcos;
using namespace bcos::storage;
using namespace bcos::crypto;

namespace std
{
inline ostream& operator<<(ostream& os, const std::optional<Entry>& entry)
{
    os << entry.has_value();
    return os;
}

inline ostream& operator<<(ostream& os, const std::optional<Table>& table)
{
    os << table.has_value();
    return os;
}

inline ostream& operator<<(ostream& os, const std::unique_ptr<Error>& error)
{
    os << error->what();
    return os;
}
}  // namespace std

namespace bcos
{
namespace test
{
struct TableFactoryFixture
{
    TableFactoryFixture()
    {
        hashImpl = make_shared<Header256Hash>();
        memoryStorage = make_shared<StateStorage>(nullptr, hashImpl, 0);
        BOOST_TEST(memoryStorage != nullptr);
        tableFactory = make_shared<StateStorage>(memoryStorage, hashImpl, m_blockNumber);
        BOOST_TEST(tableFactory != nullptr);
    }

    ~TableFactoryFixture() {}
    bool createDefaultTable()
    {
        std::promise<bool> createPromise;
        tableFactory->asyncCreateTable(
            testTableName, valueField, [&](std::optional<Error>&& error, bool success) {
                BOOST_CHECK(!error);
                createPromise.set_value(success);
            });
        return createPromise.get_future().get();
    }
    std::shared_ptr<crypto::Hash> hashImpl = nullptr;
    std::shared_ptr<StorageInterface> memoryStorage = nullptr;
    protocol::BlockNumber m_blockNumber = 0;
    std::shared_ptr<StateStorage> tableFactory = nullptr;
    std::string testTableName = "t_test";
    std::string keyField = "key";
    std::string valueField = "value";
};
BOOST_FIXTURE_TEST_SUITE(TableFactoryTest, TableFactoryFixture)

BOOST_AUTO_TEST_CASE(constructor)
{
    auto threadPool = ThreadPool("a", 1);
    auto tf = std::make_shared<StateStorage>(memoryStorage, nullptr, 0);
}

BOOST_AUTO_TEST_CASE(create_Table)
{
    std::string tableName("t_test1");
    auto table = tableFactory->openTable(tableName);

    BOOST_TEST(!table);
    auto ret = tableFactory->createTable(tableName, valueField);
    BOOST_TEST(ret == true);

    table = tableFactory->openTable(tableName);
    BOOST_TEST(table);

    ret = tableFactory->createTable(tableName, valueField);
    BOOST_TEST(ret == false);
}

BOOST_AUTO_TEST_CASE(rollback)
{
    auto ret = createDefaultTable();
    BOOST_TEST(ret == true);
    auto table = tableFactory->openTable(testTableName);

    auto deleteEntry = table->newEntry();
    deleteEntry.setStatus(Entry::DELETED);
    BOOST_CHECK_EQUAL(table->setRow("name", deleteEntry), true);

    auto entry = std::make_optional(table->newEntry());
    // entry->setField("key", "name");
    BOOST_CHECK_NO_THROW(entry->setField("value", "Lili"));
    entry->setVersion(1);
    BOOST_CHECK_EQUAL(table->setRow("name", *entry), true);
    entry = table->getRow("name");
    BOOST_TEST(entry);
    // BOOST_TEST(table->dirty() == true);
    BOOST_TEST(entry->dirty() == true);
    BOOST_TEST(entry->getField("value") == "Lili");

    auto savePoint = tableFactory->savepoint();

    entry = table->newEntry();
    // entry->setField("key", "id");
    entry->setField("value", "12345");
    table->setRow("id", *entry);
    entry = table->getRow("id");
    BOOST_TEST(entry);
    entry = table->getRow("name");
    BOOST_TEST(entry);
    // BOOST_TEST(table->dirty() == true);

    auto savePoint1 = tableFactory->savepoint();

    entry = table->newEntry();
    // entry->setField("key", "balance");
    entry->setField("value", "500");
    table->setRow("balance", *entry);

    entry = table->getRow("balance");
    BOOST_TEST(entry);

    entry = table->getRow("name");
    BOOST_TEST(entry);
    // BOOST_TEST(table->dirty() == true);

    auto savePoint2 = tableFactory->savepoint();

    auto deleteEntry2 = std::make_optional(table->newDeletedEntry());
    deleteEntry2->setVersion(entry->version() + 1);
    table->setRow("name", *deleteEntry2);

    // table->remove("name");
    entry = table->getRow("name");
    BOOST_CHECK_EQUAL(entry->status(), Entry::DELETED);

    tableFactory->rollback(savePoint2);
    entry = table->getRow("name");
    BOOST_CHECK_NE(entry->status(), Entry::DELETED);
    // BOOST_TEST(table->dirty() == true);

    tableFactory->rollback(savePoint1);
    entry = table->getRow("name");
    BOOST_TEST(entry);
    entry = table->getRow("balance");
    BOOST_TEST(!entry);
    // BOOST_TEST(table->dirty() == true);

    // auto dbHash0 = tableFactory->hash();
    tableFactory->rollback(savePoint);
    entry = table->getRow("name");
    BOOST_TEST(entry);
    entry = table->getRow("balance");
    BOOST_TEST(!entry);
    entry = table->getRow("id");
    BOOST_TEST(!entry);

    // insert without version
    entry = table->newEntry();
    entry->setField("value", "new record");
    BOOST_CHECK_EQUAL(table->setRow("id", *entry), true);
    // BOOST_TEST(table->dirty() == true);

    // tableFactory->commit();
    // TODO: add some ut, setRow remove rollback getRow setRow, setRow setRow remove rollback getRow
}

BOOST_AUTO_TEST_CASE(rollback2)
{
    auto hash0 = tableFactory->tableHashes();
    auto savePoint0 = tableFactory->savepoint();
    auto ret = createDefaultTable();
    BOOST_TEST(ret == true);
    auto table = tableFactory->openTable(testTableName);

    auto deleteEntry = table->newDeletedEntry();
    table->setRow("name", deleteEntry);
    auto entry = std::make_optional(table->newEntry());
    // entry->setField("key", "name");
    entry->setField("value", "Lili");
    entry->setVersion(1);
    table->setRow("name", *entry);
    entry = table->getRow("name");
    BOOST_TEST(entry);
    // BOOST_TEST(table->dirty() == true);
    BOOST_TEST(entry->dirty() == true);
    BOOST_TEST(entry->getField("value") == "Lili");

    auto savePoint = tableFactory->savepoint();

    entry = table->newEntry();
    // entry->setField("key", "id");
    entry->setField("value", "12345");
    table->setRow("id", *entry);
    entry = table->getRow("id");
    BOOST_TEST(entry);
    entry = table->getRow("name");
    BOOST_TEST(entry);
    // BOOST_TEST(table->dirty() == true);

    tableFactory->rollback(savePoint);
    entry = table->getRow("name");
    BOOST_TEST(entry);
    entry = table->getRow("balance");
    BOOST_TEST(!entry);
    entry = table->getRow("id");
    BOOST_TEST(!entry);
    // BOOST_TEST(table->dirty() == true);
    tableFactory->rollback(savePoint0);
    auto hash00 = tableFactory->tableHashes();
    // BOOST_CHECK_EQUAL_COLLECTIONS(hash0.begin(), hash0.end(), hash00.begin(), hash00.end());
    // BOOST_TEST(hash00 == hash0);
    table = tableFactory->openTable(testTableName);
    BOOST_TEST(!table);
}

BOOST_AUTO_TEST_CASE(hash)
{
    auto ret = createDefaultTable();
    BOOST_TEST(ret == true);
    auto table = tableFactory->openTable(testTableName);
    auto entry = std::make_optional(table->newEntry());
    // entry->setField("key", "name");
    entry->setField("value", "Lili");
    ret = table->setRow("name", *entry);
    BOOST_TEST(ret == true);
    entry = table->getRow("name");
    BOOST_TEST(entry);
    // BOOST_TEST(table->dirty() == true);
    auto dbHash0 = tableFactory->tableHashes();
    // auto data0 = tableFactory->exportData(m_blockNumber);
    auto tableFactory0 = make_shared<StateStorage>(tableFactory, hashImpl, m_blockNumber);
    // tableFactory0->importData(data0.first, data0.second);
    /*
    BOOST_TEST(dbHash0 == tableFactory0->hash());
    tableFactory0 = make_shared<TableFactory>(memoryStorage, hashImpl, m_blockNumber);
    tableFactory0->importData(data0.first, data0.second, false);
    BOOST_TEST(crypto::HashType() == tableFactory0->hash());
    */

    entry = std::make_optional(table->newEntry());
    // entry->setField("key", "id");
    entry->setField("value", "12345");
    ret = table->setRow("id", *entry);
    BOOST_TEST(ret == true);
    entry = table->getRow("id");
    BOOST_TEST(entry);
    entry = table->getRow("name");
    BOOST_TEST(entry);
    // BOOST_TEST(table->dirty() == true);
    auto keys = table->getPrimaryKeys({});
    BOOST_TEST(keys.size() == 2);

    auto entries = table->getRows(keys);
    BOOST_TEST(entries.size() == 2);

    /*
    auto data1 = tableFactory->exportData(m_blockNumber);
    auto dbHash1 = tableFactory->hash();
    BOOST_TEST(dbHash1 != dbHash0);
    auto tableFactory1 = make_shared<TableFactory>(memoryStorage, hashImpl, m_blockNumber);
    tableFactory1->importData(data1.first, data1.second);
    BOOST_TEST(dbHash1 == tableFactory1->hash());
    tableFactory1 = make_shared<TableFactory>(memoryStorage, hashImpl, m_blockNumber);
    tableFactory1->importData(data0.first, data0.second, false);
    BOOST_TEST(crypto::HashType() == tableFactory1->hash());
    */

    auto savePoint = tableFactory->savepoint();
    auto idEntry = table->getRow("id");

    auto deletedEntry = std::make_optional(table->newDeletedEntry());
    deletedEntry->setVersion(idEntry->version() + 1);
    BOOST_CHECK_EQUAL(table->setRow("id", *deletedEntry), true);
    entry = table->getRow("id");
    BOOST_CHECK_EQUAL(entry->status(), Entry::DELETED);
    /*
    auto data2 = tableFactory->exportData(m_blockNumber);
    auto dbHash2 = tableFactory->hash();
    auto tableFactory2 = make_shared<TableFactory>(memoryStorage, hashImpl, m_blockNumber);
    tableFactory2->importData(data2.first, data2.second);
    BOOST_TEST(dbHash2 == tableFactory2->hash());
    cout << LOG_KV("dbHash1", dbHash1) << LOG_KV("dbHash2", dbHash2);

    BOOST_TEST(dbHash1 != dbHash2);
    */

    tableFactory->rollback(savePoint);
    entry = table->getRow("name");
    BOOST_TEST(entry);
    entry = table->getRow("balance");
    BOOST_TEST(!entry);
    // BOOST_TEST(table->dirty() == true);

    /*
    auto data3 = tableFactory->exportData(m_blockNumber);
    auto dbHash3 = tableFactory->hash();
    cout << LOG_KV("dbHash3", dbHash3) << LOG_KV("dbHash1", dbHash1);
    BOOST_TEST(dbHash3 == dbHash1);
    auto tableFactory3 = make_shared<TableFactory>(memoryStorage, hashImpl, m_blockNumber);
    tableFactory3->importData(data3.first, data3.second);
    BOOST_TEST(dbHash3 == tableFactory3->hash());
    tableFactory->commit();
    tableFactory = make_shared<TableFactory>(memoryStorage, hashImpl, 1);
    table = tableFactory->openTable(testTableName);
    */

    // getPrimaryKeys and getRows
    entry = table->newEntry();
    // entry->setField("key", "id");
    entry->setField("value", "12345");
    entry->setVersion(entry->version() + 1);
    ret = table->setRow("id", *entry);
    BOOST_TEST(ret == true);
    entry = table->getRow("name");
    entry->setField("value", "Wang");
    entry->setVersion(entry->version() + 1);
    ret = table->setRow("name", *entry);
    BOOST_TEST(ret == true);
    entry = table->newEntry();
    // entry->setField("key", "balance");
    entry->setField("value", "12345");
    ret = table->setRow("balance", *entry);
    BOOST_TEST(ret == true);
    BOOST_TEST(entry);
    keys = table->getPrimaryKeys({});
    BOOST_TEST(keys.size() == 3);

    entries = table->getRows(keys);
    BOOST_TEST(entries.size() == 3);
    entry = table->getRow("name");
    BOOST_TEST(entry);
    entry = table->getRow("balance");
    BOOST_TEST(entry);
    entry = table->getRow("balance1");
    BOOST_TEST(!entry);

    auto nameEntry = table->getRow("name");
    auto deletedEntry2 = std::make_optional(table->newDeletedEntry());
    deletedEntry2->setVersion(nameEntry->version() + 1);
    ret = table->setRow("name", *deletedEntry2);
    BOOST_TEST(ret == true);
    entry = table->getRow("name");
    BOOST_CHECK_EQUAL(entry->status(), Entry::DELETED);
    keys = table->getPrimaryKeys({});
    BOOST_TEST(keys.size() == 2);

    entries = table->getRows(keys);
    BOOST_TEST(entries.size() == 2);

    auto idEntry2 = table->getRow("id");
    auto deletedEntry3 = std::make_optional(table->newDeletedEntry());
    deletedEntry3->setVersion(idEntry2->version() + 1);
    ret = table->setRow("id", *deletedEntry3);
    BOOST_TEST(ret == true);
    entry = table->getRow("id");
    BOOST_CHECK_EQUAL(entry->status(), Entry::DELETED);
    keys = table->getPrimaryKeys({});
    BOOST_TEST(keys.size() == 1);

    entries = table->getRows(keys);
    BOOST_TEST(entries.size() == 1);
    // tableFactory->asyncCommit([](Error::Ptr, size_t) {});
}

BOOST_AUTO_TEST_CASE(open_sysTables)
{
    auto table = tableFactory->openTable(StorageInterface::SYS_TABLES);
    BOOST_TEST(table);
}

BOOST_AUTO_TEST_CASE(openAndCommit)
{
    auto hashImpl2 = make_shared<Header256Hash>();
    auto memoryStorage2 = make_shared<StateStorage>(nullptr, hashImpl2, 0);
    auto tableFactory2 = make_shared<StateStorage>(memoryStorage2, hashImpl2, 10);

    for (int i = 10; i < 20; ++i)
    {
        BOOST_TEST(tableFactory2 != nullptr);

        std::string tableName = "testTable" + boost::lexical_cast<std::string>(i);
        auto key = "testKey" + boost::lexical_cast<std::string>(i);
        tableFactory2->createTable(tableName, "value");
        auto table = tableFactory2->openTable(tableName);

        auto entry = std::make_optional(table->newEntry());
        entry->setField("value", "hello world!");
        table->setRow(key, *entry);

        std::promise<bool> getRow;
        table->asyncGetRow(key, [&](auto&& error, auto&& result) {
            BOOST_CHECK(!error);
            BOOST_CHECK_EQUAL(result->getField("value"), "hello world!");

            getRow.set_value(true);
        });

        getRow.get_future().get();
    }
}

BOOST_AUTO_TEST_CASE(chainLink)
{
    std::vector<StateStorage::Ptr> storages;
    auto valueFields = "value1,value2,value3";

    StateStorage::Ptr prev = nullptr;
    for (int i = 0; i < 20; ++i)
    {
        auto tableStorage = std::make_shared<StateStorage>(prev, hashImpl, i);
        for (int j = 0; j < 10; ++j)
        {
            auto tableName = "table_" + boost::lexical_cast<std::string>(i) + "_" +
                             boost::lexical_cast<std::string>(j);
            BOOST_CHECK_EQUAL(tableStorage->createTable(tableName, valueFields), true);

            auto table = tableStorage->openTable(tableName);
            BOOST_TEST(table);

            for (int k = 0; k < 100; ++k)
            {
                auto entry = std::make_optional(table->newEntry());
                auto key =
                    boost::lexical_cast<std::string>(i) + boost::lexical_cast<std::string>(k);
                entry->setField("value1", boost::lexical_cast<std::string>(i));
                entry->setField("value2", boost::lexical_cast<std::string>(j));
                entry->setField("value3", boost::lexical_cast<std::string>(k));
                BOOST_CHECK_EQUAL(table->setRow(key, *entry), true);
            }
        }

        prev = tableStorage;
        storages.push_back(tableStorage);
    }

    for (int index = 0; index < 20; ++index)
    {
        auto storage = storages[index];
        // Data count must be 10 * 100 + 10
        tbb::atomic<size_t> totalCount = 0;
        storage->parallelTraverse(false, [&](auto&&, auto&&, auto&&) {
            ++totalCount;
            return true;
        });

        BOOST_CHECK_EQUAL(totalCount, 10 * 100 + 10);  // extra 100 for s_tables

        // Dirty data count must be 10 * 100 + 10
        tbb::atomic<size_t> dirtyCount = 0;
        storage->parallelTraverse(true, [&](auto&&, auto&&, auto&&) {
            ++dirtyCount;
            return true;
        });

        BOOST_CHECK_EQUAL(dirtyCount, 10 * 100 + 10);  // extra 100 for s_tables

        // Low level can't touch high level's data
        for (int i = 0; i < 20; ++i)
        {
            for (int j = 0; j < 10; ++j)
            {
                auto tableName = "table_" + boost::lexical_cast<std::string>(i) + "_" +
                                 boost::lexical_cast<std::string>(j);

                auto table = storage->openTable(tableName);
                if (i > index)
                {
                    BOOST_TEST(!table);
                }
                else
                {
                    BOOST_TEST(table);

                    for (int k = 0; k < 100; ++k)
                    {
                        auto key = boost::lexical_cast<std::string>(i) +
                                   boost::lexical_cast<std::string>(k);

                        auto entry = table->getRow(key);
                        if (i > index)
                        {
                            BOOST_TEST(!entry);
                        }
                        else
                        {
                            BOOST_TEST(entry);

                            BOOST_CHECK_GT(entry->capacityOfHashField(), 0);
                            if (i == index)
                            {
                                BOOST_CHECK_EQUAL(entry->dirty(), true);
                            }
                            else
                            {
                                BOOST_CHECK_EQUAL(entry->dirty(), false);
                            }
                            BOOST_CHECK_EQUAL(
                                entry->getField("value1"), boost::lexical_cast<std::string>(i));
                            BOOST_CHECK_EQUAL(
                                entry->getField("value2"), boost::lexical_cast<std::string>(j));
                            BOOST_CHECK_EQUAL(
                                entry->getField("value3"), boost::lexical_cast<std::string>(k));
                        }
                    }
                }
            }
        }

        // After reading, current storage should include previous storage's data, previous data's
        // dirty should be false
        totalCount = 0;
        tbb::concurrent_vector<std::function<void()>> checks;
        storage->parallelTraverse(false, [&](auto&& table, auto&&, auto&& entry) {
            checks.push_back([index, table, entry] {
                // BOOST_CHECK_NE(tableInfo, nullptr);
                if (table != "s_tables")
                {
                    auto i = boost::lexical_cast<int>(entry.getField("value1"));
                    auto j = boost::lexical_cast<int>(entry.getField("value2"));
                    auto k = boost::lexical_cast<int>(entry.getField("value3"));

                    BOOST_CHECK_LE(i, index);
                    BOOST_CHECK_LE(j, 10);
                    BOOST_CHECK_LE(k, 100);
                }
            });

            ++totalCount;
            return true;
        });

        for (auto& it : checks)
        {
            it();
        }

        BOOST_CHECK_EQUAL(totalCount, (10 * 100 + 10) * (index + 1));

        checks.clear();
        dirtyCount = 0;
        storage->parallelTraverse(true, [&](auto&& table, auto&&, auto&& entry) {
            checks.push_back([index, table, entry]() {
                // BOOST_CHECK_NE(tableInfo, nullptr);
                if (table != "s_tables")
                {
                    auto i = boost::lexical_cast<int>(entry.getField("value1"));
                    auto j = boost::lexical_cast<int>(entry.getField("value2"));
                    auto k = boost::lexical_cast<int>(entry.getField("value3"));

                    if (i == index)
                    {
                        BOOST_CHECK_EQUAL(entry.dirty(), true);
                    }
                    else
                    {
                        BOOST_CHECK_EQUAL(entry.dirty(), false);
                    }

                    BOOST_CHECK_LE(j, 10);
                    BOOST_CHECK_LE(k, 100);
                }
            });

            ++dirtyCount;
            return true;
        });

        for (auto& it : checks)
        {
            it();
        }

        BOOST_CHECK_EQUAL(dirtyCount, 10 * 100 + 10);
    }
}

BOOST_AUTO_TEST_CASE(getRows)
{
    std::vector<StateStorage::Ptr> storages;
    auto valueFields = "value1,value2,value3";

    StateStorage::Ptr prev = nullptr;
    prev = std::make_shared<StateStorage>(prev, hashImpl, 0);
    auto tableStorage = std::make_shared<StateStorage>(prev, hashImpl, 1);

    BOOST_CHECK_EQUAL(prev->createTable("t_test", valueFields), true);

    auto table = prev->openTable("t_test");
    BOOST_TEST(table);

    for (size_t i = 0; i < 100; ++i)
    {
        auto entry = table->newEntry();
        entry.importFields({"data" + boost::lexical_cast<std::string>(i), "data2", "data3"});
        table->setRow("key" + boost::lexical_cast<std::string>(i), entry);
    }

    // query 50-150
    std::vector<std::string> keys;
    for (size_t i = 50; i < 150; ++i)
    {
        keys.push_back("key" + boost::lexical_cast<std::string>(i));
    }

    auto queryTable = tableStorage->openTable("t_test");
    BOOST_TEST(queryTable);

    std::vector<std::string_view> views;
    for (auto& key : keys)
    {
        views.push_back(key);
    }
    auto values = queryTable->getRows(views);

    for (size_t i = 0; i < 100; ++i)
    {
        auto entry = values[i];
        if (i + 50 < 100)
        {
            BOOST_TEST(entry);
            BOOST_CHECK_EQUAL(entry->dirty(), false);
            BOOST_CHECK_EQUAL(entry->num(), 0);
            BOOST_CHECK_GT(entry->capacityOfHashField(), 0);
        }
        else
        {
            BOOST_TEST(!entry);
        }
    }

    for (size_t i = 0; i < 10; ++i)
    {
        auto entry = queryTable->newEntry();
        entry.importFields({"data" + boost::lexical_cast<std::string>(i), "data2", "data3"});
        queryTable->setRow("key" + boost::lexical_cast<std::string>(i), entry);
    }

    // Query 0-30 local(0-9) prev(10-29)
    keys.clear();
    for (size_t i = 0; i < 30; ++i)
    {
        keys.push_back("key" + boost::lexical_cast<std::string>(i));
    }

    views.clear();
    for (auto& key : keys)
    {
        views.push_back(key);
    }
    values = queryTable->getRows(views);

    for (size_t i = 0; i < 30; ++i)
    {
        auto entry = values[i];
        if (i < 10)
        {
            BOOST_TEST(entry);
            BOOST_CHECK_EQUAL(entry->dirty(), true);
            BOOST_CHECK_EQUAL(entry->num(), 1);
        }
        else
        {
            BOOST_TEST(entry);
            BOOST_CHECK_EQUAL(entry->dirty(), false);
            BOOST_CHECK_EQUAL(entry->num(), 0);
        }
    }
}

BOOST_AUTO_TEST_CASE(checkVersion)
{
    BOOST_CHECK_NO_THROW(tableFactory->createTable("testTable", "value1, value2, value3"));
    auto table = tableFactory->openTable("testTable");

    Entry value1;
    value1.importFields({"v1"});
    table->setRow("abc", std::move(value1));

    Entry value2;
    value2.importFields({"v2"});
    BOOST_CHECK_THROW(table->setRow("abc", std::move(value2)), bcos::Error);

    Entry value3;
    value3.importFields({"v3"});
    tableFactory->setCheckVersion(false);
    BOOST_CHECK_NO_THROW(table->setRow("abc", std::move(value3)));
}

BOOST_AUTO_TEST_SUITE_END()
}  // namespace test
}  // namespace bcos