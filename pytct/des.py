from pathlib import Path
from pytct.dat_info import DatInfo
from typing import List

from .libtct import call_program as __call

from .automaton_display import DAT_FILE_EXTENSION, DES_FILE_EXTENSION
from .config import Config
from .des_check import gen_prm, del_prm, check_exist, check_ret_code, get_path

conf = Config.get_instance()

def init(name: str, overwrite: bool = False):
    p = Path(name)
    if not overwrite and p.exists():
        raise FileExistsError(f"Directory {name} is already exists. If you can overwrite, "
                              f"please set overwrite arg.\nsample: init('{name}', overwrite=True)")
    # create directory.
    p.mkdir(parents=True, exist_ok=True)
    
    conf.SAVE_FOLDER = p

def create(name: str, size: int, trans: list, marker: list):
    """Create

    Create an automaton model of DES

    Args:
        name (str): DES model name.
        size (int): number of states.
        trans (list): transition tuple list. [(state, event, next state), (...)]
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

    marker_list = ["%d" % mark for mark in marker]
    marker_list.append("-1")
    trans_list = ["%d %d %d" % ent for ent in trans]

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


def sync(new_plant: str, *plant_names: str):
    for plant_name in plant_names:
        check_exist(plant_name + DES_FILE_EXTENSION)

    prm_filename = "sync_%s.prm" % new_plant
    plant_names_with_path = list(map(lambda x: get_path(x), plant_names))

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


def mutex(new_name: str, plant_name: str, name_2: str, state_pair: List[tuple]):
    for name in [plant_name, name_2]:
        check_exist(name + DES_FILE_EXTENSION)

    prm_filename = "mutex_%s.prm" % plant_name
    state_pair_list = [f"{st[0]} {st[1]}" for st in state_pair]
    
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