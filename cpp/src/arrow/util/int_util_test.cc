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

#include <algorithm>
#include <cstdint>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "arrow/testing/gtest_util.h"
#include "arrow/testing/random.h"
#include "arrow/type.h"
#include "arrow/util/int_util.h"

namespace arrow {
namespace internal {

static std::vector<uint8_t> all_widths = {1, 2, 4, 8};

template <typename T>
void CheckUIntWidth(const std::vector<T>& values, uint8_t expected_width) {
  for (const uint8_t min_width : all_widths) {
    uint8_t width =
        DetectUIntWidth(values.data(), static_cast<int64_t>(values.size()), min_width);
    ASSERT_EQ(width, std::max(min_width, expected_width));
    width = DetectUIntWidth(values.data(), nullptr, static_cast<int64_t>(values.size()),
                            min_width);
    ASSERT_EQ(width, std::max(min_width, expected_width));
  }
}

template <typename T>
void CheckUIntWidth(const std::vector<T>& values, const std::vector<uint8_t>& valid_bytes,
                    uint8_t expected_width) {
  for (const uint8_t min_width : all_widths) {
    uint8_t width = DetectUIntWidth(values.data(), valid_bytes.data(),
                                    static_cast<int64_t>(values.size()), min_width);
    ASSERT_EQ(width, std::max(min_width, expected_width));
  }
}

template <typename T>
void CheckIntWidth(const std::vector<T>& values, uint8_t expected_width) {
  for (const uint8_t min_width : all_widths) {
    uint8_t width =
        DetectIntWidth(values.data(), static_cast<int64_t>(values.size()), min_width);
    ASSERT_EQ(width, std::max(min_width, expected_width));
    width = DetectIntWidth(values.data(), nullptr, static_cast<int64_t>(values.size()),
                           min_width);
    ASSERT_EQ(width, std::max(min_width, expected_width));
  }
}

template <typename T>
void CheckIntWidth(const std::vector<T>& values, const std::vector<uint8_t>& valid_bytes,
                   uint8_t expected_width) {
  for (const uint8_t min_width : all_widths) {
    uint8_t width = DetectIntWidth(values.data(), valid_bytes.data(),
                                   static_cast<int64_t>(values.size()), min_width);
    ASSERT_EQ(width, std::max(min_width, expected_width));
  }
}

template <typename T>
std::vector<T> MakeRandomVector(const std::vector<T>& base_values, int n_values) {
  std::default_random_engine gen(42);
  std::uniform_int_distribution<int> index_dist(0,
                                                static_cast<int>(base_values.size() - 1));

  std::vector<T> values(n_values);
  for (int i = 0; i < n_values; ++i) {
    values[i] = base_values[index_dist(gen)];
  }
  return values;
}

template <typename T>
std::vector<std::pair<std::vector<T>, std::vector<uint8_t>>> AlmostAllNullValues(
    int n_values, T null_value, T non_null_value) {
  std::vector<std::pair<std::vector<T>, std::vector<uint8_t>>> vectors;
  vectors.reserve(n_values);
  for (int i = 0; i < n_values; ++i) {
    std::vector<T> values(n_values, null_value);
    std::vector<uint8_t> valid_bytes(n_values, 0);
    values[i] = non_null_value;
    valid_bytes[i] = 1;
    vectors.push_back({std::move(values), std::move(valid_bytes)});
  }
  return vectors;
}

template <typename T>
std::vector<std::vector<T>> AlmostAllZeros(int n_values, T nonzero_value) {
  std::vector<std::vector<T>> vectors;
  vectors.reserve(n_values);
  for (int i = 0; i < n_values; ++i) {
    std::vector<T> values(n_values, 0);
    values[i] = nonzero_value;
    vectors.push_back(std::move(values));
  }
  return vectors;
}

std::vector<uint64_t> valid_uint8 = {0, 0x7f, 0xff};
std::vector<uint64_t> valid_uint16 = {0, 0x7f, 0xff, 0x1000, 0xffff};
std::vector<uint64_t> valid_uint32 = {0, 0x7f, 0xff, 0x10000, 0xffffffffULL};
std::vector<uint64_t> valid_uint64 = {0, 0x100000000ULL, 0xffffffffffffffffULL};

TEST(UIntWidth, NoNulls) {
  std::vector<uint64_t> values{0, 0x7f, 0xff};
  CheckUIntWidth(values, 1);

  values = {0, 0x100};
  CheckUIntWidth(values, 2);

  values = {0, 0xffff};
  CheckUIntWidth(values, 2);

  values = {0, 0x10000};
  CheckUIntWidth(values, 4);

  values = {0, 0xffffffffULL};
  CheckUIntWidth(values, 4);

  values = {0, 0x100000000ULL};
  CheckUIntWidth(values, 8);

  values = {0, 0xffffffffffffffffULL};
  CheckUIntWidth(values, 8);
}

TEST(UIntWidth, Nulls) {
  std::vector<uint8_t> valid10{true, false};
  std::vector<uint8_t> valid01{false, true};

  std::vector<uint64_t> values{0, 0xff};
  CheckUIntWidth(values, valid01, 1);
  CheckUIntWidth(values, valid10, 1);

  values = {0, 0x100};
  CheckUIntWidth(values, valid01, 2);
  CheckUIntWidth(values, valid10, 1);

  values = {0, 0xffff};
  CheckUIntWidth(values, valid01, 2);
  CheckUIntWidth(values, valid10, 1);

  values = {0, 0x10000};
  CheckUIntWidth(values, valid01, 4);
  CheckUIntWidth(values, valid10, 1);

  values = {0, 0xffffffffULL};
  CheckUIntWidth(values, valid01, 4);
  CheckUIntWidth(values, valid10, 1);

  values = {0, 0x100000000ULL};
  CheckUIntWidth(values, valid01, 8);
  CheckUIntWidth(values, valid10, 1);

  values = {0, 0xffffffffffffffffULL};
  CheckUIntWidth(values, valid01, 8);
  CheckUIntWidth(values, valid10, 1);
}

TEST(UIntWidth, NoNullsMany) {
  constexpr int N = 40;
  for (const auto& values : AlmostAllZeros<uint64_t>(N, 0xff)) {
    CheckUIntWidth(values, 1);
  }
  for (const auto& values : AlmostAllZeros<uint64_t>(N, 0xffff)) {
    CheckUIntWidth(values, 2);
  }
  for (const auto& values : AlmostAllZeros<uint64_t>(N, 0xffffffffULL)) {
    CheckUIntWidth(values, 4);
  }
  for (const auto& values : AlmostAllZeros<uint64_t>(N, 0xffffffffffffffffULL)) {
    CheckUIntWidth(values, 8);
  }
  auto values = MakeRandomVector(valid_uint8, N);
  CheckUIntWidth(values, 1);

  values = MakeRandomVector(valid_uint16, N);
  CheckUIntWidth(values, 2);

  values = MakeRandomVector(valid_uint32, N);
  CheckUIntWidth(values, 4);

  values = MakeRandomVector(valid_uint64, N);
  CheckUIntWidth(values, 8);
}

TEST(UIntWidth, NullsMany) {
  constexpr uint64_t huge = 0x123456789abcdefULL;
  constexpr int N = 40;
  for (const auto& p : AlmostAllNullValues<uint64_t>(N, 0, 0xff)) {
    CheckUIntWidth(p.first, p.second, 1);
  }
  for (const auto& p : AlmostAllNullValues<uint64_t>(N, huge, 0xff)) {
    CheckUIntWidth(p.first, p.second, 1);
  }
  for (const auto& p : AlmostAllNullValues<uint64_t>(N, 0, 0xffff)) {
    CheckUIntWidth(p.first, p.second, 2);
  }
  for (const auto& p : AlmostAllNullValues<uint64_t>(N, huge, 0xffff)) {
    CheckUIntWidth(p.first, p.second, 2);
  }
  for (const auto& p : AlmostAllNullValues<uint64_t>(N, 0, 0xffffffffULL)) {
    CheckUIntWidth(p.first, p.second, 4);
  }
  for (const auto& p : AlmostAllNullValues<uint64_t>(N, huge, 0xffffffffULL)) {
    CheckUIntWidth(p.first, p.second, 4);
  }
  for (const auto& p : AlmostAllNullValues<uint64_t>(N, 0, 0xffffffffffffffffULL)) {
    CheckUIntWidth(p.first, p.second, 8);
  }
  for (const auto& p : AlmostAllNullValues<uint64_t>(N, huge, 0xffffffffffffffffULL)) {
    CheckUIntWidth(p.first, p.second, 8);
  }
}

TEST(IntWidth, NoNulls) {
  std::vector<int64_t> values{0, 0x7f, -0x80};
  CheckIntWidth(values, 1);

  values = {0, 0x80};
  CheckIntWidth(values, 2);

  values = {0, -0x81};
  CheckIntWidth(values, 2);

  values = {0, 0x7fff, -0x8000};
  CheckIntWidth(values, 2);

  values = {0, 0x8000};
  CheckIntWidth(values, 4);

  values = {0, -0x8001};
  CheckIntWidth(values, 4);

  values = {0, 0x7fffffffLL, -0x80000000LL};
  CheckIntWidth(values, 4);

  values = {0, 0x80000000LL};
  CheckIntWidth(values, 8);

  values = {0, -0x80000001LL};
  CheckIntWidth(values, 8);

  values = {0, 0x7fffffffffffffffLL, -0x7fffffffffffffffLL - 1};
  CheckIntWidth(values, 8);
}

TEST(IntWidth, Nulls) {
  std::vector<uint8_t> valid100{true, false, false};
  std::vector<uint8_t> valid010{false, true, false};
  std::vector<uint8_t> valid001{false, false, true};

  std::vector<int64_t> values{0, 0x7f, -0x80};
  CheckIntWidth(values, valid100, 1);
  CheckIntWidth(values, valid010, 1);
  CheckIntWidth(values, valid001, 1);

  values = {0, 0x80, -0x81};
  CheckIntWidth(values, valid100, 1);
  CheckIntWidth(values, valid010, 2);
  CheckIntWidth(values, valid001, 2);

  values = {0, 0x7fff, -0x8000};
  CheckIntWidth(values, valid100, 1);
  CheckIntWidth(values, valid010, 2);
  CheckIntWidth(values, valid001, 2);

  values = {0, 0x8000, -0x8001};
  CheckIntWidth(values, valid100, 1);
  CheckIntWidth(values, valid010, 4);
  CheckIntWidth(values, valid001, 4);

  values = {0, 0x7fffffffLL, -0x80000000LL};
  CheckIntWidth(values, valid100, 1);
  CheckIntWidth(values, valid010, 4);
  CheckIntWidth(values, valid001, 4);

  values = {0, 0x80000000LL, -0x80000001LL};
  CheckIntWidth(values, valid100, 1);
  CheckIntWidth(values, valid010, 8);
  CheckIntWidth(values, valid001, 8);

  values = {0, 0x7fffffffffffffffLL, -0x7fffffffffffffffLL - 1};
  CheckIntWidth(values, valid100, 1);
  CheckIntWidth(values, valid010, 8);
  CheckIntWidth(values, valid001, 8);
}

TEST(IntWidth, NoNullsMany) {
  constexpr int N = 40;
  // 1 byte wide
  for (const int64_t value : {0x7f, -0x80}) {
    for (const auto& values : AlmostAllZeros<int64_t>(N, value)) {
      CheckIntWidth(values, 1);
    }
  }
  // 2 bytes wide
  for (const int64_t value : {0x80, -0x81, 0x7fff, -0x8000}) {
    for (const auto& values : AlmostAllZeros<int64_t>(N, value)) {
      CheckIntWidth(values, 2);
    }
  }
  // 4 bytes wide
  for (const int64_t value : {0x8000LL, -0x8001LL, 0x7fffffffLL, -0x80000000LL}) {
    for (const auto& values : AlmostAllZeros<int64_t>(N, value)) {
      CheckIntWidth(values, 4);
    }
  }
  // 8 bytes wide
  for (const int64_t value : {0x80000000LL, -0x80000001LL, 0x7fffffffffffffffLL}) {
    for (const auto& values : AlmostAllZeros<int64_t>(N, value)) {
      CheckIntWidth(values, 8);
    }
  }
}

TEST(IntWidth, NullsMany) {
  constexpr int64_t huge = 0x123456789abcdefLL;
  constexpr int N = 40;
  // 1 byte wide
  for (const int64_t value : {0x7f, -0x80}) {
    for (const auto& p : AlmostAllNullValues<int64_t>(N, 0, value)) {
      CheckIntWidth(p.first, p.second, 1);
    }
    for (const auto& p : AlmostAllNullValues<int64_t>(N, huge, value)) {
      CheckIntWidth(p.first, p.second, 1);
    }
  }
  // 2 bytes wide
  for (const int64_t value : {0x80, -0x81, 0x7fff, -0x8000}) {
    for (const auto& p : AlmostAllNullValues<int64_t>(N, 0, value)) {
      CheckIntWidth(p.first, p.second, 2);
    }
    for (const auto& p : AlmostAllNullValues<int64_t>(N, huge, value)) {
      CheckIntWidth(p.first, p.second, 2);
    }
  }
  // 4 bytes wide
  for (const int64_t value : {0x8000LL, -0x8001LL, 0x7fffffffLL, -0x80000000LL}) {
    for (const auto& p : AlmostAllNullValues<int64_t>(N, 0, value)) {
      CheckIntWidth(p.first, p.second, 4);
    }
    for (const auto& p : AlmostAllNullValues<int64_t>(N, huge, value)) {
      CheckIntWidth(p.first, p.second, 4);
    }
  }
  // 8 bytes wide
  for (const int64_t value : {0x80000000LL, -0x80000001LL, 0x7fffffffffffffffLL}) {
    for (const auto& p : AlmostAllNullValues<int64_t>(N, 0, value)) {
      CheckIntWidth(p.first, p.second, 8);
    }
    for (const auto& p : AlmostAllNullValues<int64_t>(N, huge, value)) {
      CheckIntWidth(p.first, p.second, 8);
    }
  }
}

TEST(TransposeInts, Int8ToInt64) {
  std::vector<int8_t> src = {1, 3, 5, 0, 3, 2};
  std::vector<int32_t> transpose_map = {1111, 2222, 3333, 4444, 5555, 6666, 7777};
  std::vector<int64_t> dest(src.size());

  TransposeInts(src.data(), dest.data(), 6, transpose_map.data());
  ASSERT_EQ(dest, std::vector<int64_t>({2222, 4444, 6666, 1111, 4444, 3333}));
}

void BoundsCheckPasses(const std::shared_ptr<DataType>& type,
                       const std::string& indices_json, uint64_t upper_limit) {
  auto indices = ArrayFromJSON(type, indices_json);
  ASSERT_OK(IndexBoundsCheck(*indices->data(), upper_limit));
}

void BoundsCheckFails(const std::shared_ptr<DataType>& type,
                      const std::string& indices_json, uint64_t upper_limit) {
  auto indices = ArrayFromJSON(type, indices_json);
  ASSERT_RAISES(IndexError, IndexBoundsCheck(*indices->data(), upper_limit));
}

TEST(IndexBoundsCheck, Batching) {
  auto rand = random::RandomArrayGenerator(/*seed=*/0);

  const int64_t length = 200;

  auto indices = rand.Int16(length, 0, 0, /*null_probability=*/0);
  ArrayData* index_data = indices->data().get();
  index_data->buffers[0] = *AllocateBitmap(length);

  int16_t* values = index_data->GetMutableValues<int16_t>(1);
  uint8_t* bitmap = index_data->buffers[0]->mutable_data();
  BitUtil::SetBitsTo(bitmap, 0, length, true);

  ASSERT_OK(IndexBoundsCheck(*index_data, 1));

  // We'll place out of bounds indices at various locations
  values[99] = 1;
  ASSERT_RAISES(IndexError, IndexBoundsCheck(*index_data, 1));

  // Make that value null
  BitUtil::ClearBit(bitmap, 99);
  ASSERT_OK(IndexBoundsCheck(*index_data, 1));

  values[199] = 1;
  ASSERT_RAISES(IndexError, IndexBoundsCheck(*index_data, 1));

  // Make that value null
  BitUtil::ClearBit(bitmap, 199);
  ASSERT_OK(IndexBoundsCheck(*index_data, 1));
}

TEST(IndexBoundsCheck, SignedInts) {
  auto CheckCommon = [&](const std::shared_ptr<DataType>& ty) {
    BoundsCheckPasses(ty, "[0, 0, 0]", 1);
    BoundsCheckFails(ty, "[0, 0, 0]", 0);
    BoundsCheckFails(ty, "[-1]", 1);
    BoundsCheckFails(ty, "[-128]", 1);
    BoundsCheckFails(ty, "[0, 100, 127]", 127);
    BoundsCheckPasses(ty, "[0, 100, 127]", 128);
  };

  CheckCommon(int8());

  CheckCommon(int16());
  BoundsCheckPasses(int16(), "[0, 999, 999]", 1000);
  BoundsCheckFails(int16(), "[0, 1000, 1000]", 1000);
  BoundsCheckPasses(int16(), "[0, 32767]", 1 << 15);

  CheckCommon(int32());
  BoundsCheckPasses(int32(), "[0, 999999, 999999]", 1000000);
  BoundsCheckFails(int32(), "[0, 1000000, 1000000]", 1000000);
  BoundsCheckPasses(int32(), "[0, 2147483647]", 1LL << 31);

  CheckCommon(int64());
  BoundsCheckPasses(int64(), "[0, 9999999999, 9999999999]", 10000000000LL);
  BoundsCheckFails(int64(), "[0, 10000000000, 10000000000]", 10000000000LL);
}

TEST(IndexBoundsCheck, UnsignedInts) {
  auto CheckCommon = [&](const std::shared_ptr<DataType>& ty) {
    BoundsCheckPasses(ty, "[0, 0, 0]", 1);
    BoundsCheckFails(ty, "[0, 0, 0]", 0);
    BoundsCheckFails(ty, "[0, 100, 200]", 200);
    BoundsCheckPasses(ty, "[0, 100, 200]", 201);
  };

  CheckCommon(uint8());
  BoundsCheckPasses(uint8(), "[255, 255, 255]", 1000);
  BoundsCheckFails(uint8(), "[255, 255, 255]", 255);

  CheckCommon(uint16());
  BoundsCheckPasses(uint16(), "[0, 999, 999]", 1000);
  BoundsCheckFails(uint16(), "[0, 1000, 1000]", 1000);
  BoundsCheckPasses(uint16(), "[0, 65535]", 1 << 16);

  CheckCommon(uint32());
  BoundsCheckPasses(uint32(), "[0, 999999, 999999]", 1000000);
  BoundsCheckFails(uint32(), "[0, 1000000, 1000000]", 1000000);
  BoundsCheckPasses(uint32(), "[0, 4294967295]", 1LL << 32);

  CheckCommon(uint64());
  BoundsCheckPasses(uint64(), "[0, 9999999999, 9999999999]", 10000000000LL);
  BoundsCheckFails(uint64(), "[0, 10000000000, 10000000000]", 10000000000LL);
}

}  // namespace internal
}  // namespace arrow
