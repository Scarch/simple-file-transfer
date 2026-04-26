- Sten-Egert Märtson
# A Simple C++ File Transfer Tool

Authors:
- Sten-Egert Märtson
- Merit Matt

## Setup

### Asio

- Download Asio 1.36 from https://sourceforge.net/projects/asio/files/asio/1.36.0%20%28Stable%29/
  - The relevant file is `asio-1.36.0.zip`
- Unzip the contents and move them to `lib/asio`
  - `lib/asio` should as a result contain directories like `lib/asio/include` and `lib/asio/src`

### Bulding the project

Run while in the root directory of this project:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
```

## How to use

Check available CLI arguments:

```bash
./build/simple-file-transfer --help
```

Run receiver (terminal 1):

```bash
./build/simple-file-transfer --ip 127.0.0.1 -p 8080 --receive --file ./received
```

Run sender (terminal 2):

```bash
./build/simple-file-transfer --ip 127.0.0.1 -p 8080 --send --file ./my_file.txt
```

Notes:
- `--ip`, `--port` (`-p`), and mode (`--send` or `--receive`) are required.
- `--file` is required in send mode.
- In receive mode, if `--file` is omitted, files are saved to the current directory.
