# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Default target executed when no arguments are given to make.
default_target: all
.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:

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
CMAKE_SOURCE_DIR = /u/riker/u96/ptpatel/cs252/task_manager

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /u/riker/u96/ptpatel/cs252/task_manager

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "No interactive CMake dialog available..."
	/usr/bin/cmake -E echo No\ interactive\ CMake\ dialog\ available.
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache
.PHONY : edit_cache/fast

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/bin/cmake --regenerate-during-build -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache
.PHONY : rebuild_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /u/riker/u96/ptpatel/cs252/task_manager/CMakeFiles /u/riker/u96/ptpatel/cs252/task_manager//CMakeFiles/progress.marks
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /u/riker/u96/ptpatel/cs252/task_manager/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean
.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named task_manager

# Build rule for target.
task_manager: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 task_manager
.PHONY : task_manager

# fast build rule for target.
task_manager/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/task_manager.dir/build.make CMakeFiles/task_manager.dir/build
.PHONY : task_manager/fast

gui/tabs_cpu.o: gui/tabs_cpu.c.o
.PHONY : gui/tabs_cpu.o

# target to build an object file
gui/tabs_cpu.c.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/task_manager.dir/build.make CMakeFiles/task_manager.dir/gui/tabs_cpu.c.o
.PHONY : gui/tabs_cpu.c.o

gui/tabs_cpu.i: gui/tabs_cpu.c.i
.PHONY : gui/tabs_cpu.i

# target to preprocess a source file
gui/tabs_cpu.c.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/task_manager.dir/build.make CMakeFiles/task_manager.dir/gui/tabs_cpu.c.i
.PHONY : gui/tabs_cpu.c.i

gui/tabs_cpu.s: gui/tabs_cpu.c.s
.PHONY : gui/tabs_cpu.s

# target to generate assembly for a file
gui/tabs_cpu.c.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/task_manager.dir/build.make CMakeFiles/task_manager.dir/gui/tabs_cpu.c.s
.PHONY : gui/tabs_cpu.c.s

gui/tabs_memory.o: gui/tabs_memory.c.o
.PHONY : gui/tabs_memory.o

# target to build an object file
gui/tabs_memory.c.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/task_manager.dir/build.make CMakeFiles/task_manager.dir/gui/tabs_memory.c.o
.PHONY : gui/tabs_memory.c.o

gui/tabs_memory.i: gui/tabs_memory.c.i
.PHONY : gui/tabs_memory.i

# target to preprocess a source file
gui/tabs_memory.c.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/task_manager.dir/build.make CMakeFiles/task_manager.dir/gui/tabs_memory.c.i
.PHONY : gui/tabs_memory.c.i

gui/tabs_memory.s: gui/tabs_memory.c.s
.PHONY : gui/tabs_memory.s

# target to generate assembly for a file
gui/tabs_memory.c.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/task_manager.dir/build.make CMakeFiles/task_manager.dir/gui/tabs_memory.c.s
.PHONY : gui/tabs_memory.c.s

gui/tabs_process.o: gui/tabs_process.c.o
.PHONY : gui/tabs_process.o

# target to build an object file
gui/tabs_process.c.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/task_manager.dir/build.make CMakeFiles/task_manager.dir/gui/tabs_process.c.o
.PHONY : gui/tabs_process.c.o

gui/tabs_process.i: gui/tabs_process.c.i
.PHONY : gui/tabs_process.i

# target to preprocess a source file
gui/tabs_process.c.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/task_manager.dir/build.make CMakeFiles/task_manager.dir/gui/tabs_process.c.i
.PHONY : gui/tabs_process.c.i

gui/tabs_process.s: gui/tabs_process.c.s
.PHONY : gui/tabs_process.s

# target to generate assembly for a file
gui/tabs_process.c.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/task_manager.dir/build.make CMakeFiles/task_manager.dir/gui/tabs_process.c.s
.PHONY : gui/tabs_process.c.s

gui/tabs_system.o: gui/tabs_system.c.o
.PHONY : gui/tabs_system.o

# target to build an object file
gui/tabs_system.c.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/task_manager.dir/build.make CMakeFiles/task_manager.dir/gui/tabs_system.c.o
.PHONY : gui/tabs_system.c.o

gui/tabs_system.i: gui/tabs_system.c.i
.PHONY : gui/tabs_system.i

