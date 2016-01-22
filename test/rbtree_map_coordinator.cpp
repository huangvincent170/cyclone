// Implement a distrbuted transaction co-ordinator for the red black tree

TOID_DECLARE(uint64_t, TOID_NUM_BASE);

static TOID(uint64_t) txid;
void **quorum_handles;
int me;
int quorums;


TOID(char) nvheap_setup(TOID(char) recovered,
			PMEMobjpool *state)
{
  TOID(char) store;
  if(TOID_IS_NULL(recovered)) {
    txid = TX_ALLOC(uint64_t, sizeof(uint64_t));
    *D_RW(txid) = 1;
    store = TX_ALLOC(char, sizeof(TOID(uint64_t)));
    TX_MEMCPY(D_RW(store), &txid, sizeof(TOID(uint64_t)));
  }
  else {
    TX_MEMCPY(&txid, D_RO(store), sizeof(TOID(uint64_t)));
  }
  return store; 
}


int leader_callback(const unsigned char *data,
		    const int len,
		    unsigned char **follower_data,
		    int * follower_data_size, 
		    void **return_value)
{
  rbtree_tx_t * tx = (rbtree_tx_t *)data;

  for(int i=0;i<tx->num_locks;i++) {
  }

  for(int i=0;i<tx->num_versions;i++) {
  }

  for(int i=0;i<tx->num_inserts;i++) {
  }

  for(int i=0;i<tx->num_deletes;i++) {
  }

  for(int i=0;i<tx->num_locks;i++) {
  }
  
}

int follower_callback(const unsigned char *data,
		      const int len,
		      unsigned char *follower_data,
		      int follower_data_size, 
		      void **return_value)
{
  struct coordinator_status stat =
    (struct coordinator_status *)follower_data;
  TX_ADD(txid);
  uint64_t *txidp = pmemobj_direct(txid);
  *txidp = *txidp + stat->delta_txid;
  *return_value = malloc(sizeof(int));
  *(int *)*return_value = stat->tx_status;
  return sizeof(int);
}

int main(int argc, char *argv[])
{
  if(argc < 7) {
    fprintf(stderr,
	    "Usage: %s me_client me_server clients replicas quorums quorum1.ini ...\n",
	    argv[0]);
    
  }
  me_client    = atoi(argv[1]);
  me_server    = atoi(argv[2]);
  int clients  = atoi(argv[3]);
  int replicas = atoi(argv[4]);
  quorums  = atoi(argv[5]);
  quorum_handles = new void *[quorums];
  for(int i=0;i<quorums;i++) {
    quorum_handles[i] = cyclone_client_init(me_client,
					    replicas,
					    clients,
					    argv[5 + i]);
  }
}