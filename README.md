# PyTCT
Python TCT binding Library

## Requirement
Python >= 3.7

## Build
1. install [poetry](https://python-poetry.org/) as build mananger 

Linux, macOS, Windows (WSL)
```bash
curl -sSL https://install.python-poetry.org | python3 -
```

Windows (Powershell)
```powershell
(Invoke-WebRequest -Uri https://install.python-poetry.org -UseBasicParsing).Content | py -
```

2. install dependency
```bash
poetry install
```

3. install plugin
```bash
poetry self add "poetry-dynamic-versioning[plugin]"
```

4. build PyTCT
```bash
poetry build
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
Please see [examples folder](./examples).


## Relate Informatiom
Graphviz Document  
https://graphviz.readthedocs.io/en/stable/index.html

Graphviz Sourcecode  
https://github.com/xflr6/graphviz