# target to preprocess a source file
gui/tabs_system.c.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/task_manager.dir/build.make CMakeFiles/task_manager.dir/gui/tabs_system.c.i
.PHONY : gui/tabs_system.c.i

gui/tabs_system.s: gui/tabs_system.c.s
.PHONY : gui/tabs_system.s

# target to generate assembly for a file
gui/tabs_system.c.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/task_manager.dir/build.make CMakeFiles/task_manager.dir/gui/tabs_system.c.s
.PHONY : gui/tabs_system.c.s

main.o: main.c.o
.PHONY : main.o

# target to build an object file
main.c.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/task_manager.dir/build.make CMakeFiles/task_manager.dir/main.c.o
.PHONY : main.c.o

main.i: main.c.i
.PHONY : main.i

# target to preprocess a source file
main.c.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/task_manager.dir/build.make CMakeFiles/task_manager.dir/main.c.i
.PHONY : main.c.i

main.s: main.c.s
.PHONY : main.s

# target to generate assembly for a file
main.c.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/task_manager.dir/build.make CMakeFiles/task_manager.dir/main.c.s
.PHONY : main.c.s

memory_info.o: memory_info.c.o
.PHONY : memory_info.o

# target to build an object file
memory_info.c.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/task_manager.dir/build.make CMakeFiles/task_manager.dir/memory_info.c.o
.PHONY : memory_info.c.o

memory_info.i: memory_info.c.i
.PHONY : memory_info.i

# target to preprocess a source file
memory_info.c.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/task_manager.dir/build.make CMakeFiles/task_manager.dir/memory_info.c.i
.PHONY : memory_info.c.i

memory_info.s: memory_info.c.s
.PHONY : memory_info.s

# target to generate assembly for a file
memory_info.c.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/task_manager.dir/build.make CMakeFiles/task_manager.dir/memory_info.c.s
.PHONY : memory_info.c.s

process_info.o: process_info.c.o
.PHONY : process_info.o

# target to build an object file
process_info.c.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/task_manager.dir/build.make CMakeFiles/task_manager.dir/process_info.c.o
.PHONY : process_info.c.o

process_info.i: process_info.c.i
.PHONY : process_info.i

# target to preprocess a source file
process_info.c.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/task_manager.dir/build.make CMakeFiles/task_manager.dir/process_info.c.i
.PHONY : process_info.c.i

process_info.s: process_info.c.s
.PHONY : process_info.s

# target to generate assembly for a file
process_info.c.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/task_manager.dir/build.make CMakeFiles/task_manager.dir/process_info.c.s
.PHONY : process_info.c.s

system_info.o: system_info.c.o
.PHONY : system_info.o

# target to build an object file
system_info.c.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/task_manager.dir/build.make CMakeFiles/task_manager.dir/system_info.c.o
.PHONY : system_info.c.o

system_info.i: system_info.c.i
.PHONY : system_info.i

# target to preprocess a source file
system_info.c.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/task_manager.dir/build.make CMakeFiles/task_manager.dir/system_info.c.i
.PHONY : system_info.c.i

system_info.s: system_info.c.s
.PHONY : system_info.s

# target to generate assembly for a file
system_info.c.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/task_manager.dir/build.make CMakeFiles/task_manager.dir/system_info.c.s
.PHONY : system_info.c.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... edit_cache"
	@echo "... rebuild_cache"
	@echo "... task_manager"
	@echo "... gui/tabs_cpu.o"
	@echo "... gui/tabs_cpu.i"
	@echo "... gui/tabs_cpu.s"
	@echo "... gui/tabs_memory.o"
	@echo "... gui/tabs_memory.i"
	@echo "... gui/tabs_memory.s"
	@echo "... gui/tabs_process.o"
	@echo "... gui/tabs_process.i"
	@echo "... gui/tabs_process.s"
	@echo "... gui/tabs_system.o"
	@echo "... gui/tabs_system.i"
	@echo "... gui/tabs_system.s"
	@echo "... main.o"
	@echo "... main.i"
	@echo "... main.s"
	@echo "... memory_info.o"
	@echo "... memory_info.i"
	@echo "... memory_info.s"
	@echo "... process_info.o"
	@echo "... process_info.i"
	@echo "... process_info.s"
	@echo "... system_info.o"
	@echo "... system_info.i"
	@echo "... system_info.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system
