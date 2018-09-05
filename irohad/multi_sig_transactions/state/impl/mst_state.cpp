/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "multi_sig_transactions/state/mst_state.hpp"

#include <boost/range/algorithm/find.hpp>
#include <boost/range/combine.hpp>
#include <utility>

#include "backend/protobuf/transaction.hpp"
#include "common/set.hpp"

namespace iroha {

  // ------------------------------| public api |-------------------------------

  MstState MstState::empty(const CompleterType &completer) {
    return MstState(completer);
  }

  StateAndCompleteStatus MstState::operator+=(const DataType &rhs) {
    auto result = MstState::empty(completer_);
    auto complete_status = insertOne(result, rhs);
    return StateAndCompleteStatus{result, complete_status};
  }

  MstState MstState::operator+=(const MstState &rhs) {
    auto result = MstState::empty(completer_);
    for (auto &&rhs_tx : rhs.internal_state_) {
      insertOne(result, rhs_tx);
    }
    return result;
  }

  MstState MstState::operator-(const MstState &rhs) const {
    return MstState(this->completer_,
                    set_difference(this->internal_state_, rhs.internal_state_));
  }

  bool MstState::operator==(const MstState &rhs) const {
    const auto &lhs_batches = getBatches();
    const auto &rhs_batches = rhs.getBatches();

    return std::equal(lhs_batches.begin(),
                      lhs_batches.end(),
                      rhs_batches.begin(),
                      rhs_batches.end(),
                      [](const auto &l, const auto &r) { return *l == *r; });
  }

  bool MstState::isEmpty() const {
    return internal_state_.empty();
  }

  std::vector<DataType> MstState::getBatches() const {
    std::vector<DataType> result(internal_state_.begin(),
                                 internal_state_.end());
    // sorting is provided for clear comparison of states
    // TODO: 15/08/2018 @muratovv Rework return type with set IR-1621
    std::sort(
        result.begin(), result.end(), [](const auto &left, const auto &right) {
          return left->reducedHash().hex() < right->reducedHash().hex();
        });
    return result;
  }

  MstState MstState::eraseByTime(const TimeType &time) {
    MstState out = MstState::empty(completer_);
    while (not index_.empty() and (*completer_)(index_.top(), time)) {
      auto iter = internal_state_.find(index_.top());

      out += *iter;
      internal_state_.erase(iter);
      index_.pop();
    }
    return out;
  }

  // ------------------------------| private api |------------------------------

  /**
   * Merge signatures in batches
   * @param target - batch for inserting
   * @param donor - batch with transactions to copy signatures from
   * @return return if at least one new signature was inserted
   */
  bool mergeSignaturesInBatch(DataType &target, const DataType &donor) {
    auto inserted_new_signatures = false;
    for (auto zip :
         boost::combine(target->transactions(), donor->transactions())) {
      const auto &target_tx = zip.get<0>();
      const auto &donor_tx = zip.get<1>();
      inserted_new_signatures = std::accumulate(
          std::begin(donor_tx->signatures()),
          std::end(donor_tx->signatures()),
          inserted_new_signatures,
          [&target_tx](bool accumulator, const auto &signature) {
            return target_tx->addSignature(signature.signedData(),
                                           signature.publicKey())
                or accumulator;
          });
    }
    return inserted_new_signatures;
  }

  MstState::MstState(const CompleterType &completer)
      : MstState(completer, InternalStateType{}) {}

  MstState::MstState(const CompleterType &completer,
                     const InternalStateType &transactions)
      : completer_(completer),
        internal_state_(transactions.begin(), transactions.end()),
        index_(transactions.begin(), transactions.end()) {
    log_ = logger::log("MstState");
  }

  bool MstState::insertOne(MstState &out_state, const DataType &rhs_batch) {
    log_->info("batch: {}", rhs_batch->toString());
    auto corresponding = internal_state_.find(rhs_batch);
    if (corresponding == internal_state_.end()) {
      // when state not contains transaction
      rawInsert(rhs_batch);
      out_state.rawInsert(rhs_batch);
      return false;
    }

    DataType found = *corresponding;
    // Append new signatures to the existing state
    auto inserted_new_signatures = mergeSignaturesInBatch(found, rhs_batch);

    if ((*completer_)(found)) {
      // state already has completed transaction,
      // remove from state and return it
      out_state += found;
      internal_state_.erase(internal_state_.find(found));
      return true;
    }

    // if batch still isn't completed, return it, if new signatures were updated
    if (inserted_new_signatures) {
      out_state.rawInsert(found);
    }
    return false;
  }

  void MstState::rawInsert(const DataType &rhs_batch) {
    internal_state_.insert(rhs_batch);
    index_.push(rhs_batch);
  }

}  // namespace iroha
