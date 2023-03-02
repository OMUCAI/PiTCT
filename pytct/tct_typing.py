from __future__ import annotations
from typing import Union, Literal  # List, Tupleは3.9から非推奨

Event = Union[int, str]
State = Union[int, str]
CorUC = Literal['c', 'u']  # Controllable or Uncontrollable
Trans1 = tuple[State, Event, State]
Trans2 = tuple[State, Event, State, CorUC]
Trans = Union[Trans1, Trans2]
TransList = list[Trans]
StateList = list[State]
EventList = list[Event]