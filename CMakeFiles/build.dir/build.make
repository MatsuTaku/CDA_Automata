# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.9

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
CMAKE_COMMAND = /opt/local/bin/cmake

# The command to remove a file.
RM = /opt/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/takumamatsumoto/Labolatory/array-fsa/array-fsa

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/takumamatsumoto/Labolatory/array-fsa/array-fsa

# Include any dependencies generated for this target.
include CMakeFiles/Build.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/Build.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/Build.dir/flags.make

CMakeFiles/Build.dir/build.cpp.o: CMakeFiles/Build.dir/flags.make
CMakeFiles/Build.dir/build.cpp.o: build.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/takumamatsumoto/Labolatory/array-fsa/array-fsa/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/Build.dir/build.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Build.dir/build.cpp.o -c /Users/takumamatsumoto/Labolatory/array-fsa/array-fsa/build.cpp

CMakeFiles/Build.dir/build.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Build.dir/build.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/takumamatsumoto/Labolatory/array-fsa/array-fsa/build.cpp > CMakeFiles/Build.dir/build.cpp.i

CMakeFiles/Build.dir/build.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Build.dir/build.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/takumamatsumoto/Labolatory/array-fsa/array-fsa/build.cpp -o CMakeFiles/Build.dir/build.cpp.s

CMakeFiles/Build.dir/build.cpp.o.requires:

.PHONY : CMakeFiles/Build.dir/build.cpp.o.requires

CMakeFiles/Build.dir/build.cpp.o.provides: CMakeFiles/Build.dir/build.cpp.o.requires
	$(MAKE) -f CMakeFiles/Build.dir/build.make CMakeFiles/Build.dir/build.cpp.o.provides.build
.PHONY : CMakeFiles/Build.dir/build.cpp.o.provides

CMakeFiles/Build.dir/build.cpp.o.provides.build: CMakeFiles/Build.dir/build.cpp.o


CMakeFiles/Build.dir/ArrayFSABuilder.cpp.o: CMakeFiles/Build.dir/flags.make
CMakeFiles/Build.dir/ArrayFSABuilder.cpp.o: ArrayFSABuilder.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/takumamatsumoto/Labolatory/array-fsa/array-fsa/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/Build.dir/ArrayFSABuilder.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Build.dir/ArrayFSABuilder.cpp.o -c /Users/takumamatsumoto/Labolatory/array-fsa/array-fsa/ArrayFSABuilder.cpp

CMakeFiles/Build.dir/ArrayFSABuilder.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Build.dir/ArrayFSABuilder.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/takumamatsumoto/Labolatory/array-fsa/array-fsa/ArrayFSABuilder.cpp > CMakeFiles/Build.dir/ArrayFSABuilder.cpp.i

CMakeFiles/Build.dir/ArrayFSABuilder.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Build.dir/ArrayFSABuilder.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/takumamatsumoto/Labolatory/array-fsa/array-fsa/ArrayFSABuilder.cpp -o CMakeFiles/Build.dir/ArrayFSABuilder.cpp.s

CMakeFiles/Build.dir/ArrayFSABuilder.cpp.o.requires:

.PHONY : CMakeFiles/Build.dir/ArrayFSABuilder.cpp.o.requires

CMakeFiles/Build.dir/ArrayFSABuilder.cpp.o.provides: CMakeFiles/Build.dir/ArrayFSABuilder.cpp.o.requires
	$(MAKE) -f CMakeFiles/Build.dir/build.make CMakeFiles/Build.dir/ArrayFSABuilder.cpp.o.provides.build
.PHONY : CMakeFiles/Build.dir/ArrayFSABuilder.cpp.o.provides

CMakeFiles/Build.dir/ArrayFSABuilder.cpp.o.provides.build: CMakeFiles/Build.dir/ArrayFSABuilder.cpp.o


CMakeFiles/Build.dir/NArrayFSABuilder.cpp.o: CMakeFiles/Build.dir/flags.make
CMakeFiles/Build.dir/NArrayFSABuilder.cpp.o: NArrayFSABuilder.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/takumamatsumoto/Labolatory/array-fsa/array-fsa/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/Build.dir/NArrayFSABuilder.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Build.dir/NArrayFSABuilder.cpp.o -c /Users/takumamatsumoto/Labolatory/array-fsa/array-fsa/NArrayFSABuilder.cpp

