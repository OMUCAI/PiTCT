import umsgpack
import graphviz as gv
from pathlib import Path
from datetime import datetime
import tempfile

from pitct.name_converter import NameConverter
from .util import is_env_notebook
from .config import Config, DES_FILE_EXTENSION
import base64

BASE_HTML = '<img width="{}" src="data:image/svg+xml;base64,{}" >'

conf = Config.get_instance()

class AutomatonDisplay(object):
    def __init__(self, plant: str, convert: bool = True, color: bool = False):
        self.__path = Path(conf.SAVE_FOLDER / (plant + DES_FILE_EXTENSION))
        self.__byte = self.__path.read_bytes()
        self.__data = umsgpack.unpackb(self.__byte)

        states = self.__data["states"]

        self.__graph = gv.Digraph("finite_state_machine", strict=False)
        self.__graph.attr(rankdir="LR")

        if self.__data["size"] < 1:
            self.__graph.node("[empty]", color="white")
            return

        # add the initial entry state
        self.__graph.node("_a", shape="point", color="white")

        # add states
        for label, state in states.items():
            conv_label = str(NameConverter.state_decode(plant, label, convert))
            if state["marked"]:
                self.__graph.node(conv_label, shape="doublecircle")
            else:
                self.__graph.node(conv_label, shape="circle")

        # add the initial entry edge
        initial_state = str(NameConverter.state_decode(plant, 0, convert))
        self.__graph.edge("_a", initial_state)

        # add transitions
        for label in states:
            trans = states[label]["next"]
            if trans is not None:
                for tran in trans:
                    label_text = str(NameConverter.event_decode(tran[0], convert))
                    if color:
                        self.__graph.edge(
                            str(NameConverter.state_decode(plant, label, convert)),
                            str(NameConverter.state_decode(plant, tran[1], convert)),
                            label=label_text,
                            color="red" if tran[0] % 2 == 1 else "green",
                        )
                    else:
                        self.__graph.edge(
                            str(NameConverter.state_decode(plant, label, convert)),
                            str(NameConverter.state_decode(plant, tran[1], convert)),
                            label=label_text
                        )

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
        fileformat: str = 'png',
        layout: str = "dot",
        dpi: int = 300,
        label: str = None,
        timelabel: bool = True,
        **kwargs
    ):
        self.set_attr(layout=layout, dpi=dpi, label=label, timelabel=timelabel, **kwargs)

        path_filename = Path(conf.SAVE_FOLDER / filename)
        self.__graph.render(path_filename, format=fileformat, cleanup=True)

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
            with tempfile.NamedTemporaryFile(delete=False, suffix='.gv') as tmp:
                tmp_path = tmp.name
            return self.__graph.render(tmp_path, view=True, format=format, cleanup=True)

    def _repr_html_(self):
        svg = self.__graph._repr_svg_()
        # svg文字列をb64エンコードしてから埋め込み
        html = BASE_HTML.format("100%", base64.b64encode(svg.encode()).decode())
        return html

    def _repr_svg_(self):
        return self.__graph._repr_svg_()
