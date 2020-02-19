import umsgpack
import graphviz as gv
from pathlib import Path
from datetime import datetime

DES_FILE_EXTENSION = ".DES"


class PlantDisplay(object):
    def __init__(self, plant: str, color: bool = False):
        self.__path = Path(plant + DES_FILE_EXTENSION)
        self.__byte = self.__path.read_bytes()
        self.__data = umsgpack.unpackb(self.__byte)

        states = self.__data["states"]

        self.__graph = gv.Digraph("finite_state_machine", strict=False)
        self.__graph.attr(rankdir="LR")

        if self.__data["size"] < 1:
            self.__graph.node("[empty]", color="white")
            return

        # add the initial entry state
        self.__graph.node("a", shape="point", color="white")

        # add states
        for label, state in states.items():
            if state["marked"]:
                self.__graph.node(str(label), shape="doublecircle")
            else:
                self.__graph.node(str(label), shape="circle")

        # add the initial entry edge
        self.__graph.edge("a", "0")

        # add transitions
        for label in states:
            trans = states[label]["next"]
            if trans is not None:
                for tran in trans:
                    if color:
                        self.__graph.edge(
                            str(label),
                            str(tran[1]),
                            label=str(tran[0]),
                            color="red" if tran[0] % 2 == 1 else "green",
                        )
                    else:
                        self.__graph.edge(str(label), str(tran[1]), label=str(tran[0]))


    def save(
        self,
        filename: str,
        fileformat: str,
        layout="dot",
        dpi=96,
        label=None,
        timelabel=True,
        **kwargs
    ):
        new_label = self.__path.name if label is None else str(label)

        if timelabel:
            self.__graph.attr(
                "graph",
                label="{}\n{}".format(new_label, datetime.now().isoformat(sep=" ")),
            )
        else:
            self.__graph.attr("graph", label=new_label)

        self.__graph.attr("graph", layout=layout, dpi=str(dpi))
        if len(kwargs) > 0:
            self.__graph.attr("graph", **kwargs)

        self.__graph.render(filename, format=fileformat, cleanup=True)
