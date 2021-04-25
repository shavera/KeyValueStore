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
    auto value = kvStore.getValue(1);
    ASSERT_EQ(typeid(valueString), value.type());
    std::string string;
    ASSERT_NO_THROW(string = std::any_cast<std::string>(value));
    EXPECT_EQ(string, valueString);
  }

  {
    SCOPED_TRACE("Second value string");
    auto value = kvStore.getValue(123);
    ASSERT_EQ(typeid(otherValueString), value.type());
    std::string string;
    ASSERT_NO_THROW(string = std::any_cast<std::string>(value));
    EXPECT_EQ(string, otherValueString);
  }

  {
    SCOPED_TRACE("double value");
    auto value = kvStore.getValue(9999);
    ASSERT_EQ(typeid(someNumber), value.type());
    double number;
    ASSERT_NO_THROW(number = std::any_cast<double>(value));
    EXPECT_EQ(number, someNumber);
  }
}

} // namespace
