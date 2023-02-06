from pathlib import Path
import random
import umsgpack
from pytct.automaton_display import AutomatonDisplay
from pytct.dat_info import DatInfo
from pytct.des_info import DesInfo
from pytct.ext_des_info import ExtDesInfo
from pytct.distance import path_event_list
from typing import List, Optional

from pytct.name_converter import NameConverter
from pytct.tct_typing import State, Event, TransList, StateList

from .libtct import call_program as __call

from .config import DAT_FILE_EXTENSION, DES_FILE_EXTENSION, RST_FILE_EXTENSION, EDES_FILE_EXTENSION
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

    conv_trans = NameConverter.encode_all(name, trans)
    check_state_num(conv_trans, size)
    trans_list = ["%d %d %d" % ent for ent in conv_trans]

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


def selfloop(new_name: str, plant_name: str, lst: list):
    check_exist(plant_name + DES_FILE_EXTENSION)

    prm_filename = "selfloop_%s.prm" % plant_name

    selfloop_list = ["%d" % state for state in lst]

    NameConverter.register(new_name, plant_name)
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

    NameConverter.register(new_name, plant_name)
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


def sync(new_plant: str, *plant_names: str):
    for plant_name in plant_names:
        check_exist(plant_name + DES_FILE_EXTENSION)

    prm_filename = "sync_%s.prm" % new_plant
    plant_names_with_path = list(map(lambda x: get_path(x), plant_names))

    NameConverter.register(new_plant, *plant_names)
    prm_string = "{name1}\n{num}\n{names}\n".format(
        name1=get_path(new_plant),
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

    NameConverter.register(new_plant, *plant_names)
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

    NameConverter.register(sup, plant, spec)
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

    NameConverter.register(new_name, plant_name)
    prm_string = "{name1}\n{name2}\n{entry}\n".format(
        name1=get_path(plant_name),
        name2=get_path(new_name),
        entry=1
    )
    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(7, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)


def mutex(new_name: str, plant_name: str, name_2: str, state_pair: List[tuple]):
    for name in [plant_name, name_2]:
        check_exist(name + DES_FILE_EXTENSION)

    prm_filename = "mutex_%s.prm" % plant_name
    state_pair_list = [f"{st[0]} {st[1]}" for st in state_pair]
    
    NameConverter.register(new_name, plant_name, name_2)
    prm_string = "{name1}\n{name2}\n{name3}\n{statepair}".format(
        name1=get_path(plant_name),
        name2=get_path(name_2),
        name3=get_path(new_name),
        statepair=f"\n".join(state_pair_list) 
    )
    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(8, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)


def complement(new_name: str, plant_name: str, auxiliary_events: list):
    check_exist(plant_name + DES_FILE_EXTENSION)

    prm_filename = "complement_%s.prm" % plant_name
    auxiliary_events_list = [f"{event}" for event in auxiliary_events] 

    NameConverter.register(new_name, plant_name)
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

    NameConverter.register(des1, des2)
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

    NameConverter.register(new_name, plant_name, sup_name)
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

    NameConverter.register(new_name, plant_name, sup_name)
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

    NameConverter.register(des1_name, des2_name)
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

def printdat(new_name: str, dat_name: str) -> DatInfo:
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
    
    return DatInfo(text=text)


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


def supconrobs(new_name: str, plant_name: str, spec_name: str, obs: list):
    for name in [plant_name, spec_name]:
        check_exist(name + DES_FILE_EXTENSION)
    
    prm_filename = "supconrobs_%s.prm" % new_name
    obs_list = [f"{num}" for num in obs] 

    NameConverter.register(new_name, plant_name, spec_name)
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


def project(new_name: str, plant_name: str, obs: list):
    check_exist(plant_name + DES_FILE_EXTENSION)
    
    prm_filename = "project_%s.prm" % new_name
    obs_list = [f"{num}" for num in obs] 

    NameConverter.register(new_name, plant_name)
    prm_string = "{name1}\n{name2}\n{obs}".format(
        name1=get_path(plant_name),
        name2=get_path(new_name),
        obs="\n".join(obs_list)
    )
    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(17, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)


def localize(loc_names: list, plant_name: str, sup_name: str, components: list):
    check_exist(plant_name + DES_FILE_EXTENSION)
    check_exist(sup_name + DES_FILE_EXTENSION)
    for agent in components:
        check_exist(agent + DES_FILE_EXTENSION)
    
    prm_filename = "localize_%s.prm" % plant_name
    loc_list = [f"{get_path(loc)}" for loc in loc_names]  # str変換
    components_list = [f"{get_path(com)}" for com in components]

    for loc in loc_names:
        NameConverter.register(loc, plant_name, sup_name)
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


def force(new_name: str, plant_name: str, forcible_list: list, preemptible_list: list, timeout_event: int):
    check_exist(plant_name + DES_FILE_EXTENSION)
    prm_filename = "force_%s.prm" % new_name

    NameConverter.register(new_name, plant_name)

    # TODO: consider string event
    forcible = [f"{fl}" for fl in forcible_list]
    preemptible = [f"{pl}" for pl in preemptible_list]

    prm_string = "{name1}\n{name2}\n{timeout}\n{forcible}\n{preemptible}\n".format(
        name1=get_path(plant_name),
        name2=get_path(new_name),
        timeout=timeout_event,
        forcible="\n".join(forcible),
        preemptible="\n".join(preemptible)
    )

    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(20, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)


def convert(new_name: str, plant_name: str, state_pair: list):
    check_exist(plant_name + DES_FILE_EXTENSION)

    prm_filename = "convert_%s.prm" % plant_name
    # TODO: consider string event
    state_pair_list = [f"{st[0]} {st[1]}" for st in state_pair]
    
    NameConverter.register(new_name, plant_name)
    prm_string = "{name1}\n{name2}\n{statepair}\n".format(
        name1=get_path(plant_name),
        name2=get_path(new_name),
        statepair="\n".join(state_pair_list) 
    )
    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(21, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)


def relabel(new_name: str, plant_name: str, state_pair: list):
    convert(new_name, plant_name, state_pair)


def supnorm(new_name: str, plant_name: str, sup_name: str, null_list: list):
    for name in [plant_name, sup_name]:
        check_exist(name + DES_FILE_EXTENSION)

    prm_filename = "supnorm_%s.prm" % new_name
    # TODO: consider string event
    null = [f"{num}" for num in null_list] 

    NameConverter.register(new_name, plant_name, sup_name)
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


def supscop(new_name: str, plant_name: str, sup_name: str, null_list: list):
    for name in [plant_name, sup_name]:
        check_exist(name + DES_FILE_EXTENSION)

    prm_filename = "supscop_%s.prm" % new_name
    # TODO: consider string event
    null = [f"{num}" for num in null_list] 

    NameConverter.register(new_name, plant_name, sup_name)
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


def supqc(new_name: str, plant_name: str, mode: str, null_list: list):
    check_exist(plant_name + DES_FILE_EXTENSION)
    prm_filename = "supqc_%s.prm" % new_name
    result_filename = "supqc_result"
    # TODO: consider string event
    null = [f"{num}" for num in null_list]
    if mode == "qc":
        mode_flg = 1
    elif mode == "sqc":
        mode_flg = 2
    else:
        raise ValueError("Unknown mode. You can select 'qc' or 'sqc'.")
    
    NameConverter.register(new_name, plant_name)
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


def observable(plant_1: str, plant_2: str, mode: str, null_list: list) -> bool:
    for name in [plant_1, plant_2]:
        check_exist(name + DES_FILE_EXTENSION)
    prm_filename = "observable_%s.prm" % plant_1
    result_filename = "observable_result"
    # TODO: consider string event
    null = [f"{num}" for num in null_list]
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


def natobs(new_name1: str, new_name2: str, plant_name: str, image_list: list):
    check_exist(plant_name + DES_FILE_EXTENSION)

    prm_filename = "natobs_%s.prm" % new_name1
    # TODO: consider string event
    image = [f"{num}" for num in image_list] 

    NameConverter.register(new_name1, plant_name)
    NameConverter.register(new_name2, plant_name)

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


def suprobs(new_name: str, plant_name: str, sup_name: str, null_list: list, mode: int = 1):
    for name in [plant_name, sup_name]:
        check_exist(name + DES_FILE_EXTENSION)

    if mode != 1:
        raise ValueError("Unknown Mode. You can select 1.")

    prm_filename = "suprobs_%s.prm" % new_name
    # TODO: consider string event
    null = [f"{num}" for num in null_list] 

    NameConverter.register(new_name, plant_name, sup_name)
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

    NameConverter.register(new_name, plant_name)
    prm_string = "{name1}\n{name2}\n".format(
        name1=get_path(plant_name),
        name2=get_path(new_name)
    )
    prm_path = gen_prm(prm_filename, prm_string)

    ret_code = __call(28, prm_path)
    check_ret_code(ret_code)
    del_prm(prm_filename)


def ext_suprobs(new_name: str, plant_name: str, legal_lang_name: str, ambient_lang_name: str,
               controllable_list: list, null_list: list, algorithm: int):
    for name in [plant_name, legal_lang_name, ambient_lang_name]:
        check_exist(name + DES_FILE_EXTENSION)
    prm_filename = "ext_suprobs_%s.prm" % new_name

    NameConverter.register(new_name, plant_name, legal_lang_name, ambient_lang_name)

    # TODO: consider string event
    controllable = [f"{c}" for c in controllable_list]
    controllable.append("-1")
    null = [f"{n}" for n in null_list]

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
               controllable_list: list, null_list: list):
    alg_flag = 2  # language based
    ext_suprobs(new_name, plant_name, legal_lang_name, ambient_lang_name, controllable_list, null_list, alg_flag)


def tb_suprobs(new_name: str, plant_name: str, legal_lang_name: str, ambient_lang_name: str,
               controllable_list: list, null_list: list):
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