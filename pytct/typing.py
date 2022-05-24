from __future__ import annotations
from typing import Union


Event = Union[int, str]
State = Union[int, str]
Trans1 = tuple[State, Event, State]
Trans2 = tuple[State, Event, State, bool]
Trans = Union[Trans1, Trans2]
TransList = list[Trans]
StateList = list[State]