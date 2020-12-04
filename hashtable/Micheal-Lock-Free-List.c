/* 
* Implementation of Micheal-Lock-Free-List (reference)
* This data-structure is an enhancement over Harris-Linked-List
 */

//******************************************************************************
// system includes
//******************************************************************************

#include <stdlib.h>     // NULL
#include "channel/lib/prof-lean/stdatomic.h"  // atomic_fetch_add



//******************************************************************************
// local includes
//******************************************************************************

#include "Micheal-Lock-Free-List.h"
#include "splay-tree/splay-uint64.h"



//******************************************************************************
// macros
//******************************************************************************

#define DEBUG 0
#define debug_print(fmt, ...) \
    do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)



//******************************************************************************
// type definitions
//******************************************************************************

// #define atomic_load(p)  ({ typeof(*p) __tmp = *(p); load_barrier (); __tmp; })



//******************************************************************************
// hashtable definitions
//******************************************************************************

#define st_insert				\
  typed_splay_insert(int)

#define st_lookup				\
  typed_splay_lookup(int)

#define st_delete				\
  typed_splay_delete(int)

#define st_forall				\
  typed_splay_forall(int)

#define st_count				\
  typed_splay_count(int)


// we have a special case where we only need to search for keys,
// we are not concerned with the value. Can we remove the value parameter from the struct?
typedef struct typed_splay_node(int) {
  struct typed_splay_node(int) *left;
  struct typed_splay_node(int) *right;
  uint64_t key;
  int val;
} typed_splay_node(int);


typedef typed_splay_node(int) splay_t;


typed_splay_impl(int)



//******************************************************************************
// local data
//******************************************************************************

// variables for hazard pointers
__thread hazard_ptr_node *local_hp_head;


// variables for SMR
__thread NodeType *retired_list_head;
__thread uint retired_node_count = 0;


__thread splay_t *private_ht_root = 0;



//******************************************************************************
// private operations
//******************************************************************************

// function to create Markable pointer type
static MarkPtrType create_mark_pointer(NodeType *node, uintptr_t mask_bit) {
    return (MarkPtrType) ((uintptr_t)node | mask_bit);
}


static NodeType* get_node(MarkPtrType m_ptr) {
    return (NodeType*) (((uintptr_t)m_ptr) & ~((uintptr_t)(0x1)));
}


static uintptr_t get_mask_bit(MarkPtrType m_ptr) {
    return (uintptr_t)m_ptr & 0x1;
}


static bool is_node_deleted(MarkPtrType m_ptr) {
    return (get_mask_bit(m_ptr) == 1);
}


static hazard_ptr_node* get_thread_hazard_pointers(hashtable *htab) {
    if (!local_hp_head) {
        /* how will we recycle/clear pointers for finished threads? 
        * if that is possible, we need not always malloc */
        hazard_ptr_node *hp;
        ANNOTATE_HAPPENS_BEFORE(hp);
        //hp = malloc(sizeof(hazard_ptr_node) * 3);
        sol_ht_object_t *sol_obj = sol_ht_malloc();
        hp = &(sol_obj->details.hpn);

        atomic_store(&hp[0].next, &hp[1]);
        atomic_store(&hp[1].next, &hp[2]);
        hazard_ptr_node *null_hp = NULL;
        local_hp_head = hp;


        if (atomic_compare_exchange_strong(&htab->hp_head, &null_hp, local_hp_head)) {
            // do we need to check if this returns true?
            atomic_compare_exchange_strong(&htab->hp_tail, &null_hp, &local_hp_head[2]);
        } else {
            hazard_ptr_node *current_tail = atomic_load(&htab->hp_tail);
            if(atomic_compare_exchange_strong(&(atomic_load(&htab->hp_tail)->next), &null_hp, local_hp_head)) {
                // do we need to check if this returns true?
                atomic_compare_exchange_strong(&htab->hp_tail, &current_tail, &local_hp_head[2]);
            }
            ANNOTATE_HAPPENS_AFTER(local_hp_head);
        }
        atomic_fetch_add(&htab->hazard_pointers_count, 3);
    }
    return local_hp_head;
}


static NodeType* get_hazard_pointer(hashtable *htab, int index) {
    hazard_ptr_node *hpn = get_thread_hazard_pointers(htab);
    NodeType *hp = atomic_load(&hpn[index].hp);
    return hp;
}


