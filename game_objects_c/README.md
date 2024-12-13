# Game Objects In C
A simple example of one way to do game objects in components in C without needing OPP or C++ features.

## Structure

The concept is built around the GameObject structure. It holds an array of GameObjectComponents. Each component is defened by a enum and a void* pointer to hold its data.
Functions exist to create and destory game objects and add components to them, as well as know if an object has a specific component.
When a component is needed, the array is checked for an entry with the desired enum type.

The main game then runs systems on these objects by finding all the objects with the components the system needs and does processing.

While this may not be as 'clean' as C++ code with templates, interfaces, and object inerheritance, it is very useable.
