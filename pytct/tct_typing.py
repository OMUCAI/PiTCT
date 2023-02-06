from __future__ import annotations
from typing import Union, Tuple, List, Literal  # List, Tupleは3.9から非推奨

Event = Union[int, str]
State = Union[int, str]
CorUC = Literal['c', 'u']  # Controllable or Uncontrollable
Trans1 = Tuple[State, Event, State]
Trans2 = Tuple[State, Event, State, CorUC]
Trans = Union[Trans1, Trans2]
TransList = List[Trans]
StateList = List[State]