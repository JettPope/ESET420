# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.25

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/BJAP/Desktop/ESET420/BaseUnit

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/BJAP/Desktop/ESET420/BaseUnit/build

# Include any dependencies generated for this target.
include CMakeFiles/BaseUnit.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/BaseUnit.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/BaseUnit.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/BaseUnit.dir/flags.make

CMakeFiles/BaseUnit.dir/BaseUnit.c.o: CMakeFiles/BaseUnit.dir/flags.make
CMakeFiles/BaseUnit.dir/BaseUnit.c.o: /home/BJAP/Desktop/ESET420/BaseUnit/BaseUnit.c
CMakeFiles/BaseUnit.dir/BaseUnit.c.o: CMakeFiles/BaseUnit.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/BJAP/Desktop/ESET420/BaseUnit/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/BaseUnit.dir/BaseUnit.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/BaseUnit.dir/BaseUnit.c.o -MF CMakeFiles/BaseUnit.dir/BaseUnit.c.o.d -o CMakeFiles/BaseUnit.dir/BaseUnit.c.o -c /home/BJAP/Desktop/ESET420/BaseUnit/BaseUnit.c

CMakeFiles/BaseUnit.dir/BaseUnit.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/BaseUnit.dir/BaseUnit.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/BJAP/Desktop/ESET420/BaseUnit/BaseUnit.c > CMakeFiles/BaseUnit.dir/BaseUnit.c.i

CMakeFiles/BaseUnit.dir/BaseUnit.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/BaseUnit.dir/BaseUnit.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/BJAP/Desktop/ESET420/BaseUnit/BaseUnit.c -o CMakeFiles/BaseUnit.dir/BaseUnit.c.s

# Object files for target BaseUnit
BaseUnit_OBJECTS = \
"CMakeFiles/BaseUnit.dir/BaseUnit.c.o"

# External object files for target BaseUnit
BaseUnit_EXTERNAL_OBJECTS =

BaseUnit: CMakeFiles/BaseUnit.dir/BaseUnit.c.o
BaseUnit: CMakeFiles/BaseUnit.dir/build.make
BaseUnit: /usr/lib/aarch64-linux-gnu/libSDL2.so
BaseUnit: CMakeFiles/BaseUnit.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/BJAP/Desktop/ESET420/BaseUnit/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable BaseUnit"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/BaseUnit.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/BaseUnit.dir/build: BaseUnit
.PHONY : CMakeFiles/BaseUnit.dir/build

CMakeFiles/BaseUnit.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/BaseUnit.dir/cmake_clean.cmake
.PHONY : CMakeFiles/BaseUnit.dir/clean

CMakeFiles/BaseUnit.dir/depend:
	cd /home/BJAP/Desktop/ESET420/BaseUnit/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/BJAP/Desktop/ESET420/BaseUnit /home/BJAP/Desktop/ESET420/BaseUnit /home/BJAP/Desktop/ESET420/BaseUnit/build /home/BJAP/Desktop/ESET420/BaseUnit/build /home/BJAP/Desktop/ESET420/BaseUnit/build/CMakeFiles/BaseUnit.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/BaseUnit.dir/depend

