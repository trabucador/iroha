/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "bindings/simple_transaction_proto.hpp"
#include "interfaces/polymorphic_wrapper.hpp"
#include "primitive.pb.h"

namespace shared_model {
  namespace proto {
    Transaction SimpleTransactionProto::signAndAddSignature(
        UnsignedWrapper<Transaction> &tx, const crypto::Keypair &keypair) {
      return tx.signAndAddSignature(keypair);
    }

    const iroha::protocol::Transaction &SimpleTransactionProto::getTransport(
        const Transaction &tx) {
      return tx.getTransport();
    }
  }  // namespace proto
}  // namespace shared_model