CMakeFiles/Build.dir/NArrayFSABuilder.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Build.dir/NArrayFSABuilder.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/takumamatsumoto/Labolatory/array-fsa/array-fsa/NArrayFSABuilder.cpp > CMakeFiles/Build.dir/NArrayFSABuilder.cpp.i

CMakeFiles/Build.dir/NArrayFSABuilder.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Build.dir/NArrayFSABuilder.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/takumamatsumoto/Labolatory/array-fsa/array-fsa/NArrayFSABuilder.cpp -o CMakeFiles/Build.dir/NArrayFSABuilder.cpp.s

CMakeFiles/Build.dir/NArrayFSABuilder.cpp.o.requires:

.PHONY : CMakeFiles/Build.dir/NArrayFSABuilder.cpp.o.requires

CMakeFiles/Build.dir/NArrayFSABuilder.cpp.o.provides: CMakeFiles/Build.dir/NArrayFSABuilder.cpp.o.requires
	$(MAKE) -f CMakeFiles/Build.dir/build.make CMakeFiles/Build.dir/NArrayFSABuilder.cpp.o.provides.build
.PHONY : CMakeFiles/Build.dir/NArrayFSABuilder.cpp.o.provides

CMakeFiles/Build.dir/NArrayFSABuilder.cpp.o.provides.build: CMakeFiles/Build.dir/NArrayFSABuilder.cpp.o


CMakeFiles/Build.dir/ArrayFSATailBuilder.cpp.o: CMakeFiles/Build.dir/flags.make
CMakeFiles/Build.dir/ArrayFSATailBuilder.cpp.o: ArrayFSATailBuilder.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/takumamatsumoto/Labolatory/array-fsa/array-fsa/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/Build.dir/ArrayFSATailBuilder.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Build.dir/ArrayFSATailBuilder.cpp.o -c /Users/takumamatsumoto/Labolatory/array-fsa/array-fsa/ArrayFSATailBuilder.cpp

CMakeFiles/Build.dir/ArrayFSATailBuilder.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Build.dir/ArrayFSATailBuilder.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/takumamatsumoto/Labolatory/array-fsa/array-fsa/ArrayFSATailBuilder.cpp > CMakeFiles/Build.dir/ArrayFSATailBuilder.cpp.i

CMakeFiles/Build.dir/ArrayFSATailBuilder.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Build.dir/ArrayFSATailBuilder.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/takumamatsumoto/Labolatory/array-fsa/array-fsa/ArrayFSATailBuilder.cpp -o CMakeFiles/Build.dir/ArrayFSATailBuilder.cpp.s

CMakeFiles/Build.dir/ArrayFSATailBuilder.cpp.o.requires:

.PHONY : CMakeFiles/Build.dir/ArrayFSATailBuilder.cpp.o.requires

CMakeFiles/Build.dir/ArrayFSATailBuilder.cpp.o.provides: CMakeFiles/Build.dir/ArrayFSATailBuilder.cpp.o.requires
	$(MAKE) -f CMakeFiles/Build.dir/build.make CMakeFiles/Build.dir/ArrayFSATailBuilder.cpp.o.provides.build
.PHONY : CMakeFiles/Build.dir/ArrayFSATailBuilder.cpp.o.provides

CMakeFiles/Build.dir/ArrayFSATailBuilder.cpp.o.provides.build: CMakeFiles/Build.dir/ArrayFSATailBuilder.cpp.o


CMakeFiles/Build.dir/NArrayFSATextEdgeBuilder.cpp.o: CMakeFiles/Build.dir/flags.make
CMakeFiles/Build.dir/NArrayFSATextEdgeBuilder.cpp.o: NArrayFSATextEdgeBuilder.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/takumamatsumoto/Labolatory/array-fsa/array-fsa/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/Build.dir/NArrayFSATextEdgeBuilder.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Build.dir/NArrayFSATextEdgeBuilder.cpp.o -c /Users/takumamatsumoto/Labolatory/array-fsa/array-fsa/NArrayFSATextEdgeBuilder.cpp

