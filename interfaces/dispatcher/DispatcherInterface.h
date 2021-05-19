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
 * @brief interface of Dispatcher
 * @file DispatcherInterface.h
 * @author: xingqiangbai
 * @date: 2021-05-17
 */
#pragma once
#include "../../libutilities/Error.h"
#include "../crypto/CommonType.h"
#include "../protocol/Block.h"
#include <functional>
#include <memory>

namespace bcos
{
namespace dispatcher
{
class DispatcherInterface : public std::enable_shared_from_this<DispatcherInterface>
{
public:
    using Ptr = std::shared_ptr<DispatcherInterface>;
    DispatcherInterface() = default;
    virtual ~DispatcherInterface() {}
    virtual void asyncExecuteBlock(protocol::Block::Ptr _block, bool _verify,
        std::function<void(Error::Ptr, protocol::BlockHeader::Ptr)> _callback) = 0;
    virtual void asyncGetBlockHeaderHash(protocol::BlockNumber _number,
        std::function<void(Error::Ptr, std::shared_ptr<crypto::HashType>)> _callback) = 0;
    virtual void asyncGetLatestBlock(
        std::function<void(Error::Ptr, protocol::Block::Ptr)> _callback) = 0;
    virtual void asyncNotifyExecutionResult(Error::Ptr _error,
        std::shared_ptr<protocol::BlockHeader> _header,
        std::function<void(Error::Ptr)> _callback) = 0;
};

}  // namespace dispatcher
}  // namespace bcos