// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#pragma once

#include <cstdint>
#include <memory>

#include "arrow/array/array_base.h"
#include "arrow/array/data.h"
#include "arrow/result.h"
#include "arrow/status.h"
#include "arrow/type.h"
#include "arrow/util/macros.h"
#include "arrow/util/visibility.h"

namespace arrow {

// ----------------------------------------------------------------------
// DictionaryArray

/// \brief Array type for dictionary-encoded data with a
/// data-dependent dictionary
///
/// A dictionary array contains an array of non-negative integers (the
/// "dictionary indices") along with a data type containing a "dictionary"
/// corresponding to the distinct values represented in the data.
///
/// For example, the array
///
///   ["foo", "bar", "foo", "bar", "foo", "bar"]
///
/// with dictionary ["bar", "foo"], would have dictionary array representation
///
///   indices: [1, 0, 1, 0, 1, 0]
///   dictionary: ["bar", "foo"]
///
/// The indices in principle may have any integer type (signed or unsigned),
/// though presently data in IPC exchanges must be signed int32.
class ARROW_EXPORT DictionaryArray : public Array {
 public:
  using TypeClass = DictionaryType;

  explicit DictionaryArray(const std::shared_ptr<ArrayData>& data);

  DictionaryArray(const std::shared_ptr<DataType>& type,
                  const std::shared_ptr<Array>& indices,
                  const std::shared_ptr<Array>& dictionary);

  /// \brief Construct DictionaryArray from dictionary and indices
  /// array and validate
  ///
  /// This function does the validation of the indices and input type. It checks if
  /// all indices are non-negative and smaller than the size of the dictionary
  ///
  /// \param[in] type a dictionary type
  /// \param[in] dictionary the dictionary with same value type as the
  /// type object
  /// \param[in] indices an array of non-negative signed
  /// integers smaller than the size of the dictionary
  static Result<std::shared_ptr<Array>> FromArrays(
      const std::shared_ptr<DataType>& type, const std::shared_ptr<Array>& indices,
      const std::shared_ptr<Array>& dictionary);

  ARROW_DEPRECATED("Use Result-returning version")
  static Status FromArrays(const std::shared_ptr<DataType>& type,
                           const std::shared_ptr<Array>& indices,
                           const std::shared_ptr<Array>& dictionary,
                           std::shared_ptr<Array>* out);

  /// \brief Transpose this DictionaryArray
  ///
  /// This method constructs a new dictionary array with the given dictionary type,
  /// transposing indices using the transpose map.
  /// The type and the transpose map are typically computed using
  /// DictionaryUnifier.
  ///
  /// \param[in] type the new type object
  /// \param[in] dictionary the new dictionary
  /// \param[in] transpose_map transposition array of this array's indices
  ///   into the target array's indices
  /// \param[in] pool a pool to allocate the array data from
  Result<std::shared_ptr<Array>> Transpose(
      const std::shared_ptr<DataType>& type, const std::shared_ptr<Array>& dictionary,
      const int32_t* transpose_map, MemoryPool* pool = default_memory_pool()) const;

  ARROW_DEPRECATED("Use Result-returning version")
  Status Transpose(MemoryPool* pool, const std::shared_ptr<DataType>& type,
                   const std::shared_ptr<Array>& dictionary, const int32_t* transpose_map,
                   std::shared_ptr<Array>* out) const;

  /// \brief Determine whether dictionary arrays may be compared without unification
  bool CanCompareIndices(const DictionaryArray& other) const;

  /// \brief Return the dictionary for this array, which is stored as
  /// a member of the ArrayData internal structure
  std::shared_ptr<Array> dictionary() const;
  std::shared_ptr<Array> indices() const;

  /// \brief Return the ith value of indices, cast to int64_t
  int64_t GetValueIndex(int64_t i) const;

  const DictionaryType* dict_type() const { return dict_type_; }

 private:
  void SetData(const std::shared_ptr<ArrayData>& data);
  const DictionaryType* dict_type_;
  std::shared_ptr<Array> indices_;

  // Lazily initialized when invoking dictionary()
  mutable std::shared_ptr<Array> dictionary_;
};

}  // namespace arrow