# Combining distance fields with path finding

![sdf-sample-05](https://github.com/zet23t/examples-c/assets/1007258/7e71e376-e6a8-4ee1-ae19-767d138a1b8a)

This example demonstrates how to use signed distance fields (SDF) for improving
pathfinding.

* Unit size: A path may have a requirement for a minimum width to allow passage.
  By using the SDF values, we can block paths that are too narrow for the unit.
* Path preferences: A unit may prefer to stay close to walls or avoid them. The example
  demonstrates how to influence the pathfinding by using the SDF values.
* Varying step distances: Using SDF values to adjust step distances during path finding,
  resulting in curved paths.

The example is not optimized for performance, but rather to demonstrate the concept.
