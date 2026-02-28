import pytest
from pitct.dat_info import DatInfo
from pitct.name_converter import NameConverter


@pytest.fixture(autouse=True)
def reset_nc():
    NameConverter.reset()
    yield
    NameConverter.reset()


def _make_dat_text(controllable: bool = True) -> str:
    """Construct a DatInfo-compatible text string.

    Format requirements (all 0-indexed after split('\\n')):
      - index 7: controllability line
      - index 12..-2: control data lines
      - last 2 elements (from trailing '\\n\\n') are empty strings
    """
    ctrl_line = "CONTROLLABLE" if controllable else "NOT CONTROLLABLE"
    return (
        "line0\nline1\nline2\nline3\nline4\nline5\nline6\n"
        f"{ctrl_line}\nline8\nline9\nline10\ncontrol data:\n"
        "0:  1   3\n"
        "1:  3   1\n"
        "\n"
    )


class TestDatInfoIsControllable:
    def test_controllable(self):
        dat = DatInfo(_make_dat_text(controllable=True), convert=False)
        assert dat.is_controllable is True

    def test_not_controllable(self):
        dat = DatInfo(_make_dat_text(controllable=False), convert=False)
        assert dat.is_controllable is False


class TestDatInfoControlData:
    def test_control_data_has_expected_states(self):
        dat = DatInfo(_make_dat_text(), convert=False)
        cd = dat.control_data
        assert 0 in cd
        assert 1 in cd

    def test_control_data_values_no_convert(self):
        dat = DatInfo(_make_dat_text(), convert=False)
        cd = dat.control_data
        assert cd[0] == [1, 3]
        assert cd[1] == [3, 1]

    def test_control_data_with_named_events(self):
        # Register event names so convert=True maps numbers to names
        NameConverter.event_encode_dict[1] = "go"
        NameConverter.event_encode_dict[3] = "stop"
        dat = DatInfo(_make_dat_text(), convert=True)
        cd = dat.control_data
        assert cd[0] == ["go", "stop"]
        assert cd[1] == ["stop", "go"]

    def test_empty_control_data_section(self):
        ctrl_line = "CONTROLLABLE"
        # Control data section has only empty lines (no state entries)
        text = (
            "l0\nl1\nl2\nl3\nl4\nl5\nl6\n"
            f"{ctrl_line}\nl8\nl9\nl10\ncontrol data:\n"
            "\n"
            "\n"
        )
        dat = DatInfo(text, convert=False)
        # No valid entries => control_data is empty
        assert dat.control_data == {}


class TestDatInfoText:
    def test_str_returns_string(self):
        dat = DatInfo(_make_dat_text(), convert=False)
        assert isinstance(str(dat), str)

    def test_repr_contains_classname(self):
        dat = DatInfo(_make_dat_text(), convert=False)
        r = repr(dat)
        assert "DatInfo" in r

    def test_repr_contains_is_controllable(self):
        dat = DatInfo(_make_dat_text(), convert=False)
        r = repr(dat)
        assert "is_controllable" in r

    def test_text_no_convert_matches_original(self):
        text = _make_dat_text()
        dat = DatInfo(text, convert=False)
        assert dat.text == text

    def test_text_with_convert_rebuilds_control_data(self):
        dat = DatInfo(_make_dat_text(), convert=True)
        text = dat.text
        assert "control data:" in text
