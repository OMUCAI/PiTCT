from pathlib import Path
import random
import umsgpack
from pitct.automaton_display import AutomatonDisplay
from pitct.dat_info import DatInfo
from pitct.des_info import DesInfo
from pitct.ext_des_info import ExtDesInfo
from pitct.distance import path_event_list
from typing import Optional

from pitct.name_converter import NameConverter
from pitct.tct_typing import State, Event, TransList, StateList, EventList

from .libtct import call_program as __call

from .config import DAT_FILE_EXTENSION, DES_FILE_EXTENSION, RST_FILE_EXTENSION, EDES_FILE_EXTENSION, TXT_FILE_EXTENSION
from .config import Config
from .des_check import gen_prm, del_prm, check_exist, check_ret_code, get_path, check_state_num

conf = Config.get_instance()

def init(name: str, overwrite: bool = False):
    p = Path(name)
    if not overwrite and p.exists():
        raise FileExistsError(f"Directory {name} is already exists. If you can overwrite, "
                              f"please set overwrite arg.\nsample: init('{name}', overwrite=True)")
    # create directory.
    p.mkdir(parents=True, exist_ok=True)
    NameConverter.reset()
    
    conf.SAVE_FOLDER = p

def create(name: str, size: int, trans: TransList, marker: StateList):
    """Create

    Create an automaton model of DES

    Args:
        name (str): DES model name.
        size (int): number of states.
        trans (list): transition tuple list. [(state, event, next_state), (...)]
        marker (list): marker states list.

    Raises:
        RuntimeError: Cannot create .DES file.

    Examples:
        >>> delta = [(0,11,1),
                     (1,10,0),
                     (1,12,2),
                     (2,14,3),
                     (2,13,0),
                     (0,15,4)]
        >>> Qm = [0,1]
        >>> Q = 5
        >>> create("TEST", Q, delta, Qm)
    """
    prm_filename = "create_%s.prm" % name

    if not trans == []:
        conv_trans = NameConverter.encode_all(name, trans)
        check_state_num(conv_trans, size)
        trans_list = ["%d %d %d" % ent for ent in conv_trans]
    else:  # even if 'trans' is empty, want 'display_automaton' to work well
        trans_list = []

    marker_list = [f"{NameConverter.state_encode(name, mark, create=False)}" for mark in marker]
    marker_list.append("-1")

    prm_string = "{des_name}\n{state_num}\n{marker_states}\n{transitions}\n"
    prm_string = prm_string.format(
        des_name=get_path(name),
        state_num=size,
        marker_states=" ".join(marker_list),
        transitions="\n".join(trans_list),
    )

    prm_path = gen_prm(prm_filename, prm_string)
    ret_code = __call(0, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)


def selfloop(new_name: str, plant_name: str, event_list: EventList):
    check_exist(plant_name + DES_FILE_EXTENSION)
    prm_filename = "selfloop_%s.prm" % plant_name

    selfloop_list = [f"{NameConverter.event_encode(event, create=False)}" for event in event_list]

    prm_string = "{name1}\n{name2}\n{ls}\n".format(
        name1=get_path(plant_name),
        name2=get_path(new_name),
        ls="\n".join(selfloop_list)
    )

    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(1, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)


def trim(new_name: str, plant_name: str):
    check_exist(plant_name + DES_FILE_EXTENSION)

    prm_filename = "trim_%s.prm" % plant_name

    prm_string = "{name1}\n{name2}\n".format(
        name1=get_path(plant_name),
        name2=get_path(new_name)
    )
    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(2, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)


def printdes(new_name: str, plant_name: str):
    check_exist(plant_name + DES_FILE_EXTENSION)

    prm_filename = "print_%s.prm" % plant_name

    prm_string = "{name1}\n{name2}\n".format(
        name1=get_path(plant_name),
        name2=get_path(new_name)
    )
    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(3, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)


def sync(name: str, *plant_names: str, table: bool = False, convert: bool = False) -> Optional[str]:
    for plant_name in plant_names:
        check_exist(plant_name + DES_FILE_EXTENSION)

    is_enhanced_mode = table or convert

    prm_filename = "sync_%s.prm" % name
    plant_names_with_path = list(map(lambda x: get_path(x), plant_names))
    if is_enhanced_mode:
        # enhanced sync
        prm_string = "{name1}\n{out_name}\n{num}\n{names}\n".format(
            name1=get_path(name),
            out_name=get_path(name),
            num=len(plant_names),
            names="\n".join(plant_names_with_path)
        )
        prm_path = gen_prm(prm_filename, prm_string)

        ret_code = __call(31, prm_path)
        check_ret_code(ret_code)
        del_prm(prm_filename)

        # read output textfile
        with open(conf.SAVE_FOLDER / (name + TXT_FILE_EXTENSION)) as f:
            text = f.read()
        # convert string state
        if convert:
            for line in text.splitlines():
                # register state label
                state_num, states_str = line.split(": ")
                conv_states = [NameConverter.state_decode(name=plant_names[idx], state=int(state), convert=convert)
                            for idx, state in enumerate(states_str.split(","))]
                NameConverter.state_encode(name, ",".join(map(str, conv_states)))
        if table:
            return text
    else:
        # original sync
        prm_string = "{name1}\n{num}\n{names}\n".format(
            name1=get_path(name),
            num=len(plant_names),
            names="\n".join(plant_names_with_path)
        )
        prm_path = gen_prm(prm_filename, prm_string)

        ret_code = __call(4, prm_path)
        check_ret_code(ret_code)
        del_prm(prm_filename)


