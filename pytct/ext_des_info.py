from .des_info import DesInfo

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
    def is_reached(self, state_num: int) -> bool:
        if state_num >= len(self._des_dict) or state_num < 0:
            raise RuntimeError("Out of index.")
        
        reached = self._des_dict[state_num]['reached']
        return reached

    def all_reached(self) -> bool:
        return all(info['reached'] for info in self._des_dict.values())

    def reached(self) -> list:
        reached = []
        for state_num, info in self._des_dict.items():
            if info['reached']:
                reached.append(state_num)
        return reached

    def is_coreach(self, state_num: int) -> bool:
        if state_num >= len(self._des_dict) or state_num < 0:
            raise RuntimeError("Out of index.")
        
        coreach = self._des_dict[state_num]['coreach']
        return coreach

    def all_coreach(self) -> bool:
        return all(info['coreach'] for info in self._des_dict.values())

    def coreach(self) -> list:
        coreach = []
        for state_num, info in self._des_dict.items():
            if info['coreach']:
                coreach.append(state_num)
        return coreach


    


