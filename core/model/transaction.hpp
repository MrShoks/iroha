/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
http://soramitsu.co.jp
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
         http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#ifndef CORE_DOMAIN_TRANSACTIONS_TRANSACTION_HPP_
#define CORE_DOMAIN_TRANSACTIONS_TRANSACTION_HPP_

#include "../crypto/signature.hpp"
#include "../util/datetime.hpp"
#include "../crypto/hash.hpp"

#include "commands/command.hpp"
#include "../service/executor.hpp"

#include <algorithm>
#include <cstdint>

namespace transaction {

class Transaction {
  public:
    struct TxSignature{
        std::string publicKey;
        std::string signature;

        TxSignature(
                std::string pubKey,
                std::string sig
        ):
            publicKey(pubKey),
            signature(sig)
        {}
    };

    std::string hash;
    std::vector<TxSignature> txSignatures_;
    std::int64_t    timestamp;
    std::string     senderPublicKey;
    std::string     ownerPublicKey; // okay?

    command::Command command;

    Transaction(
      std::string senderPublicKey,
      command::Command&& cmd
    ):
        timestamp(datetime::unixtime()),
        senderPublicKey(senderPublicKey),
        command(std::move(cmd))
    {}

    Transaction():
        timestamp(datetime::unixtime())
    {}

    void execution(){
      Executor executor;
      command.execute(executor);
    }

    auto getHash() {
        return hash::sha3_256_hex( command.getHash() + std::to_string(timestamp) + senderPublicKey);
    }

    std::vector<TxSignature> txSignatures() const{
        return txSignatures_;
    }

    void addTxSignature(const std::string& pubKey,const std::string& signature){
        txSignatures_.push_back(TxSignature(pubKey, signature));
    }

    bool isValidSignatures(){
        return std::count_if(
            txSignatures_.begin(), txSignatures_.end(),
            [hash = getHash()](TxSignature sig){
                return signature::verify(sig.signature, hash, sig.publicKey);
            }
        ) == txSignatures_.size();
    }

};

}  // namespace transaction

#endif  // CORE_DOMAIN_TRANSACTIONS_TRANSACTION_HPP_
