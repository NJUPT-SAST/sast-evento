#!/usr/bin/env python3
"""Deploy application as AppImage"""

import os
import platform
import re
import shutil
import tempfile
import argparse
from elf_dependencies import (
    get_dependencies,
    get_dependencies_for_lib_dir,
    get_dependencies_for_files,
)
from grouping import Grouping

REGEX_LD_LINUX = re.compile(r"ld-linux.*\.so(?:\.\d+)*")
REGEX_LIB_ARCH_ROOT = re.compile(r"[^/]*?-linux-gnu/")
REGEX_GLIBC = re.compile(
    r"ld-.*\.so"
    + r"|ld-linux.*\.so(?:\.\d+)*"
    + r"|libBrokenLocale-.*\.so"
    + r"|libBrokenLocale\.so(?:\.\d+)*"
    + r"|libSegFault\.so"
    + r"|libanl-.*\.so"
    + r"|libanl\.so(?:\.\d+)*"
    + r"|libc-.*\.so"
    + r"|libc\.so(?:\.\d+)*"
    + r"|libcrypt\.so(?:\.\d+)*"
    + r"|libcrypt\.so(?:\.\d+)*"
    + r"|libdl-.*\.so"
    + r"|libdl\.so(?:\.\d+)*"
    + r"|libgcc_s\.so(?:\.\d+)*"
    + r"|libm-.*\.so"
    + r"|libm\.so(?:\.\d+)*"
    + r"|libmemusage\.so(?:\.\d+)*"
    + r"|libmvec-.*\.so"
    + r"|libmvec\.so(?:\.\d+)*"
    + r"|libnsl-.*\.so"
    + r"|libnsl\.so(?:\.\d+)*"
    + r"|libnss_compat-.*\.so"
    + r"|libnss_compat\.so(?:\.\d+)*"
    + r"|libnss_dns-.*\.so"
    + r"|libnss_dns\.so(?:\.\d+)*"
    + r"|libnss_files-.*\.so"
    + r"|libnss_files\.so(?:\.\d+)*"
    + r"|libnss_hesiod-.*\.so"
    + r"|libnss_hesiod\.so(?:\.\d+)*"
    + r"|libnss_nis-.*\.so"
    + r"|libnss_nis\.so(?:\.\d+)*"
    + r"|libnss_nisplus-.*\.so"
    + r"|libnss_nisplus\.so(?:\.\d+)*"
    + r"|libpcprofile\.so"
    + r"|libpthread-.*\.so"
    + r"|libpthread\.so(?:\.\d+)*"
    + r"|libresolv-.*\.so"
    + r"|libresolv\.so(?:\.\d+)*"
    + r"|librt-.*\.so"
    + r"|librt\.so(?:\.\d+)*"
    + r"|libthread_db-.*\.so"
    + r"|libthread_db\.so(?:\.\d+)*"
    + r"|libutil-.*\.so"
    + r"|libutil\.so(?:\.\d+)*"
    + r"|libz\.so(?:\.\d+)*"
    + r"|libz\.so(?:\.\d+)*"
)
REGEX_QT_CORE = re.compile(r"libQt(\d*)Core\.so(?:\.(\d+))?")
REGEX_QT_GUI = re.compile(r"libQt(\d*)Gui\.so(?:\.(\d+))?")
REGEX_QT_IF_OPENGL = re.compile(
    r"libQt(\d*)(?:Gui|OpenGL|XcbQpa)\.so(?:\.(\d+))?|libxcb-glx.so"
)
REGEX_QT_PRINT_SUPPORT = re.compile(r"libQt(\d*)PrintSupport\.so(?:\.(\d+))?")
REGEX_QT_NETWORK = re.compile(r"libQt(\d*)Network\.so(?:\.(\d+))?")
REGEX_QT_SQL = re.compile(r"libQt(\d*)Sql\.so(?:\.(\d+))?")
REGEX_QT_POSITIONING = re.compile(r"libQt(\d*)Positioning\.so(?:\.(\d+))?")
REGEX_QT_MULTIMEDIA = re.compile(r"libQt(\d*)Multimedia\.so(?:\.(\d+))?")


