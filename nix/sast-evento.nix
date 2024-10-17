{ lib,
  fetchgit,
  fetchzip,
  cmake,
  pkg-config,
  git,
  gcc14Stdenv,
  
  openssl_3_3,
  boost184,
  spdlog,
  nlohmann_json,
  tomlplusplus,
  liburing,
  gettext,
  qtbase,
  qtwayland,
  wrapQtAppsHook,
  libsecret,
  slint,
  ...
}:

gcc14Stdenv.mkDerivation rec {
  pname = "sast-evento";
  version = "2.0.60";
    
  src = fetchgit {
    url = "https://github.com/NJUPT-SAST/sast-evento.git";
    rev = "refs/tags/${version}";
    deepClone = true;
    fetchSubmodules = true; 
    hash = "sha256-bNHo1f+a31MXzMZVKCUxpmmI3CgMB2buKjV8IBRVSBM=";
  };
  
  buildInputs = [
    boost184
    openssl_3_3
    spdlog
    nlohmann_json
    tomlplusplus
    liburing
    gettext
    qtbase
    libsecret
    slint
  ] ++ lib.optional gcc14Stdenv.hostPlatform.isLinux qtwayland;
  nativeBuildInputs = [
    cmake
    pkg-config
    wrapQtAppsHook
    git
  ];

  postPatch = ''
    sed -i 's/''${GIT_CACHE_PATH}\/index//' .cmake/Version.cmake
  '';

  cmakeFlags = [
     "-DSLINT_FEATURE_RENDERER_SKIA=ON"
     "-DSLINT_FEATURE_RENDERER_FEMTOVG=OFF"
     "-DSlint_DIR=${slint}/lib/cmake/Slint"
    ];
  
}