static void set_hazard_pointer(hashtable *htab, NodeType* node, int index) {
    hazard_ptr_node *hpn = get_thread_hazard_pointers(htab);

    /* setting hp to null because hpn is thread local.
    * Other threads dont depend on this hazard pointer */

    /*
    NodeType *null_hp = NULL;
    hpn[index].hp = null_hp;
    atomic_compare_exchange_strong(&hpn[index].hp, &null_hp, node);
    */
    // no need for atomic CAS for thread local changes
    atomic_store(&hpn[index].hp, node);
}


static void initialize_hazard_pointers(hashtable *htab) {
    /*  This function is just to fix helgrind data-race errors
    * get_thread_hazard_pointers allocates memory for thread local hazard pointers
    * list_find is usually responsible for calling the same. But it may be possbile that list_find exits
    * before this call. Helgrind is reporting that there are conflcting calls for initialization from list_insert
    * list_delete and list_search. These reports are incorrect since thread local hazard pointer cant be accessed 
    * by concurrent threads. */
    get_thread_hazard_pointers(htab);
}


static void clear_hazard_pointers(hashtable *htab) {
    hazard_ptr_node *hp = get_thread_hazard_pointers(htab);
    atomic_store(&hp[0].hp, NULL);
    atomic_store(&hp[1].hp, NULL);
    atomic_store(&hp[2].hp, NULL);
}


static MarkPtrType list_find(hashtable *htab, NodeType *head, so_key_t so_key, MarkPtrType *out_prev) {
    debug_print("list_find: %u\n", so_key);
    MarkPtrType prev = NULL, cur = NULL, next = NULL;
    initialize_hazard_pointers(htab);

    try_again:
        prev = head;
        cur = atomic_load(&get_node(prev)->next);
        /*set_hazard_pointer(cur, 1);
        if (atomic_load(prev) != create_mark_pointer(get_node(cur), 0)) {
            goto try_again;
        }*/
        while(true) {
            if (get_node(cur) == NULL) {
                goto done;
            }
            bool cmark = get_mask_bit(cur);
            //ANNOTATE_HAPPENS_AFTER(cur);
            next = atomic_load(&get_node(cur)->next);
            // ANNOTATE_HAPPENS_BEFORE_FORGET_ALL(cur);
            //ANNOTATE_HAPPENS_BEFORE_FORGET_ALL(get_node(cur));
            so_key_t ckey = cur->so_key;
            set_hazard_pointer(htab, next, 0);
            /* commented because cant see in kumpera implementation
            if (atomic_load(&cur) != create_mark_pointer(get_node(next), cmark)) {
                goto try_again;
            }*/
            if (atomic_load(&prev->next) != create_mark_pointer(get_node(cur), 0)) {
               goto try_again;
            }
            if (!cmark) {
                if (ckey >= so_key) {
                    goto done;
                }
                prev = cur; // get_node(cur)->next; why does commented code work in paper and Kumpera?
                set_hazard_pointer(htab, cur, 2);
            } else {
                MarkPtrType expected = create_mark_pointer(get_node(cur), 0);
                if (prev == expected) {
                    prev = create_mark_pointer(get_node(next), 0);
                    retire_node(htab, cur);
                } else {
                    goto try_again;
                }
            }
            cur = next;
            set_hazard_pointer(htab, next, 1);
        }
    done:
        *out_prev = prev;
        MarkPtrType result = cur;

        // checking to see if the node that we have fetched is deleted
        // cur->next will contain pointer to next node. Its last bit will denote if cur is marked for deletion.
        MarkPtrType cur_next_ptr;
        if (cur && (cur_next_ptr = atomic_load(&cur->next))) { // || get_node(cur) == NULL
            bool isDeleted = is_node_deleted(cur_next_ptr);
            if (isDeleted) {
                result = NULL;
            }
        }
        return result;
}


splay_t* splay_node(uint64_t key) {
  splay_t *node = (splay_t *) malloc(sizeof(splay_t));
  //node->left = node->right = NULL;
  node->key = key;
  return node;
}


static void local_scan_for_reclaimable_nodes(hazard_ptr_node *hp_head) {
    // stage1: Scan hp_head list and insert all non-null nodes to private hashtable phtable
    hazard_ptr_node *hp_ref = hp_head;
    while (hp_ref) {
        NodeType *n;
        if (n = atomic_load(&hp_ref->hp)) {
            st_insert(&private_ht_root, splay_node((uint64_t)n));
        }
        hp_ref = atomic_load(&hp_ref->next);
    }

    // stage2: check for all nodes in retired_list_head list to see if its present in phtable
    // if no: node is safe for reclamation/reuse, else do nothing
    NodeType *retired_list_ref = retired_list_head;
    while (retired_list_ref) {
        if (st_lookup(&private_ht_root, (uint64_t)retired_list_ref) == NULL) {
            // node can be safely reclaimed

            /* what do we do with nodes that are safe for reclamation? We can push such nodes to 
            * another private list of free nodes. Each thread will first check if it has elements
            * in its free-list before mallocing */
        }
        retired_list_ref = atomic_load(&retired_list_ref->next);
    }
}


