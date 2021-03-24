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
 * @file: TransactionStatus.h
 * @author: xingqiangbai
 * @date: 20200608
 */

#pragma once

#include "Exceptions.h"
#include <iostream>
namespace bcos
{
struct Exception;
namespace protocol
{
// TODO: redefine the status value when everything ready
enum class TransactionStatus : uint32_t
{
    None = 0,
    Unknown = 1,
    OutOfGasLimit = 2,  ///< Too little gas to pay for the base transaction cost.
    NotEnoughCash = 7,  // TODO: remove this?
    BadInstruction = 10,
    BadJumpDestination = 11,
    OutOfGas = 12,    ///< Ran out of gas executing code of the transaction.
    OutOfStack = 13,  ///< Ran out of stack executing code of the transaction.
    StackUnderflow = 14,
    NonceCheckFail = 15,
    BlockLimitCheckFail = 16,
    PrecompiledError = 21,
    RevertInstruction = 22,
    ContractAddressAlreadyUsed = 24,
    PermissionDenied = 25,
    CallAddressError = 26,
    GasOverflow = 27,
    ContractFrozen = 30,
    AccountFrozen = 31,
    TxPoolIsFull = 10000,
    TransactionRefused = 10001,
    AlreadyInTxPool = 10002,  /// txPool related errors
    TxAlreadyInChain = 10003,
    InvalidChainId = 10004,
    InvalidGroupId = 10005,
    RequestNotBelongToTheGroup = 10006,
};
TransactionStatus toTransactionStatus(Exception const& _e);
std::ostream& operator<<(std::ostream& _out, TransactionStatus const& _er);
inline std::string toString(protocol::TransactionStatus const& _i)
{
    std::stringstream stream;
    stream << "0x" << std::hex << static_cast<int>(_i);
    return stream.str();
}
}  // namespace protocol
}  // namespace bcos