from typing import Optional
from pitct.name_converter import NameConverter
from pitct.tct_typing import State, Event, TransList
class DesInfo:
    def __init__(self, name: str, des_states: dict) -> None:
        self.name = name
        self._des_dict = des_states
        """
        {
            0: {'marked': True, 'next': [[1, 1], [15, 4]], 'vocal': 0},
            1: {'marked': True, 'next': [[0, 0], [3, 2]], 'vocal': 0},
            2: {'marked': False, 'next': [[0, 3], [5, 0]], 'vocal': 0},
            3: {'marked': False, 'next': None, 'vocal': 0},
            4: {'marked': False, 'next': None, 'vocal': 0}
        }
        """

    def next(self, state: State, convert: bool = True) -> Optional[list]:
        _from = NameConverter.state_encode(self.name, state, create=False)
        if _from >= len(self._des_dict) or _from < 0:
            raise RuntimeError("Out of index. ")

        _next = self._des_dict[_from]['next']
        if _next is None:
            return None
        else:
            result = []
            for (event_num, next_state_num) in _next:
                event = NameConverter.event_decode(event_num, convert=convert)
                next_state = NameConverter.state_decode(self.name, next_state_num, convert=convert)
                result.append([event, next_state])
            return result

    def next_state(self, state: State, event: Event, convert: bool = True) -> Optional[State]:
        event = NameConverter.event_encode(event, create=False)

        _next = self.next(state, convert=False)
        filter_next = list(filter(lambda x: x[0] == event, _next))
        if not filter_next:
            return None
        next_state = filter_next[0][1]
        next_state_conv = NameConverter.state_decode(self.name, next_state, convert=convert)
        return next_state_conv

    def marked(self, convert: bool = True) -> list[State]:
        marked = []
        for state_num, info in self._des_dict.items():
            if info['marked']:
                marked.append(state_num)
        result = [NameConverter.state_decode(self.name, state, convert=convert) for state in marked]
        return result

    def is_marked(self, state: State) -> bool:
        state = NameConverter.state_encode(self.name, state, create=False)
        if state >= len(self._des_dict) or state < 0:
            raise RuntimeError("Out of index. ")
        
        marked = self._des_dict[state]['marked']
        return marked

    def trans(self, convert: bool = True) -> TransList:
        # get transition function \delta
        delta = []
        for state_num, info in self._des_dict.items():
            if info['next'] is None:
                continue
            for event_num, next_state_num in info['next']:
                s = NameConverter.state_decode(self.name, state_num, convert=convert)
                e = NameConverter.event_decode(event_num, convert=convert)
                ns = NameConverter.state_decode(self.name, next_state_num, convert=convert)
                if type(e) == str:
                    c_or_u = NameConverter.get_controllable_or_uncontrollable(e)
                    d = (s, e, ns, c_or_u)
                    delta.append(d)
                else:
                    d = (s, e, ns)
                    delta.append(d)
        return delta

    def events(self, convert: bool = True) -> list[Event]:
        events = set()
        for state_num, info in self._des_dict.items():
            if info['next'] is None:
                continue
            for event_num, _ in info['next']:
                event = NameConverter.event_decode(event_num, convert=convert)
                events.add(event)
        return list(events)

    def __repr__(self) -> str:
        return self._des_dict.__repr__()

    def __iter__(self):
        yield from self._des_dict.values()

    def __getitem__(self, state: int):
        return self._des_dict[state]
