import pytest
from pitct.name_converter import NameConverter, get_key_from_value, check_mixed


@pytest.fixture(autouse=True)
def reset_nc():
    NameConverter.reset()
    yield
    NameConverter.reset()


class TestGetKeyFromValue:
    def test_basic_string_value(self):
        d = {0: "a", 1: "b"}
        assert get_key_from_value(d, "a") == 0
        assert get_key_from_value(d, "b") == 1

    def test_integer_value(self):
        d = {5: 100}
        assert get_key_from_value(d, 100) == 5


class TestCheckMixed:
    def test_all_string_ok(self):
        trans = [("a", "go", "b", "c"), ("b", "stop", "a", "u")]
        check_mixed(trans, str)  # should not raise

    def test_all_int_ok(self):
        trans = [(0, 1, 1), (1, 2, 0)]
        check_mixed(trans, int)  # should not raise

    def test_mixed_state_int_str_raises(self):
        # state 0 is int, next_state "b" is str => mixed up
        trans = [(0, "go", "b", "c")]
        with pytest.raises(RuntimeError, match="string and int states"):
            check_mixed(trans, str)

    def test_mixed_event_raises(self):
        # event 1 is int, event_type expected str
        trans = [("a", 1, "b")]
        with pytest.raises(RuntimeError, match="string and int event"):
            check_mixed(trans, str)

    def test_mixed_next_state_raises(self):
        # state "a" is str, next_state 1 is int
        trans = [("a", "go", 1, "c")]
        with pytest.raises(RuntimeError, match="string and int states"):
            check_mixed(trans, str)

    def test_mixed_source_state_across_transitions_raises(self):
        # First transition has int source state; second has str source state.
        # This triggers the `if type(s) != state_type` branch.
        trans = [(0, "go", 0), ("bad", "stay", 0)]
        with pytest.raises(RuntimeError, match="string and int states"):
            check_mixed(trans, str)


class TestNameConverterReset:
    def test_reset_clears_event_dict(self):
        NameConverter.event_encode("go", is_uncontrollable=False, create=True)
        NameConverter.reset()
        assert NameConverter.event_encode_dict == {}

    def test_reset_clears_state_dict(self):
        NameConverter.state_encode("PLANT", "idle")
        NameConverter.reset()
        assert NameConverter.state_encode_dict == {}

    def test_reset_resets_counters(self):
        NameConverter.event_encode("go", is_uncontrollable=False, create=True)
        NameConverter.reset()
        # After reset, next controllable event should again be 1
        num = NameConverter.event_encode("start", is_uncontrollable=False, create=True)
        assert num == 1


class TestEventEncode:
    def test_first_controllable_event_is_1(self):
        num = NameConverter.event_encode("go", is_uncontrollable=False, create=True)
        assert num == 1

    def test_first_uncontrollable_event_is_0(self):
        num = NameConverter.event_encode("fail", is_uncontrollable=True, create=True)
        assert num == 0

    def test_same_event_twice_returns_same_num(self):
        n1 = NameConverter.event_encode("go", is_uncontrollable=False, create=True)
        n2 = NameConverter.event_encode("go", is_uncontrollable=False, create=True)
        assert n1 == n2

    def test_int_event_returns_as_is(self):
        assert NameConverter.event_encode(5) == 5
        assert NameConverter.event_encode(0, is_uncontrollable=True) == 0

    def test_create_false_undefined_raises(self):
        with pytest.raises(RuntimeError, match="Undefined Event"):
            NameConverter.event_encode("unknown", create=False)

    def test_conflict_controllable_uncontrollable_raises(self):
        NameConverter.event_encode("go", is_uncontrollable=False, create=True)
        with pytest.raises(RuntimeError):
            NameConverter.event_encode("go", is_uncontrollable=True, create=True)

    def test_multiple_controllable_events_are_odd(self):
        n1 = NameConverter.event_encode("start", is_uncontrollable=False)
        n2 = NameConverter.event_encode("stop", is_uncontrollable=False)
        assert n1 == 1
        assert n2 == 3

    def test_multiple_uncontrollable_events_are_even(self):
        n1 = NameConverter.event_encode("fail1", is_uncontrollable=True)
        n2 = NameConverter.event_encode("fail2", is_uncontrollable=True)
        assert n1 == 0
        assert n2 == 2

    def test_interleaved_c_uc_events(self):
        c1 = NameConverter.event_encode("c_ev1", is_uncontrollable=False)  # 1
        u1 = NameConverter.event_encode("u_ev1", is_uncontrollable=True)   # 0
        c2 = NameConverter.event_encode("c_ev2", is_uncontrollable=False)  # 3
        u2 = NameConverter.event_encode("u_ev2", is_uncontrollable=True)   # 2
        assert c1 % 2 == 1
        assert u1 % 2 == 0
        assert c2 % 2 == 1
        assert u2 % 2 == 0

    def test_event_stored_in_dict(self):
        num = NameConverter.event_encode("go", is_uncontrollable=False)
        assert NameConverter.event_encode_dict[num] == "go"


