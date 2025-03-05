# Wx1 - CMake #

Created a copy of Pauls test repo, looking to get a Cake working for each project to see if I can create something that will translate between what's been done to date and Pauls planned content for the module. 

## Known Issues ##

* OpenGL issue may relate to graphics drivers, or perhaps some recent changes to EGL. 
  * Notes:
    * Doesn't work with 
      * ```GDK_BACKEND=x11 ./wx1-opengl``` - x11 backend
      * ```GDK_BACKEND=wayland ./wx1-opengl``` - wayland backend
* RC files on Linux hasn't been explored.
* pyramid-test:
  * Only works with the wayland backend. 

### Projects ###

| Name | Description | Linux | Windows |
| wx1 | WxWidgets only example | **OK** | **OK** |
| wx2-opengl | WxWidgets & opengl | **BROKEN** | **OK** |
| wx2-opengl-scribble2 | WxWidgets & opengl | **BROKEN** | **OK** |
| w3-lists | Lists | **OK** | **OK** |
| w3-lists-colsort | Lists with column sorting. | **OK** | **OK** |
| w3-virtual-lists | Lists with RC files? | **BROKEN** | **OK** |
| pyramid-test | wxWidgets glcanvas example | **BROKEN** | **OK** |