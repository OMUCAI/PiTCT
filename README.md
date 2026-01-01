# PiTCT

[![Latest PyPI version](https://img.shields.io/pypi/v/pitct?logo=pypi)](https://pypi.python.org/pypi/pitct)
[![Documentation Page Status](https://img.shields.io/github/actions/workflow/status/OMUCAI/PiTCT-docs/build-docs.yml?label=docs)](https://omucai.github.io/PiTCT-docs/)

The TCT software package is designed for the synthesis of supervisory controls for untimed discrete-event systems (DES).
PiTCT provides Python Binding of the TCT so that TCT can be used from Python.

TCT is based on [this repository](https://github.com/TCT-Wonham/TCT)

> [!WARNING]
> `PiTCT` was renamed when it was changed to OSS as v1, having previously been provided as `pytct`.  
> The migration documentation is [here](https://omucai.github.io/PiTCT-docs/migration/v0_to_v1/). 

## How To Use

For a quick tutorial and API reference, see the [documentation](https://omucai.github.io/PiTCT-docs/).

### Install

Requirements:

- Python 3.9+
- Graphviz (system package)

1. Install pitct library
    ```bash
    pip install pitct
    ```

2. Install graphviz
    PiTCT depends on [Graphviz](https://graphviz.org/).  
    Please install graphviz from:

    - [Windows](https://graphviz.org/download/#windows) 
    - [Mac](https://graphviz.org/download/#mac)
    - [Linux](https://graphviz.org/download/)

## License

This project uses multiple licenses due to the inclusion of third-party code. It is licensed under the Apache 2.0 License, with the exception of the content in the `libtct` directory.

- The python source code (`/pitct` directory and other root files) is licensed under the **Apache 2.0 License**. See [LICENSE](LICENSE) for more details.
- The tct source code (`/libtct` directory) is licensed under the **BSD 3-Clause License**. See [libtct/LICENSE](libtct/LICENSE) for more details.


## Development Information

### Build
1. (optional) create virtual environment
    ```
    python -m venv venv
    ```

2. (when using virtual environment) Activate virtual environment
    ```
    source venv/bin/activate
    ```

3. install dependencies
    ```bash
    pip install -e "."
    pip install -e ".[dev]" 
    ```

4. build PiTCT
    ```bash
    python -m build --wheel --sdist
    ```

    PiTCT distributable file is generated in dist/ folder.

### Related Information

- [Graphviz Documentation](https://graphviz.readthedocs.io/en/stable/index.html)
- [Graphviz Source Code](https://github.com/xflr6/graphviz)
