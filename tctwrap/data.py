import umsgpack
import graphviz as gv
from pathlib import Path
from datetime import datetime
import tempfile
from .util import is_env_notebook
from .config import Config
import base64

DES_FILE_EXTENSION = ".DES"
DAT_FILE_EXTENSION = ".DAT"
BASE_HTML = '<img width="{}" src="data:image/svg+xml;base64,{}" >'

conf = Config.get_instance()

class PlantDisplay(object):
    def __init__(self, plant: str, color: bool = False):
        self.__path = Path(conf.SAVE_FOLDER / (plant + DES_FILE_EXTENSION))
        print(self.__path)
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

    def set_attr(self,
        layout="dot",
        dpi=None,
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

        if dpi:
            self.__graph.attr("graph", dpi=str(dpi))
        
        self.__graph.attr("graph", layout=layout)
        if len(kwargs) > 0:
            self.__graph.attr("graph", **kwargs)

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
        self.set_attr(layout=layout, dpi=dpi, label=label, timelabel=timelabel, **kwargs)

        self.__graph.render(filename, format=fileformat, cleanup=True)

    def render(self,
        layout="dot",
        dpi=None,
        label=None,
        timelabel=True,
        format="png",
        **kwargs
    ):
        self.set_attr(layout=layout, dpi=dpi, label=label, timelabel=timelabel, **kwargs)
        if is_env_notebook():
            # Jupyter Environment
            return self
        else:
            # shell
            return self.__graph.render(tempfile.mktemp('.gv'), view=True, format=format, cleanup=True)

    def _repr_html_(self):
        svg = self.__graph._repr_svg_()
        # svg文字列をb64エンコードしてから埋め込み
        html = BASE_HTML.format("100%", base64.b64encode(svg.encode()).decode())
        return html