CMakeFiles/Build.dir/NArrayFSATextEdgeBuilder.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Build.dir/NArrayFSATextEdgeBuilder.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/takumamatsumoto/Labolatory/array-fsa/array-fsa/NArrayFSATextEdgeBuilder.cpp > CMakeFiles/Build.dir/NArrayFSATextEdgeBuilder.cpp.i

CMakeFiles/Build.dir/NArrayFSATextEdgeBuilder.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Build.dir/NArrayFSATextEdgeBuilder.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/takumamatsumoto/Labolatory/array-fsa/array-fsa/NArrayFSATextEdgeBuilder.cpp -o CMakeFiles/Build.dir/NArrayFSATextEdgeBuilder.cpp.s

CMakeFiles/Build.dir/NArrayFSATextEdgeBuilder.cpp.o.requires:

.PHONY : CMakeFiles/Build.dir/NArrayFSATextEdgeBuilder.cpp.o.requires

CMakeFiles/Build.dir/NArrayFSATextEdgeBuilder.cpp.o.provides: CMakeFiles/Build.dir/NArrayFSATextEdgeBuilder.cpp.o.requires
	$(MAKE) -f CMakeFiles/Build.dir/build.make CMakeFiles/Build.dir/NArrayFSATextEdgeBuilder.cpp.o.provides.build
.PHONY : CMakeFiles/Build.dir/NArrayFSATextEdgeBuilder.cpp.o.provides

CMakeFiles/Build.dir/NArrayFSATextEdgeBuilder.cpp.o.provides.build: CMakeFiles/Build.dir/NArrayFSATextEdgeBuilder.cpp.o


CMakeFiles/Build.dir/PlainFSABuilder.cpp.o: CMakeFiles/Build.dir/flags.make
CMakeFiles/Build.dir/PlainFSABuilder.cpp.o: PlainFSABuilder.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/takumamatsumoto/Labolatory/array-fsa/array-fsa/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/Build.dir/PlainFSABuilder.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Build.dir/PlainFSABuilder.cpp.o -c /Users/takumamatsumoto/Labolatory/array-fsa/array-fsa/PlainFSABuilder.cpp

CMakeFiles/Build.dir/PlainFSABuilder.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Build.dir/PlainFSABuilder.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/takumamatsumoto/Labolatory/array-fsa/array-fsa/PlainFSABuilder.cpp > CMakeFiles/Build.dir/PlainFSABuilder.cpp.i

CMakeFiles/Build.dir/PlainFSABuilder.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Build.dir/PlainFSABuilder.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/takumamatsumoto/Labolatory/array-fsa/array-fsa/PlainFSABuilder.cpp -o CMakeFiles/Build.dir/PlainFSABuilder.cpp.s

CMakeFiles/Build.dir/PlainFSABuilder.cpp.o.requires:

.PHONY : CMakeFiles/Build.dir/PlainFSABuilder.cpp.o.requires

CMakeFiles/Build.dir/PlainFSABuilder.cpp.o.provides: CMakeFiles/Build.dir/PlainFSABuilder.cpp.o.requires
	$(MAKE) -f CMakeFiles/Build.dir/build.make CMakeFiles/Build.dir/PlainFSABuilder.cpp.o.provides.build
.PHONY : CMakeFiles/Build.dir/PlainFSABuilder.cpp.o.provides

CMakeFiles/Build.dir/PlainFSABuilder.cpp.o.provides.build: CMakeFiles/Build.dir/PlainFSABuilder.cpp.o


CMakeFiles/Build.dir/StringDictBuilder.cpp.o: CMakeFiles/Build.dir/flags.make
CMakeFiles/Build.dir/StringDictBuilder.cpp.o: StringDictBuilder.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/takumamatsumoto/Labolatory/array-fsa/array-fsa/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object CMakeFiles/Build.dir/StringDictBuilder.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Build.dir/StringDictBuilder.cpp.o -c /Users/takumamatsumoto/Labolatory/array-fsa/array-fsa/StringDictBuilder.cpp

CMakeFiles/Build.dir/StringDictBuilder.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Build.dir/StringDictBuilder.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/takumamatsumoto/Labolatory/array-fsa/array-fsa/StringDictBuilder.cpp > CMakeFiles/Build.dir/StringDictBuilder.cpp.i

CMakeFiles/Build.dir/StringDictBuilder.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Build.dir/StringDictBuilder.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/takumamatsumoto/Labolatory/array-fsa/array-fsa/StringDictBuilder.cpp -o CMakeFiles/Build.dir/StringDictBuilder.cpp.s