def get_lib_compat_path(path, mapping):
    """Remove common prefixes from path"""
    common_prefixes = [
        "/usr/lib/",
        "/lib/",
        "/usr/lib64/",
        "/lib64/",
        "/usr/local/lib/",
        "/usr/local/lib64/",
    ]
    for prefix, replacement in mapping.items():
        if path.startswith(prefix):
            return path.replace(prefix, replacement)
    for prefix in common_prefixes:
        if path.startswith(prefix):
            path = path[len(prefix) :]
            arch_root_match = REGEX_LIB_ARCH_ROOT.match(path)
            if arch_root_match:
                path = path[len(arch_root_match.group(0) or "") :]
            return path
    if path.startswith("/"):
        return path[1:]
    return path


def find_qt(deps):
    """Find Qt Core library and version"""
    for dep in deps:
        qt_core_match = REGEX_QT_CORE.match(os.path.basename(dep))
        if qt_core_match:
            return dep, qt_core_match.group(1) or qt_core_match.group(2)
    return None, None


def find_qt_dir(lib_qt_core, qt_version):
    """Find Qt dir based on Qt Core library"""
    possible_qt_dirs = [
        os.path.join(os.path.dirname(lib_qt_core), f"Qt{qt_version}"),
        os.path.join(os.path.dirname(lib_qt_core), f"qt{qt_version}"),
        os.path.join(os.path.dirname(lib_qt_core), ".."),
        os.path.join(os.path.dirname(lib_qt_core), "..", f"Qt{qt_version}"),
        os.path.join(os.path.dirname(lib_qt_core), "..", f"qt{qt_version}"),
    ]
    if "QTDIR" in os.environ:
        possible_qt_dirs.append(os.path.join(os.environ["QTDIR"]))
    if "QT_ROOT_DIR" in os.environ:
        possible_qt_dirs.append(os.path.join(os.environ["QT_ROOT_DIR"]))
    if "QT_PLUGIN_PATH" in os.environ:
        possible_qt_dirs.append(os.path.join(os.environ["QT_PLUGIN_PATH"], ".."))
    qt_dir = None
    for x in possible_qt_dirs:
        if os.path.exists(os.path.join(x, "plugins")):
            qt_dir = x
            break
    return os.path.abspath(qt_dir)


def match_lib(deps, regex):
    """Find the first library that matches the regex"""
    for dep in deps:
        if regex.match(os.path.basename(dep)):
            return dep
    return None


def try_deploy_qt(deps, env, lib_compat_mapping):
    """Deploy Qt plugins if Qt Core library is found"""
    lib_qt_core, qt_version = find_qt(deps)
    if not lib_qt_core:
        print("Qt Core library not found, skipping deploying Qt")
        return None
    print(f"Found Qt Core: {lib_qt_core}, adding Qt plugins")

    qt_dir = find_qt_dir(lib_qt_core, qt_version)
    if not qt_dir:
        print("Qt dir not found, skipping deploying Qt")
        return None
    print(f"Found Qt dir: {qt_dir}")

    compat_qt_dir = os.path.relpath(qt_dir, os.path.dirname(lib_qt_core))
    lib_compat_mapping[qt_dir] = compat_qt_dir
    env["QT_PLUGIN_PATH"] = f"$APPDIR/lib/{compat_qt_dir}/plugins"

    get_dependencies(os.path.join(qt_dir, "plugins", "platforms", "libqxcb.so"), deps)
    if match_lib(deps, REGEX_QT_GUI):
        print("Found Qt GUI, adding imageformats, iconengines, platforminputcontexts")
        get_dependencies_for_lib_dir(
            os.path.join(qt_dir, "plugins", "iconengines"), deps
        )
        get_dependencies_for_lib_dir(
            os.path.join(qt_dir, "plugins", "imageformats"), deps
        )
        get_dependencies_for_lib_dir(
            os.path.join(qt_dir, "plugins", "platforminputcontexts"), deps
        )
    if match_lib(deps, REGEX_QT_IF_OPENGL):
        print("Found Qt GUI/OpenGL/XcbQpa, adding platforms, xcbglintegrations")
        get_dependencies_for_lib_dir(
            os.path.join(qt_dir, "plugins", "xcbglintegrations"), deps
        )
    if match_lib(deps, REGEX_QT_PRINT_SUPPORT):
        print("Found Qt PrintSupport, adding printsupport")
        get_dependencies_for_lib_dir(
            os.path.join(qt_dir, "plugins", "printsupport"), deps
        )
    if match_lib(deps, REGEX_QT_NETWORK):
        print("Found Qt Network, adding bearer")
        get_dependencies_for_lib_dir(os.path.join(qt_dir, "plugins", "bearer"), deps)
    if match_lib(deps, REGEX_QT_SQL):
        print("Found Qt SQL, adding sqldrivers")
        get_dependencies_for_lib_dir(
            os.path.join(qt_dir, "plugins", "sqldrivers"), deps
        )
    if match_lib(deps, REGEX_QT_POSITIONING):
        print("Found Qt Positioning, adding position")
        get_dependencies_for_lib_dir(os.path.join(qt_dir, "plugins", "position"), deps)
    if match_lib(deps, REGEX_QT_MULTIMEDIA):
        print("Found Qt Multimedia, adding mediaservice, audio")
        get_dependencies_for_lib_dir(
            os.path.join(qt_dir, "plugins", "mediaservice"), deps
        )
        get_dependencies_for_lib_dir(os.path.join(qt_dir, "plugins", "audio"), deps)
    return qt_dir


