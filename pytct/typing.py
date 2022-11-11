from __future__ import annotations
from typing import Union, Tuple, List  # List, Tupleは3.9から非推奨
try:
    from typing import Literal
except ImportError:
    # python3.7
    from typing_extensions import Literal

Event = Union[int, str]
State = Union[int, str]
CorUC = Literal['c', 'u']  # Controllable or Uncontrollable
Trans1 = Tuple[State, Event, State]
Trans2 = Tuple[State, Event, State, CorUC]
Trans = Union[Trans1, Trans2]
TransList = List[Trans]
StateList = List[State]