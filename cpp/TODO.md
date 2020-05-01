# TODO

The list of everything C++ related that needs to get done.

## Long Term
- Serialize all files (wait for better data)
    - ~~Automate with build_data~~
        - *do a better job of this with a serialize.sh file and structure with*
- Fully completed canvas class
    - ~~Add overlapping fill with individual canvases for each shape~~
- Unit tests for geometry, graphs
- Documentation

## Serialization
- ~~Make `Graph` by finding vertices and edges~~
    - ~~Serialize graph data~~
- ~~Add graph sorting functions~~ no longer necessary, removed

- *Finish parsing files*:
    - Look into third party parsing functionality
    - Look into voter registration format, add parsing function
    - Finish `build_data.list` for automation, finish `build_data.sh`

### Graphics
- *Refactor for speed with background setting on a different call*
- Fun with antialiasing
- Speed up fill algorithm and find one that doesn't require a center point
- ~~Add graph visualization~~
- Export as `png`, `svg`, `mp4`
    - ~~To BMP with SDL screenshots~~
    - To ppm, use `system(convert)`

### Scripts
- `draw_communities` with arguments for animation, random fill color, saving output

### Algorithm
- #### Communities

    - Add community class
    - Better structure for execution
    - ~~New initial config implementation with backtracing~~
        - ~~Implement Karger-Stein~~
    - Update refinement processes

- #### Quantification
    - Update to use population weighted by area
    - Partisanship factor needs some help...