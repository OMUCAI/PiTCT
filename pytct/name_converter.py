from typing import List, Tuple, Union

Event = Union[int, str]
State = Union[int, str]
Trans1 = Tuple[State, Event, State]
Trans2 = Tuple[State, Event, State, bool]
Trans = Union[Trans1, Trans2]
TransList = List[Trans]

def get_key_from_value(d, val):
    matched = [k for k, v in d.items() if v == val]
    return matched[0]


class NameConverter:
    # event_encode_dict = {
    #     'SAMPLE': {
    #         0: 'run',
    #         1: 'broken'
    #     }
    # }
    event_encode_dict = {}
    state_encode_dict = {}
    event_already_use = -1
    event_uncont_already_use = -2

    @classmethod
    def encode_all(cls, name: str, trans_list: TransList):
        if name in cls.state_encode_dict.keys():
            # reset already exist
            cls.state_encode_dict[name] = {}

        encoded = []
        for s, e, ns, *uc in trans_list:
            state_num = cls._state_encode(name, s)
            if len(uc) == 0:
                is_uncontrollable = False
            else:
                is_uncontrollable = uc[0]
            event_num = cls._event_encode(name, e, is_uncontrollable)
            next_state_num = cls._state_encode(name, ns)
            encoded.append((state_num, event_num, next_state_num))
        return encoded

    @classmethod
    def _event_encode(cls, name: str, event: Event, is_uncontrollable = False) -> int:
        if isinstance(event, str):
            if not name in cls.event_encode_dict.keys():
                cls.event_encode_dict[name] = {}

            if event in cls.event_encode_dict[name].values():
                # alredy register
                return get_key_from_value(cls.event_encode_dict[name], event)

            # calculate event number
            if is_uncontrollable:
                attach_num = cls.event_uncont_already_use + 2
                cls.event_uncont_already_use = attach_num
            else:
                attach_num = cls.event_already_use + 2
                cls.event_already_use = attach_num
            
            # register event num : event str mapping
            cls.event_encode_dict[name][attach_num] = event
            return attach_num
        else:
            return event

    @classmethod
    def _state_encode(cls, name: str, state: State) -> int:
        if isinstance(state, str):
            if not name in cls.state_encode_dict.keys():
                cls.state_encode_dict[name] = {}

            if state in cls.state_encode_dict[name].values():
                # alredy register
                return get_key_from_value(cls.state_encode_dict[name], state)

            attach_num = len(cls.state_encode_dict[name].keys())

            # register state num : state str mapping
            cls.state_encode_dict[name][attach_num] = state
            return attach_num
        else:
            return state

    @classmethod
    def event_decode(cls, name: str, event: int, convert: bool = True) -> str:
        if not convert:
            return str(event)
        
        if not name in cls.event_encode_dict.keys():
            return str(event)
        
        conv = cls.event_encode_dict[name]
        try:
            return conv[event]
        except KeyError:
            return str(event)

    @classmethod
    def state_decode(cls, name: str, state: int, convert: bool = True) -> str:
        if not convert:
            return str(state)

        if not name in cls.state_encode_dict.keys():
            return str(state)
        
        conv = cls.state_encode_dict[name]
        try:
            return conv[state]
        except KeyError:
            return str(state)

    @classmethod
    def register(cls, name: str, *args):
        cls.event_encode_dict[name] = {}

        for mixed in args:
            if not mixed in cls.event_encode_dict.keys():
                continue
            tmp_dict = cls.event_encode_dict[mixed]
            cls.event_encode_dict[name].update(tmp_dict)
