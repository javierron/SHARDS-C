#include <stdio.h>
#include <pthread.h>
#include <string.h>




void* do_entropy(void* size){
	char command[100] = "./entropy ";
	char num[10];

	int * size_int = (int*) size;
	sprintf(num, "%d", *size_int);

	strcat(command, num);
	printf("Excecuting %s\n", command);
	system(command);

}

void* do_shards(void* size){
	char command[100] = "./shards_test_zmq 16 10 0.1 mrc.dat ";
	char num[10];
	int * size_int = (int*) size;
	sprintf(num, "%d", *size_int);

	strcat(command, num);
	printf("Excecuting %s\n", command);
	system(command);
}


int main(int argc, char** argv){

	//argv[0] dataset size

	pthread_t entropy_thread;
	pthread_t shards_thread;

	pthread_attr_t attr;
	int op;
	int size = strtol(argv[1], NULL, 10);

	printf("%d\n", size);

	pthread_attr_init(&attr);

	//while(1){
		printf("1. SHARDS\n");
		printf("2. Entropía\n");
		printf("3. Ambos\n");

		scanf("%d", &op);

		switch(op){
			case 1:
				pthread_create(&shards_thread, &attr, do_shards, &size);
				pthread_join(shards_thread, NULL);
			break;
			case 2:
				pthread_create(&entropy_thread, &attr, do_entropy, &size);
				pthread_join(entropy_thread, NULL);
			break;
			case 3:
				pthread_create(&shards_thread, &attr, do_shards, &size);
				pthread_create(&entropy_thread, &attr, do_entropy, &size);
				pthread_join(shards_thread, NULL);
				pthread_join(entropy_thread, NULL);
			break;
		}

	//}

	return 0;
}