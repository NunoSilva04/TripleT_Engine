# TripleT_UI
UI library from scratch done in C


might need basic UI libraries for C like libstd and whatnot from package manager and also only needs the vulkan packages such as vulkan-headers for arch and libvulkan-dev for ubuntu and more but need to figure this out (it's easier for users to simply use the package manager to download the vulkan development tools themselves. Most of them probably already come pre installed anyway but it's probably easier).
Vulkan 1.3 support (but for users in linux it's best if they simply go with the package manager instead of downloading the files themselves through lunarG. Almost every single linux distribution comes with vulkan headers)
If debug is not starting it's probably a layer issue "sudo pacman -S vulkan-devel vulkan-validation-layers" -> use this to have layers
bear -- make x11 to resolve path errors

Two things the user must be aware of are the GLSLC when building the library and the VULKAN_LIBRARY_PATH when linking
