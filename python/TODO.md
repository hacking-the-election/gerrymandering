# TODO

## Long Term
- Serialization
- Community algorithm
- Quantification algorithm
- Unit tests
- Documentation (sphinx?)
- Animations (also for development)

## Serialization

- Parse election, geo, and pop data.
- Multipolygon splitting
- Hole removing
- Making a graph:
   - Loop through all the precincts and make them each a node.
   - Loop through all of them again and create edges based on which precincts they're bordering.

## Community Algorithm

- Initial Configuration
   - Implement backtracking algorithm.
- Partisanship Refinement
- Compactness
- Population
- `__main__.py` containing complete algorithm and iterative method implementation

## Graphics

- initial configuration animation: colored precincts
- graph visual: after serialization (Pillow)
   - Node radius corresponding to population
   - Node colors based on partisanship
- community algorithm: save frame after every precinct change
   - Colors based on partisanship
