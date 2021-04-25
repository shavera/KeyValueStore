//
// Created by alexs on 4/25/21.
//

#include "KeyValueStore/KeyValueStore.h"

namespace {
std::optional<std::chrono::time_point<std::chrono::system_clock>> calculateDeadline(
    const KeyValueStore::OptionalDuration & lifetime);
} // namespace

KeyValueStore::ValueAdded KeyValueStore::addValue(
    KeyValueStore::KeyType key, std::any value, KeyValueStore::OptionalDuration pairLifetime) {
  auto overridden = 0 == store_.count(key) ? ValueAdded::DidNotOverride : ValueAdded::DidOverride;

  ValuePair valuePair{std::move(value), calculateDeadline(pairLifetime)};
  store_.emplace(key, std::move(valuePair));

  return overridden;
}

std::optional<std::any> KeyValueStore::getValue(KeyValueStore::KeyType key) {
  if(0 == store_.count(key)){
    return {};
  }
  const auto& [value, deadline] = store_.at(key);
  if(deadline.has_value() && std::chrono::system_clock::now() > deadline.value()){
    // removing this from the table so that follow-on override checks don't accidentally count it
    store_.erase(key);
    return {};
  }
  return value;
}

namespace {
std::optional<std::chrono::time_point<std::chrono::system_clock>> calculateDeadline(
    const KeyValueStore::OptionalDuration& lifetime){
  if(!lifetime.has_value()){
    return {};
  }
  return std::chrono::system_clock::now() + lifetime.value();
}
} // namespace
