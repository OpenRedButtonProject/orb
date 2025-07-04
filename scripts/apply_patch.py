import subprocess
import sys
import os

def apply_patch(target_path, patch_file_path):
    """Applies a patch file to a source folder."""
    try:
        abspath_target_path = os.path.abspath(target_path)
        subprocess.run(
            ['patch', '-p1', '-i', os.path.abspath(patch_file_path)],
            cwd=abspath_target_path,
            check=True, # Raise CalledProcessError if command returns non-zero exit status
            text=True,  # Capture stdout/stderr as text
            capture_output=True # Capture output for debugging
        )
        print(f"Patch '{patch_file_path}' applied successfully to folder '{abspath_target_path}'.")
    except subprocess.CalledProcessError as e:
        print(f"ERROR: Failed to apply patch '{patch_file_path}' to folder '{abspath_target_path}'.")
        print(f"Command: {' '.join(e.cmd)}")
        print(f"Stdout:\n{e.stdout}")
        print(f"Stderr:\n{e.stderr}")
        sys.exit(1) # Exit with an error code to fail the gclient sync
    except FileNotFoundError as e:
        print(f"ERROR:\n{e}")
        sys.exit(1)

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Usage: python apply_patch.py <target_path> <patch_file_path>")
        sys.exit(1)
    apply_patch(sys.argv[1], sys.argv[2])