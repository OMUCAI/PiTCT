# PyTCT
Python TCT binding Library

## Requirement
Python >= 3.8

## Build
1. (optional) create virtual environment
```
python -m venv venv
```

2. install dependency
```bash
pip install -e "." ".[dev]" 
```

3. build PyTCT
```bash
python -m build --wheel --sdist
```

PyTCT distributable file is generated in dist/ folder.

## PyTCT Install
1. copy pytct-***.whl
2. Install PyTCT
```bash
pip install pytct-****.whl
```
3. Install graphviz(mac, linux)
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

## How To Use
Please see [Documents](https://omucai.github.io/PyTCT-docs/).


## Relate Informatiom
Graphviz Document  
https://graphviz.readthedocs.io/en/stable/index.html

Graphviz Sourcecode  
https://github.com/xflr6/graphviz

