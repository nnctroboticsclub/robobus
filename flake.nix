{
  inputs.nixpkgs.url = "github:nixos/nixpkgs/25.11";

  inputs.roboenv.url = "github:nnctroboticsclub/roboenv-nix";
  inputs.roboenv.inputs.nixpkgs.follows = "nixpkgs";

  inputs.nano.url = "github:nnctroboticsclub/nano";
  inputs.nano.inputs.nixpkgs.follows = "nixpkgs";
  inputs.nano.inputs.roboenv.follows = "roboenv";

  inputs.syoch-robotics.url = "github:nnctroboticsclub/syoch-robotics";
  inputs.syoch-robotics.inputs.nano.follows = "nano";
  inputs.syoch-robotics.inputs.nixpkgs.follows = "nixpkgs";
  inputs.syoch-robotics.inputs.roboenv.follows = "roboenv";

  outputs =
    {
      nixpkgs,
      roboenv,
      nano,
      syoch-robotics,
      ...
    }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs {
        inherit system;
      };
      roboPkgs = roboenv.legacyPackages.${system};

      devPkgs = [
        roboPkgs.cmake-libs
        roboPkgs.clang-toolchain
        roboPkgs.cmsis5
        syoch-robotics.packages.${system}.default
        nano.packages.${system}.default
      ];
    in
    {
      packages.x86_64-linux.default = roboPkgs.rlib.buildCMakeProject {
        pname = "robobus";
        version = "v1.0.0";
        src = ./.;

        cmakeBuildInputs = devPkgs;
      };
      devShells.x86_64-linux.default = roboPkgs.roboenv {
        name = "robobus";

        rust.enable = true;
        STM32.enable = true;
        c_cpp.enable = true;
        c_cpp.toolchain = "clang";
        tool.usb.enable = true;

        frameworks = [
          {
            type = "StaticMbedOS";
            mbedTarget = "NUCLEO_F446RE";
          }
        ];

        cmakeInputs = devPkgs;
        buildInputs = [
          pkgs.commitizen
        ];
      };
    };
}