def meet(new_plant: str, *plant_names: str):
    for plant_name in plant_names:
        check_exist(plant_name + DES_FILE_EXTENSION)

    prm_filename = "meet_%s.prm" % new_plant
    plant_names_with_path = list(map(lambda x: get_path(x), plant_names))

    prm_string = "{name1}\n{num}\n{names}\n".format(
        name1=get_path(new_plant),
        num=len(plant_names),
        names="\n".join(plant_names_with_path)
    )
    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(5, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)


def supcon(sup: str, plant: str, spec: str):
    for plant_name in [plant, spec]:
        check_exist(plant_name + DES_FILE_EXTENSION)

    prm_filename = "supcon_%s.prm" % sup

    prm_string = "{name1}\n{name2}\n{supervisor}\n".format(
        name1=get_path(plant),
        name2=get_path(spec),
        supervisor=get_path(sup)
    )
    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(6, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)


def allevents(new_name: str, plant_name: str):
    check_exist(plant_name + DES_FILE_EXTENSION)

    prm_filename = "trim_%s.prm" % plant_name

    prm_string = "{name1}\n{name2}\n{entry}\n".format(
        name1=get_path(plant_name),
        name2=get_path(new_name),
        entry=1
    )
    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(7, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)


def mutex(new_name: str, name_1: str, name_2: str, state_pair: list[tuple[State, State]]):
    for name in [name_1, name_2]:
        check_exist(name + DES_FILE_EXTENSION)

    prm_filename = "mutex_%s.prm" % name_1
    state_pair_list = [f"{NameConverter.state_encode(name_1, st1, False)} {NameConverter.state_encode(name_2, st2, False)}" \
                        for st1, st2 in state_pair]
    
    prm_string = "{name1}\n{name2}\n{name3}\n{statepair}".format(
        name1=get_path(name_1),
        name2=get_path(name_2),
        name3=get_path(new_name),
        statepair=f"\n".join(state_pair_list) 
    )
    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(8, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)


def complement(new_name: str, plant_name: str, auxiliary_events: EventList = []):
    check_exist(plant_name + DES_FILE_EXTENSION)

    prm_filename = "complement_%s.prm" % plant_name
    auxiliary_events_list = [f"{NameConverter.event_encode(event, create=False)}" for event in auxiliary_events] 

    prm_string = "{name1}\n{name2}\n{eventpair}".format(
        name1=get_path(plant_name),
        name2=get_path(new_name),
        eventpair="\n".join(auxiliary_events_list)
    )
    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(9, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)


def nonconflict(des1: str, des2: str) -> bool:
    for name in [des1, des2]:
        check_exist(name + DES_FILE_EXTENSION)

    prm_filename = "nonconflict_%s.prm" % des1

    prm_string = "{name1}\n{name2}\n".format(
        name1=get_path(des1),
        name2=get_path(des2),
    )
    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(10, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)
    if ret_code == 0:
        return False
    elif ret_code == 1:
        return True
    else:
        raise RuntimeError("Unknown Error.")


def condat(new_name: str, plant_name: str, sup_name: str):
    for name in [plant_name, sup_name]:
        check_exist(name + DES_FILE_EXTENSION)
    
    prm_filename = "condat_%s.prm" % new_name

    prm_string = "{name1}\n{name2}\n{name3}\n".format(
        name1=get_path(plant_name),
        name2=get_path(sup_name),
        name3=get_path(new_name)
    )
    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(11, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)


def supreduce(new_name: str, plant_name: str, sup_name: str, dat_name: str, mode: int = 0, slb_flg: bool = True):
    for name in [plant_name, sup_name]:
        check_exist(name + DES_FILE_EXTENSION)
    
    check_exist(dat_name + DAT_FILE_EXTENSION)
    
    prm_filename = "supreduce_%s.prm" % new_name

    prm_string = "{name1}\n{name2}\n{name3}\n{name4}\n{mode}\n{slb_flg}\n".format(
        name1=get_path(plant_name),
        name2=get_path(sup_name),
        name3=get_path(dat_name),
        name4=get_path(new_name),
        mode=mode,
        slb_flg=1 if slb_flg else 0
    )
    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(12, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)


def isomorph(des1_name: str, des2_name: str):
    for name in [des1_name, des2_name]:
        check_exist(name + DES_FILE_EXTENSION)
    
    prm_filename = "isomorph_%s.prm" % des1_name

    prm_string = "{name1}\n{name2}\n".format(
        name1=get_path(des1_name),
        name2=get_path(des2_name)
    )
    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(13, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)
    if ret_code == 0:
        return False
    elif ret_code == 1:
        return True
    else:
        raise RuntimeError("Unknown Error")

