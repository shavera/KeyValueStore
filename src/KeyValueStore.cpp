//
// Created by alexs on 4/25/21.
//

#include "KeyValueStore/KeyValueStore.h"

KeyValueStore::ValueAdded KeyValueStore::addValue(
    uint64_t key, std::any value, KeyValueStore::OptionalDuration pairLifetime) {
  ValuePair valuePair{std::move(value), {}};
  store_.emplace(key, std::move(valuePair));
  return ValueAdded::DidNotOverride;
}

std::optional<std::any> KeyValueStore::getValue(uint64_t key) {
  if(0 == store_.count(key)){
    return {};
  }
  const auto& [value, deadline] = store_.at(key);
  return value;
}
