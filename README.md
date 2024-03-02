TODO FOR ANDREW:
Updated todo : 
Edit file `src/parcsr_ls/par_stats.c`
Time the following (in this order) : 
1. hypre's current method (`hypre_DataExchangeList`)
2. `MPIX_Dist_graph_create_adjacent`
3. `MPIX_Comm_topo_init` 
4. Each of the 4 alltoallv crs methods in MPI Advance
5. `MPIX_Neighbor_alltoallv_init`
6. `MPIX_Start + MPIX_Wait`

You will need to create an `MPIX_Info` object as well but this just creates a local struct, no need to time it:
```
MPIX_Info* xinfo;
MPIX_Info_init(&xinfo);
...
MPIX_Info_free(&xinfo);
```

You will also want to free the `MPIX_Comm` created with `MPIX_Dist_graph_create_adjacent` as well as the `MPIX_Request` created with `MPIX_Neighbor_alltoallv_init`.  These can be freed with `MPIX_Comm_free(&xcomm)` and `MPIX_Request_free(&xrequest)`, respectively.

Let me know if you have any questions come up!





Ignore below, but keeping it in case it is useful for mapping variables and such
Edit file `src/parcsr_mv/new_commpkg.c`
Replace `hypre_DataExchangeList` with `MPI_Alltoallv_crs` (start with `alltoallv_crs_personalized`).
Change this method first, we may or may not want to change other call to this method `https://github.com/bienz2/neighbor_test/blob/c110b4436f59bd4db87e63b947df45920cb2207f/src/parcsr_mv/new_commpkg.c#L368`
Available Alltoallv methods are listed here : `https://github.com/mpi-advance/locality_aware/blob/sparse_alltoall/src/neighborhood/sparse_coll.h`

Here is the method that currently implements the alltoallv\_crs type algorithm in hypre `https://github.com/bienz2/neighbor_test/blob/c16444ce3d88759864b84b5eb5bc921b44117a4d/src/utilities/exchange_data.c#L93`

Mapping of variables to `MPI_Alltoallv_crs`:
- `send_nnz = num_contacts`
- `dest = contact_proc_list`
- `sendvals = contact_send_buf`
- `sdispls = contact_send_buf_starts`
- `sendtype` is the type of `sendvals` that hypre is passing.  Available HYPRE MPI variables are here :` https://github.com/hypre-space/hypre/blob/8d0953e780663c7ab4a55adf5c154b37364a9346/src/utilities/HYPRE_utilities.h#L45` (if hypre is passing `HYPRE_Big_Int`, you want to pass `HYPRE_MPI_BIG_INT` as the sendtype)
- `send_size = contact_send_buf_starts[num_contacts]`
- You will need to create `sendcounts` (int*, size of array = `send_nnz` integers).  To fill in the array,` sendcounts[i] = sdispls[i+1] - sdispls[i]`

For recv variables : 
You will need to allocate variables, so first you need to find `recv_nnz` and `recv_size`.  To do this, create an `int* recv_info = new int[2*num_procs]` allocated to all 0's.  Add to the arrays with something like this:
```
for i = 0 to send_nnz:
    proc = dest[i]
    sendcounts[i] = sdispls[i+1] - sdispls[i]
    // First half of recv_info gives num_recvs info
    recv_info[proc] = 1
    // Second half of recv_info gives recv_size info
    recv_info[proc+num_procs] = sendcounts[i]
MPI_Allreduce(MPI_IN_PLACE, send_info, 2*num_procs, MPI_INT, MPI_SUM, comm)
recv_nnz = send_info[rank]
recv_size = send_info[rank + num_procs]
```
Then, allocate the other recv variables to pass to the method:
```
src = new int[recv_nnz]
rdispls = new int[recv_nnz+1]
recvcounts = new int[recv_nnz]
recvvals = new T[recv_size]
```
`T` is the same type as sendvals (hypre's big int?)
`recvtype` is the same as sendtype

I can help you map these to the existing variables when we meet friday, but I believe that
`hypre's num_sends = recv_nnz`
`orig_send_map_starts = rdispls`
`orig_send_elements = recvvals`
you can throw away `recvcounts` after the mpi advance method, and hypre usually uses `rdispls[recv_nnz]` instead of `recv_size`, so that is likely also thrown away.

Then you can let hypre continue their reordering methods as they currently do them.

<!--
Copyright (c) 1998 Lawrence Livermore National Security, LLC and other
HYPRE Project Developers. See the top-level COPYRIGHT file for details.

SPDX-License-Identifier: (Apache-2.0 OR MIT)
-->

![](src/docs/hypre-wwords.png)


[HYPRE](http://www.llnl.gov/casc/hypre/) is a library of high performance
preconditioners and solvers featuring multigrid methods for the solution of
large, sparse linear systems of equations on massively parallel computers.

For documentation, see our [readthedocs page](https://hypre.readthedocs.io/en/latest/).

To install HYPRE, please see either the documentation or the file [INSTALL.md](./INSTALL.md).

An overview of the HYPRE release history can be found in the file [CHANGELOG](./CHANGELOG).

We appreciate feedback from users.  Please submit comments, suggestions, and
report issues on our [issues page](https://github.com/hypre-space/hypre/issues).
See also [SUPPORT.md](./SUPPORT.md).


License
----------------

HYPRE is distributed under the terms of both the MIT license and the Apache
License (Version 2.0). Users may choose either license, at their option.

All new contributions must be made under both the MIT and Apache-2.0 licenses.

See [LICENSE-MIT](./LICENSE-MIT), [LICENSE-APACHE](./LICENSE-APACHE),
[COPYRIGHT](./COPYRIGHT), and [NOTICE](./NOTICE) for details.

SPDX-License-Identifier: (Apache-2.0 OR MIT)

LLNL-CODE-778117

