import subprocess
import sys
import os

def apply_patch(patch_file):
    """Applies a patch file to current folder."""
    try:
        # Check if the patch applies cleanly (without actually applying)
        # This will fail if the patch has already been applied exactly.
        patch_file_path = os.path.abspath(patch_file)
        subprocess.run(
            ['git', 'apply', '-p1', '--check', patch_file_path],
            check=True,
            capture_output=True, # Capture output to avoid printing it unless there's an error
            cwd=os.getcwd() # cwd is already set by DEPS
        )
        print(f"Applying patch: {patch_file_path} in {os.getcwd()}")
        
        # Apply the patch (if check passed)
        subprocess.run(
            ['git', 'apply', '-p1', patch_file_path],
            check=True,
            capture_output=True,
            cwd=os.getcwd()
        )
        print(f"Patch '{patch_file_path}' applied successfully to folder {os.getcwd()}.")

        # Stage the changes
        print(f"Staging changes from {patch_file_path}")
        subprocess.run(['git', 'add', '.'], check=True, cwd=os.getcwd())

        # Commit the changes locally
        print(f"Committing changes from {patch_file_path}")
        subprocess.run(
            ['git', 'commit', '-m', f'DEPS Hook: Applied {patch_file_path}'],
            check=True,
            cwd=os.getcwd()
        )
        print(f"Successfully applied and committed {patch_file_path}")
    except subprocess.CalledProcessError as e:
        if "patch does not apply" in e.stderr.decode():
             print(f"Patch {patch_file_path} already applied. Skipping.", file=sys.stderr)
             # Exit 0 so gclient sync continues, as the desired state is met.
             sys.exit(0)
        else:
            print(f"Error applying patch {patch_file_path}: {e}", file=sys.stderr)
            print(f"Command: {' '.join(e.cmd)}")
            print(f"Stdout: {e.stdout.decode()}", file=sys.stderr)
            print(f"Stderr: {e.stderr.decode()}", file=sys.stderr)
            sys.exit(e.returncode)
    except FileNotFoundError as e:
        print(f"ERROR:\n{e}")
        sys.exit(1)

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python3 apply_patch.py <patch_file_path>")
        sys.exit(1)
    apply_patch(sys.argv[1])