def printdat(new_name: str, dat_name: str, convert: bool = True) -> DatInfo:
    check_exist(dat_name + DAT_FILE_EXTENSION)
    
    prm_filename = "printdat_%s.prm" % dat_name
    prm_string = "{name1}\n{name2}\n".format(
        name1=get_path(dat_name),
        name2=get_path(new_name)
    )
    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(14, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)

    with open(conf.SAVE_FOLDER / (new_name + ".TXT")) as f:
        text = f.read()
    
    return DatInfo(text=text, convert=convert)


def getdes_parameter(name: str, format: int = 0) -> list:
    if format == 0:
        check_exist(name + DES_FILE_EXTENSION)
    else:
        check_exist(name + DAT_FILE_EXTENSION)
    
    prm_filename = "getdes_parameter_%s.prm" % name
    result_filename = "getdes_result"
    prm_string = "{name1}\n{result_name}\n{format}".format(
        name1=get_path(name),
        result_name=get_path(result_filename),
        format=format
    )
    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(15, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)

    with open(get_path(result_filename + '.RST')) as f:
        res = []
        for l in f:
            res.append(l.rstrip())

    if res[0] != '0':
        raise RuntimeError("Getdes return other than OK code.")
    
    if res[4] == '2':
        is_controllable = None  # No check 
    else:
        is_controllable = (res[4] == '1')

    return {
        'state_size': int(res[1]),
        'tran_size': int(res[2]),
        'is_deterministic': res[3] == '1',
        'is_controllable': is_controllable,
    }

def statenum(name: str) -> int:
    des_info = getdes_parameter(name)
    return des_info['state_size']

def transnum(name: str) -> int:
    des_info = getdes_parameter(name)
    return des_info['tran_size']


def supconrobs(new_name: str, plant_name: str, spec_name: str, obs: EventList):
    for name in [plant_name, spec_name]:
        check_exist(name + DES_FILE_EXTENSION)
    
    prm_filename = "supconrobs_%s.prm" % new_name
    obs_list = [f"{NameConverter.event_encode(num, create=False)}" for num in obs]

    prm_string = "{name1}\n{name2}\n{name3}\n{obs}".format(
        name1=get_path(plant_name),
        name2=get_path(spec_name),
        name3=get_path(new_name),
        obs="\n".join(obs_list)
    )
    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(16, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)


def project(new_name: str, plant_name: str, obs: EventList):
    check_exist(plant_name + DES_FILE_EXTENSION)
    
    prm_filename = "project_%s.prm" % new_name
    obs_list = [f"{NameConverter.event_encode(num, create=False)}" for num in obs]

    prm_string = "{name1}\n{name2}\n{obs}".format(
        name1=get_path(plant_name),
        name2=get_path(new_name),
        obs="\n".join(obs_list)
    )
    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(17, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)


def localize(loc_names: list, plant_name: str, sup_name: str, components: list[str]):
    check_exist(plant_name + DES_FILE_EXTENSION)
    check_exist(sup_name + DES_FILE_EXTENSION)
    for agent in components:
        check_exist(agent + DES_FILE_EXTENSION)
    
    prm_filename = "localize_%s.prm" % plant_name
    loc_list = [f"{get_path(loc)}" for loc in loc_names]  # str変換
    components_list = [f"{get_path(com)}" for com in components]

    prm_string = "{name1}\n{name2}\n{num_component}\n{component}\n{num_loc}\n{loc}\n".format(
        name1=get_path(plant_name),
        name2=get_path(sup_name),
        num_component=len(components),
        component="\n".join(components_list),
        num_loc=len(loc_names),
        loc="\n".join(loc_list)
    )
    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(18, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)


def minstate(new_name: str, plant_name: str):
    check_exist(plant_name + DES_FILE_EXTENSION)
    prm_filename = "minstate_%s.prm" % new_name

    prm_string = "{name1}\n{name2}\n".format(
        name1=get_path(plant_name),
        name2=get_path(new_name)
    )
    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(19, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)


def force(new_name: str, plant_name: str, forcible_list: EventList, preemptible_list: EventList, timeout_event: Event):
    check_exist(plant_name + DES_FILE_EXTENSION)
    prm_filename = "force_%s.prm" % new_name

    forcible = [f"{NameConverter.event_encode(fl, create=False)}" for fl in forcible_list]
    preemptible = [f"{NameConverter.event_encode(pl, create=False)}" for pl in preemptible_list]

    prm_string = "{name1}\n{name2}\n{timeout}\n{forcible}\n{preemptible}\n".format(
        name1=get_path(plant_name),
        name2=get_path(new_name),
        timeout=NameConverter.event_encode(timeout_event, create=False),
        forcible="\n".join(forcible),
        preemptible="\n".join(preemptible)
    )

    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(20, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)


def convert(new_name: str, plant_name: str, event_pair: list):
    check_exist(plant_name + DES_FILE_EXTENSION)

    prm_filename = "convert_%s.prm" % plant_name
    # TODO: consider string event
    event_pair_list = [f"{old} {new}" for old, new in event_pair]
    
    prm_string = "{name1}\n{name2}\n{statepair}\n".format(
        name1=get_path(plant_name),
        name2=get_path(new_name),
        statepair="\n".join(event_pair_list) 
    )
    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(21, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)


