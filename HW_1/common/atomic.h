//
// Created by taeguk on 2017-04-05.
//

#ifndef EMBEDDED_SYSTEM_SOFTWARE_ASSIGNMENT_ATOMIC_H
#define EMBEDDED_SYSTEM_SOFTWARE_ASSIGNMENT_ATOMIC_H

#include <stdbool.h>

void atomic_store_bool (volatile bool *ptr, bool new_value);
bool atomic_load_bool (volatile bool *ptr);
bool atomic_exchange_bool (volatile bool *ptr, bool new_value);

#endif //EMBEDDED_SYSTEM_SOFTWARE_ASSIGNMENT_ATOMIC_H
