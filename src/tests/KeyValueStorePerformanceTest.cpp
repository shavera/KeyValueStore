//
// Created by alexs on 4/25/21.
//


#include "KeyValueStore/KeyValueStore.h"

#include "gtest/gtest.h"

#include <cstdint>
#include <random>
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

//constexpr bool DEBUG_TEST{true};
constexpr bool DEBUG_TEST{false};

//constexpr int microsecondResolution{1};
constexpr int microsecondResolution{100};

constexpr size_t pairCount{DEBUG_TEST ? 10000 : 10'000'000};

// choosing random, but fixed, seed for reproducibility
std::mt19937_64 rng{0xede26709};

// for the data, to keep it simple to generate, we'll use 'double' as our data type
std::uniform_int_distribution<size_t> keyDistribution{0, SIZE_MAX - 1};
std::uniform_real_distribution<double> valueDistribution{-1000, 1000};

using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

class KVStorePerformanceTest : public Test{
public:
  void SetUp() override{
    for(auto i = 0u; i < pairCount; ++i){
      auto key = keyDistribution(rng);
      auto value = valueDistribution(rng);
      keyValueStore.addValue(key, value);
      expectedValues.emplace(key, value);
    }
  }

  void TearDown() override{
    std::cout << "[";
    for(const auto& v : timeHistogram){
      std::cout << v << ", ";
    }
    std::cout << "]" << std::endl;
  }

  void updateHistogram(const TimePoint& start, const TimePoint& stop){
    std::chrono::microseconds elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    if(DEBUG_TEST){
      // if debugging the test print the double value of debug time to show it's not trivially 0.0
      std::chrono::duration<double> debugTime{start - stop};
      std::cout << debugTime.count() << std::endl;

      ASSERT_TRUE(elapsedTime.count() >= 0);
    }
    size_t bin = elapsedTime.count()/microsecondResolution;
    timeHistogram[bin] += 1;
  }

  std::unordered_map<size_t, double> expectedValues;

  KeyValueStore keyValueStore;

  // Going to bin by 100 microsecond bins up to 10 milliseconds
  // (note, altering microsecond resolution variable will keep 100 bins but each bin will be that resolution in width)
  std::array<size_t, 100> timeHistogram{0};
};

TEST_F(KVStorePerformanceTest, AccuracyAndTimeTest){
  auto index{0u};

  for(const auto& [key, value] : expectedValues){
    auto start = std::chrono::system_clock::now();
    auto result = keyValueStore.getValue(key);
    auto stop = std::chrono::system_clock::now();
    updateHistogram(start, stop);
    double actualValue{};
    testAndCast(result, actualValue);
    EXPECT_EQ(value, actualValue) << index;
    ++index;
  }
}

// In my environment the KVStorePerformanceTest takes approximately 20 seconds to run, so allowing deadlines up to 60
// seconds in length
std::uniform_int_distribution<> deadlineDistribution{0, DEBUG_TEST ? 10 : 60000};

class KVStorePerformanceWithDeadlinesTest : public KVStorePerformanceTest{
public:
  void SetUp() override{
    const auto start = std::chrono::system_clock::now();
    for(auto i = 0u; i < pairCount; ++i){
      auto key = keyDistribution(rng);
      auto value = valueDistribution(rng);
      auto lifetime = std::chrono::milliseconds{deadlineDistribution(rng)};
      auto deadline = std::chrono::system_clock::now() + lifetime;
      keyValueStore.addValue(key, value, lifetime);
      expectedValues.emplace(key, value);
      expectedDeadlines.emplace(key, deadline);
    }
    const auto stop = std::chrono::system_clock::now();
    const std::chrono::duration<double> elapsedTime = stop - start;
    std::cout << "Set Up elapsed time: " << elapsedTime.count() << std::endl;
  }

  std::unordered_map<size_t, std::chrono::time_point<std::chrono::system_clock>> expectedDeadlines;
};

TEST_F(KVStorePerformanceWithDeadlinesTest, deadlineChecked){
  // we've already checked above that the values are consistent, this test just confirms deadline expectations
  auto expiredCount{0u};
  for(const auto& [key, value] : expectedValues){
    auto start = std::chrono::system_clock::now();
    auto result = keyValueStore.getValue(key);
    auto stop = std::chrono::system_clock::now();
    updateHistogram(start, stop);
    if(expectedDeadlines.at(key) < start){
      if(DEBUG_TEST){
        std::cout << ".";
      }
      ++expiredCount;
      EXPECT_FALSE(result.has_value());
    } else {
      if(DEBUG_TEST){
        std::cout << ",";
      }
      EXPECT_TRUE(result.has_value());
    }
  }
  std::cout << "Encountered " << expiredCount << " expired entries.";
}

} // namespace