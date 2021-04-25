//
// Created by alexs on 4/25/21.
//

#include "KeyValueStore/KeyValueStore.h"

KeyValueStore::ValueAdded KeyValueStore::addValue(
    KeyValueStore::KeyType key, std::any value, KeyValueStore::OptionalDuration pairLifetime) {
  auto overridden = 0 == store_.count(key) ? ValueAdded::DidNotOverride : ValueAdded::DidOverride;
  ValuePair valuePair{std::move(value), {}};
  store_.emplace(key, std::move(valuePair));
  return overridden;
}

std::optional<std::any> KeyValueStore::getValue(KeyValueStore::KeyType key) {
  if(0 == store_.count(key)){
    return {};
  }
  const auto& [value, deadline] = store_.at(key);
  return value;
}
