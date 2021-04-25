//
// Created by alexs on 4/25/21.
//

#ifndef KEYVALUESTORE_KEYVALUESTORE_H
#define KEYVALUESTORE_KEYVALUESTORE_H

#include <any>
#include <chrono>
#include <optional>
#include <unordered_map>

class KeyValueStore {
public:
  using OptionalDuration = std::optional<std::chrono::milliseconds>;
  void addValue(uint64_t key, std::any value, OptionalDuration pairLifetime = {});

  std::any getValue(uint64_t key);

private:
  using OptionalDeadline =
      std::optional<std::chrono::time_point<std::chrono::system_clock>>;

  using ValuePair = std::pair<std::any, OptionalDeadline>;

  std::unordered_map<uint64_t, ValuePair> store_;
};

#endif // KEYVALUESTORE_KEYVALUESTORE_H
