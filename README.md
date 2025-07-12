# PiTCT
Python TCT binding Library

## Requirement
Python >= 3.8

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

## PiTCT Install
1. copy pitct-***.whl
2. Install PiTCT
```bash
pip install pitct-****.whl
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

