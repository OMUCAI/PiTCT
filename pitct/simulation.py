from pitct.des import des_info
from pitct.name_converter import NameConverter
from pitct.tct_typing import Event, State
import random

def sample_automaton(name: str, length: int, strict: bool = True, convert: bool = True) -> list[State]:
    des = des_info(name)
    state = NameConverter.state_decode(name, 0, convert=convert)
    result = [state]
    for _ in range(length):
        next = des.next(state, convert=convert)
        if not next:
            if strict:
                raise RuntimeError(f"No path with length {length} was found. Current result: {result}, Current length: {len(result) - 1}")
            else:
                return result
        _, state = random.choice(next)
        result.append(state)
    return result


def simulate_automaton(name: str, event_string: list[Event], convert: bool = True) -> list[State]:
    des = des_info(name)
    current_state = NameConverter.state_decode(name, 0, convert=convert)
    result = [current_state]
    for e in event_string:
        next_state = des.next_state(current_state, e, convert=convert)
        if next_state is None:
            raise RuntimeError(f"The simulation could not be completed. Current result: {result}")
        result.append(next_state)
        current_state = next_state
    return result
