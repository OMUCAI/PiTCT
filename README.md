# PiTCT
Python TCT binding Library

TCT is based on [this repository](https://github.com/TCT-Wonham/TCT)

## Requirement
Python >= 3.8

## How To Use

### Install

1. Install pitct library
```bash
pip install pitct
```

2. Install graphviz
- Mac
```bash
brew install graphviz
```

- Linux(Ubuntu)
```bash
apt install graphviz
```

- Windows  
Download graphviz installer from [here](https://graphviz.org/download/)

### How to Use

Please see [Documents](https://omucai.github.io/PyTCT-docs/).

## Relate Informatiom
Graphviz Document  
https://graphviz.readthedocs.io/en/stable/index.html

Graphviz Sourcecode  
https://github.com/xflr6/graphviz


----

## Build
1. (optional) create virtual environment
```
python -m venv venv
```

1-1. (when use virtual environment) Activate virtual environment
```
source venv/bin/activate
```

2. install dependency
```bash
pip install -e "."
pip install -e ".[dev]" 
```

3. build PiTCT
```bash
python -m build --wheel --sdist
```

PiTCT distributable file is generated in dist/ folder.

## PiTCT Install from wheel
1. copy pitct-***.whl
2. Install PiTCT
```bash
pip install pitct-****.whl
```
3. Install graphviz
- Mac
```bash
brew install graphviz
```

- Linux(Ubuntu)
```bash
apt install graphviz
```

- Windows  
Download graphviz installer from [here](https://graphviz.org/download/)

## License

This project uses multiple licenses due to the inclusion of third-party code. It is licensed under the Apache 2.0 License, with the exception of the content in the `libtct` directory.

- The python source code (`/pitct` directory and other root files) is licensed under the **Apache 2.0 License**. See [LICENSE](LICENSE) for more details.
- The tct source code (`/libtct` directory) is licensed under the **BSD 3-Clause License**. See [libtct/LICENSE](libtct/LICENSE) for more details.