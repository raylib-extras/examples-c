# Unsorted Billboards
This example shows how to use a shader to discard pixles with an alpha of 0, so they do not get written to the depth buffer.
This allows billboards to be drawn in any order and still use the depth buffer, removing the need to depth sort the billboards.

![unsorted_billobards](https://user-images.githubusercontent.com/322174/148694937-f7c4b166-b81a-4d48-af8f-eefbc1a3f487.gif)