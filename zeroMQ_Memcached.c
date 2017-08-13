#include "k_v_benchmark.h"
#include "SHARDS.h"
#include <zmq.h>
#include <assert.h>
#include <inttypes.h>
 

int main (int argc, char *argv [])
{

    #define MAX_NUMBER_OF_SLAB_CLASSES (63 + 1)
    #define POWER_SMALLEST 1
    #define CHUNK_ALIGN_BYTES 8
    //int chunk_size = 48;
    const double factor = 1.25;
    int slab_chunk_size_max = 1024 * 1024;
    int NUMBER_OF_SHARDS =0;

    // NAmes of files where the curves are going to be written
    FILE *mrc_file=NULL;
    char* mrc_path = "./Results/";
    char file_name[40];

    int power_largest;
    int i = POWER_SMALLEST - 1;

    SHARDS *shards_array[MAX_NUMBER_OF_SLAB_CLASSES];
    unsigned int item_sizes[MAX_NUMBER_OF_SLAB_CLASSES];
    
    /*
        This line:
        unsigned int size = sizeof(item) + settings.chunk_size;
        is replaced with the following

    */
    /*
    int slab_sizes[] = { 96, 
                        120, 
                        152, 
                        192, 
                        240, 
                        304, 
                        384, 
                        480, 
                        600,
                        752, 
                        944, 
                        1184, 
                        1480, 
                        1856,
                        2320, 
                        2904, 
                        3632, 
                        4544, 
                        5680, 
                        7104, 
                        8880, 
                        11104, 
                        13880,
                        17352, 
                        21696, 
                        27120, 
                        33904, 
                        42384, 
                        52984, 
                        66232, 
                        82792, 
                        103496, 
                        129376, 
                        161720, 
                        202152, 
                        252696, 
                        315872, 
                        394840, 
                        493552,
                        616944, 
                        771184, 
                        1048576 };
                        */

    int *slab_sizes =NULL;
    unsigned int size = 96;
    //printf("max chunk size: %d\n", slab_chunk_size_max);
    //printf("DOUBLE: %f\n",  slab_chunk_size_max/factor);
    while (++i < MAX_NUMBER_OF_SLAB_CLASSES-1) {

        if (slab_sizes != NULL) {
            if (slab_sizes[i-1] == 0)
                break;
            size = slab_sizes[i-1];
        } else if (size >= slab_chunk_size_max / factor) {
            //printf("BREAK\n");
            break;
        }
        /* Make sure items are always n-byte aligned */
        if (size % CHUNK_ALIGN_BYTES)
            size += CHUNK_ALIGN_BYTES - (size % CHUNK_ALIGN_BYTES);

      
        
        //initialize each SHARDS struct in the shards array. Index is [i-1] because the numbering starts at 
        // one and no at zero.
        shards_array[i-1] = SHARDS_fixed_size_init(16000, 10, Uint64);
        item_sizes[i-1] = size;
        fprintf(stderr,"JORGE SIZE %2d: %10u\n", i, size);
        if (slab_sizes == NULL)
            size *= factor;

       
    }

    power_largest = i;
    NUMBER_OF_SHARDS = i;
    size = slab_chunk_size_max;
    shards_array[i-1] = SHARDS_fixed_size_init(16000, 10, Uint64);
    item_sizes[i-1] = size;
    fprintf(stderr,"JORGE SIZE %2d: %10u\n", i, size);

    // End initialization
    // Display the SHARDS structs
    for(int j=0; j < NUMBER_OF_SHARDS; j++){
        printf("R Value %2d: %1.3f\n",j+1, shards_array[j]->R);

    }



    //  Socket to talk to server
    
    void *context = zmq_ctx_new ();
    void *subscriber = zmq_socket (context, ZMQ_SUB);
    int rc = zmq_connect (subscriber, "tcp://localhost:5555");
    assert (rc == 0);

  
    //rc = zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE, filter, strlen (filter));
    rc = zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE, NULL, 0);
    assert (rc == 0);

    int OBJECT_LIMIT= 611968;
    unsigned int epoch =1;
    int number_of_objects=0;

         
    bm_op_t rec_op = {BM_WRITE_OP, 0, 0};
    uint64_t *object = malloc(sizeof(uint64_t));
    *object = rec_op.key_hv;
    
    int cnt =0;
    unsigned int slab_ID = 0;

    fprintf(stderr,"LISTENING FOR OBJECTS....\n");

    while(1){

        

        zmq_recv(subscriber,&rec_op , sizeof(bm_op_t), 0);
        slab_ID = rec_op.slab_id - 128;

        printf("%12"PRIu64":  %"PRIu8"\n", rec_op.key_hv, slab_ID);

        *object = rec_op.key_hv;
    
        SHARDS_feed_obj(shards_array[slab_ID -1] ,object , sizeof(uint64_t));
        number_of_objects ++;

        if(number_of_objects==OBJECT_LIMIT){
        printf("CALCULATING Miss Rate Curves...\n");

            for( int k =0; k< NUMBER_OF_SHARDS; k++){
                printf("PING: %d\n", k+1);
                if(shards_array[k]->total_objects !=0 ){
                    printf("SHARDS has objects.\n");
                    snprintf(file_name,40,"%sMRC_SLAB_%02d_%05d.csv",mrc_path, k+1, epoch);
                    fprintf(stderr, "Calculating MRC of Slab %2d (size %2u)\n", k+1, item_sizes[k]);

                    
                    GHashTable *mrc = MRC_fixed_size_empty(shards_array[k]);

                    GList *keys = g_hash_table_get_keys(mrc);
                    keys = g_list_sort(keys, (GCompareFunc) intcmp);

                    mrc_file = fopen(file_name,"w");

                    printf("WRITING MRC FILE...\n");
                    while(1){
                        //printf("key: %7d  Value: %1.6f\n",*(int*)keys->data, *(double*)g_hash_table_lookup(mrc, keys->data) );
                        fprintf(mrc_file,"%7d,%1.7f\n",*(int*)keys->data, *(double*)g_hash_table_lookup(mrc, keys->data) );

                        if(keys->next==NULL)
                            break;
                        keys=keys->next;
                    }

                    fclose(mrc_file);
                    printf("MRC FILE WRITTEN! :D\n");
                    keys = g_list_first(keys);
                    g_list_free(keys);
                    g_hash_table_destroy(mrc);

                }

                

            }
            number_of_objects = 0;
            epoch++;

        }
          
        object = malloc(sizeof(uint64_t)); 
        cnt++;
        

    }
    
    

    

    zmq_close (subscriber);
    zmq_ctx_destroy (context);
    return 0;
}