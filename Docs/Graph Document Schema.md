# Graph document schema (v1)

This document defines the minimal, versioned JSON format for saving one graph. NE-M1-02 implements the in-memory JSON save/load foundation through `GraphView::SaveDocument` and `GraphView::LoadDocument`; file dialogs and file I/O are not included.

## Example

```json
{
  "schema_version": 1,
  "graph_type": "Blueprint",
  "next_node_id": "4",
  "nodes": [
    {
      "id": "1",
      "type": "Start",
      "position": { "x": 80.0, "y": 120.0 },
      "properties": {}
    },
    {
      "id": "2",
      "type": "Print Message",
      "position": { "x": 320.0, "y": 120.0 },
      "properties": { "message": "Hello from my workflow" }
    },
    {
      "id": "3",
      "type": "End",
      "position": { "x": 640.0, "y": 120.0 },
      "properties": {}
    }
  ],
  "connections": [
    {
      "from": { "node_id": "1", "pin_key": "exec.out", "direction": "output" },
      "to":   { "node_id": "2", "pin_key": "exec.in", "direction": "input" }
    },
    {
      "from": { "node_id": "2", "pin_key": "exec.out", "direction": "output" },
      "to":   { "node_id": "3", "pin_key": "exec.in", "direction": "input" }
    }
  ]
}
```

The document uses ARC's `ResourceManagement::Serialization::ISerializationInterface::Serialzer_t` JSON archive. `GraphView` implements that interface, enabling ARC's `SaveToFile` and `LoadFromFile` helpers. Loading validates and stages nodes and links before replacing graph contents. Schema v1 is the only supported version; no migration from an earlier format is needed.

## Fields

- `schema_version`: required integer. Version 1 is the first supported format. Reject unsupported versions with a clear diagnostic; add migrations only when a later version actually changes the schema.
- `graph_type`: required, case-sensitive graph registry key. Version 1 stores exactly one graph per document.
- `next_node_id`: required positive decimal string. It is the next graph-scoped monotonic node instance ID to assign. Keep it greater than every node `id` in the document so deleted IDs are not reused after reload.
- `nodes`: array of node placements. Each requires a unique positive decimal-string `id`, a case-sensitive `type` key registered within `graph_type`, a finite graph-space `position`, and a `properties` object for user-authored per-instance state.
- `connections`: array of links. Each endpoint identifies a node by persistent `node_id`, a stable definition pin key, and direction (`input` or `output`). Direction is part of the endpoint so a node can have an input and output with the same pin key.

For the first Blueprint node, the `Print Message` factory supplies the default string. `properties.message` stores the configured per-instance string. Empty/default `properties` are valid for Start and End.

## Identity and derived state

Graph/node `type` values are registry keys, not display titles. Treat them as case-sensitive stable identifiers. A node title is recreated by its registered factory.

Node `id` values are graph-document-scoped monotonic IDs serialized as decimal strings. Assign a fresh ID on node duplication. The current generated runtime `GraphNode::NodeId` remains separate and is never serialized.

Each pin definition has an explicit, stable `pin_key` separate from its user-facing `label`. Keys are immutable identifiers; labels may change or be localized without changing saved links. Keys are unique across all pins on one node definition, including inputs and outputs. Example keys are `exec.in`, `exec.out`, `condition.true`, and `condition.false`; labels can remain `Execute`, `True`, or `False`. `GraphPin::Name` is presentation text and `GraphPin::PinId` is a layout index; neither is a persistent key. Recreate pins, keys, labels, roles, cardinality, and compatibility from registered factories.

Connection IDs are regenerated when links are loaded. Do not serialize runtime node IDs, pin indices, connection hashes, pointers, widget state, titles, callbacks, or derived defaults. Do not serialize viewport zoom/scroll in v1. Arbitrary node-specific user configuration belongs in `properties`; data-value transfer and generic pin values are out of scope for v1.

## Validation and loading

Validate the document before replacing the active graph, so a failed load does not leave a partially loaded graph visible.

- Require an object root, all required fields with the specified types, `schema_version == 1`, and a registered graph type.
- Require canonical positive decimal strings for node IDs, unique IDs, and a valid `next_node_id` greater than every node ID. Reject overflow rather than wrapping.
- Require every node type key to be registered for the graph type. Construct through its factory, then apply `properties`; unknown node types or invalid properties produce actionable diagnostics.
- Require finite numeric positions and unique stable pin keys across each node definition.
- For each link, require existing source/destination node IDs and pins, output-to-input direction, distinct owning nodes, and compatibility accepted by both pins' `AllowConnectionWith` policies. Enforce each endpoint's `AllowMultipleConnections` policy using the reconstructed pins. Reject duplicate links and broken references.
- Report unsupported schema versions and malformed fields with a path to the offending field or connection where possible.
- Save structurally valid but not-yet-runnable graphs. Do not require a Start node, complete flow paths, or successful execution to save/load; `BlueprintGraph::Execute` applies execution validation separately.

## Deterministic output

For readable diffs, serialize nodes in ascending numeric instance-ID order and connections in stable endpoint order (source node ID/pin key, then destination node ID/pin key). JSON object key order is not semantically meaningful.

## Branching compatibility

The schema can represent a node with several flow output pins because every output has its own stable key and each saved connection names the exact output key. Each individual flow output pin allows at most one outgoing link; a node may expose several distinct output pins. A branch node can define `condition.true` and `condition.false` outputs and the executor can later follow the key selected by that node's execution result. The v1 linear executor does not yet support selecting among multiple flow outputs; branching execution is a separate runtime task.

## Out of scope

Multiple graphs per file, graph-instance IDs, generic graph migrations, Lua/plugin schema negotiation, pin value serialization, runtime packaging, viewport/editor session state, and unknown-node placeholders are deferred.
