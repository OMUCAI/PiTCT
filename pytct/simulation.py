from pytct.des import des_info
import random
import warnings

def sample_automaton(name: str, length: int, strict: bool = True) -> list:
    des = des_info(name)
    state = 0
    result = [state]
    for _ in range(length):
        next = des.next(state)
        if not next:
            if strict:
                raise RuntimeError(f"No path with length {length} was found. Current result: {result}, Current length: {len(result) - 1}")
            else:
                return result
        _, state = random.choice(next)
        result.append(state)
    return result


def simulate_automaton(name: str, event_string: list) -> list:
    des = des_info(name)
    current_state = 0
    result = [current_state]
    for e in event_string:
        next_state = des.next_state(current_state, e)
        if next_state is None:
            raise RuntimeError(f"The simulation could not be completed. Current result: {result}")
        result.append(next_state)
        current_state = next_state
    return result
