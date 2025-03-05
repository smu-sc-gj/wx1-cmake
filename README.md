# Wx1 - CMake #

Created a copy of Pauls test repo, looking to get a Cake working for each project to see if I can create something that will translate between what's been done to date and Pauls planned content for the module. 

## Known Issues ##

* Linux version is sensitive to backend:
  * Works with the wayland backend:
    *  ```GDK_BACKEND=wayland ./wx1-opengl``` 
    *  Default on most modern linux systems?
  * Doesn't work with 
      * ```GDK_BACKEND=x11 ./wx1-opengl``` - x11 backend
     
* RC files on Linux hasn't been explored.
* pyramid-test:
  * Added to confirm shaders would work and issues were setup related. 

### Projects ###

| Name | Description | Linux | Windows |
| wx1 | WxWidgets only example | **OK** | **OK** |
| wx2-opengl | WxWidgets & opengl | **OK** | **OK** |
| wx2-opengl-scribble2 | WxWidgets & opengl | **BROKEN** | **OK** |
| w3-lists | Lists | **OK** | **OK** |
| w3-lists-colsort | Lists with column sorting. | **OK** | **OK** |
| w3-virtual-lists | Lists with RC files? | **BROKEN** | **OK** |
| pyramid-test | wxWidgets glcanvas example | **OK** | **OK** |