def deploy_general_dependencies(deps, app_dir, lib_compat_mapping):
    """Deploy general dependencies for AppDir"""
    compat_lib_dir = os.path.join(app_dir, "lib")
    os.makedirs(compat_lib_dir, exist_ok=True)
    for dep in deps:
        if dep.startswith(os.path.abspath(app_dir)):
            continue
        lib_compat_path = get_lib_compat_path(dep, lib_compat_mapping)
        if not REGEX_GLIBC.match(os.path.basename(dep)):
            print(f"Found dependencies: {dep}")
            dest = os.path.join(compat_lib_dir, lib_compat_path)
            os.makedirs(os.path.dirname(dest), exist_ok=True)
            shutil.copy(dep, dest)


def deploy_compat_runtime(deps, app_dir, lib_compat_mapping):
    """Deploy compat runtime for AppDir"""
    runtime_dir = os.path.join(app_dir, "runtime")
    compat_runtime_dir = os.path.join(runtime_dir, "compat")
    os.makedirs(compat_runtime_dir, exist_ok=True)
    for dep in deps:
        if dep.startswith(os.path.abspath(app_dir)):
            continue
        lib_compat_path = get_lib_compat_path(dep, lib_compat_mapping)
        if REGEX_GLIBC.match(os.path.basename(dep)):
            print(f"Found runtime: {dep}")
            dest = os.path.join(compat_runtime_dir, lib_compat_path)
            os.makedirs(os.path.dirname(dest), exist_ok=True)
            shutil.copy(dep, dest)


def setup_default_runtime(deps, app_dir, env, lib_compat_mapping, executables):
    """Setup runtime for AppDir"""
    runtime_dir = os.path.join(app_dir, "runtime")
    default_runtime_dir = os.path.join(runtime_dir, "default")
    os.makedirs(default_runtime_dir, exist_ok=True)
    ld_linux = match_lib(deps, REGEX_LD_LINUX)
    if ld_linux:
        print(f"Found ld-linux: {ld_linux}")
        ld_compat_path = get_lib_compat_path(ld_linux, lib_compat_mapping)

        default_in_system = "/lib64/" + os.path.basename(ld_linux)
        if not os.path.exists(default_in_system):
            default_in_system = "/lib/" + os.path.basename(ld_linux)

        default_in_package = os.path.join(default_runtime_dir, ld_compat_path)
        os.makedirs(os.path.dirname(default_in_package), exist_ok=True)
        if os.path.exists(default_in_package):
            os.remove(default_in_package)
        os.symlink(default_in_system, default_in_package)

        for exe in executables:
            print(f"Updating interpreter for {exe}")
            os.system(f'patchelf --set-interpreter "{ld_compat_path}" "{exe}"')

        env["APPDIR_LIBC_LINKER_PATH"] = ld_compat_path


