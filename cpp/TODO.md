# TODO

The list of everything C++ related that needs to get done.

## Long Term
- Serialize all files
- Real time visualization
- Fully completed canvas class
- Unit tests for geometry
- Documentation

## Serialization
- Make `Graph` by finding vertices and edges
- `Community` should be a child of `Graph` where nodes have id's and pop/voter info
- Finish parsing files:
    - Look into voter registration format, add parsing function
    - Finish `build_data.list` for automation, finish `build_data.sh`

### Graphics
- Speed up fill algorithm
- Add graph visualization
- Export as `png`, `svg`, `mp4`

### Scripts
- [ ] Add `draw_communities` with arguments for animation, random fill color


### Algorithm
- #### Communities
    - Better structure for execution
    - New initial config implementation with backtracing
    - Update refinement processes

- #### Quantification
    - Update to use population weighted by area
    - Something else