#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <time.h>
#include <stdint.h>
#include <zmq.h>
#include "SHARDS.h"
#include "k_v_benchmark.h"

int main(int argc, char** argv){

	/*	argv[1] = length of each object.
		argv[2] = bucket size
		argv[3] = R
		argv[4] = mrc file 
	*/


	void* context = zmq_ctx_new ();
    void* consumer = zmq_socket (context, ZMQ_SUB);
    zmq_connect (consumer, "tcp://localhost:5555");
    zmq_setsockopt(consumer, ZMQ_SUBSCRIBE, NULL, 0);
    fprintf (stderr, "Connected consumer...\n");
	
	printf("SHARDS\n");
	int obj_length = strtol(argv[1],NULL,10);

	char* object = (char*)calloc((obj_length+2),sizeof(char));

	int bucket = strtol(argv[2],NULL,10);
	
	

	double R = strtod(argv[3], NULL);

	SHARDS *shards = SHARDS_fixed_size_init_R( 32000,R,  (unsigned int)bucket, String);

	//FILE *file;
	//file = fopen(argv[4], "r");
	clock_t start_time = clock();

	bm_op_t rec_op = {BM_WRITE_OP, 0};

	while (1) {
		//char buffer[sizeof(bm_op_t)];
        int nbytes = zmq_recv(consumer, &rec_op, sizeof(bm_op_t), 0);

        if (sizeof(bm_op_t) == nbytes) {
            //bm_op_t* op_ptr = (bm_op_t*) buffer;
            *object = (uint64_t)(rec_op.key_hv);
			SHARDS_feed_obj(shards, object, obj_length);
	
			object = (char*)calloc((obj_length+2),sizeof(char));
        }
    }
	free(object);

    //fclose(file);


	GHashTable *mrc = MRC_fixed_size_empty(shards);

	
	FILE *mrc_file = fopen(argv[4],"w");
	GList *keys = g_hash_table_get_keys(mrc);
	keys = g_list_sort(keys, (GCompareFunc) intcmp);
    GList *first = keys;
	while(keys!=NULL){
		//printf("%d,%1.7f\n",*(int*)keys->data, *(double*)g_hash_table_lookup(mrc, keys->data) );
		fprintf(mrc_file,"%7d,%1.7f\n",*(int*)keys->data, *(double*)g_hash_table_lookup(mrc, keys->data) );


		keys=keys->next;
	}
	clock_t end_time = clock();
	
	fclose(mrc_file);
	g_list_free(first);
	g_hash_table_destroy(mrc);
	
	printf("%ld\n", start_time);
	printf("%ld\n", end_time);
	int total_time = ((end_time - start_time))/CLOCKS_PER_SEC;
	printf("TIME: %d\n", total_time);
	unsigned int objects_parsed =  shards->total_objects;
    
	double throughput = objects_parsed/(total_time+1);
    SHARDS_free(shards);
	printf("Throughput: %f\n", throughput);
    return 0;

}
