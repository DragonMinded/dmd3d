A WiringX-based GPIO driver for a 128x64 Pinball DMD and a bunch of utilities that allow you to convert static images for display as well as a bunch of code and utilities that make up a simple 3D engine. The engine is tailor-made to the display so it is missing a lot of things that a 3D engine might have such as lighting because it simply is not needed. Requires a modern version of Python3 to run some of the 3D engine as well as all of the static image converter scripts.

Note that this should also work with any 128x32 display as well, since that's simply outputting half as many lines. You will need to adjust the scripts which scale images as well as the projection matrix for the 3D engine in order to get good results.

Requires a build of wiringX, currently based off of https://github.com/wiringX/wiringX.git revision 56ff8e87f737872b8d5672a842c86d6d754c5fb7.
