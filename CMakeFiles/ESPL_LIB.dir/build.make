# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.7

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/cmake-3.7.2-Linux-x86_64/bin/cmake

# The command to remove a file.
RM = /opt/cmake-3.7.2-Linux-x86_64/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /DIST/home/lab_espl_stud10/git-tutorial1

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /DIST/home/lab_espl_stud10/git-tutorial1

# Include any dependencies generated for this target.
include CMakeFiles/ESPL_LIB.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/ESPL_LIB.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/ESPL_LIB.dir/flags.make

CMakeFiles/ESPL_LIB.dir/lib/espl_lib.c.o: CMakeFiles/ESPL_LIB.dir/flags.make
CMakeFiles/ESPL_LIB.dir/lib/espl_lib.c.o: lib/espl_lib.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/DIST/home/lab_espl_stud10/git-tutorial1/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/ESPL_LIB.dir/lib/espl_lib.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/ESPL_LIB.dir/lib/espl_lib.c.o   -c /DIST/home/lab_espl_stud10/git-tutorial1/lib/espl_lib.c

CMakeFiles/ESPL_LIB.dir/lib/espl_lib.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/ESPL_LIB.dir/lib/espl_lib.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /DIST/home/lab_espl_stud10/git-tutorial1/lib/espl_lib.c > CMakeFiles/ESPL_LIB.dir/lib/espl_lib.c.i

CMakeFiles/ESPL_LIB.dir/lib/espl_lib.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/ESPL_LIB.dir/lib/espl_lib.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /DIST/home/lab_espl_stud10/git-tutorial1/lib/espl_lib.c -o CMakeFiles/ESPL_LIB.dir/lib/espl_lib.c.s

CMakeFiles/ESPL_LIB.dir/lib/espl_lib.c.o.requires:

.PHONY : CMakeFiles/ESPL_LIB.dir/lib/espl_lib.c.o.requires

CMakeFiles/ESPL_LIB.dir/lib/espl_lib.c.o.provides: CMakeFiles/ESPL_LIB.dir/lib/espl_lib.c.o.requires
	$(MAKE) -f CMakeFiles/ESPL_LIB.dir/build.make CMakeFiles/ESPL_LIB.dir/lib/espl_lib.c.o.provides.build
.PHONY : CMakeFiles/ESPL_LIB.dir/lib/espl_lib.c.o.provides

CMakeFiles/ESPL_LIB.dir/lib/espl_lib.c.o.provides.build: CMakeFiles/ESPL_LIB.dir/lib/espl_lib.c.o


# Object files for target ESPL_LIB
ESPL_LIB_OBJECTS = \
"CMakeFiles/ESPL_LIB.dir/lib/espl_lib.c.o"

# External object files for target ESPL_LIB
ESPL_LIB_EXTERNAL_OBJECTS =

libESPL_LIB.a: CMakeFiles/ESPL_LIB.dir/lib/espl_lib.c.o
libESPL_LIB.a: CMakeFiles/ESPL_LIB.dir/build.make
libESPL_LIB.a: CMakeFiles/ESPL_LIB.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/DIST/home/lab_espl_stud10/git-tutorial1/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C static library libESPL_LIB.a"
	$(CMAKE_COMMAND) -P CMakeFiles/ESPL_LIB.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ESPL_LIB.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/ESPL_LIB.dir/build: libESPL_LIB.a

.PHONY : CMakeFiles/ESPL_LIB.dir/build

CMakeFiles/ESPL_LIB.dir/requires: CMakeFiles/ESPL_LIB.dir/lib/espl_lib.c.o.requires

.PHONY : CMakeFiles/ESPL_LIB.dir/requires

CMakeFiles/ESPL_LIB.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/ESPL_LIB.dir/cmake_clean.cmake
.PHONY : CMakeFiles/ESPL_LIB.dir/clean

CMakeFiles/ESPL_LIB.dir/depend:
	cd /DIST/home/lab_espl_stud10/git-tutorial1 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /DIST/home/lab_espl_stud10/git-tutorial1 /DIST/home/lab_espl_stud10/git-tutorial1 /DIST/home/lab_espl_stud10/git-tutorial1 /DIST/home/lab_espl_stud10/git-tutorial1 /DIST/home/lab_espl_stud10/git-tutorial1/CMakeFiles/ESPL_LIB.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/ESPL_LIB.dir/depend

