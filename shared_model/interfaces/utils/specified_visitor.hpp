/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#ifndef IROHA_SPECIFIED_VISITOR_HPP
#define IROHA_SPECIFIED_VISITOR_HPP

#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include "utils/polymorphic_wrapper.hpp"

namespace shared_model {
  namespace interface {
    template <typename Type>
    class SpecifiedVisitor
        : public boost::static_visitor<boost::optional<const Type &>> {
     private:
      using Y = shared_model::detail::PolymorphicWrapper<
          std::remove_const_t<std::remove_reference_t<Type>>>;

     public:
      /**
       * Match polymorphic wrapper for template type
       * @param t polymorphic wrapper object of specified type
       * @return const reference to value stored in polymorphic wrapper
       */
      boost::optional<const Type &> operator()(const Y &t) const {
        return *t;
      }

      /**
       * Match template type
       * @param t const reference to object of specified type
       * @return const reference to value
       */
      boost::optional<const Type &> operator()(const Type &t) const {
        return t;
      }

      /**
       * Match any other type that was not matched by methods above
       * @tparam T any type not matched by two types above
       * @param t object of specified type
       * @return none
       */
      template <typename T>
      boost::optional<const Type &> operator()(const T &t) const {
        return boost::none;
      }
    };
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_SPECIFIED_VISITOR_HPP
