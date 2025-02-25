#!/usr/bin/env python3
import sys
import subprocess

if len(sys.argv) < 4:
    print("Usage: embed_file.py <input_file> <output_object_file> <arch>")
    sys.exit(1)

input_file = sys.argv[1]
output_object = sys.argv[2]
arch = sys.argv[3]  # Architecture from GN (e.g., "x86", "x64", "arm", "arm64")

# Map GN's `current_cpu` to the appropriate target machine flag
arch_flags = {
    "x86": "-melf_i386",
    "x64": "-melf_x86_64",
    "arm": "-marmelf",
    "arm64": "-maarch64elf",
}

if arch not in arch_flags:
    print(f"Error: Unsupported architecture '{arch}'")
    sys.exit(1)

cmd = [
    "ld",
    "-r",
    "--no-warn-rwx-segments",
    "-z", "noexecstack",
    arch_flags[arch],
    "-b", "binary",  # Treat input as raw binary data
    "-o", output_object,
    input_file
]

subprocess.run(cmd, check=True)
print(f"Generated object file: {output_object} for {arch}")