CMakeFiles/Build.dir/StringDictBuilder.cpp.o.requires:

.PHONY : CMakeFiles/Build.dir/StringDictBuilder.cpp.o.requires

CMakeFiles/Build.dir/StringDictBuilder.cpp.o.provides: CMakeFiles/Build.dir/StringDictBuilder.cpp.o.requires
	$(MAKE) -f CMakeFiles/Build.dir/build.make CMakeFiles/Build.dir/StringDictBuilder.cpp.o.provides.build
.PHONY : CMakeFiles/Build.dir/StringDictBuilder.cpp.o.provides

CMakeFiles/Build.dir/StringDictBuilder.cpp.o.provides.build: CMakeFiles/Build.dir/StringDictBuilder.cpp.o


# Object files for target Build
Build_OBJECTS = \
"CMakeFiles/Build.dir/build.cpp.o" \
"CMakeFiles/Build.dir/ArrayFSABuilder.cpp.o" \
"CMakeFiles/Build.dir/NArrayFSABuilder.cpp.o" \
"CMakeFiles/Build.dir/ArrayFSATailBuilder.cpp.o" \
"CMakeFiles/Build.dir/NArrayFSATextEdgeBuilder.cpp.o" \
"CMakeFiles/Build.dir/PlainFSABuilder.cpp.o" \
"CMakeFiles/Build.dir/StringDictBuilder.cpp.o"

# External object files for target Build
Build_EXTERNAL_OBJECTS =

libBuild.a: CMakeFiles/Build.dir/build.cpp.o
libBuild.a: CMakeFiles/Build.dir/ArrayFSABuilder.cpp.o
libBuild.a: CMakeFiles/Build.dir/NArrayFSABuilder.cpp.o
libBuild.a: CMakeFiles/Build.dir/ArrayFSATailBuilder.cpp.o
libBuild.a: CMakeFiles/Build.dir/NArrayFSATextEdgeBuilder.cpp.o
libBuild.a: CMakeFiles/Build.dir/PlainFSABuilder.cpp.o
libBuild.a: CMakeFiles/Build.dir/StringDictBuilder.cpp.o
libBuild.a: CMakeFiles/Build.dir/build.make
libBuild.a: CMakeFiles/Build.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/takumamatsumoto/Labolatory/array-fsa/array-fsa/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Linking CXX static library libBuild.a"
	$(CMAKE_COMMAND) -P CMakeFiles/Build.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Build.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/Build.dir/build: libBuild.a

.PHONY : CMakeFiles/Build.dir/build

CMakeFiles/Build.dir/requires: CMakeFiles/Build.dir/build.cpp.o.requires
CMakeFiles/Build.dir/requires: CMakeFiles/Build.dir/ArrayFSABuilder.cpp.o.requires
CMakeFiles/Build.dir/requires: CMakeFiles/Build.dir/NArrayFSABuilder.cpp.o.requires
CMakeFiles/Build.dir/requires: CMakeFiles/Build.dir/ArrayFSATailBuilder.cpp.o.requires
CMakeFiles/Build.dir/requires: CMakeFiles/Build.dir/NArrayFSATextEdgeBuilder.cpp.o.requires
CMakeFiles/Build.dir/requires: CMakeFiles/Build.dir/PlainFSABuilder.cpp.o.requires
CMakeFiles/Build.dir/requires: CMakeFiles/Build.dir/StringDictBuilder.cpp.o.requires

.PHONY : CMakeFiles/Build.dir/requires

CMakeFiles/Build.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/Build.dir/cmake_clean.cmake
.PHONY : CMakeFiles/Build.dir/clean

CMakeFiles/Build.dir/depend:
	cd /Users/takumamatsumoto/Labolatory/array-fsa/array-fsa && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/takumamatsumoto/Labolatory/array-fsa/array-fsa /Users/takumamatsumoto/Labolatory/array-fsa/array-fsa /Users/takumamatsumoto/Labolatory/array-fsa/array-fsa /Users/takumamatsumoto/Labolatory/array-fsa/array-fsa /Users/takumamatsumoto/Labolatory/array-fsa/array-fsa/CMakeFiles/Build.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/Build.dir/depend

