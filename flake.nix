{
  description = "SAST Evento Dev Env";

  inputs = {
    flake-parts.url = "github:hercules-ci/flake-parts";
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = inputs@{ flake-parts, ... }:
    flake-parts.lib.mkFlake { inherit inputs; } {
      systems = [ "x86_64-linux" "aarch64-linux" "aarch64-darwin" "x86_64-darwin" ];
      perSystem = { config, pkgs, ... }: {
        packages.default = config.packages.sast-evento;

        packages.sast-evento = pkgs.qt6Packages.callPackage ./nix/sast-evento.nix { slint=config.packages.slint-cpp-bin; };
	packages.slint-cpp = pkgs.callPackage ./nix/slint.nix { };
	packages.slint-cpp-bin = pkgs.callPackage ./nix/slint-bin.nix { };

        # checks.sast-evento = pkgs.callPackage ./nix/test.nix {
          # sast-evento = config.packages.sast-evento;
        # };
      };
    };

  nixConfig = {
    substituters = [
      "https://mirrors.ustc.edu.cn/nix-channels/store"
      "https://cache.nixos.org"
    ];
  };
}
