Collision detection by plane intersections

If two 3D solids intersect, the intersection will be enclosed by a combination of the surfaces (2D shape in 3D space) of the two solids. Unless one solid is completely inside the other solid, the surfaces of the two solids will intersect. The intersection of the surfaces is a curve (1D shape in 3D space).

If two closed polygonal shapes intesect each other the intersection will also be a closed polygonal shape enclosed by a polygonal surface that is a combination of the two polygonal surfaces. Unless one is completely inside the other, the two polygonal surfaces will intersect. The intersection of the surfaces is a series of line segments. 

Triangle-triangle intersection test from moller1997b:

1. Compute plane equation of triangle 2.
2. Reject as trivial if all points of triangle 1 are on same side.
3. Compute plane equation of triangle 1.
4. Reject as trivial if all points of triangle 2 are on same side.
5. Compute intersection line and project onto largest axis.
6. Compute the intervals for each triangle.
7. Intersect the intervals.


Collision detection by TSDF

Suitable for rigid bodies

128*128*128 bytes = 2097152 bytes = 2MB