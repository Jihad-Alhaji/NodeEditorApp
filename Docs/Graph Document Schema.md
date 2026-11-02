# Graph document schema (v1)

This document defines the minimal, versioned JSON format for saving one graph. NE-M1-02 implements the in-memory JSON save/load foundation through `GraphView::SaveDocument` and `GraphView::LoadDocument`; file dialogs and file I/O are not included. The registry assigns each created `GraphView` its graph type key, which save/load use directly for document identity and validation rather than discovering the type from runtime class information.

## NE-M1-03 manual round-trip procedure and status

This acceptance workflow exercises the first useful Blueprint graph with a non-default `Print Message` string. It is a manual application check, not an automated test.

1. Start the app and create or load a Blueprint graph containing `Start → Print Message → End`. Record each node's document ID and graph position.
2. Set the Print Message text to `NE-M1-03 round-trip check` (or another clearly non-default value).
3. Trigger the app's save action. Confirm the resource document contains the same three node IDs and positions, both `exec.out` to `exec.in` connections, and `properties.message` with the authored string on the Print Message node.
4. Close and restart the app (or clear the graph and invoke load). Confirm the node types, IDs, positions, links, and authored message match the saved document.
5. Execute the graph. Confirm the application log emits the authored string and the execution result succeeds.

Status (2026-10-31): manual acceptance passed. After reloading, the app displayed the authored text `this is a non default message`; the persisted `Resources/BlueprintGraph.json` showed node IDs `1`, `2`, and `3`, their graph positions, both expected `exec.out` → `exec.in` links, and `properties.message` on node `2`. The graph UI showed Start → Print Message → End. The reopened graph was executed, and the product owner confirmed that the application log emitted `this is a non default message`. The task is Done. The resource file is user-authored/generated graph data and must not be treated as a code fixture or committed without an explicit decision.

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
      "from": { "node_id": "1", "pin_key": "exec.out" },
      "to":   { "node_id": "2", "pin_key": "exec.in" }
    },
    {
      "from": { "node_id": "2", "pin_key": "exec.out" },
      "to":   { "node_id": "3", "pin_key": "exec.in" }
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
- `connections`: array of links. Each endpoint identifies a node by persistent `node_id` and stable definition `pin_key`. The endpoint slot supplies direction: `from` always resolves to an output pin and `to` to an input pin.

For the first Blueprint node, the `Print Message` factory supplies the default string. `properties.message` stores the configured per-instance string. Empty/default `properties` are valid for Start and End.

## Identity and derived state

Graph/node `type` values are registry keys, not display titles. Treat them as case-sensitive stable identifiers. A node title is recreated by its registered factory.

Node `id` values are graph-document-scoped monotonic IDs serialized as decimal strings. Assign a fresh ID on node duplication. The current generated runtime `GraphNode::NodeId` remains separate and is never serialized.

Each pin definition has an explicit, stable `pin_key` separate from its user-facing `label`. Keys are immutable identifiers; labels may change or be localized without changing saved links. Keys are unique across all pins on one node definition, including inputs and outputs. Example keys are `exec.in`, `exec.out`, `condition.true`, and `condition.false`; labels can remain `Execute`, `True`, or `False`. `GraphPin::Name` is presentation text and `GraphPin::PinId` is a layout index; neither is a persistent key. Recreate pins, keys, labels, roles, cardinality, and compatibility from registered factories.

New saves omit a separate direction field because it is determined by the `from`/`to` endpoint slot and the corresponding node pin collection. For compatibility, the loader accepts older v1 endpoints that include `direction`, and rejects a present value that does not match the endpoint slot. Connection IDs are regenerated when links are loaded. Do not serialize runtime node IDs, pin indices, connection hashes, pointers, widget state, titles, callbacks, or derived defaults. Do not serialize viewport zoom/scroll in v1. Arbitrary node-specific user configuration belongs in `properties`; data-value transfer and generic pin values are out of scope for v1.

## Validation and loading

Validate the document before replacing the active graph, so a failed load does not leave a partially loaded graph visible.

- Require an object root, all required fields with the specified types, `schema_version == 1`, and a registered graph type.
- Require canonical positive decimal strings for node IDs, unique IDs, and a valid `next_node_id` greater than every node ID. Reject overflow rather than wrapping.
- Require every node type key to be registered for the graph type. Construct through its factory, then apply `properties`; unknown node types or invalid properties produce actionable diagnostics.
- Require finite numeric positions and unique stable pin keys across each node definition.
- For each link, resolve `from` only against source-node outputs and `to` only against destination-node inputs; require existing nodes/pins, distinct owning nodes, and compatibility accepted by both pins' `AllowConnectionWith` policies. If a legacy `direction` field is present, require it to match the endpoint slot. Enforce each endpoint's `AllowMultipleConnections` policy using the reconstructed pins. Reject duplicate links and broken references.
- Report unsupported schema versions and malformed fields with a path to the offending field or connection where possible.
- Save structurally valid but not-yet-runnable graphs. Do not require a Start node, complete flow paths, or successful execution to save/load; `BlueprintGraph::Execute` applies execution validation separately.

## NE-M1-04 implementation and verification

`GraphView::SaveDocument` now writes connection endpoints using only `node_id` and `pin_key`. `GraphView::LoadDocument` resolves `from` against outputs and `to` against inputs, and accepts an optional legacy `direction` only when it is a string matching the endpoint slot. These checks occur while staging links, before `ReplaceContents`; existing compatibility, cardinality, uniqueness, and graph identity checks remain in place. The schema remains version 1.

Verification on 2026-11-02: the development MSBuild attempt compiled the changed `Private/GraphView.cpp`, but linking in this agent environment failed with `LNK1101: incorrect MSPDB140.DLL version` (reproduced on retry). The product owner then built the app and manually confirmed save/load worked and the saved JSON omitted endpoint `direction`. Matching legacy direction and malformed/conflicting-value rejection without graph replacement were not separately exercised. The product owner accepted the implementation as complete.

## Deterministic output

For readable diffs, serialize nodes in ascending numeric instance-ID order and connections in stable endpoint order (source node ID/pin key, then destination node ID/pin key). JSON object key order is not semantically meaningful.

## Branching compatibility

The schema can represent a node with several flow output pins because every output has its own stable key and each saved connection names the exact output key. Each individual flow output pin allows at most one outgoing link; a node may expose several distinct output pins. A branch node can define `condition.true` and `condition.false` outputs and the executor can later follow the key selected by that node's execution result. The v1 linear executor does not yet support selecting among multiple flow outputs; branching execution is a separate runtime task.

## Out of scope

Multiple graphs per file, graph-instance IDs, generic graph migrations, Lua/plugin schema negotiation, pin value serialization, runtime packaging, viewport/editor session state, and unknown-node placeholders are deferred.
