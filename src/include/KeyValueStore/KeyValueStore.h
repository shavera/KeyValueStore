//
// Created by alexs on 4/25/21.
//

#ifndef KEYVALUESTORE_KEYVALUESTORE_H
#define KEYVALUESTORE_KEYVALUESTORE_H

#include <any>
#include <chrono>
#include <optional>
#include <unordered_map>

/**
 * Class that can store size_t type keys mapped to 'std::any' values. Entries (key-value pairs) may also allow for
 * a lifetime to be specified. If more time passes between when the entry is added and when it is requested, the
 * request will not return a valid result.
 *
 * size_t type was chosen as a key type because it is the result of std::hash methods. So if any type of key is desired,
 * one may define a hash method that creates a 'size_t' type and use that for the key. The usual
 * considerations of hash collisions must be considered whether appropriate.
 */
class KeyValueStore {
public:
  /// Alias for type/unit to specify allowed lifetime of entry
  using OptionalDuration = std::optional<std::chrono::milliseconds>;

  /// Enums representing whether addValue overrode an existing value
  enum class ValueAdded : bool {DidNotOverride = false, DidOverride = true};

  using KeyType = size_t;

  /**
   * @brief Add an entry to the store
   *
   * If an element of existing key exists, the value will be overridden.
   *
   * @param key A uint64_t key. This library assumes that if other key types are desired, they may be hashed to
   * uint64_t values to act as a key here.
   *
   * @param value A std::any representing any kind of data to be stored here. Users of this class will be responsible
   * for performing the relevant 'any_cast' upon retrieval.
   *
   * @param pairLifetime An optional duration that may be specified. If this amount of time elapses prior to requesting
   * the value, no valid value will be returned.
   *
   * @returns an enum indicating whether this entry overrode an existing value in the table.
   */
  ValueAdded addValue(KeyType key, std::any value, OptionalDuration pairLifetime = {});

  /**
   * @brief Attempt to retrieve a value associated with a given key.
   *
   * @param key Key to search for. Similar to 'addValue' method, if some other method is generating a hash for this
   * key, the same hash must be generated to search for here.
   *
   * @returns An optional any. If the optional does not have a value it could not find a valid result.
   */
  [[nodiscard]] std::optional<std::any> getValue(KeyType key);

private:
  using OptionalDeadline =
      std::optional<std::chrono::time_point<std::chrono::system_clock>>;

  using ValuePair = std::pair<std::any, OptionalDeadline>;

  std::unordered_map<KeyType, ValuePair> store_;
};

#endif // KEYVALUESTORE_KEYVALUESTORE_H
