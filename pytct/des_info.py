from typing import Optional

class DesInfo:
    def __init__(self, des_states: dict) -> None:
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

    def next(self, _from: int) -> list:
        if _from >= len(self._des_dict) or _from < 0:
            raise RuntimeError("Out of index. ")

        next = self._des_dict[_from]['next']
        return next

    def next_state(self, _from: int, event: int) -> Optional[int]:
        next = self.next(_from)
        filter_next = list(filter(lambda x: x[0] == event, next))
        if not filter_next:
            return None
        next_state = filter_next[0][1]
        return next_state

    def marked(self) -> list:
        marked = []
        for state_num, info in self._des_dict.items():
            if info['marked']:
                marked.append(state_num)
        return marked

    def is_marked(self, state: int) -> bool:
        if state >= len(self._des_dict) or state < 0:
            raise RuntimeError("Out of index. ")
        
        marked = self._des_dict[state]['marked']
        return marked

    def trans(self) -> list:
        # get transition function \delta
        delta = []
        for state_num, info in self._des_dict.items():
            if info['next'] is None:
                continue
            for event, next_state_num in info['next']:
                d = (state_num, event, next_state_num)
                delta.append(d)
        return delta

    def events(self) -> list:
        events = set()
        for state_num, info in self._des_dict.items():
            if info['next'] is None:
                continue
            for event, _ in info['next']:
                events.add(event)
        return list(events)

    def __repr__(self) -> str:
        return self._des_dict.__repr__()

    def __iter__(self):
        yield from self._des_dict.values()

    def __getitem__(self, state: int):
        return self._des_dict[state]

    