static void global_scan_for_reclaimable_nodes() {
    /* completed/idle threads may leave behind hazard pointer nodes and elements in
    * their retired_list_head. global_scan_for_reclaimable_nodes should identify such nodes and remove them.
    * 
    * Can we identify a thread is complete and its hazard pointer head, freelist and
    * rlist can be reused */
}



//******************************************************************************
// interface operations
//******************************************************************************

// retired nodes are nodes marked for deletion, but cant be freed/reused unless checked
void retire_node(hashtable *htab, NodeType *node) {
    debug_print("retiring node: %u\n", node->key);
    /* In the paper, RETIRE_THRESHOLD = H + omega(H); where H is total no. of hazard pointers
    * what is omega(H)? Shouldnt RETIRE_THRESHOLD < H and not >= H? 
    * RETIRE_THRESHOLD should be small so that unneeded nodes are removed on regular basis
    * large threshold values will cause problems in case of idle threads */
    uint RETIRE_THRESHOLD = atomic_load(&htab->hazard_pointers_count) + 10;

    // can we safely change the next pointer of node to null?
    atomic_store(&node->next, NULL);

    if (retired_list_head) {
        atomic_store(&retired_list_head->next, node);
    } else {
        retired_list_head = node;
    }
    retired_node_count++;
    if (retired_node_count >= RETIRE_THRESHOLD) {
        local_scan_for_reclaimable_nodes(atomic_load(&htab->hp_head));
        global_scan_for_reclaimable_nodes();
    }
}


MarkPtrType list_search(hashtable *htab, MarkPtrType head, so_key_t key) {
    MarkPtrType prev;
    MarkPtrType node = list_find(htab, head, key, &prev);
    clear_hazard_pointers(htab);
    return node;
}


bool list_insert(hashtable *htab, MarkPtrType head, NodeType *node) {
    debug_print("list_insert: %p\n", node);
    bool result;
    MarkPtrType cur, prev;
    so_key_t so_key = node->so_key;

    while (true) {
        cur = list_find(htab, head, so_key, &prev);
        if (cur && cur->so_key == so_key) {
            result = false;
            break;
        }

        // since we are calling find, we are inserting the element in a sorted order in the list
        // sort order is based on split-order i.e reversed bits
        // creating a link from prev->node and node->cur (while removing prev->cur)
        atomic_store(&node->next, create_mark_pointer(get_node(cur), 0));
        // ANNOTATE_HAPPENS_AFTER(node->next);
        //ANNOTATE_HAPPENS_AFTER(node);
        MarkPtrType expected = create_mark_pointer(get_node(cur), 0);
        if (atomic_compare_exchange_strong(&(prev->next), &expected, create_mark_pointer(node, 0))) {
            result = true;
            break;
        }
    }
    clear_hazard_pointers(htab);
    return result;
}


bool list_delete(hashtable *htab, MarkPtrType head, so_key_t key) {
    bool result;
    MarkPtrType cur, next, prev;

    while (true) {
        cur = list_find(htab, head, key, &prev);
        if (!cur || cur->so_key != key) {
            result = false;
            break;
        }
        next = get_hazard_pointer(htab, 0);

        /* cur->next will contain pointer to next node. Its last bit will denote if cur is marked for deletion.
        * bit=0/1 -> not-deleted/deleted */

        // expected: cur points to next and cur is not marked for deletion
        MarkPtrType expected = create_mark_pointer(next, 0);

        // if expected matches, set cur marked for deletion
        if (!atomic_compare_exchange_strong(&(cur->next), &expected, create_mark_pointer(next, 1))) {
            continue;
        }

        // expected: prev points to cur and prev is not marked for deletion
        expected = create_mark_pointer(cur, 0);

        // if expected matches, make prev point to next
        if (atomic_compare_exchange_strong(&(prev->next), &expected, create_mark_pointer(next, 0))) {
            retire_node(htab, cur);
        } else {
            list_find(htab, head, key, &prev); // Note: Kumpera implementation commented this
        }
        result = true;
        break;
    }
    clear_hazard_pointers(htab);
    return result;
}