def deploy(app_dir, executables, target_arch="x86_64"):
    """Deploy dependencies for AppDir"""
    glibc_version = platform.libc_ver()[1]
    env = {}
    env["APPDIR"] = "$ORIGIN"
    env["APPDIR_EXEC_PATH"] = "$APPDIR/" + os.path.relpath(executables[0], app_dir)
    env["APPDIR_EXEC_ARGS"] = "$@"
    env["APPDIR_LIBRARY_PATH"] = "$APPDIR/lib"
    env["APPDIR_LIBC_LIBRARY_PATH"] = "$APPDIR/runtime/compat"
    env["APPDIR_LIBC_VERSION"] = glibc_version
    env["XDG_DATA_DIRS"] = "$APPDIR/usr/local/share:$APPDIR/usr/share:$XDG_DATA_DIRS"
    env["XDG_CONFIG_DIRS"] = "$APPDIR/etc/xdg:$XDG_CONFIG_DIRS"
    env["PATH"] = "$APPDIR/usr/bin:$PATH"
    env["GTK_EXE_PREFIX"] = "$APPDIR/usr"
    env["GTK_DATA_PREFIX"] = "$APPDIR/usr"

    lib_compat_mapping = {}
    deps = get_dependencies_for_files(executables)
    with Grouping("Deploy Qt"):
        try_deploy_qt(deps, env, lib_compat_mapping)
    with Grouping("Deploy general dependencies"):
        deploy_general_dependencies(deps, app_dir, lib_compat_mapping)
    with Grouping("Deploy compat runtime"):
        deploy_compat_runtime(deps, app_dir, lib_compat_mapping)
    with Grouping("Setup default runtime"):
        setup_default_runtime(deps, app_dir, env, lib_compat_mapping, executables)

    with Grouping("Write AppRun.env"):
        with open(os.path.join(app_dir, "AppRun.env"), "w", encoding="utf-8") as f:
            for key, value in env.items():
                print(f"{key}={value}")
                f.write(f"{key}={value}\n")

    url_libapprun_hooks = f"https://github.com/AppImageCrafters/AppRun/releases/download/v2.0.0/libapprun_hooks-Release-{target_arch}.so"
    url_apprun = f"https://github.com/AppImageCrafters/AppRun/releases/download/v2.0.0/AppRun-Release-{target_arch}"
    with Grouping("Download AppRun hooks"):
        print(f"Fetching {url_libapprun_hooks}")
        os.system(f'wget -O "{app_dir}/lib/libapprun_hooks.so" "{url_libapprun_hooks}"')
    with Grouping("Download AppRun"):
        print(f"Fetching {url_apprun}")
        os.system(f'wget -O "{app_dir}/AppRun" "{url_apprun}"')
        os.system(f'chmod +x "{app_dir}/AppRun"')


def make_app_image(app_dir, dest, host_arch="x86_64"):
    """Make AppImage from AppDir"""
    temp_dir = tempfile.mkdtemp()
    url_appimagetool = f"https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-{host_arch}.AppImage"
    with Grouping("Download AppImage tool"):
        print(f"Fetching {url_appimagetool}")
        os.system(f'wget -O "{temp_dir}/appimagetool.AppImage" "{url_appimagetool}"')
        os.system(f'chmod +x "{temp_dir}/appimagetool.AppImage"')
    with Grouping("Make AppImage"):
        os.system(f'"{temp_dir}/appimagetool.AppImage" "{app_dir}" "{dest}" -v')
    shutil.rmtree(temp_dir)


def main():
    """Main function"""
    parser = argparse.ArgumentParser(description="Deploy application as AppImage")
    parser.add_argument("--app-dir", required=True, help="Path to AppDir")
    parser.add_argument("--executables", required=True, help="Path to executables")
    parser.add_argument("--target", required=True, help="Path to AppImage")
    args = parser.parse_args()
    deploy(args.app_dir, args.executables.split(":"))
    make_app_image(args.app_dir, args.target)


if __name__ == "__main__":
    main()
