# Graph API: first execution flow contract

This document defines the current contract for Blueprint graph connections and synchronous flow execution. The C++ graph model identifies flow pins with `EPinRole::Flow`, and `BlueprintGraph::Execute` validates and runs flow paths.

## Scope

The first sample is `Start → Print Message("Hello from my workflow") → End`. Links in this slice carry control flow only: they select which node executes next and carry no values. `Print Message` owns its configured string. General typed data pins, conversions, branches, loops, async execution, and cancellation are out of scope.

## Flow link

A flow link connects an output pin on one node to an input pin on another. A link is valid only when both endpoint pins and their owning nodes exist in the same graph, the source pin is an output, and the destination pin is an input. Direction is source-to-destination; reverse-oriented and self-node links are invalid. Pin role must be explicit in node/pin metadata (or an equivalent flow marker); pin direction alone does not identify a flow pin. A graph may also contain data links; they are not traversed as control flow. Do not infer data types from pin names or add a general typed-pin framework for this slice.

Connection compatibility and cardinality are pin policies. `GraphPin::AllowConnectionWith(other)` decides whether a pin accepts a connection to the other pin, and `GraphPin::AllowMultipleConnections()` decides whether it may participate in multiple links. `GraphView::AddConnection` checks both endpoint policies and rejects a proposed connection that either endpoint disallows. The default policies permit multiple connections and require matching roles; flow pins override cardinality so outputs allow one connection while inputs allow many. Data pins keep their own compatibility and cardinality policy. Do not apply flow cardinality rules to data pins: for example, a data output may fan out to multiple compatible consumers.

Control-flow execution follows flow pins only. Each flow output has at most one successor, while a flow input may receive multiple links, allowing paths from different Start nodes to converge. Execution selects the unique outgoing link from the current node's flow output; it never chooses among competing outgoing links.

## Entry and termination

A runnable graph contains one or more Start nodes. Each Start is an independent entry point and its reachable flow path is executed separately, synchronously. A Start accepts no incoming flow links and must have exactly one outgoing flow link. End has no outgoing flow links and completes that path successfully when reached. The sample remains Start → Print Message → End. Reaching a node already visited within a path or exceeding the finite execution step limit is an execution error; looping semantics are not defined.

`BlueprintGraph::Execute` orders Starts by ascending `NodeId`. It validates all Start paths, graph links, and endpoint pin references before invoking any node behavior, so a malformed later path cannot be discovered after an earlier path has already produced side effects. Once validation succeeds, paths execute sequentially and fail fast on the first node failure. Each path gets its own visited set and step limit (1024 nodes by default); `StepsExecuted` is aggregate, and each `StartResult` reports its Start ID, whether it executed, its success, path step count, and diagnostic. Starts after a runtime failure remain present with `Executed == false`. A graph with no Starts fails validation. If paths converge, the shared downstream nodes execute once for each Start path that reaches them.

## Validation and errors

Validate each Start path before invoking node behavior. Reject a Start without a successor or with incoming flow; any node with multiple outgoing flow links; a flow link with an absent/expired endpoint, endpoint outside the graph, wrong direction/role, self-link, or missing destination; and a reachable path that terminates without End. Report the offending node/link when available. Do not choose an arbitrary successor, skip broken links, or treat a malformed path as successful completion. A valid path that reaches End returns success; node behavior failure, a broken link, a cycle, or a step-limit breach returns failure for that path. Data links do not determine control-flow traversal.

Unreachable non-Start nodes need not execute. The validator may report them as warnings, but they do not alter the first slice's execution result. Whether unreachable nodes should become hard validation errors is deferred until authoring requirements demand it.

## Implementation note

`GraphPin` stores input/output direction, an explicit `EPinRole` (`Data` or `Flow`), and connections by generated ID. Its virtual `AllowMultipleConnections` and `AllowConnectionWith` methods define connection policy. `GraphView::AddConnection` consults the rules of both pins before inserting a connection. Blueprint flow pins use the dedicated `BlueprintFlowPin` class; Start and End use dedicated node classes, and `BlueprintNode` exposes virtual identity, flow input/output policy, and execution behavior. `BlueprintGraph::Execute` validates connection endpoints and pin references, then traverses the unique outgoing flow link for each Start path. The configurable finite step limit defaults to 1024 executed nodes, including Start and End. A node behavior failure returns the node name and ID in the diagnostic. The Start context-menu shortcut delegates to this graph executor.

This API remains control-flow only; data pins have independent policies and this task does not define data value transfer.
