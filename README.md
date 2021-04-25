# KeyValueStore
Coding Exercise for a fast retrieval key-value store

The caller of the library should be able to:

* Put a key/value pair
* Put a key/value pair that has an optional expiration value
* Retrieve a key/value pair
* Delete a key/value pair
* Be used within a process that includes multiple threads

Performance targets:

* Retrieve a key with a 95th percentile time of less than 1 millisecond
* Retrieve a key with a 99th percentile time of less than 5 milliseconds
* Handle up to 10 million key/value pairs

Additional notes:

* Each key is unique, pushing to same key overwrites existing key
* You can choose any type you would like for the key, it should be consistent. You should document it and be prepared to
  discuss the tradeoffs inherent to the chosen type. The values should support any data type. The data types supported 
  by the system should be usable on a per set operation basis and can be different from one operation to the next.
  - Choosing uint64_t as key - Elsewhere a user could define some hashing function to calculate a uint64_t hash for
    the data type they're using.
  - For the data type, to support the requirement that 'any' kind of data be stored, I'll be using the 'std::any' type.
    It's a newer type and I'm aware of at least one bug in some compilers dealing with it, but hopefully that won't 
    impact this code. It will be up to the user to perform the appropriate `any_cast` to the appropriate type.

Implementation plan:

1. Test and implement the library functionality
2. Write tests around performance metrics
3. Profile and optimize to meet performance metrics
