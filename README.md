# examples-c
<img align="left" src="https://github.com/raysan5/raylib/raw/master/logo/raylib_logo_animation.gif" width="64">
Examples and samples made for raylib using C

## Building
The examples use premake, assumes a raylib folder is created manualy or using the submodule. Get premake5 for your system and run it for your build target. Batch file for visual studio is included.

# Circle In View 2D
Code showing how to check if a circle is inside a view cone. Shows examples of using dot products to find what side of a vector a point is on, and how to compute the nearest point on a vector to another point.
![circle_in_view](https://user-images.githubusercontent.com/322174/149637442-c701f9cb-883a-4d44-96fe-f91e8a86dea6.gif)

# Texture Repeat
An example of how to do a repeating background inside a camera 2d
![repeat](https://user-images.githubusercontent.com/322174/149637440-b7f7b1d2-1e1d-4ad8-a465-e11ce2388793.gif)

# Unsorted Billboards
An example of using shaders to discard pixels with 0 alpha to allow billboards to be drawn in any order.
![unsorted_billobards](https://user-images.githubusercontent.com/322174/148694937-f7c4b166-b81a-4d48-af8f-eefbc1a3f487.gif)
