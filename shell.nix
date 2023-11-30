let pkgs = import (builtins.fetchTarball "http://nixos.org/channels/nixos-23.05/nixexprs.tar.xz") {};
in
pkgs.mkShell {
  packages = [
    # pkgs.openssl.dev
    pkgs.gcc13
  ];

  NIX_HARDENING_ENABLE = "";
}
