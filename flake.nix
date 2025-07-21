{
  description = "HyperIntegrated MetaSystem";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.05";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { nixpkgs, flake-utils, ... }: 
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
      in
      {
        packages.default = pkgs.stdenv.mkDerivation {
          pname = "1amcode";
          version = "1";

          src = ./.;

          buildInputs = [
            pkgs.clang
            pkgs.gmp
            pkgs.cairo
            pkgs.xorg.libxcb
            pkgs.libxkbcommon
            pkgs.pipewire
            pkgs.pkg-config
          ];

          buildPhase = "make";

          installPhase = ''
            mkdir -p $out/bin
            install -Dm755 ./program.exe $out/bin/program.exe
          '';
        };

        devShells.default = pkgs.mkShell {
          buildInputs = [
            pkgs.clang
            pkgs.gmp
            pkgs.cairo
            pkgs.xorg.libxcb
            pkgs.pipewire
            pkgs.pkg-config
            pkgs.libxkbcommon
          ];
        };
      });
}