def relabel(new_name: str, plant_name: str, state_pair: list):
    convert(new_name, plant_name, state_pair)


def supnorm(new_name: str, plant_name: str, sup_name: str, null_list: EventList):
    for name in [plant_name, sup_name]:
        check_exist(name + DES_FILE_EXTENSION)

    prm_filename = "supnorm_%s.prm" % new_name
    null = [f"{NameConverter.event_encode(num, create=False)}" for num in null_list]

    prm_string = "{name1}\n{name2}\n{name3}\n{null}\n".format(
        name1=get_path(sup_name),
        name2=get_path(plant_name),
        name3=get_path(new_name),
        null="\n".join(null)
    )
    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(22, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)


def supscop(new_name: str, plant_name: str, sup_name: str, null_list: EventList):
    for name in [plant_name, sup_name]:
        check_exist(name + DES_FILE_EXTENSION)

    prm_filename = "supscop_%s.prm" % new_name
    null = [f"{NameConverter.event_encode(num, create=False)}" for num in null_list]

    prm_string = "{name1}\n{name2}\n{name3}\n{null}\n".format(
        name1=get_path(sup_name),
        name2=get_path(plant_name),
        name3=get_path(new_name),
        null="\n".join(null)
    )
    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(23, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)


def supqc(new_name: str, plant_name: str, mode: str, null_list: EventList):
    check_exist(plant_name + DES_FILE_EXTENSION)
    prm_filename = "supqc_%s.prm" % new_name
    result_filename = "supqc_result"

    null = [f"{NameConverter.event_encode(num, create=False)}" for num in null_list]
    if mode == "qc":
        mode_flg = 1
    elif mode == "sqc":
        mode_flg = 2
    else:
        raise ValueError("Unknown mode. You can select 'qc' or 'sqc'.")
    
    prm_string = "{mode_flg}\n{name1}\n{name2}\n{name3}\n{null}\n".format(
        mode_flg=mode_flg,
        name1=get_path(plant_name),
        name2=get_path(new_name),
        name3=get_path(result_filename),
        null="\n".join(null)
    )
    prm_path = gen_prm(prm_filename, prm_string)
    ret_code = __call(24, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)
    # TODO: load rst file


def observable(plant_1: str, plant_2: str, mode: str, null_list: EventList) -> bool:
    for name in [plant_1, plant_2]:
        check_exist(name + DES_FILE_EXTENSION)
    prm_filename = "observable_%s.prm" % plant_1
    result_filename = "observable_result"

    null = [f"{NameConverter.event_encode(num, create=False)}" for num in null_list]
    if mode == "o":
        mode_flg = 1
    elif mode == "so":
        mode_flg = 2
    else:
        raise ValueError("Unknown mode. You can select 'o' or 'so'.")
    
    prm_string = "{mode_flg}\n{name1}\n{name2}\n{name3}\n{null}\n".format(
        mode_flg=mode_flg,
        name1=get_path(plant_1),
        name2=get_path(plant_2),
        name3=get_path(result_filename),
        null="\n".join(null)
    )
    prm_path = gen_prm(prm_filename, prm_string)
    ret_code = __call(25, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)

    with open(get_path(result_filename + RST_FILE_EXTENSION)) as f:
        res = []
        for l in f:
            res.append(l.rstrip())

    if res[0] != '0':
        raise RuntimeError("Observable return other than OK code.")
    
    if res[1] == '1':
        is_observable = True
    else:
        is_observable = False

    return is_observable


def natobs(new_name1: str, new_name2: str, plant_name: str, image_list: EventList):
    check_exist(plant_name + DES_FILE_EXTENSION)

    prm_filename = "natobs_%s.prm" % new_name1
    image = [f"{NameConverter.event_encode(num, create=False)}" for num in image_list]

    prm_string = "{name1}\n{name2}\n{name3}\n{image}\n".format(
        name1=get_path(plant_name),
        name2=get_path(new_name1),
        name3=get_path(new_name2),
        image="\n".join(image)
    )
    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(26, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)


def suprobs(new_name: str, plant_name: str, sup_name: str, null_list: EventList, mode: int = 1):
    for name in [plant_name, sup_name]:
        check_exist(name + DES_FILE_EXTENSION)

    if mode != 1:
        raise ValueError("Unknown Mode. You can select 1.")

    prm_filename = "suprobs_%s.prm" % new_name

    null = [f"{NameConverter.event_encode(num, create=False)}" for num in null_list]
    prm_string = "{name1}\n{name2}\n{name3}\n{mode}\n{null}\n".format(
        name1=get_path(plant_name),
        name2=get_path(sup_name),
        name3=get_path(new_name),
        mode=mode,
        null="\n".join(null)
    )
    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(27, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)


def recode(new_name: str, plant_name: str):
    check_exist(plant_name + DES_FILE_EXTENSION)

    prm_filename = "record_%s.prm" % new_name

    prm_string = "{name1}\n{name2}\n".format(
        name1=get_path(plant_name),
        name2=get_path(new_name)
    )
    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(28, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)