class TestStateEncode:
    def test_first_string_state_is_0(self):
        num = NameConverter.state_encode("PLANT", "idle")
        assert num == 0

    def test_same_state_twice_returns_same_num(self):
        n1 = NameConverter.state_encode("PLANT", "idle")
        n2 = NameConverter.state_encode("PLANT", "idle")
        assert n1 == n2

    def test_sequential_state_numbers(self):
        n0 = NameConverter.state_encode("PLANT", "idle")
        n1 = NameConverter.state_encode("PLANT", "running")
        n2 = NameConverter.state_encode("PLANT", "error")
        assert n0 == 0
        assert n1 == 1
        assert n2 == 2

    def test_int_state_returns_as_is(self):
        assert NameConverter.state_encode("PLANT", 0) == 0
        assert NameConverter.state_encode("PLANT", 5) == 5

    def test_different_automata_are_independent(self):
        n1 = NameConverter.state_encode("A", "x")
        n2 = NameConverter.state_encode("B", "x")
        assert n1 == 0
        assert n2 == 0

    def test_create_false_undefined_raises(self):
        with pytest.raises(RuntimeError, match="Undefined State"):
            NameConverter.state_encode("PLANT", "nonexistent", create=False)

    def test_create_false_with_existing_state_ok(self):
        NameConverter.state_encode("PLANT", "idle")
        num = NameConverter.state_encode("PLANT", "idle", create=False)
        assert num == 0

    def test_auto_creates_dict_for_new_automaton(self):
        NameConverter.state_encode("NEW_AUTO", "s0")
        assert "NEW_AUTO" in NameConverter.state_encode_dict


class TestEventDecode:
    def test_decode_registered_event(self):
        NameConverter.event_encode("go", is_uncontrollable=False)
        result = NameConverter.event_decode(1, convert=True)
        assert result == "go"

    def test_decode_unregistered_returns_int(self):
        result = NameConverter.event_decode(99, convert=True)
        assert result == 99

    def test_convert_false_returns_int(self):
        NameConverter.event_encode("go", is_uncontrollable=False)
        result = NameConverter.event_decode(1, convert=False)
        assert result == 1


class TestStateDecode:
    def test_decode_registered_state(self):
        NameConverter.state_encode("PLANT", "idle")
        result = NameConverter.state_decode("PLANT", 0, convert=True)
        assert result == "idle"

    def test_decode_unknown_automaton_returns_int(self):
        result = NameConverter.state_decode("UNKNOWN", 5, convert=True)
        assert result == 5

    def test_convert_false_returns_int(self):
        NameConverter.state_encode("PLANT", "idle")
        result = NameConverter.state_decode("PLANT", 0, convert=False)
        assert result == 0

    def test_decode_out_of_range_returns_int(self):
        NameConverter.state_encode("PLANT", "idle")
        result = NameConverter.state_decode("PLANT", 99, convert=True)
        assert result == 99


class TestGetControllableOrUncontrollable:
    def test_odd_int_is_controllable(self):
        assert NameConverter.get_controllable_or_uncontrollable(1) == "c"
        assert NameConverter.get_controllable_or_uncontrollable(3) == "c"
        assert NameConverter.get_controllable_or_uncontrollable(15) == "c"

    def test_even_int_is_uncontrollable(self):
        assert NameConverter.get_controllable_or_uncontrollable(0) == "u"
        assert NameConverter.get_controllable_or_uncontrollable(2) == "u"

    def test_registered_controllable_string_event(self):
        NameConverter.event_encode("go", is_uncontrollable=False)
        assert NameConverter.get_controllable_or_uncontrollable("go") == "c"

    def test_registered_uncontrollable_string_event(self):
        NameConverter.event_encode("fail", is_uncontrollable=True)
        assert NameConverter.get_controllable_or_uncontrollable("fail") == "u"

    def test_unknown_string_raises(self):
        with pytest.raises(RuntimeError, match="unknown event"):
            NameConverter.get_controllable_or_uncontrollable("unknown")


class TestEncodeAll:
    def test_basic_string_events_controllable(self):
        trans = [("idle", "go", "running", "c"), ("running", "stop", "idle", "c")]
        encoded = NameConverter.encode_all("PLANT", trans)
        assert len(encoded) == 2
        for t in encoded:
            assert len(t) == 3
            assert all(isinstance(v, int) for v in t)

    def test_int_events_passthrough(self):
        trans = [(0, 1, 1), (1, 2, 0)]
        encoded = NameConverter.encode_all("PLANT", trans)
        assert encoded == [(0, 1, 1), (1, 2, 0)]

    def test_uncontrollable_event_gets_even_number(self):
        trans = [("idle", "fault", "error", "u")]
        encoded = NameConverter.encode_all("PLANT", trans)
        event_num = encoded[0][1]
        assert event_num % 2 == 0

    def test_controllable_event_gets_odd_number(self):
        trans = [("idle", "go", "running", "c")]
        encoded = NameConverter.encode_all("PLANT", trans)
        event_num = encoded[0][1]
        assert event_num % 2 == 1

    def test_no_uc_label_warns(self):
        trans = [("idle", "go", "running")]
        with pytest.warns(UserWarning):
            NameConverter.encode_all("PLANT", trans)

    def test_invalid_uc_label_raises(self):
        trans = [("idle", "go", "running", "x")]
        with pytest.raises(RuntimeError, match="Unknown argument"):
            NameConverter.encode_all("PLANT", trans)

    def test_extra_fields_in_trans_raise(self):
        trans = [("idle", "go", "running", "c", "extra")]
        with pytest.raises(RuntimeError, match="Unknown delta argument"):
            NameConverter.encode_all("PLANT", trans)

    def test_mixed_int_str_states_raises(self):
        # state 0 is int, next_state "b" is str
        trans = [(0, "go", "b", "c")]
        with pytest.raises(RuntimeError):
            NameConverter.encode_all("PLANT", trans)

    def test_state_dict_created_for_automaton(self):
        trans = [("idle", "go", "running", "c")]
        NameConverter.encode_all("AUTO", trans)
        assert "AUTO" in NameConverter.state_encode_dict
