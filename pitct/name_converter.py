from pitct.tct_typing import TransList, State, Event, CorUC
import warnings

def get_key_from_value(d, val):
    # get key from dict value.
    # example: dict = { 0: '111' } 
    # 0 = get_key_from_value(dict, '111')

    matched = [k for k, v in d.items() if v == val]
    return matched[0]

def check_mixed(trans_list: TransList, event_type: type):
    # Check string or integer state/event are mixed up.
    state, *_ = trans_list[0]
    state_type = type(state)

    for s, e, ns, *uc in trans_list:
        if type(s) != state_type:
            raise RuntimeError("Discovered that the string and int states are mixed up. Please unify them.")
        elif type(e) != event_type:
            raise RuntimeError("Discovered that the string and int event are mixed up. Please unify them.")
        elif type(ns) != state_type:
            raise RuntimeError("Discovered that the string and int states are mixed up. Please unify them.")


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
    event_type = type(object)  # use as event type checking

    @classmethod
    def reset(cls):
        cls.event_encode_dict = {}
        cls.state_encode_dict = {}
        cls.event_already_use = -1
        cls.event_uncont_already_use = -2
        cls.event_type = type(object)

    @classmethod
    def encode_all(cls, name: str, trans_list: TransList) -> TransList:
        cls.state_encode_dict[name] = {}
        encoded = []

        # set event type (first only)
        if cls.event_type == type(object):
            cls.event_type = type(trans_list[0][1])

        check_mixed(trans_list, cls.event_type)

        for s, e, ns, *uc in trans_list:
            state_num = cls.state_encode(name, s)
            if isinstance(e, int):
                # meaningless settings
                is_uncontrollable = False
            elif len(uc) == 0:
                warnings.warn("controllable and uncontrollable status of event is not specified. All events are set to be controllable.")
                is_uncontrollable = False
                # raise RuntimeError("Please set 'u' or 'c'. example: (0, 'event', 1, 'c')")
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
            event_num = cls.event_encode(e, is_uncontrollable, create=True)
            next_state_num = cls.state_encode(name, ns)
            encoded.append((state_num, event_num, next_state_num))
        return encoded

    @classmethod
    def event_encode(cls, event: Event, is_uncontrollable = False, create: bool = True) -> int:
        if isinstance(event, str):
            if event in cls.event_encode_dict.values():
                # alredy register
                event_num = get_key_from_value(cls.event_encode_dict, event)
                calc_uncont = event_num % 2 == 0
                if is_uncontrollable != calc_uncont and create:
                    raise RuntimeError(f"Detect same name controllable and uncontrollable event (event: {event}). If you change c or u, please run pitct.init().")
                return event_num
            elif create is False:
                # readonly mode
                raise RuntimeError(f"Undefined Event: {event}")


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
    def event_decode(cls, event: int, convert: bool = True) -> Event:
        if not convert:
            return event
        
        try:
            return cls.event_encode_dict[event]
        except KeyError:
            return event

    @classmethod
    def state_decode(cls, name: str, state: int, convert: bool = True) -> State:
        if not convert:
            return state

        if not name in cls.state_encode_dict.keys():
            return state
        
        conv = cls.state_encode_dict[name]
        try:
            return conv[state]
        except KeyError:
            return state

    @classmethod
    def get_controllable_or_uncontrollable(cls, event: Event) -> CorUC:
        if type(event) == int:
            if event % 2 == 0:
                return "u"
            else:
                return "c"
        if event in cls.event_encode_dict.values():
            # alredy register
            event_num = get_key_from_value(cls.event_encode_dict, event)
            if event_num % 2 == 0:
                return "u"
            else:
                return "c"
        else:
            raise RuntimeError(f"unknown event. cannot detect 'c' or 'u'. input: {event}")