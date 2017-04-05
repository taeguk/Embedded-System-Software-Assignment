//
// Created by taeguk on 2017-04-06.
//

#include "atomic.h"

inline void atomic_store_bool (volatile bool *ptr, bool new_value)
{
  __atomic_store_n (ptr, new_value, __ATOMIC_SEQ_CST);
}

inline bool atomic_load_bool (volatile bool *ptr)
{
  return __atomic_load_n (ptr, __ATOMIC_SEQ_CST);
}

inline bool atomic_exchange_bool (volatile bool *ptr, bool new_value)
{
  return __atomic_exchange_n (ptr, new_value, __ATOMIC_SEQ_CST);
}