TODO FOR ANDREW:
Edit file src/parcsr_mv/new_commpkg.c
Replace hypre_DataExchangeList with MPI_Alltoallv_crs (start with alltoallv_crs_personalized).
Change this method first, we may or may not want to change other call to this method https://github.com/bienz2/neighbor_test/blob/c110b4436f59bd4db87e63b947df45920cb2207f/src/parcsr_mv/new_commpkg.c#L368
Available Alltoallv methods are listed here : https://github.com/mpi-advance/locality_aware/blob/sparse_alltoall/src/neighborhood/sparse_coll.h

Here is the method that currently implements the alltoallv_crs type algorithm in hypre https://github.com/bienz2/neighbor_test/blob/c16444ce3d88759864b84b5eb5bc921b44117a4d/src/utilities/exchange_data.c#L93

Mapping of variables to MPI_Alltoallv_crs:
- send_nnz = num_contacts
- dest = contact_proc_list
- sendvals = contact_send_buf
- sdispls = contact_send_buf_starts
- sendtype is the type of sendvals that hypre is passing.  Available HYPRE MPI variables are here : https://github.com/hypre-space/hypre/blob/8d0953e780663c7ab4a55adf5c154b37364a9346/src/utilities/HYPRE_utilities.h#L45 (if hypre is passing HYPRE_Big_Int, you want to pass HYPRE_MPI_BIG_INT as the sendtype)
- send_size = contact_send_buf_starts[num_contacts]
- You will need to create sendcounts (int*, size of array = send_nnz integers).  To fill in the array, sendcounts[i] = sdispls[i+1] - sdispls[i]

For recv variables : 


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

