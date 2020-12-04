#ifndef _SPLIT_ORDERED_LISTS_
#define _SPLIT_ORDERED_LISTS_

//******************************************************************************
// system includes
//******************************************************************************

#include <stdint.h>     // uint64_t, uintptr_t
#include <pthread.h>    // pthread_rwlock_t, pthread_rwlock_init
#include "channel/lib/prof-lean/stdatomic.h"  // atomic_fetch_add, atomic_size_t



//******************************************************************************
// local includes
//******************************************************************************

#include "Micheal-Lock-Free-List.h"
#include "channel/hashtable-memory-manager.h"



//******************************************************************************
// interface operations
//******************************************************************************

hashtable* hashtable_initialize
(
    void
);


void hashtable_destroy
(
    hashtable *htab
);


bool map_insert
(
    hashtable *htab,
    t_key key,
    val_t val
);


val_t map_search
(
    hashtable *htab,
    t_key key
);


bool map_delete
(
    hashtable *htab,
    t_key key
);

#endif  //_SPLIT_ORDERED_LISTS_
