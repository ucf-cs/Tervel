/*****
 Time-stamp: <2013-11-26 12:05:52 jonatanlinden>
 * Verification of the linearizability of the priority queue algorithm
 * presented in the paper, and that the algorithm implements a priority
 * queue.
 * 
 * Some optimizations can be done, like reusing variables, replacing
 * the CAS macros, changing
 * some types from byte to bit (i.e. if limiting the number of levels
 * to two). We've omitted them here for the sake of clarity.
 * 
 * Adapted from Martin Vechev et al., Experience with Model Checking
 * Linearizability, 2009.
 *
 * 
 */

#define IF if ::
#define FI :: else fi

#define CAS(a, d, o, n) \
    cas_success = 0;     \
    if :: (d == 0 && a == o) -> a = n; cas_success = 1; \
    :: else fi

#define FAO(a,v) \
     a; a = v;

#define WHILE do ::
#define ELIHW :: else -> break; od

#define GCASSERT(new, old) \
    assert(nodes[new].recycled == 0 || nodes[old].recycled);

#define NLEVELS 3 /* 3 level skiplist */
#define THREADS 3 /* 2 threads */

#define MAX_OPS 3 /* no. of random ops per thread */
#define BOUNDOFFSET 2 /* restructure offset */

#define NODES 15  /* total memory */

/* Operation types. */
#define INS   0
#define DEL   1

/* types */
#define key_t byte
#define idx_t byte

typedef node_t {
  key_t key;
  byte level;
  bit inserting;
  bit recycled;
  /* the following 2 fields are colocated in one mem pos,
   * and should be treated as such. */
  bit d;
  idx_t next[NLEVELS];
}

typedef queue_t {
  idx_t head, tail;
}

/* this is the memory */
node_t nodes[NODES];

/**********  declaration of global variables *************/


queue_t q; /* the priority queue */
byte seqq[NODES]; /* the sequential spec. */
idx_t glob_entry; /* pointer to free memory */

/********* sequential specification **************/

/* adding */
inline seq_add(entry, k) {
  assert(seqq[k] == 0);
  seqq[k] = 1;
}

/* removing - element should be the smallest */
inline seq_remove(kl) {
  assert(seqq[kl]);
    for (j : 0..kl-1) {
    assert(seqq[j] == 0);
  }
  seqq[kl] = seqq[kl] - 1;
}
/* if empty, no entry in queue */
inline seq_empty() {
  for (j : 0..(NODES-1)) {
    assert(seqq[j] == 0);
  }
}

/************* Handling nodes/memory *****************/

inline get_entry(ptr)
{
  d_step{
    ptr = glob_entry;
    assert(ptr < NODES - 1);
    glob_entry++;
  }
}


/* return index pointing to a node being free to use */
inline alloc_node(new, k)
{
  atomic {
    get_entry(new);
    nodes[new].key = k;
    select(i : 0..(NLEVELS - 1)); /* ok, since called before locatepreds */
    nodes[new].level = i;
    nodes[new].inserting = 1;
  }
}


/*******************************************************************
 * BEGIN PRIORITY QUEUE ALGORITHM
 *******************************************************************/


/* CAS(addr, d, old, new) - representing a CAS, that will update addr
 * to new, given that addr = old and d = 0. d represents hence the
 * delete bit being a part of old. */

/* FAO(addr, val) - representing a Fetch-and-Or, that will update
 * addr to *addr | val. */

inline LocatePreds(key) {
  d_step { /* resetting some local vars */
    cur = 0; pred = 0; d = 0; del = 0;
    i = NLEVELS; pred = q.head
  }
  /* NB: index i is offset by one in comparison to paper,
   * due to lack of negative bytes in promela */
  WHILE (i > 0) ->  /* for each level */
    d_step { /* colocated together */
      cur = nodes[pred].next[i-1];
      d = nodes[pred].d
    }
    WHILE (nodes[cur].key < key || nodes[cur].d || (d && i == 1)) ->
      atomic {
        IF (d && i == 1) -> del = cur FI;
        pred = cur; /* local */
        /* colocated together */
        cur = nodes[pred].next[i-1]; 
        d = nodes[pred].d
      }
    ELIHW;
    atomic { /* local vars */
      preds[i-1] = pred;
      succs[i-1] = cur;
      i-- /* descend to next level */
    }
  ELIHW
}

inline Insert(key) {
  alloc_node(new, key)

retry:
  LocatePreds(key)
  /* bail out if key already exists */
  IF (nodes[succs[0]].key == key && !nodes[preds[0]].d
      && nodes[preds[0]].next[0] == succs[0]) -> goto end_insert FI;
  nodes[new].next[0] = succs[0];
  /* Lowest level */
  atomic { /* linearization point of non-failed insert */
    CAS(nodes[preds[0]].next[0], nodes[preds[0]].d, succs[0], new);
    if :: (cas_success) ->
          seq_add(new, key)
          GCASSERT(succs[0], new)
       :: else -> goto retry /* restart */
    fi
  }
  /* swing upper levels */
  j = 1; /* i is being used in locatepreds */
  WHILE (j <= nodes[new].level) -> 
    nodes[new].next[j] = succs[j];
    IF (nodes[new].d || nodes[succs[i]].d || succs[i] == del) -> goto end_insert FI;
    atomic {
      CAS(nodes[preds[j]].next[j], 0, succs[j], new);
      IF (cas_success) ->
        GCASSERT(succs[j], new)
        j++
      FI
    }
    IF (!cas_success) ->
      LocatePreds(key) /* update preds, succs and del */
      IF (succs[0] != new) -> goto end_insert FI
    FI
  ELIHW;
end_insert:
  nodes[new].inserting = 0
}