def ext_suprobs(new_name: str, plant_name: str, legal_lang_name: str, ambient_lang_name: str,
               controllable_list: EventList, null_list: EventList, algorithm: int):
    for name in [plant_name, legal_lang_name, ambient_lang_name]:
        check_exist(name + DES_FILE_EXTENSION)
    prm_filename = "ext_suprobs_%s.prm" % new_name

    controllable = [f"{NameConverter.event_encode(c, create=False)}" for c in controllable_list]
    controllable.append("-1")
    null = [f"{NameConverter.event_encode(num, create=False)}" for num in null_list]

    prm_string = "{name1}\n{name2}\n{name3}\n{name4}\n{algorithm}\n{controllable}\n{null}\n".format(
        name1=get_path(plant_name),
        name2=get_path(legal_lang_name),
        name3=get_path(ambient_lang_name),
        name4=get_path(new_name),
        algorithm=algorithm,
        controllable="\n".join(controllable),
        null="\n".join(null)
    )

    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(29, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)


def lb_suprobs(new_name: str, plant_name: str, legal_lang_name: str, ambient_lang_name: str,
               controllable_list: EventList, null_list: EventList):
    alg_flag = 2  # language based
    ext_suprobs(new_name, plant_name, legal_lang_name, ambient_lang_name, controllable_list, null_list, alg_flag)


def tb_suprobs(new_name: str, plant_name: str, legal_lang_name: str, ambient_lang_name: str,
               controllable_list: EventList, null_list: EventList):
    alg_flag = 1  # transition based
    ext_suprobs(new_name, plant_name, legal_lang_name, ambient_lang_name, controllable_list, null_list, alg_flag)


def des_info(name: str) -> DesInfo:
    check_exist(name + DES_FILE_EXTENSION)
    path = Path(conf.SAVE_FOLDER / (name + DES_FILE_EXTENSION))
    byte = path.read_bytes()
    raw_data = umsgpack.unpackb(byte)
    states = raw_data["states"]
    """
    {
     0: {'marked': True, 'next': [[1, 1], [15, 4]], 'vocal': 0},
     1: {'marked': True, 'next': [[0, 0], [3, 2]], 'vocal': 0},
     2: {'marked': False, 'next': [[0, 3], [5, 0]], 'vocal': 0},
     3: {'marked': False, 'next': None, 'vocal': 0},
     4: {'marked': False, 'next': None, 'vocal': 0}
    }
    """
    des = DesInfo(name, states)
    return des

def _create_ext_des_info(name: str):
    check_exist(name + DES_FILE_EXTENSION)
    prm_filename = "get_ext_des_%s.prm" % name

    prm_string = "{name1}\n".format(
        name1=get_path(name)
    )
    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(30, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)


def ext_des_info(name: str) -> ExtDesInfo:
    _create_ext_des_info(name)
    check_exist(name + EDES_FILE_EXTENSION)
    path = Path(conf.SAVE_FOLDER / (name + EDES_FILE_EXTENSION))
    byte = path.read_bytes()
    raw_data = umsgpack.unpackb(byte)
    states = raw_data["states"]
    edes = ExtDesInfo(name, states)
    return edes

def is_reachable(name: str, state: State = -1) -> bool:
    ext_des = ext_des_info(name)
    if isinstance(state, int) and state < 0:
        # Whether all state is reachable or not
        return ext_des.all_reached()
    else:
        # Whether a state is reachable or not
        return ext_des.is_reached(state)

def is_coreachable(name: str, state: State = -1) -> bool:
    ext_des = ext_des_info(name)
    if isinstance(state, int) and state < 0:
        # Whether all state is reachable or not
        return ext_des.all_coreach()
    else:
        # Whether a state is reachable or not
        return ext_des.is_coreach(state)

def shortest_string(name: str, start_state: State, reach_state: State, convert: bool = True) -> Optional[Event]:
    event_path = path_event_list(name, start_state, reach_state, convert=convert)
    if not event_path:
        return None
    return event_path

def reachable_string(name: str, state: State, convert: bool = True) -> Optional[Event]:
    if not is_reachable(name, state):
        return None
    return shortest_string(name, start_state=0, reach_state=state, convert=convert)

def coreachable_string(name: str, state: State, marker_state: State = -1, convert: bool = True) -> Optional[Event]:
    if not is_coreachable(name, state):
        return None
    
    if isinstance(marker_state, int) and marker_state < 0:
        # auto search (Search all marker states and return the first route found.)
        markers = des_info(name).marked(convert=False)
        for marker in markers:
            event_path = shortest_string(name, start_state=state, reach_state=marker, convert=convert)
            if not event_path:
                continue
            return event_path
        return None
    else:
        # set marker state
        is_marker = des_info(name).is_marked(marker_state)
        if not is_marker:
            return None
        return shortest_string(name, start_state=state, reach_state=marker_state, convert=convert)

def is_trim(name: str) -> bool:
    new_name = f"{name}_{random.randint(100, 999)}"
    trim(new_name, name)
    return isomorph(new_name, name)

def is_nonblocking(name: str) -> bool:
    ext_des = ext_des_info(name)

    reached = set(ext_des.reached(convert=False))
    coreach = set(ext_des.coreach(convert=False))
    return reached.issubset(coreach)

