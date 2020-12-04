// -*-Mode: C++;-*- // technically C99

// * BeginRiceCopyright *****************************************************
//
// --------------------------------------------------------------------------
// Part of HPCToolkit (hpctoolkit.org)
//
// Information about sources of support for research and development of
// HPCToolkit is at 'hpctoolkit.org' and in 'README.Acknowledgments'.
// --------------------------------------------------------------------------
//
// Copyright ((c)) 2002-2020, Rice University
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// * Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
//
// * Neither the name of Rice University (RICE) nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// This software is provided by RICE and contributors "as is" and any
// express or implied warranties, including, but not limited to, the
// implied warranties of merchantability and fitness for a particular
// purpose are disclaimed. In no event shall RICE or contributors be
// liable for any direct, indirect, incidental, special, exemplary, or
// consequential damages (including, but not limited to, procurement of
// substitute goods or services; loss of use, data, or profits; or
// business interruption) however caused and on any theory of liability,
// whether in contract, strict liability, or tort (including negligence
// or otherwise) arising in any way out of the use of this software, even
// if advised of the possibility of such damage.
//
// ******************************************************* EndRiceCopyright *

#ifndef SOL_HT_MEMORY_MANAGER_H
#define SOL_HT_MEMORY_MANAGER_H



//******************************************************************************
// local includes
//******************************************************************************

#include "lib/prof-lean/bistack.h"
#include "hpcrun/gpu/gpu-activity.h"
#include "../Micheal-Lock-Free-List.h"



//************************ Forward Declarations ******************************

typedef struct cct_node_t cct_node_t;



//******************************************************************************
// type declarations
//******************************************************************************

typedef struct sol_ht_object_channel_t sol_ht_object_channel_t;


typedef struct gpu_activity_channel_t gpu_activity_channel_t;


typedef struct __hp_node hazard_ptr_node;

struct __hp_node {
    _Atomic(NodeType*) hp;
    _Atomic(hazard_ptr_node*) next;
};


typedef struct sol_ht_object_details_t {
  union {
    hazard_ptr_node hpn;
    NodeType node;
  };
  gpu_activity_channel_t *initiator_channel;
} sol_ht_object_details_t;


typedef struct sol_ht_object_t {
  s_element_ptr_t next;
  sol_ht_object_channel_t *channel;
  sol_ht_object_details_t details;
} sol_ht_object_t;



//******************************************************************************
// interface operations
//******************************************************************************

sol_ht_object_t *
sol_ht_malloc
(
 void
);


void
sol_ht_free
(
 sol_ht_object_t *
);

#endif  // SOL_HT_MEMORY_MANAGER_H