inline Restructure() {
  i = NLEVELS - 1; pred = q.head;
re_continue: 
  WHILE (i > 0) ->
    h = nodes[q.head].next[i];
    cur = nodes[pred].next[i];
    IF (!nodes[h].d) -> i--; goto re_continue FI;
    WHILE (nodes[cur].d) ->
      pred = cur;
      cur = nodes[pred].next[i]
    ELIHW;
    atomic {
      CAS(nodes[q.head].next[i], 0, h, nodes[pred].next[i]);
      IF (cas_success) ->
        GCASSERT(nodes[pred].next[i], q.head)
        i--
      FI
    }
  ELIHW
}

inline DeleteMin () {
  d_step {  
    d = 1; x = q.head; offset = 0;
    obshead = nodes[x].next[0]
  }  
  WHILE (d) ->
    atomic {
      offset ++;
      /* nxt & d colocated */
      nxt = nodes[x].next[0];
      d   = nodes[x].d;
      IF (nxt == q.tail) ->
        /* empty: got linearized when reading nxt */
        seq_empty()
        goto end_remove
      FI
    }
    IF (nodes[x].inserting && newhead == NODES) ->
      newhead = x
    FI;
    atomic {
      /* linearization point */
      d = FAO(nodes[x].d, 1) 
      IF (!d) ->
        /* check linearization */
        key = nodes[nodes[x].next[0]].key;
        seq_remove(key)
      FI
    }
    x = nodes[x].next[0]
  ELIHW;
  IF (offset <= BOUNDOFFSET) -> goto end_remove FI;
  IF (newhead == NODES) -> newhead = x FI;
  atomic {
    CAS(nodes[q.head].next[0], 0, obshead,newhead);
    if :: (cas_success) -> GCASSERT(newhead, q.head)
       :: else -> goto end_remove
    fi
  }
  Restructure()
  cur = obshead;
  WHILE (cur != newhead) ->
       nxt = nodes[cur].next[0];
       nodes[cur].recycled = 1; /* MarkRecycle */
       cur = nxt
  ELIHW;
end_remove:
}


/*******************************************************************
 * END ALGORITHM
 *******************************************************************/

/* random key generator.  */
inline pick_key(var) { 
  atomic {
    select(var : 1..5);
  }
}


inline start_op() {
  init_locals();
};

inline end_op() {
  d_step {
  key = 0;
  op = 0;
  new = 0;
  }
}

inline exec_op(key) {
  start_op();
  assert(key < NODES);
  if
    :: op = INS;
       pick_key(key);
       Insert (key);
    :: op = DEL; DeleteMin();
  fi;

  end_op();
}


inline execute()
{
  byte _dummy1;
  for (_dummy1 : 0..(MAX_OPS)) {
	  exec_op(key);
  }
}

inline init_locals()
{
  d_step {
    pred = 0;
    cur = 0;
    d = 0;
    preds[0] = 0;
    preds[1] = 0;
    succs[0] = 0;
    succs[1] = 0;
    op = 0;
    offset = 0;
    obshead = 0;
    del = 0; /* ok, succs will never be 0 */
    cas_success = 0;
    h = 0;
    i = 0;
    j = 0;
    new = 0;
    key = 0;
    x = 0;
    nxt = 0;
    newhead = NODES;
  }
}

inline define_locals()
{
  idx_t pred, cur, obshead, offset, newhead, h, x, nxt;
  idx_t preds[NLEVELS], succs[NLEVELS], del;
  byte i,j;
  bit op, d, cas_success;
  byte key;
  
  idx_t new;
  init_locals();
}


proctype client() {
  define_locals();
  execute();
}


inline init_globals()
{
  /* init the structure */
  atomic {
    glob_entry = 0;
    /* tail */
    alloc_node(new, 7);
    q.tail = new;
    nodes[q.tail].level = 1;
    nodes[q.tail].inserting = 0;

    alloc_node(new, 0);
    q.head = new;
    nodes[q.head].level = 1;
    nodes[q.head].inserting = 0;
    for (j : 0..1) { /* levels */
      nodes[q.head].next[j] = q.tail;
    };
  }
}


init {
  atomic{
    byte _dummy0;
    define_locals();
    init_globals();
for ( _dummy0 : 1..(THREADS - 1)) {
      run client();
    }
  }
  execute();

  /* wait until the other process finishes. */
  _nr_pr == 1;
  i = nodes[q.head].next[0];
  printf("h, %d -> ", nodes[q.head].d);
  do :: (i != q.tail) ->
	printf("%d,%d ->", nodes[i].key, nodes[i].d);
	i = nodes[i].next[0];
     :: else -> break;
  od;
  printf("t\n");
}