def blocking_states(name: str, convert: bool = True) -> StateList:
    ext_des = ext_des_info(name)

    reached = set(ext_des.reached(convert=convert))
    coreach = set(ext_des.coreach(convert=convert))
    result = list(reached - coreach)
    return result

def marker(name: str, convert: bool = True) -> StateList:
    des = des_info(name)
    return des.marked(convert=convert)

def trans(name: str, convert: bool = True) -> TransList:
    des = des_info(name)
    return des.trans(convert=convert)

def events(name: str, convert: bool = True) -> list[Event]:
    des = des_info(name)
    return des.events(convert=convert)

def display_automaton(name: str, convert: bool = True, color: bool = False, **kwargs) -> AutomatonDisplay:
    return AutomatonDisplay(name, convert=convert, color=color, **kwargs)

# Create a new automaton with some states and transitions removed
def subautomaton(new_automaton_name: str, automaton_name: str, del_states: StateList = [], del_trans: TransList = []) -> TransList:
    new_trans = trans(automaton_name) # get the set of transitions

    del_trans_wo_CorU = [t[:3] for t in del_trans]  # delete 'c' or 'u' params
    del_trans_list = [] # the list of transitions you delete
    for tran in new_trans:
        # record transitions you delete
        tran_wo_CorU = tran[:3]  # delete 'c' or 'u' params
        if tran[0] in del_states or tran[2] in del_states or tran_wo_CorU in del_trans_wo_CorU:
            del_trans_list.append(tran)

    for del_tran in del_trans_list:
        # delete transitions
        new_trans.remove(del_tran)
    
    # create a new automaton
    # calculate state size(Q)
    state_num = 0
    if len(new_trans) > 0:
        init_state = new_trans[0][0]
        if type(init_state) == str:
            # string state
            state_set = set()
            for s, a, ns, *uc in new_trans:
                state_set.add(s)
                state_set.add(ns)
            state_num = len(state_set)
        else:
            # int state
            state_num = statenum(automaton_name)

    marker_states = marker(automaton_name)
    create(new_automaton_name, state_num, new_trans, marker_states)
    return del_trans_list

def is_controllable(automaton_name: str, spec_name: str) -> bool:
  dat_name = spec_name + 'DAT'
  condat(dat_name, automaton_name, spec_name)
  txt_name = dat_name + '_txt'
  return printdat(txt_name, dat_name)._extract_is_controllable()

def uncontrollable_states(plant_name: str, spec_name: str):
    # collect uncontrollable events of plant
    u_events = []
    p_trans = trans(plant_name)
    s_trans = trans(spec_name)
    if len(p_trans) == 0: # if 'p_trans' is empty
        return
    else:
        if type(p_trans[0][1]) == int: # if the type of events is not string
            return
    for tran in p_trans:
        if tran[3] == 'u': # if p_tran[1] is uncontrollable
            u_events.append(tran[1])

    sync_name = plant_name+'_'+spec_name+'_SYNC'
    sync(sync_name, plant_name, spec_name, convert=True)
    state_pairs = [] # 'plant's state, spec's state'
    k_trans = trans(sync_name)
    for k_tran in k_trans:
        if not k_tran[0] in state_pairs:
            state_pairs.append(k_tran[0])
        if not k_tran[2] in state_pairs:
            state_pairs.append(k_tran[2])

    x_events = [] # events defined at state 'x' of plant e.g.) x_events[0] mean all events at state '0' of plant
    for i in range(statenum(plant_name)):
        x_events.append([])
    for p_tran in p_trans:
        x_events[p_tran[0]].append(p_tran[1])

    y_events = [] # events defined at state 'y' of spec e.g.) y_events[1] mean all events at state '1' of spec
    for i in range(statenum(spec_name)):
        y_events.append([])
    for s_tran in s_trans:
        y_events[s_tran[0]].append(s_tran[1])

    uncontrollable_states = []
    for state_pair in state_pairs:
        state_pair = state_pair.split(',') # split 'state_pair' by ','

        x = int(state_pair[0])
        y = int(state_pair[1])

        for u_event in u_events:
            if not u_event in x_events[x]:
                continue
            if not u_event in y_events[y]:
                if not y in uncontrollable_states:
                    uncontrollable_states.append(y)

    return uncontrollable_states

def conact(plant_name:str, spec_name:str) -> str:
    dat_name = spec_name + 'DAT'
    condat(dat_name, plant_name, spec_name)
    txt_name = dat_name + '_txt'

    data = printdat(txt_name, dat_name).control_data
    
    string = ''
    for key, item in data.items():
        string += str(key) + ': '
        for i in range(len(item)):
            string += str(item[i]) + ', '
        string = string[:-2] # delete the last ', '
        string += '\n'
    string = string[:-1] # delete the last '\n'
    return string

