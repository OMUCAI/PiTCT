from typing import List, Tuple, Union

from pytct.typing import TransList, State, Event

def get_key_from_value(d, val):
    matched = [k for k, v in d.items() if v == val]
    return matched[0]


class NameConverter:
    # state_encode_dict = {
    #     'SAMPLE': {
    #         0: 'aaaa',
    #         1: 'bbbb'
    #     }
    # }

    # event_encode_dict = {
    #     1: 'go',
    #     3: 'back'
    # }
    
    event_encode_dict = {}
    state_encode_dict = {}
    event_already_use = -1
    event_uncont_already_use = -2

    @classmethod
    def reset(cls):
        cls.event_encode_dict = {}
        cls.state_encode_dict = {}
        cls.event_already_use = -1
        cls.event_uncont_already_use = -2

    @classmethod
    def encode_all(cls, name: str, trans_list: TransList) -> TransList:
        cls.state_encode_dict[name] = {}
        
        encoded = []
        for s, e, ns, *uc in trans_list:
            state_num = cls.state_encode(name, s)
            if isinstance(e, int):
                # meaningless settings
                is_uncontrollable = False
            elif len(uc) == 0:
                raise RuntimeError("Please set 'u' or 'c'. example: (0, 'event', 1, 'c')")
            elif len(uc) == 1:
                str_uc = uc[0]
                if str_uc == 'u':
                    is_uncontrollable = True
                elif str_uc == 'c':
                    is_uncontrollable = False
                else:
                    raise RuntimeError("Unknown argument. Select 'u' or 'c'")
            else:
                raise RuntimeError("Unknown delta argument")
            event_num = cls.event_encode(name, e, is_uncontrollable)
            next_state_num = cls.state_encode(name, ns)
            encoded.append((state_num, event_num, next_state_num))
        return encoded

    @classmethod
    def event_encode(cls, name: str, event: Event, is_uncontrollable = False) -> int:
        if isinstance(event, str):
            if event in cls.event_encode_dict.values():
                # alredy register
                event_num = get_key_from_value(cls.event_encode_dict, event)
                calc_uncont = event_num % 2 == 0
                if is_uncontrollable != calc_uncont:
                    raise RuntimeError(f"Detect same name controllable and uncontrollable event (event: {event}). If you change c or u, please run pytct.init().")
                return event_num

            # calculate event number
            if is_uncontrollable:
                attach_num = cls.event_uncont_already_use + 2
                cls.event_uncont_already_use = attach_num
            else:
                attach_num = cls.event_already_use + 2
                cls.event_already_use = attach_num
            
            # register event num : event str mapping
            cls.event_encode_dict[attach_num] = event
            return attach_num
        else:
            return event

    @classmethod
    def state_encode(cls, name: str, state: State, create: bool = True) -> int:
        if isinstance(state, str):
            if not name in cls.state_encode_dict.keys():
                cls.state_encode_dict[name] = {}

            if state in cls.state_encode_dict[name].values():
                # alredy register
                return get_key_from_value(cls.state_encode_dict[name], state)
            elif create is False:
                raise RuntimeError(f"Undefined State: {state}")
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
        
        try:
            return cls.event_encode_dict[event]
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
