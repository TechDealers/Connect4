{
  description = "Description for the project";

  inputs = {
    nixpkgs = {
      url = "github:NixOS/nixpkgs/nixos-unstable";
    };
    flake-utils = {
      url = "github:numtide/flake-utils";
    };
    devshell = {
      url = "github:numtide/devshell";
    };
  };

  outputs = {
    nixpkgs,
    flake-utils,
    devshell,
    ...
  }:
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = import nixpkgs {
        inherit system;
        overlays = [
          devshell.overlays.default
        ];
      };
    in {
      devShells.default =
        pkgs.devshell.mkShell {
          packages = with pkgs; [
            gnumake
            gcc
            clang-tools
            bear
          ];
        };
    });
}