def plantification(spec: str, plantified_spec_name: str): # plantification : to convert the specification into a plant
    #creation of the parameters for the plantified specification
    statenum_var = statenum(spec) #same number of state as the specification

    trans_var = trans(spec) #same trans_varition as the specification

    modified_trans_var = []

    #loop in order to convert all the uncontrollable event written 'uc' by uncontrollable event written 'u'.
    for i in trans_var :
        if i[3] ==  'uc':
            trans_var_list = list(i) #convert tuple into list in order to modify it
            trans_var_list[3] = 'u'
            i = tuple(trans_var_list) #convert list into tuple
        modified_trans_var.append(i)
    trans_var = modified_trans_var

    marker_var = marker(spec) #same marked state as the specification

    #loop to check if there is at least one uncontrollable event that is not authorized from a state of the specification
    check = False
    enabled_states = []
    for j in trans(spec) : 
        if check == False and j[3] != 'c':
            for k in trans(spec) :
                if k[1] == j[1] and k[0] in list(range(statenum(spec))) :
                    enabled_states.append(k[0])
                    # print(enabled_states)
            for check in trans(spec) :
                if check[0] not in enabled_states or set(list(range(statenum(spec)))).issubset(set(enabled_states)) == False :
                    statenum_var = statenum_var + 1 #add the blocking state to the plantified specification automaton
                    check = True #boolean variable to check if new trans_varitions would be added or not
                    # print(statenum_var)
                break

    #loop to add new trans_varitions from specific states to the new blocking state
    if check == True :
        done_events = []
        for m in trans(spec) : 
            if m[3] != 'c' :
                check_list = []
                for n in trans(spec) :
                    if m[1] == n[1] and n[0] not in check_list :
                        check_list.append(n[0])
                        # print(check_list)
                if set(list(range(statenum(spec)))).issubset(set(check_list)) == False and m[1] not in done_events :
                    need_list = set(list(range(statenum(spec)))) - (set(check_list))
                    # print(need_list)
                    for need in need_list :
                        new_trans_varition = (need, m[1], statenum_var-1, 'u')
                        # print(new_trans_varition)
                        trans_var.append(new_trans_varition)
                done_events.append(m[1])

    create(str(plantified_spec_name), statenum_var, trans_var, marker_var) #creation of the plantified_spec automaton 

def supervisory_controller_synthesis(plantified_specification: str, trimed_supervisor_name: str,  sigma_f: list) :
    #Initialization of the variables 
    q_k = list(range(statenum(plantified_specification)))
    
    q_m = marker(plantified_specification)
    
    sigma = [] #Alphabet list initialization
    for event in trans(plantified_specification) :
        if event[1] not in sigma :
            sigma.append(event[1])

    sigma_u = []
    for event in trans(plantified_specification) :
        if event[1] not in sigma_u and event[3] != 'c':
            sigma_u.append(event[1])
            
    trans_var = {(t[0], t[1]): t[2] for t in trans(plantified_specification)}

    saved_trans_var = trans(plantified_specification)
    
    # print('Initial trans_varitions : ',  trans_var)

    #Main Loop

    old_q_k = []
    old_trans_var = []
    
    while q_k != old_q_k or trans_var != old_trans_var :

        #STEP 1 : find non-blocking states
        
        nb = set(q_m) & set(q_k)
        nb_condition = True
    
        while nb_condition :
            nb_condition = False
            new_nb = nb
            for state in q_k :
                if state not in nb :
                    for event in sigma :
                        if (state, event) in trans_var and trans_var[(state, event)] in nb :
                            new_nb.add(state)
                            nb_condition = True
    
            nb = new_nb
        # print('Non-blocking states : ', nb)
        
        
        #STEP 2 : find bad states
        
        b = set(q_k) - nb
        forced_states = set()
        b_condition = True
    
        while b_condition :
            b_condition = False
            new_b = b
            new_forced_states = forced_states
            for state in q_k :
                if state not in b :
                    for u_event in sigma_u :
                        if (state, u_event) in trans_var and trans_var[(state, u_event)] in b :
                            if any((state, f_event) in trans_var for f_event in sigma_f) :
                                if all((state, f_event) not in trans_var or trans_var[(state, f_event)] in b for f_event in sigma_f) :
                                    new_b.add(state)
                                    b_condition = True
                                else :
                                    new_forced_states.add(state)
                            else :
                                new_b.add(state)
                                b_condition = True
                                    
            b = new_b
            forced_states = new_forced_states
    
        # print('bad states : ', b)
        # print('forced states : ', forced_states)
        
    
        #STEP 3 : Update of states en trans_varitions
        
        new_q_k = set(q_k) - b
        new_trans_var = {      #new dictionary 
                (state, event): next_state
                for (state, event), next_state in trans_var.items()
                if state in new_q_k and next_state in new_q_k}
    
        for state in forced_states: #remove uncontrollable events that start from a forcible state
                for event in sigma:
                    if event not in sigma_f and (state, event) in trans_var:
                        del trans_var[(state, event)]

        old_q_k = q_k
        old_trans_var = trans_var
        q_k = new_q_k
        trans_var = new_trans_var
    
        # print('new current states : ', new_q_k)
        # print('new current trans_varitions :', new_trans_var)
        
        
    
    #STEP 4 : CREATE & TRIM
    
    #State number of the supervisor
    statenum_var = len(q_k)
    # print('Current state number of the supervisor :', statenum_var)
    
    #Conversion of the trans_varition dictionary into a list
    trans_varition = [(key[0], key[1], value) for key, value in trans_var.items()] 
    # print(trans_varition)
    #Loop in order to add the information about controllability of the event for each trans_varition
    compteur = 0
    for trans_vari in trans_varition :
        for trans_var_s in saved_trans_var :
            if trans_vari[1] == trans_var_s[1] :
                trans_varition[compteur] = trans_varition[compteur] + (trans_var_s[3],)
                compteur = compteur + 1
                break
    #Conversion of each int in trans_varition tuple into a string
    trans_varition = [
    tuple(str(element) if isinstance(element, int) else element for element in t)
    for t in trans_varition]
            
    # print('Current trans_varitions of the supervisor :',trans_varition)
    # print(q_m)    
    supervisor_name = "before_trimming_" + trimed_supervisor_name
    create(supervisor_name, statenum_var, trans_varition, [str(marked_states) for marked_states in q_m])
    trim(trimed_supervisor_name, supervisor_name)

