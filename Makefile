CC=gcc
CFLAGS= -Wall -I/usr/local/include -I./include -I/home/javier/Documents/k_v_store_benchmark `pkg-config --cflags --libs glib-2.0`
ODIR=obj
AR=ar
ARFLGAS=rcs
LDFLAGS= -L/usr/local/lib -lkvbenchmark -lzmq



static: obj/SHARDS.o obj/murmurhash3.o obj/splay.o
	$(AR) $(ARFLAGS) lib/libSHARDS.a $^ 

shards_test: obj/shards_test.o lib/libSHARDS.a
	$(CC) -g $^ $(CFLAGS) $(LDFLAGS) $(VERSION) -o $@

shards_test_zmq: obj/shards_test_zmq.o obj/splay.o obj/murmurhash3.o lib/libshardsc.a
	$(CC) -g $^ $(CFLAGS) $(LDFLAGS) $(VERSION) -o $@

lib/libSHARDS.a: obj/SHARDS.o obj/murmurhash3.o obj/splay.o
	$(AR) $(ARFLAGS) lib/libSHARDS.a $^ 

obj/shards_test.o: src/shards_test.c 
	$(CC) -g -c  src/shards_test.c $(CFLAGS)  $(VERSION) -o $@

obj/shards_test_zmq.o: src/shards_test_zmq.c include/SHARDS.h
	$(CC) -g -c  src/shards_test_zmq.c $(CFLAGS)  $(VERSION) -o $@

dist: dist.c
	$(CC) -g  dist.c splay.c $(CFLAGS) -o dist

obj/SHARDS.o : src/SHARDS.c 
	$(CC) -g -fPIC -c  $(CFLAGS) $(LFLAGS)  src/SHARDS.c -o $@


obj/splay.o: src/splay.c include/splay.h
	$(CC) -g  -fPIC -c $(CFLAGS) $(LFLAGS)  src/splay.c -o $@

obj/murmurhash3.o: src/murmurhash3.c include/murmurhash3.h
	$(CC) -g -fPIC -c $(CFLAGS) $(LFLAGS)  src/murmurhash3.c -o $@

obj/jenkins_hash.o: src/jenkins_hash.c include/jenkins_hash.h
	$(CC) -g -c $(CFLAGS) $(LFLAGS)  src/jenkins_hash.c -o $@
