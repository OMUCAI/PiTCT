import pytest
import umsgpack
from pathlib import Path
from unittest.mock import patch, MagicMock
from pitct.distance import (
    _dijkstra,
    _dijkstra_with_route,
    _create_edge,
    _load_states,
    min_distance,
    path_state_list,
    path_event_list,
)
from pitct.name_converter import NameConverter


@pytest.fixture(autouse=True)
def reset_nc():
    NameConverter.reset()
    yield
    NameConverter.reset()


# Test graph (integer node labels 0-3):
#   0 --(ev 0)--> 1
#   0 --(ev 1)--> 2
#   1 --(ev 2)--> 3
#   2 --(ev 3)--> 3
#   3: no outgoing edges
EDGES = [
    [[1, 0], [2, 1]],  # node 0: -> 1 via ev0, -> 2 via ev1
    [[3, 2]],          # node 1: -> 3 via ev2
    [[3, 3]],          # node 2: -> 3 via ev3
    [],                # node 3: no outgoing
]


class TestDijkstra:
    def test_start_node_distance_is_zero(self):
        node = _dijkstra(EDGES, 4)
        assert node[0] == 0

    def test_direct_neighbor_distance_is_1(self):
        node = _dijkstra(EDGES, 4)
        assert node[1] == 1
        assert node[2] == 1

    def test_two_hop_distance(self):
        node = _dijkstra(EDGES, 4)
        assert node[3] == 2

    def test_disconnected_node_is_inf(self):
        edges = [
            [[1, 0]],  # 0 -> 1
            [],        # 1: no outgoing
            [],        # 2: disconnected island
        ]
        node = _dijkstra(edges, 3)
        assert node[2] == float("inf")

    def test_single_node_graph(self):
        edges = [[]]
        node = _dijkstra(edges, 1)
        assert node[0] == 0

    def test_linear_chain(self):
        edges = [[[1, 0]], [[2, 0]], [[3, 0]], []]
        node = _dijkstra(edges, 4)
        assert node[0] == 0
        assert node[1] == 1
        assert node[2] == 2
        assert node[3] == 3

    def test_updates_r_in_inner_loop(self):
        # Graph: 0 -> 2 -> 1 (node 1 is not directly reachable from 0)
        # After processing 0, node_name=[1,2], node[1]=inf, node[2]=1.
        # r starts at node_name[0]=1 with infinite cost, then the inner
        # loop finds node 2 has cost 1 < inf, updating r=2 (covering line 25).
        edges = [
            [[2, 0]],   # 0 -> 2 via event 0
            [],         # 1: no outgoing
            [[1, 0]],   # 2 -> 1 via event 0
        ]
        node = _dijkstra(edges, 3)
        assert node[0] == 0
        assert node[2] == 1
        assert node[1] == 2


class TestDijkstraWithRoute:
    def test_path_from_0_to_3_starts_and_ends_correctly(self):
        path, node, event = _dijkstra_with_route(EDGES, 4, 0, 3)
        assert path[0] == 0
        assert path[-1] == 3

    def test_path_from_0_to_3_has_length_3(self):
        # Either 0->1->3 or 0->2->3, both length 3
        path, node, event = _dijkstra_with_route(EDGES, 4, 0, 3)
        assert len(path) == 3

    def test_start_equals_goal_returns_single_element_path(self):
        path, node, event = _dijkstra_with_route(EDGES, 4, 0, 0)
        assert path == [0]

    def test_unreachable_goal_returns_empty(self):
        edges = [
            [[1, 0]],
            [],
        ]
        path, node, event = _dijkstra_with_route(edges, 2, 1, 0)
        assert path == []
        assert node == []
        assert event == []

    def test_direct_neighbor(self):
        path, node, event = _dijkstra_with_route(EDGES, 4, 0, 1)
        assert path == [0, 1]

    def test_cost_array_length(self):
        path, node, event = _dijkstra_with_route(EDGES, 4, 0, 3)
        assert len(node) == 4

    def test_event_array_length(self):
        path, node, event = _dijkstra_with_route(EDGES, 4, 0, 3)
        assert len(event) == 4


class TestCreateEdge:
    def test_basic_conversion(self):
        states = {
            0: {"next": [[0, 1], [1, 2]], "marked": True, "vocal": 0},
            1: {"next": None, "marked": False, "vocal": 0},
        }
        edges = _create_edge(states)
        assert len(edges) == 2
        # next = [event_num, next_state_num]
        # edge entry = [next_state_num, event_num]
        assert edges[0] == [[1, 0], [2, 1]]
        assert edges[1] == []

    def test_none_next_becomes_empty_list(self):
        states = {0: {"next": None, "marked": True, "vocal": 0}}
        edges = _create_edge(states)
        assert edges == [[]]

    def test_multiple_states(self):
        states = {
            0: {"next": [[2, 1]], "marked": True, "vocal": 0},
            1: {"next": [[0, 3]], "marked": False, "vocal": 0},
            2: {"next": None, "marked": False, "vocal": 0},
        }
        edges = _create_edge(states)
        assert len(edges) == 3
        assert edges[0] == [[1, 2]]
        assert edges[1] == [[3, 0]]
        assert edges[2] == []


