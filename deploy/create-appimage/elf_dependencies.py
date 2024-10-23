#!/usr/bin/env python3
"""A module to find the dependencies of an ELF file """

import os
import platform
import subprocess
import re
import shutil
from elftools.elf.elffile import ELFFile


def get_direct_dependencies(elffile_path):
    """Get the list of direct dependencies of the ELF file"""
    deps = []
    with open(elffile_path, "rb") as f:
        elffile = ELFFile(f)
        dynamic = elffile.get_section_by_name(".dynamic")
        if not dynamic:
            return deps
        for tag in dynamic.iter_tags():
            if tag.entry.d_tag == "DT_NEEDED":
                deps.append(tag.needed)
    return deps


def expand_rpath(elffile_path, rpath_dir):
    """Expand the variable (eg. $ORIGIN) in the RPATH"""
    if "$ORIGIN" in rpath_dir:
        origin = os.path.dirname(os.path.abspath(os.path.realpath(elffile_path)))
        return rpath_dir.replace("$ORIGIN", origin)
    return rpath_dir


def get_rpath(elffile_path):
    """Get the RPATH from the ELF file"""
    rpath = []
    with open(elffile_path, "rb") as f:
        elffile = ELFFile(f)
        dynamic = elffile.get_section_by_name(".dynamic")
        if not dynamic:
            return rpath
        for tag in dynamic.iter_tags():
            if tag.entry.d_tag == "DT_RPATH":
                for rpath_dir in tag.rpath.split(":"):
                    rpath.append(expand_rpath(elffile_path, rpath_dir))

    return rpath


def get_runpath(elffile_path):
    """Get the RUNPATH from the ELF file"""
    runpath = []
    with open(elffile_path, "rb") as f:
        elffile = ELFFile(f)
        dynamic = elffile.get_section_by_name(".dynamic")
        if not dynamic:
            return runpath
        for tag in dynamic.iter_tags():
            if tag.entry.d_tag == "DT_RUNPATH":
                for runpath_dir in tag.runpath.split(":"):
                    runpath.append(expand_rpath(elffile_path, runpath_dir))
    return runpath


def get_ldconfig_paths():
    """Parse the output of 'ldconfig -p' to get known library paths"""
    ldconfig = shutil.which("ldconfig")
    if not ldconfig:
        print("Error: ldconfig command not found")
        return {}
    output = subprocess.check_output([ldconfig, "-p"]).decode("utf-8")
    expr = re.compile(r"\s+(.*)\s+\((.*)\)\s+=>\s+(.*)")
    paths = {}
    for line in output.splitlines():
        match = expr.match(line)
        if match:
            libname = match.group(1)
            libpath = match.group(3)
            paths.setdefault(libname, []).append(libpath)
    return paths


def get_default_lib_dirs():
    """Get the system's default library directories based on architecture"""
    arch = platform.architecture()[0]
    if arch == "64bit":
        return ["/lib64", "/usr/lib64", "/lib", "/usr/lib"]
    return ["/lib", "/usr/lib"]


def find_library(libname, rpath_dirs, runpath_dirs, ldconfig_paths, default_dirs):
    """Find the library file in specified directories, considering RPATH and RUNPATH differences"""

    # If the library name is an absolute path, return it directly
    if libname.startswith("/"):
        return libname

    # Prepare the search order based on dynamic linker rules
    # For RPATH and RUNPATH, we need to distinguish the search order
    search_dirs = []

    # For RPATH: It is searched before LD_LIBRARY_PATH and is ignored if RUNPATH is present
    # However, if RUNPATH is present, RPATH is ignored in modern systems.
    # To replicate the dynamic linker's behavior, we need to check which tags are present.

    # If RUNPATH is present, RPATH is ignored in favor of RUNPATH and LD_LIBRARY_PATH
    if runpath_dirs:
        # LD_LIBRARY_PATH
        ld_library_path = os.environ.get("LD_LIBRARY_PATH", "")
        if ld_library_path:
            search_dirs.extend(ld_library_path.split(":"))

        # RUNPATH directories
        search_dirs.extend(runpath_dirs)
    else:
        # RPATH directories
        search_dirs.extend(rpath_dirs)

        # LD_LIBRARY_PATH
        ld_library_path = os.environ.get("LD_LIBRARY_PATH", "")
        if ld_library_path:
            search_dirs.extend(ld_library_path.split(":"))

    # Then the directories from ldconfig
    # ldconfig paths are used as a mapping from library names to paths
    # So we'll use it directly if the library is found there
    # Otherwise, we proceed to default directories

    # Search in the accumulated search directories
    for directory in search_dirs:
        if directory:
            libpath = os.path.join(directory, libname)
            if os.path.exists(libpath):
                return libpath

    # Search in ldconfig known paths
    paths = ldconfig_paths.get(libname)
    if paths:
        return paths[0]

    # Search in default library directories
    for directory in default_dirs:
        libpath = os.path.join(directory, libname)
        if os.path.exists(libpath):
            return libpath

    return None


def get_dependencies(elffile_path, deps=None):
    """Recursively get all direct and indirect dependencies"""
    elffile_path = os.path.abspath(elffile_path)
    if elffile_path in deps:
        return deps
    deps.add(elffile_path)
    dep_names = get_direct_dependencies(elffile_path)
    rpath_dirs = get_rpath(elffile_path)
    runpath_dirs = get_runpath(elffile_path)
    default_dirs = get_default_lib_dirs()
    ldconfig_paths = get_ldconfig_paths()
    for dep_name in dep_names:
        dep_path = find_library(
            dep_name, rpath_dirs, runpath_dirs, ldconfig_paths, default_dirs
        )
        if dep_path:
            get_dependencies(dep_path, deps)
        else:
            print(
                f"Warning: Could not find library {dep_name} required by {elffile_path}"
            )
    return deps


def get_dependencies_for_files(elffile_paths, deps=None):
    """Process multiple ELF files to collect all dependencies"""
    if deps is None:
        deps = set()
    for elffile_path in elffile_paths:
        get_dependencies(elffile_path, deps)
    return deps


__REGEX_LIB_IN_PATH = re.compile(r".*\.so(?:\.\d+)?")


def get_dependencies_for_lib_dir(directory, deps=None):
    """Recursively get all dependencies for all ELF files in a directory and its subdirectories"""
    if deps is None:
        deps = set()
    for root, _, files in os.walk(directory):
        for file in files:
            if __REGEX_LIB_IN_PATH.match(file):
                get_dependencies(os.path.join(root, file), deps)
    return deps
