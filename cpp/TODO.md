# TODO
For the list of everything C++ related I need to do right now.

## Serializing
- Don't fix district inheritance issue until after serializing (make district a `Shape`, not `Precinct_Group`).
- Use better structure with `Multi_Polygon` class
- Finish serializing files:
    - [ ] Look into voter registration format, add parsing function

## Communities Algorithm

- Add `save_output` function for each step for previews and visualization

### Geometric Todo:
- Fix outer shell algorithm
- Base `boundary_precincts` off of outer shell algorithm

### Random Generation
- Add body for `get_addable_precinct` - implementation of step 1 precinct iteration
