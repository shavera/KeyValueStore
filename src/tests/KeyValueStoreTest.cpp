//
// Created by alexs on 4/25/21.
//

#include "KeyValueStore/KeyValueStore.h"

#include "gtest/gtest.h"

#include <thread>

namespace {

using namespace ::testing;
using namespace std::chrono_literals;

template <typename T>
void testAndCast(const std::optional<std::any>& inValue, T& outVar){
  ASSERT_TRUE(inValue.has_value());
  ASSERT_TRUE(inValue.value().has_value());
  ASSERT_EQ(inValue.value().type(), typeid(T));
  outVar = std::any_cast<T>(inValue.value());
}

class KeyValueStoreTest : public Test{
public:
  KeyValueStore kvStore;
};

TEST_F(KeyValueStoreTest, transactSimpleValues){
  const std::string valueString{"KVStoreTest.transactSimpleValue"};
  kvStore.addValue(1, valueString);

  const std::string otherValueString{"Beta"};
  kvStore.addValue(123, otherValueString);

  const double someNumber{3.1416};
  kvStore.addValue(9999, someNumber);

  {
    SCOPED_TRACE("First value string");
    auto result = kvStore.getValue(1);
    std::string string;
    testAndCast(result, string);
  }

  {
    SCOPED_TRACE("Second value string");
    auto result = kvStore.getValue(123);
    std::string string;
    testAndCast(result, string);
  }

  {
    SCOPED_TRACE("double value");
    auto result = kvStore.getValue(9999);
    double number;
    testAndCast(result, number);
  }
}

TEST_F(KeyValueStoreTest, canDistinguishWhenOverridingEntry){
  const auto first = kvStore.addValue(1, 1234);
  const auto second = kvStore.addValue(1, 33333);
  // want to also test that overrides allow for changing the type
  const auto third = kvStore.addValue(1, "alpha");

  EXPECT_EQ(KeyValueStore::ValueAdded::DidNotOverride, first);
  EXPECT_EQ(KeyValueStore::ValueAdded::DidOverride, second);
  EXPECT_EQ(KeyValueStore::ValueAdded::DidOverride, third);
}

TEST_F(KeyValueStoreTest, storeCanStoreEmptyAny){
  /// @test The table may store an 'any' value that is 'empty' and this is distinct from returning an invalid result
  kvStore.addValue(1234, std::any{});
  auto result = kvStore.getValue(1234);
  ASSERT_TRUE(result.has_value());
  EXPECT_FALSE(result.value().has_value());
}

TEST_F(KeyValueStoreTest, missingValueReturnsEmptyOptional){
  std::optional<std::any> result{};
  ASSERT_NO_THROW(result = kvStore.getValue(1));
  EXPECT_FALSE(result.has_value());
}

TEST_F(KeyValueStoreTest, withLifetime){
  const auto lifetime{2ms};
  const std::string expectedValue{"KeyValueStoreTest.withLifetime"};
  const size_t key{5555};
  kvStore.addValue(key, expectedValue, lifetime);
  // get value immediately
  auto result_0 = kvStore.getValue(key);

  // wait half a millisecond, should still be valid, get value
  std::this_thread::sleep_for(500us);
  auto result_1 = kvStore.getValue(key);

  // wait the lifetime (+1 ms), should no longer be valid, get value
  std::this_thread::sleep_for(lifetime + 1ms);
  auto result_2 = kvStore.getValue(key);

  std::string string_0, string_1;
  testAndCast(result_0, string_0);
  EXPECT_EQ(expectedValue, string_0);
  testAndCast(result_1, string_1);
  EXPECT_EQ(expectedValue, string_1);

  EXPECT_FALSE(result_2.has_value());
}

TEST_F(KeyValueStoreTest, deleteValue){
  const KeyValueStore::KeyType key{123455};
  kvStore.addValue(key, 123.12341);
  EXPECT_TRUE(kvStore.deleteValue(key));

  // now that it's deleted, we shouldn't be able to access it.
  const auto result = kvStore.getValue(key);
  EXPECT_FALSE(result.has_value());

  //expect that deleting an unused key returns false but doesn't throw
  bool deleted{true};
  EXPECT_NO_THROW(deleted = kvStore.deleteValue(2*key));
  EXPECT_FALSE(deleted);

  // expect that deleting the same thing we deleted above returns false
  EXPECT_FALSE(kvStore.deleteValue(key));
}

} // namespace
