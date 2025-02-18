import sys

def main():
    output_file = sys.argv[sys.argv.index("--output") + 1]
    input_files = sys.argv[sys.argv.index("--output") + 2:]

    with open(output_file, "w") as out:
        for file in input_files:
            with open(file, "r") as f:
                out.write(f.read())
                out.write("\n")

if __name__ == "__main__":
    main()
