import argparse
import os
import shutil
import subprocess
import sys

def main():
    # Get all arguments
    parser = argparse.ArgumentParser(description="Copies source directories and applies a patch.")
    parser.add_argument("--src_root", required=True, help="Path to the original source root directory.")
    parser.add_argument("--dest_root", required=True, help="Path to the destination root directory.")
    parser.add_argument("--patch_file", required=True, help="Path to the patch file.")
    parser.add_argument("--patch_target_folder", required=True, help="Path to the patch target folder.")
    parser.add_argument("--dirs_to_copy", required=True, help="Colon-separated list of subdirectories to copy (e.g., 'lib:include').")
 
    args = parser.parse_args()

    src_root = args.src_root
    dest_root = args.dest_root
    patch_file = args.patch_file
    patch_target_folder = args.patch_target_folder
    dirs_to_copy = args.dirs_to_copy.split(':') # Split the colon-separated string

    # Ensure destination root directory exists and is clean
    if os.path.exists(dest_root):
        shutil.rmtree(dest_root)
    os.makedirs(dest_root, exist_ok=True)

    # 1. Copy relevant files/directories from src_root to dest_root
    print(f"Copying directories from {src_root} to {dest_root}...")
    try:
        for subdir in dirs_to_copy:
            src_path = os.path.join(src_root, subdir)
            dest_path = os.path.join(dest_root, subdir)
            if os.path.isdir(src_path):
                shutil.copytree(src_path, dest_path, dirs_exist_ok=True)
                print(f"  Copied {subdir}/")
            elif os.path.isfile(src_path): # Handle if it's a file path
                os.makedirs(os.path.dirname(dest_path), exist_ok=True)
                shutil.copy2(src_path, dest_path)
                print(f"  Copied {subdir}")
            else:
                print(f"Warning: {src_path} not found or not a directory/file to copy.", file=sys.stderr)

        print("All specified directories/files copied.")
    except Exception as e:
        print(f"Error copying files: {e}", file=sys.stderr)
        sys.exit(1)

    # 2. Apply the patch to the copied files in dest_root
    print(f"Applying patch {patch_file} to {patch_target_folder} ...")
    try:
        subprocess.run(
            ['git', 'apply', '-p1' , patch_file],
            check=True,
            cwd=patch_target_folder,
            capture_output=True # Capture output for debugging errors
        )
        print(f"Patch {patch_file} applied successfully to {patch_target_folder}.")
    except subprocess.CalledProcessError as e:
        print(f"Error applying patch {patch_file}: {e}", file=sys.stderr)
        print(f"Command: {' '.join(e.cmd)}")
        print(f"Stdout: {e.stdout.decode()}", file=sys.stderr)
        print(f"Stderr: {e.stderr.decode()}", file=sys.stderr)
        sys.exit(e.returncode)
    except FileNotFoundError as e:
        print(f"FileNotFoundError exception: {e}")
        sys.exit(1)
    except Exception as e:
        print(f"An unexpected error occurred: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == '__main__':
    main()    

