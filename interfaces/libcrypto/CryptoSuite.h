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
 * @brief crypto suite toolkit
 * @file CryptoSuite.h
 * @author: yujiechen
 * @date 2021-03-03
 */
#pragma once
#include "Hash.h"
#include "Signature.h"
#include "SymmetricEncryption.h"
namespace bcos
{
namespace crypto
{
class CryptoSuite
{
public:
    CryptoSuite(Hash::Ptr _hashImpl, Crypto::Ptr _signatureImpl,
        SymmetricEncryption::Ptr _symmetricEncryptionHandler)
      : m_hashImpl(_hashImpl),
        m_signatureImpl(_signatureImpl),
        m_symmetricEncryptionHandler(_symmetricEncryptionHandler)
    {}
    virtual ~CryptoSuite() {}
    Hash::Ptr hashImpl() { return m_hashImpl; }
    Crypto::Ptr signatureImpl() { return m_signatureImpl; }
    SymmetricEncryption::Ptr symmetricEncryptionHandler() { return m_symmetricEncryptionHandler; }

    template <typename T>
    h256 hash(T&& _data)
    {
        return m_hashImpl->hash(_data);
    }

private:
    Hash::Ptr m_hashImpl;
    Crypto::Ptr m_signatureImpl;
    SymmetricEncryption::Ptr m_symmetricEncryptionHandler;
};
}  // namespace crypto
}  // namespace bcos
