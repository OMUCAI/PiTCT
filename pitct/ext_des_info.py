from .des_info import DesInfo
from pitct.name_converter import NameConverter
from pitct.tct_typing import State

# Extend DES Information
class ExtDesInfo(DesInfo):
    """
    {
        0: {'reached': True, 'coreach': True, 'marked': True, 'next': [[1, 1], [15, 4]], 'vocal': 0},
        1: {'reached': True, 'coreach': True, 'marked': True, 'next': [[0, 0], [3, 2]], 'vocal': 0},
        2: {'reached': True, 'coreach': True, 'marked': False, 'next': [[0, 3], [5, 0]], 'vocal': 0},
        3: {'reached': True, 'coreach': True, 'marked': False, 'next': None, 'vocal': 0},
        4: {'reached': True, 'coreach': True, 'marked': False, 'next': None, 'vocal': 0}
    }
    """
    def is_reached(self, state: State) -> bool:
        state_num = NameConverter.state_encode(self.name, state, create=False)
        if state_num >= len(self._des_dict) or state_num < 0:
            raise RuntimeError("Out of index.")
        
        reached = self._des_dict[state_num]['reached']
        return reached

    def all_reached(self) -> bool:
        return all(info['reached'] for info in self._des_dict.values())

    def reached(self, convert: bool = True) -> list[State]:
        reached = []
        for state_num, info in self._des_dict.items():
            if info['reached']:
                state = NameConverter.state_decode(self.name, state_num, convert=convert)
                reached.append(state)
        return reached

    def is_coreach(self, state: State) -> bool:
        state_num = NameConverter.state_encode(self.name, state, create=False)
        if state_num >= len(self._des_dict) or state_num < 0:
            raise RuntimeError("Out of index.")
        
        coreach = self._des_dict[state_num]['coreach']
        return coreach

    def all_coreach(self) -> bool:
        return all(info['coreach'] for info in self._des_dict.values())

    def coreach(self, convert: bool = True) -> list[State]:
        coreach = []
        for state_num, info in self._des_dict.items():
            if info['coreach']:
                state = NameConverter.state_decode(self.name, state_num, convert=convert)
                coreach.append(state)
        return coreach


    