# === Integration tests for higher-level functions (mock file I/O) ===


SAMPLE_STATES = {
    0: {"marked": True, "next": [[1, 1]], "vocal": 0},
    1: {"marked": True, "next": [[0, 0]], "vocal": 0},
    2: {"marked": False, "next": None, "vocal": 0},
}


class TestMinDistance:
    def test_returns_dict(self):
        with patch("pitct.distance._load_states", return_value=SAMPLE_STATES):
            result = min_distance("PLANT", convert=False)
        assert isinstance(result, dict)

    def test_marked_states_have_distance(self):
        with patch("pitct.distance._load_states", return_value=SAMPLE_STATES):
            result = min_distance("PLANT", convert=False)
        assert 0 in result
        assert 1 in result

    def test_non_marked_states_excluded(self):
        with patch("pitct.distance._load_states", return_value=SAMPLE_STATES):
            result = min_distance("PLANT", convert=False)
        assert 2 not in result

    def test_zero_origin_distance(self):
        # State 0 is start, also marked -> distance from start to itself is 0
        with patch("pitct.distance._load_states", return_value=SAMPLE_STATES):
            result = min_distance("PLANT", convert=False)
        assert result[0] == 0


class TestPathStateList:
    def test_returns_list(self):
        with patch("pitct.distance._load_states", return_value=SAMPLE_STATES):
            result = path_state_list("PLANT", 0, 1, convert=False)
        assert isinstance(result, list)

    def test_path_starts_at_start(self):
        with patch("pitct.distance._load_states", return_value=SAMPLE_STATES):
            result = path_state_list("PLANT", 0, 1, convert=False)
        assert result[0] == 0

    def test_path_ends_at_goal(self):
        with patch("pitct.distance._load_states", return_value=SAMPLE_STATES):
            result = path_state_list("PLANT", 0, 1, convert=False)
        assert result[-1] == 1

    def test_same_start_and_goal(self):
        with patch("pitct.distance._load_states", return_value=SAMPLE_STATES):
            result = path_state_list("PLANT", 0, 0, convert=False)
        assert result == [0]


class TestPathEventList:
    def test_returns_list(self):
        with patch("pitct.distance._load_states", return_value=SAMPLE_STATES):
            result = path_event_list("PLANT", 0, 1, convert=False)
        assert isinstance(result, list)

    def test_event_list_length_one_for_direct_neighbor(self):
        # 0 -> 1 in one hop
        with patch("pitct.distance._load_states", return_value=SAMPLE_STATES):
            result = path_event_list("PLANT", 0, 1, convert=False)
        assert len(result) == 1

    def test_same_start_and_goal_empty_list(self):
        with patch("pitct.distance._load_states", return_value=SAMPLE_STATES):
            result = path_event_list("PLANT", 0, 0, convert=False)
        assert result == []


class TestLoadStates:
    def test_returns_states_dict(self, tmp_path):
        # Build a msgpack-encoded payload matching what _load_states expects
        expected_states = {
            0: {"marked": True, "next": None, "vocal": 0},
        }
        payload = umsgpack.packb({"states": expected_states})

        from pitct.config import Config
        conf = Config.get_instance()
        original = conf.SAVE_FOLDER
        try:
            conf.SAVE_FOLDER = tmp_path
            (tmp_path / "PLANT.DES").write_bytes(payload)
            result = _load_states("PLANT")
        finally:
            conf.SAVE_FOLDER = original

        assert result == expected_states

    def test_returns_correct_state_data(self, tmp_path):
        states = {
            0: {"marked": True, "next": [[1, 1]], "vocal": 0},
            1: {"marked": False, "next": None, "vocal": 0},
        }
        payload = umsgpack.packb({"states": states})

        from pitct.config import Config
        conf = Config.get_instance()
        original = conf.SAVE_FOLDER
        try:
            conf.SAVE_FOLDER = tmp_path
            (tmp_path / "AUTOMATON.DES").write_bytes(payload)
            result = _load_states("AUTOMATON")
        finally:
            conf.SAVE_FOLDER = original

        assert 0 in result
        assert 1 in result
        assert result[0]["marked"] is True
        assert result[1]["next"] is None
