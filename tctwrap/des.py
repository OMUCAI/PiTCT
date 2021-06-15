from pathlib import Path
from typing import List

from .libtct import call_program as __call

from .data import DES_FILE_EXTENSION


def gen_prm(filename: str, contents: str):
    prm = Path(filename)
    prm.write_text(contents)


def del_prm(filename: str):
    p = Path(filename)
    if p.exists():
        p.unlink()


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

    marker.append(-1)

    marker_list = ["%d" % mark for mark in marker]
    trans_list = ["%d %d %d" % ent for ent in trans]

    prm_string = "{des_name}\n{state_num}\n{marker_states}\n{transitions}\n"
    prm_string = prm_string.format(
        des_name=name,
        state_num=size,
        marker_states=" ".join(marker_list),
        transitions="\n".join(trans_list),
    )

    gen_prm(prm_filename, prm_string)

    if __call(0, prm_filename) != 0:
        raise RuntimeError("error: create, check %s" % prm_filename)
    else:
        del_prm(prm_filename)


def selfloop(new_name: str, plant_name: str, lst: list):
    if not Path(plant_name + DES_FILE_EXTENSION).exists():
        raise FileNotFoundError()

    prm_filename = "selfloop_%s.prm" % plant_name

    selfloop_list = ["%d" % state for state in lst]
    prm_string = "{name1}\n{name2}\n{ls}\n".format(
        name1=plant_name, name2=new_name, ls="\n".join(selfloop_list)
    )

    gen_prm(prm_filename, prm_string)

    if __call(1, prm_filename) != 0:
        raise RuntimeError("error: selfloop, check %s" % prm_filename)
    else:
        del_prm(prm_filename)


def trim(new_name: str, plant_name: str):
    if not Path(plant_name + DES_FILE_EXTENSION).exists():
        raise FileNotFoundError()

    prm_filename = "trim_%s.prm" % plant_name

    prm_string = "{name1}\n{name2}\n".format(name1=plant_name, name2=new_name)
    gen_prm(prm_filename, prm_string)

    if __call(2, prm_filename) != 0:
        raise RuntimeError("error: trim, check %s" % prm_filename)
    else:
        del_prm(prm_filename)


def printdes(new_name: str, plant_name: str):
    if not Path(plant_name + DES_FILE_EXTENSION).exists():
        raise FileNotFoundError()

    prm_filename = "print_%s.prm" % plant_name

    prm_string = "{name1}\n{name2}\n".format(name1=plant_name, name2=new_name)
    gen_prm(prm_filename, prm_string)

    if __call(3, prm_filename) != 0:
        raise RuntimeError("error: printdes, check %s" % prm_filename)
    else:
        del_prm(prm_filename)


def sync(new_plant: str, *plant_names: str):
    for plant_name in plant_names:
        if not Path(plant_name + DES_FILE_EXTENSION).exists():
            raise FileNotFoundError()

    prm_filename = "sync_%s.prm" % new_plant

    prm_string = "{name1}\n{num}\n{names}\n".format(
        name1=new_plant, num=len(plant_names), names="\n".join(plant_names)
    )
    gen_prm(prm_filename, prm_string)

    if __call(4, prm_filename) != 0:
        raise RuntimeError("error: sync, check %s" % prm_filename)
    else:
        del_prm(prm_filename)


def meet(new_plant: str, *plant_names: str):
    for plant_name in plant_names:
        if not Path(plant_name + DES_FILE_EXTENSION).exists():
            raise FileNotFoundError()

    prm_filename = "meet_%s.prm" % new_plant

    prm_string = "{name1}\n{num}\n{names}\n".format(
        name1=new_plant, num=len(plant_names), names="\n".join(plant_names)
    )
    gen_prm(prm_filename, prm_string)

    if __call(5, prm_filename) != 0:
        raise RuntimeError("error: meet, check %s" % prm_filename)
    else:
        del_prm(prm_filename)


def supcon(sup: str, plant: str, spec: str):
    for plant_name in [plant, spec]:
        if not Path(plant_name + DES_FILE_EXTENSION).exists():
            raise FileNotFoundError()

    prm_filename = "supcon_%s.prm" % sup

    prm_string = "{name1}\n{name2}\n{supervisor}\n".format(
        name1=plant, name2=spec, supervisor=sup
    )
    gen_prm(prm_filename, prm_string)

    if __call(6, prm_filename) != 0:
        raise RuntimeError("error: supcon, check %s" % prm_filename)
    else:
        del_prm(prm_filename)


def allevents(new_name: str, plant_name: str):
    if not Path(plant_name + DES_FILE_EXTENSION).exists():
        raise FileNotFoundError()

    prm_filename = "trim_%s.prm" % plant_name

    prm_string = "{name1}\n{name2}\n{entry}\n".format(
        name1=plant_name, name2=new_name, entry=1
    )
    gen_prm(prm_filename, prm_string)

    if __call(7, prm_filename) != 0:
        raise RuntimeError("error: allevents, check %s" % prm_filename)
    else:
        del_prm(prm_filename)


def mutex(name_3: str, plant_name: str, name_2: str, state_pair: List[tuple]):
    for name in [plant_name, name_2]:
        if not Path(name + DES_FILE_EXTENSION).exists():
            raise FileNotFoundError()

    prm_filename = "mutex_%s.prm" % plant_name
    state_pair_list = [f"{st[0]} {st[1]}" for st in state_pair]
    
    prm_string = "{name1}\n{name2}\n{name3}\n{statepair}".format(
        name1=plant_name,
        name2=name_2,
        name3=name_3,
        statepair=f"\n".join(state_pair_list) 
    )
    gen_prm(prm_filename, prm_string)

    if __call(8, prm_filename) != 0:
        raise RuntimeError("error: allevents, check %s" % prm_filename)
    else:
        del_prm(prm_filename)