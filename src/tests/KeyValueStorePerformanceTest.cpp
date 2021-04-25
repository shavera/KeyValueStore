//
// Created by alexs on 4/25/21.
//


#include "KeyValueStore/KeyValueStore.h"

#include "gtest/gtest.h"

#include <cstdint>
#include <future>
#include <random>

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
      keys[i] = key;
      keyValueStore.addValue(key, value, lifetime);
      expectedValues.emplace(key, value);
      expectedDeadlines.emplace(key, deadline);
    }
    const auto stop = std::chrono::system_clock::now();
    const std::chrono::duration<double> elapsedTime = stop - start;
    std::cout << "Set Up elapsed time: " << elapsedTime.count() << std::endl;
  }

  /// set the value at key to 0, return the amount of time to set the value.
  std::chrono::microseconds addValueWorker(KeyValueStore::KeyType key);

  /// get the value for key from the store, return the amount of time to get the value.
  std::chrono::microseconds getValueWorker(KeyValueStore::KeyType key);

  std::array<size_t, pairCount> keys{0};
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
  std::cout << "Encountered " << expiredCount << " expired entries." << std::endl;
}

TEST_F(KVStorePerformanceWithDeadlinesTest, manyThreadAccess){
  // Test what happens when many threads attempt to perform actions possibly simultaneously on the store
  // Start with pre-seeded store so that some threads can report back their activity.

  // Note, trying to async launch _all_ keys was excessive. Since the keys are shuffled, a subset of keys
  // should be sufficient to test with
  constexpr size_t maxAsyncLaunches{std::min(pairCount, static_cast<decltype(pairCount)>(20000))};

  // not strictly necessary, but we can also profile how long it takes to set data as well
  std::array<size_t, 100> addTimeHistogram{0};
  std::vector<std::future<std::chrono::microseconds>> addFutures;
  std::vector<std::future<std::chrono::microseconds>> getFutures;

  // randomize the order of keys
  std::shuffle(keys.begin(), keys.end(), rng);

  std::bernoulli_distribution actionSelector{0.5};

  // for each key, asynchronously either set or get the value at that key, returning futures as a result.
  std::cout << "Beginning ASync command launch" << std::endl;
  for(auto i{0u}; i < maxAsyncLaunches; ++i){
    auto key = keys.at(i);
    bool useGet = actionSelector(rng);
    if(useGet){
      auto futureResult = std::async(std::launch::async, [this, key](){return getValueWorker(key);});
      getFutures.emplace_back(std::move(futureResult));
    } else {
      auto futureResult = std::async(std::launch::async, [this, key](){return addValueWorker(key);});
      addFutures.emplace_back(std::move(futureResult));
    }
  }
  std::cout<< "All Async commands launched" << std::endl;

  for(auto& future : addFutures){
    size_t bin = future.get().count()/microsecondResolution;
    addTimeHistogram[bin] += 1;
  }

  for(auto& future : getFutures){
    size_t bin = future.get().count()/microsecondResolution;
    timeHistogram[bin] += 1;
  }

}

/// set the value at key to 0, return the amount of time to set the value.
std::chrono::microseconds KVStorePerformanceWithDeadlinesTest::addValueWorker(KeyValueStore::KeyType key){
  const auto start = std::chrono::system_clock::now();
  keyValueStore.addValue(key, 0);
  const auto stop = std::chrono::system_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(stop-start);
}

/// get the value for key from the store, return the amount of time to get the value.
std::chrono::microseconds KVStorePerformanceWithDeadlinesTest::getValueWorker(KeyValueStore::KeyType key){
  const auto start = std::chrono::system_clock::now();
  auto value = keyValueStore.getValue(key);
  const auto stop = std::chrono::system_clock::now();
  if(DEBUG_TEST){
    std::cout << std::chrono::duration<double>(stop - start).count() << std::endl;
  }
  return std::chrono::duration_cast<std::chrono::microseconds>(stop-start);
}

} // namespace