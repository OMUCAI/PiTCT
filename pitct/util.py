import sys

def is_env_notebook():
    """Determine whether is the environment Jupyter Notebook"""
    try:
        shell = get_ipython().__class__.__name__
        if shell == 'ZMQInteractiveShell':
            return True   # Jupyter notebook or qtconsole
        elif shell == 'TerminalInteractiveShell':
            return False  # Terminal running IPython
        else:
            return False  # Other type (?)
    except NameError:
        return False      # Probably standard Python interpreter
    
def is_env_jupyterlite():
    """Determine whether the environment is JupyterLite (Pyodide/Emscripten)"""
    if sys.platform == "emscripten":
        try:
            shell = get_ipython().__class__.__name__
            return shell == 'Interpreter'
        except NameError:
            return False
    return False
