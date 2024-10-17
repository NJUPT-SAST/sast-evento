{ stdenv
, lib
, fetchFromGitHub
, cmake
, rustc
, cargo
, ...
}:

stdenv.mkDerivation rec {
  pname = "slint-cpp";
  version = "1.8.0";
  srcs = [
    (fetchFromGitHub {
      owner = "slint-ui";
      repo = "slint";
      rev = "refs/tags/v${version}";
      hash = "sha256-6vFr9QURVwLSeBpMmSj5UVV3FRdbvnzc6OS+iTVFJkE=";
      name = pname;
      fetchSubmodules = true; 
    })
    (fetchFromGitHub {
      owner = "corrosion-rs";
      repo = "corrosion";
      rev = "v0.4.9";
      name = "corrosion";
      hash = "sha256-wiK5HfNMJV0kaL3vNTz22JCHpeIdkqCxJNyJ8XFhZy8=";
    })
  ];

  sourceRoot = pname;
  
  patches = [
    ./patches/0001-disable-download-corrosion-from-net.patch
  ];

  postPatch = ''
    sed -i 's|''${Corrosion_SOURCE_DIR}/cmake)|${lib.elemAt srcs 1}/cmake)\ninclude(Corrosion)|' api/cpp/CMakeLists.txt
  '';
  
  nativeBuildInputs = [
    cmake
    rustc
    cargo
  ];
  
  cmakeFlags = [
    "-DCMAKE_BUILD_TYPE=MinSizeRel"
    "-DSLINT_FEATURE_BACKEND_QT=No"
    "-DSLINT_FEATURE_RENDERER_SKIA_OPENGL=Yes"
    "-DSLINT_FEATURE_RENDERER_SKIA_VULKAN=Yes"
    "-DSLINT_FEATURE_RENDERER_SOFTWARE=Yes"
  ];
  # installPhase = ''
  #   runHook preInstall

    
    
  #   runHook postInstall
  # '';
}