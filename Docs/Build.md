# Building the Project

This document explains how to build the project for your platform

# Table of Contents
- [Windows](#windows)  
- [Linux](#linux)  
    - [Requirements](#requirements)  
    - [Overview](#overview)
    - [Tutorial - How To Install The Requirements](#tutorial---how-to-install-the-requirements)
    - [Tutorial - How To Build TripleT_Engine](#tutorial---how-to-build-triplet_engine)
    - [Tutorial - How To Use TripleT_Engine For Your Projects](#tutorial---how-to-use-triplet_engine-for-your-projects)
- [MacOS](#macos)  

## Windows 
**Not Yet Implemented**

## Linux
### Requirements 
- [Git](https://git-scm.com/install/)
- C and Make Developer Tools
- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home)
- X11/Wayland System Headers

### Overview

TripleT doesn't use SDL3, GLFW, or any other windowing/input abstraction library. Window creation, event handling, and Vulkan surface setup are implemented directly against the platform's native APIs — this project mimics what those libraries do internally rather than depending on them.
On Linux this is important because at compile time you need to know whether you should build with Wayland or X11.

**Note:** Most Wayland compositors ship a compatibility layer that lets pure-X11 applications run under a Wayland session. So an X11 build will generally still work on a Wayland desktop. But a Wayland build will *not* run on an X11-only session, since there's no equivalent translation the other way. If you're not sure which to build, match it to your actual login session type (e.g. "Plasma (X11)" vs "Plasma (Wayland)") or build with X11.


### Tutorial - How To Install The Requirements
1. **Git**
```bash
   sudo apt install git      # Debian/Ubuntu
   sudo pacman -S git        # Arch
```

2. **C toolchain, Make**
```bash
   sudo apt install build-essential         # Debian/Ubuntu
   sudo pacman -S base-devel                # Arch
```

3. **Vulkan (requires Vulkan 1.3 support)**

  You have two options here — for almost everyone, **Option A is what you want**.

  **Option A — Package manager (recommended)**

Nearly every Linux distribution ships Vulkan headers and loader packages directly, so there's usually no need to download anything from LunarG manually. Just grab the developer packages for your distro:
```bash
    sudo apt install libvulkan-dev vulkan-validationlayers                           # Debian/Ubuntu
    sudo pacman -S vulkan-devel vulkan-validation-layers                             # Arch
```
   Package names vary a bit between distros (`vulkan-headers`, `vulkan-icd-loader`, `libvulkan1`, etc. may be pulled in as dependencies, or you may need to install them separately) — check your distro's package repository if the above doesn't fully cover it. 

   **Note:** The validation layers package is important *IF you build in debug mode*. So if the library fails to compile or crashes immediately it's probably due to the fact that you are missing vulkan validation layers rather than it being an actual bug.
   
   **Option B — Manual install via LunarG SDK**

  If you need the full SDK tooling (e.g. `vkconfig`, shader tools) beyond what your package manager provides, download the SDK manually.
  Navigate to [this link](https://vulkan.lunarg.com/sdk/home) and download `vulkansdk-linux-x86_64-1.x.yy.z.tar.xz`.

  Extract the contents of the folder by running the command: 
  ```bash
  tar xf path/to/vulkansdk-linux-x86_64-1.x.yy.z.tar.xz
  ``` 
  
  Install the runtime dependencies by running the following command:
  ```bash  
  sudo pacman -S libxcb libxinerama xcb-util-cursor                  # Arch
  sudo apt install libxcb-xinput0 libxcb-xinerama0 libxcb-cursor-dev # Debian/Ubuntu
  ```
  And you are set!

  **Verification**

  Navigate to the extracted folder (should have the name `1.x.yy.z` or something similar).
  Run the `setup-env.sh` script by running the following commnad:
  ```bash
  # Bash
  source path/to/vulkan/1.x.yy.z/setup-env.sh
  ```

  ```fish
  # Fish
  fisher install edc/bass
  bass source path/to/vulkan/1.x.yy.z/setup-env.sh
  ```

  Verify that Vulkan has been correctly set up by running one of the following commands: 
  ```cmd
  vulkaninfo
  ```

  Or:
  ```cmd
  vkcube
  ```

**IMPORTANT:** This command only sets up the environment variables for your current shell or terminal session. If you wish to permantly add them to your shell's startup file please consult your shell's documentation. For more information on how to install Vulkan for Linux you can check the [Official Vulkan SDK page](https://vulkan.lunarg.com/doc/sdk/1.4.357.0/linux/getting_started.html).

4. **Windowing system headers**

  Since there's no abstraction library, you need the native developer headers for whichever backend(s) you're building against:

   ```bash
   # X11
   sudo apt install libx11-dev          # Debian/Ubuntu
   sudo pacman -S libx11                # Arch

   # Wayland
   sudo apt install libwayland-dev wayland-protocols   # Debian/Ubuntu
   sudo pacman -S wayland wayland-protocols            # Arch
   ```

### Tutorial - How To Build TripleT_Engine
1. **Clone The Project**

  Use the following command to clone the project
  ```bash
  git clone https://github.com/NunoSilva04/TripleT_Engine.git
  ```

2. **Build For Your Appropriate Window Compositor**

  Build for either Wayland or X11. If you are unsure which one to use, build with X11, since Wayland ships with backwards compatibility, most if not all X11 builds will run on a Wayland session, while the opposite is rarely true.
  Use one of the following commands
  ```bash
  make x11 && sudo make install_x11             # X11
  make wayland && sudo make install_wayland     # Wayland
  ```
  The command `sudo make install_x11/wayland` will copy the library and the headers into `/usr/local/lib` and `/usr/local/include` respectively.

  **IMPORTANT**
  
  If you get an error saying that it cannot find the `GLSLC` executable (this can happen if you went with *Option B* when you were installing Vulkan), you can always specify to the build system where the executable is located by using the following command:

  ```bash
  make x11 GLSLC=path/to/glslc/exec/glslc && sudo make install_x11             # X11
  make wayland GLSLC=path/to/glslc/exec/glslc && sudo make install_wayland     # Wayland
  ```

3. **Clean And Uninstall**
  
  To clean the project and remove it from your system use one of the following commands:

  ```bash
  make clean && sudo make uninstall_x11             # X11 
  make clean && sudo make uninstall_wayland         # Wayland
  ```

### Tutorial - How To Use TripleT_Engine For Your Projects
  **IMPORTANT:** This project for now only supports integration via Makefile

  1. **Include .mk In your Makefile**

  In your Makefile add the following command:

  ```bash
  include TripleT_Engine/TripleT_Engine_X11.mk             # X11
  include TripleT_Engine/TripleT_Engine_Wayland.mk         # Wayland
  ```

  This exposes a `TRIPLET_ENGINE_X11_LIBS` or `TRIPLET_ENGINE_WAYLAND_LIBS` variable containing the linker flags needed to link against the engine — you'll use it in the next step.

  2. **Link The Appropriate Flags**
  
  When compiling your project pass the appropriate variable to your linking command:

  ```bash
  gcc your_objects.o $(TRIPLET_ENGINE_X11_LIBS) -o final_exec                 # X11
  gcc your_objects.o $(TRIPLET_ENGINE_WAYLAND_LIBS) -o final_exec             # Wayland
  ```

  **IMPORTANT**
  If you get an error saying that it cannot find the Vulkan Library (This can happen if you went with *Option B* when installing Vulkan), you can always add additional paths, by running the following command:

  ```bash
  make VULKAN_LIBRARY_PATH=path/to/vulkan/library/libvulkan.so
  ```

  And your set! To use the library simply do the following in your C files:

  ```C
  #include <TripleT_Engine/...>
  ```

  If you had any issues on how to use TripleT_Engine you can always check an example in the `Tests` folder.


## MacOS
**Not Yet Implemented**
