import sys
from unittest.mock import patch
from pitct.util import is_env_notebook, is_env_jupyterlite


def _make_shell(name: str):
    """Create a dummy shell instance whose __class__.__name__ == name."""
    return type(name, (), {})()


class TestIsEnvNotebook:
    def test_returns_false_in_standard_python(self):
        # get_ipython() is not defined in standard Python
        result = is_env_notebook()
        assert result is False

    def test_returns_true_in_zmq_shell(self):
        # Simulate Jupyter notebook: get_ipython().__class__.__name__ == 'ZMQInteractiveShell'
        with patch("pitct.util.get_ipython", return_value=_make_shell("ZMQInteractiveShell"), create=True):
            result = is_env_notebook()
        assert result is True

    def test_returns_false_in_terminal_ipython(self):
        # Simulate terminal IPython: get_ipython().__class__.__name__ == 'TerminalInteractiveShell'
        with patch("pitct.util.get_ipython", return_value=_make_shell("TerminalInteractiveShell"), create=True):
            result = is_env_notebook()
        assert result is False

    def test_returns_false_for_unknown_shell(self):
        # Simulate unknown shell type
        with patch("pitct.util.get_ipython", return_value=_make_shell("OtherShell"), create=True):
            result = is_env_notebook()
        assert result is False


class TestIsEnvJupyterlite:
    def test_returns_false_in_standard_python(self):
        # sys.platform != 'emscripten' in standard Python
        result = is_env_jupyterlite()
        assert result is False

    def test_returns_true_in_emscripten_with_interpreter(self):
        # Simulate JupyterLite: sys.platform == 'emscripten' and shell is 'Interpreter'
        with patch.object(sys, "platform", "emscripten"):
            with patch("pitct.util.get_ipython", return_value=_make_shell("Interpreter"), create=True):
                result = is_env_jupyterlite()
        assert result is True

    def test_returns_false_in_emscripten_without_get_ipython(self):
        # sys.platform == 'emscripten' but get_ipython raises NameError
        with patch.object(sys, "platform", "emscripten"):
            with patch("pitct.util.get_ipython", side_effect=NameError, create=True):
                result = is_env_jupyterlite()
        assert result is False

    def test_returns_false_in_emscripten_with_non_interpreter_shell(self):
        # sys.platform == 'emscripten' but shell is not 'Interpreter'
        with patch.object(sys, "platform", "emscripten"):
            with patch("pitct.util.get_ipython", return_value=_make_shell("OtherShell"), create=True):
                result = is_env_jupyterlite()
        assert result is False
