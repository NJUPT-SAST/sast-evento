{ stdenvNoCC
, lib
, fetchzip
, autoPatchelfHook

, libgcc
, glibc
, libxkbcommon
, systemdLibs
, libinput
, mesa
, fontconfig
, freetype
, qt5
, ...
}:
let
  pname = "slint-cpp-bin";
  version = "1.8.0";
  
  platform =
    {
      x86_64-linux = "Linux-x86_64";
      x86_64-darwin = "Darwin-x86_64";
      aarch64-darwin = "Darwin-arm64";
    }."${stdenvNoCC.system}" or (throw "unsupported system ${stdenvNoCC.hostPlatform.system}");

  hash =
    {
      x86_64-linux = "sha256-PGj/brNrZ+euszDHiisZCWo6o3jK1LDTvCSrcThJn+w=";
      x86_64-darwin = "";
      aarch64-darwin = "";
    }."${stdenvNoCC.system}" or (throw "unsupported system ${stdenvNoCC.hostPlatform.system}");

in
  
stdenvNoCC.mkDerivation {
  inherit pname version;
  src = fetchzip {
    url = "https://github.com/slint-ui/slint/releases/download/v${version}/Slint-cpp-${version}-${platform}.tar.gz";
    hash = hash;
  };

  nativeBuildInputs = lib.optional stdenvNoCC.hostPlatform.isLinux [
    autoPatchelfHook
  ];

  buildInputs = [
    glibc
    libgcc
    libxkbcommon
    systemdLibs
    libinput
    mesa
    fontconfig
    freetype
    qt5.qtbase
  ];

  dontWrapQtApps = true;

  installPhase = ''
    runHook preInstall

    mkdir -p $out
    cp -r {bin,lib,include} $out/

    runHook postInstall
  '';

  meta = {
    platforms = [
      "x86_64-linux"
      "x86_64-darwin"
      "aarch64-darwin"
    ];
  };
}