def supervisory_synthesize(plant: str, spec: str, plantified_spec_name: str, trimed_supervisor_name: str, sigma_f: list) :
    plantified_specification = plantification(spec, plantified_spec_name)
    sync_automaton_name = plant + "_and_" + plantified_spec_name + "_sync"
    sync(sync_automaton_name, plantified_spec_name, plant)
    supervisory_controller_synthesis(sync_automaton_name, trimed_supervisor_name, sigma_f)

def supconbnd(supconbnd_name:str, plant_name:str, spec_name:str, N:int):
    sync('K1',plant_name,spec_name)
    display_automaton('K1', color=True)
    
    K1_XI = trans('K1')
    K1_X_len =  statenum('K1')
    K1_SIGMA = events('K1')
    K1_x_0 = 0
    K1_X_m = marker('K1')
    
    K2_X_pair = []
    for i in range(K1_X_len):
        for j in range(N):
            K2_X_pair.append([i, j])
    K2_SIGMA = []
    K2_XI_pair = []
    K2_x_0_pair = [K1_x_0, 0]
    K2_X_m_pair = []
    for i in range(len(K1_X_m)):
        K2_X_m_pair.append([K1_X_m[i], 0])
    
    F = []
    for i in range(len(K2_X_pair)):
        F.append(False)

    ST_pair = []
    ST_pair.append(K2_x_0_pair)
    F[K2_X_pair.index(K2_x_0_pair)] = True
    
    while(bool(ST_pair)):
        x = ST_pair[0][0]
        d = ST_pair[0][1]
        del ST_pair[0]
        
        if((x in K1_X_m) == True):
            for i in range(len(K1_XI)):
                if(x == K1_XI[i][0]):
                    sigma = K1_XI[i][1]
                    x_nxt = K1_XI[i][2]
                    
                    K2_XI_pair.append([[x,0], sigma, [x_nxt, 0]])                    
                    if(F[K2_X_pair.index([x_nxt, 0])] == False):
                        ST_pair.append([x_nxt, 0])
                        F[K2_X_pair.index([x_nxt, 0])] = True
                        
        elif((x in K1_X_m) == False):
            for i in range(len(K1_XI)):
                if(x == K1_XI[i][0]):                
                    sigma = K1_XI[i][1]
                    x_nxt = K1_XI[i][2]
    
                    if((x_nxt in K1_X_m) == True):
                        d_nxt = 0
                    elif((x_nxt in K1_X_m) == False):
                        d_nxt = d + 1
                    
                    if(d_nxt == N):
                        if(i == len(K1_XI)-1):
                            break
                        else:
                            continue
                        
                    elif(d_nxt != N):
                        K2_XI_pair.append([[x,d], sigma, [x_nxt, d_nxt]])                            
                        if(F[K2_X_pair.index([x_nxt, d_nxt])] == False):
                            ST_pair.append([x_nxt, d_nxt])
                            F[K2_X_pair.index([x_nxt, d_nxt])] = True
                            
    K2_X_N_pair = [K2_x_0_pair]
    for i in range(len(K2_XI_pair)):
        if((K2_XI_pair[i][2] in K2_X_N_pair) == False):
            K2_X_N_pair.append(K2_XI_pair[i][2])

    K2_X_m_N_pair = []
    for i in range(len(K2_X_N_pair)):
        if((K2_X_N_pair[i] in K2_X_m_pair) == True):
            K2_X_m_N_pair.append(K2_X_N_pair[i])

    K2_X_m_N_indent = []
    for i in range(len(K2_X_m_pair)):    
        if((K2_X_m_pair[i] in K2_X_N_pair) == True):
            K2_X_m_N_indent.append(K2_X_N_pair.index(K2_X_m_pair[i]))
    
    K2_XI_N_indent = []
    for i in range(len(K2_XI_pair)):
        if((K2_XI_pair[i][0] in K2_X_N_pair) and (K2_XI_pair[i][2] in K2_X_N_pair)):
            K2_XI_N_indent.append([K2_X_N_pair.index(K2_XI_pair[i][0]), 
                                      K2_XI_pair[i][1], 
                                      K2_X_N_pair.index(K2_XI_pair[i][2])])

    create('K2', len(K2_X_N_pair), K2_XI_N_indent, K2_X_m_N_indent)   
    trim('K2_trm','K2')
    supcon(supconbnd_name, plant_name, 'K2_trm')
