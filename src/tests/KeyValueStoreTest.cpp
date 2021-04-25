//
// Created by alexs on 4/25/21.
//

#include "KeyValueStore/KeyValueStore.h"

#include "gtest/gtest.h"

namespace {

using namespace ::testing;

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
    ASSERT_TRUE(result.has_value());
    auto value = result.value();
    ASSERT_EQ(typeid(valueString), value.type());
    std::string string;
    ASSERT_NO_THROW(string = std::any_cast<std::string>(value));
    EXPECT_EQ(string, valueString);
  }

  {
    SCOPED_TRACE("Second value string");
    auto result = kvStore.getValue(123);
    ASSERT_TRUE(result.has_value());
    auto value = result.value();
    ASSERT_EQ(typeid(otherValueString), value.type());
    std::string string;
    ASSERT_NO_THROW(string = std::any_cast<std::string>(value));
    EXPECT_EQ(string, otherValueString);
  }

  {
    SCOPED_TRACE("double value");
    auto result = kvStore.getValue(9999);
    ASSERT_TRUE(result.has_value());
    auto value = result.value();
    ASSERT_EQ(typeid(someNumber), value.type());
    double number;
    ASSERT_NO_THROW(number = std::any_cast<double>(value));
    EXPECT_EQ(number, someNumber);
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

} // namespace
