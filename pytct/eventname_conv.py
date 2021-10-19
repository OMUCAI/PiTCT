from typing import List, Tuple, Union

Event = Union[int, str]
Trans = Tuple[int, Event, int]
TransList = List[Trans]

class EventnameConv:
    # event_encode_dict = {
    #     'SAMPLE': {
    #         0: 'run',
    #         1: 'broken'
    #     }
    # }
    event_encode_dict = {}
    already_use = 0

    @classmethod
    def encode_all(cls, name: str, trans_list: TransList):
        encoded = []
        for s, e, ns in trans_list:
            event_num = cls.encode(name, e)
            encoded.append((s, event_num, ns))
        return encoded

    @classmethod
    def encode(cls, name: str, event: Event) -> int:
        if isinstance(event, str):
            attach_num = cls.already_use + 1
            cls.already_use = attach_num
            if name in cls.event_encode_dict.keys():
                cls.event_encode_dict[name][attach_num] = event
            else:
                cls.event_encode_dict[name] = {}
                cls.event_encode_dict[name][attach_num] = event
            return attach_num
        else:
            return event

    @classmethod
    def decode(cls, name: str, event: int) -> str:
        if not name in cls.event_encode_dict.keys():
            return str(event)
        
        conv = cls.event_encode_dict[name]
        try:
            return conv[event]
        except KeyError:
            return str(event)

    @classmethod
    def register(cls, name: str, *args):
        cls.event_encode_dict[name] = {}

        for mixed in args:
            if not mixed in cls.event_encode_dict.keys():
                continue
            tmp_dict = cls.event_encode_dict[mixed]
            cls.event_encode_dict[name].update(tmp_dict)
