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

    def __repr__(self) -> str:
        return self._des_dict.__repr__()
    


