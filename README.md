# dt0-sequence
The sequence is designed as a double-ended container that also allows insertion and erasure anywhere within the container itself. The way it is structured in memory is by using so-called "heap modules" which are memory chunks with reserved space for the data and the chunk manager. These heap modules are linked together as a doubly-linked list. Once the chunk's data-reserve capacity is filled a new chunk is allocated and both the old and the new chunk are then linked bidirectionally. It also has its own custom made allocator and iterator which is bidirectional.

It comes from the dt0::cluster_unit see https://github.com/stap0t/dt0-cluster-dt0-cluster_unit for more.
