
# Breadth-First Search (BFS)

## Description

This code is based on the BFS FPGA benchmark from the OpenDwarfs project (https://github.com/vtsynergy/OpenDwarfs), and their code is originally based on the BFS benchmark from the Rodinia Benchmark Suite (https://github.com/kkushagra/rodinia).

It is an iterative algorithm that simply traverses a graph starting at a specified node by performing a breadth-first traversal, and then returns a depth value for each node.

## Usage

In the `src` directory, you can find the standalone source code to run one specific design.

To generate, compile and run multiple designs, refer to the instructions in the `scripts` directory.

## Data structure

### Graph file format
```
number_of_nodes
node0_edge_id node0_num_edges
node1_edge_id node1_num_edges
node2_edge_id node2_num_edges
...
source_node_id
total_num_edges
edge0_node_id edge0_weight
edge1_node_id edge1_weight
edge2_node_id edge2_weight
...
```

`nodeX_edge_id` is the ID of the first edge starting from this node in the list of edges.
`edgeX_node_id` is the ID of the end node of this edge in the list of nodes.
Note that the edge weights are ignored in this application, but could easily be added.


### Graph structure

In the code, the graph is represented by one vector of nodes (including edge ID and number of edges) and one vector of edges (containing only node ID).
Additionally, there are 3 structures that hold data necessary for the algorithm:

- Graph mask: A binary mask for the next nodes to visit
- Updating graph mask: A binary mask to update the previous mask synchronously
- Graph visited: A binary mask denoting the nodes already visited

Finally, a last vector holds the result of the algorithm (depth for each node)


## Algorithm
The algorithm starts at a designated starting node, then iterates until all the reachable nodes have been visited.
Each iteration has two steps.

### Step 1
```
for each node marked in the graph mask
	for each edge of this node
		if the end node has not been visited
			set the end node depth as current depth + 1
			mark the updating graph mask for the end node
```

### Step 2
```
for each node marked in the updating graph mask
	mark the graph mask for this node
	mark the visited graph mask for this node
	unmark the updating graph mask for this node
	set a global flag to continue the iterations

reset global flag
```

The algorithm ends when step 2 does not enter any node.


## OpenCL kernels
Each step is a separate kernel. Each kernel launches work-items for each node in the graph, so that the outer for loop of each step is represented by work-items ID.
The need for the two separate steps and two different graph masks is explained by the fact that work-items can potentially run in parallel.

### Knobs
There are 6 knobs with varying values in these kernels.

- `KNOB_UNROLL_FACTOR` in step 1: This is the `UNROLL_FACTOR` defined in the OpenDwarfs benchmark. It unrolls a for loop inside the inner loop that allows the kernel to process multiple edges simultaneously. It requires an additional piece of code to account for the case where there are less edges remaining than the unroll factor. In our version, this piece of code is enabled only if the unroll factor is greater than 1.
- `KNOB_COMPUTE_UNITS_1` in step 1: Number of duplicates of the first kernel that can be run simultaneously.
- `KNOB_COMPUTE_UNITS_2` in step 2: Number of duplicates of the second kernel that can be run simultaneously.
- `KNOB_SIMD_2` in step 2: Number of work-items that can be processed in a SIMD way. Note that there are no such knob for step 1 because of branching.
- `KNOB_BRANCH` in step 1: This knobs describes how to check for a visited node and update the updating graph mask value. The first case enables the code from OpenDwarfs that uses bitwise operators to avoid branching, and the second case enables the original code from Rodinia with regular _if_ statement.
- `KNOB_MASK_TYPE`: This describes the number of bits used to encode the values of the masks.

We found that the number of work-items for both kernels did not have a significant impact on the runtime and area, and therefore was fixed to 256.


## License information

The OpenCL code is a modified version of the code from the OpenDwarfs benchmark suite, release under the LGPL v2.1 license. As such, the OpenCL code is also released under the LGPL v2.1.

The original software that uses the OpenCL code is released under the BSD 3 license.



