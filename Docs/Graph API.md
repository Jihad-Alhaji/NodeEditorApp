# Graph API: first execution flow contract

This document defines the contract for the first executable Blueprint workflow. The C++ graph model identifies flow pins with `EPinRole::Flow`, and `BlueprintGraph::Execute` validates and runs the synchronous flow.

## Scope

The first sample is `Start → Print Message("Hello from my workflow") → End`. Links in this slice carry control flow only: they select which node executes next and carry no values. `Print Message` owns its configured string. General typed data pins, conversions, branches, loops, async execution, and cancellation are out of scope.

## Flow link

A flow link connects an output pin on one node to an input pin on another. A link is valid only when both endpoint pins and their owning nodes exist in the same graph, the source pin is an output, and the destination pin is an input. Direction is source-to-destination; reverse-oriented and self-node links are invalid. Pin role must be explicit in node/pin metadata (or an equivalent flow marker); pin direction alone does not identify a flow pin. All links in the first executable graph are flow links. Do not infer data types from pin names or add a general typed-pin framework for this slice.

Flow-link cardinality for this first slice is one-to-one in the execution direction: each node has at most one outgoing flow link. Incoming flow links may be many-to-one, subject to reachability and the Start/End rules below. This flow-only rule does not set cardinality for data pins. Data-pin cardinality is a separate policy to define by data type and use case; for example, an image output may fan out to multiple compatible input pins. The first execution demo has no data links. Flow links are traversed deterministically by following the unique outgoing link, so map iteration order is irrelevant.

## Entry and termination

A runnable graph contains exactly one Start node. Start has no incoming links and exactly one outgoing flow link. Execution begins at Start and advances only through flow links. End has no outgoing flow link and completes successfully when reached. The sample therefore has two links: Start to Print Message, then Print Message to End. Reaching a node already visited or exceeding the finite execution step limit is an execution error; looping semantics are not defined.

## Validation and errors

Validate the graph before invoking node behavior. Reject execution on zero or multiple Start nodes; missing Start successor; a Start with incoming links; any node with multiple outgoing flow links; a link with an absent/expired endpoint, endpoint outside the graph, wrong direction/role, self-link, or missing destination; and a reachable path that terminates without End. Report the offending node/link when available. Do not choose an arbitrary candidate for ambiguous links, skip broken links, or treat a malformed graph as successful completion. A valid graph that reaches End returns success; node behavior failure, a broken link, a cycle, or a step-limit breach returns failure and stops traversal.

Unreachable non-Start nodes need not execute. The validator may report them as warnings, but they do not alter the first slice's execution result. Whether unreachable nodes should become hard validation errors is deferred until authoring requirements demand it.

## Implementation note

`GraphPin` stores input/output direction, an explicit `EPinRole` (`Data` or `Flow`), and connections by generated ID. Blueprint flow pins use the dedicated `BlueprintFlowPin` class. Start and End use dedicated node classes; `BlueprintNode` exposes virtual start/end identity, flow-input/output policy, and execution behavior for specialization. The executor uses those virtual contracts instead of node titles. `BlueprintGraph::Execute` validates the graph connection table against endpoint ownership and pin references, then follows the unique flow edge from Start to End. The configurable finite step limit defaults to 1024 executed nodes, including Start and End. A node behavior failure stops execution with the node name and ID in the diagnostic. The Start context-menu shortcut is retained as a convenience and delegates to this graph executor.

The canvas still accepts links based on opposite direction and different owner nodes without checking role, and `GraphView::AddConnection` replaces a map entry on ID collision. These remain authoring/storage concerns; the executor rejects inconsistent endpoint references before executing behavior. Do not apply the flow-link one-outgoing rule to data pins: data-pin fan-out/cardinality remains open and may vary by data type and use case. This task does not define a data-pin typing/value-transfer API.
