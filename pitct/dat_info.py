import re

from pitct.name_converter import NameConverter

class DatInfo:
    def __init__(self, text: str, convert: bool = True) -> None:
        self._text = text  # original text 
        self._convert = convert
        self._convertd_text = self._convert_text(text)  # converted text
        
    def _extract_is_controllable(self):
        splited = self._text.split('\n')

        # Extract Controllable 
        not_controllable = "NOT CONTROLLABLE" in splited[7]
        is_controllable = not not_controllable

        return is_controllable
    
    def _extract_control_data(self):
        splited = self._text.split('\n')

        control_data_rawlist = splited[12:-2]
        control_data = {}
        for line in control_data_rawlist:
            result = re.findall(r'\d+:\s*\d+\s{1,4}\d*', line)  # e.g. ['0:  15   13', '3:  11']
            for one in result:
                extruct_data = one.split(':')  # e.g. (['0', '  15   13 '])
                state = int(extruct_data[0])  # e.g. (0)
                prohibit_raw = extruct_data[1]  # e.g. ('  15   13 ')
                prohibit = re.sub('(^\s*|\s*$)', '', prohibit_raw)  # remove start and end space e.g. ('15   13')
                prohibit = re.sub('\s+', ',', prohibit)  # replace space to , e.g. ('15,13')
                prohibit = prohibit.split(',')  # split by ',' (e.g. '15,13' -> ['15','13'])
                prohibit = [NameConverter.event_decode(int(e), convert=self._convert) for e in prohibit]  # change int e.g. ([15, 13]) and change string event
                control_data[state] = prohibit
        return control_data
    
    def _convert_text(self, text: str) -> str:
        if self._convert:
            txt = text.split("control data:")
            result = txt[0]
            result += "control data:\n\n"
            for state, prohibit in self.control_data.items():  # state: State(int|str), prohibit: Event(int|str)
                prohibit_str = [str(p) for p in prohibit]
                result += f"{state}:   {'    '.join(prohibit_str)}\n"
            result += "\n"
            return result
        else:
            return text

    def __str__(self) -> str:
        return self.text

    def __repr__(self) -> str:
        return f'DatInfo(\n  text="{self.text}",\n  is_controllable={self.is_controllable},\n  ' \
               f'control_data={self.control_data}\n)'


    @property
    def control_data(self) -> dict:
        return self._extract_control_data()

    @property
    def is_controllable(self) -> bool:
        return self._extract_is_controllable()
    
    @property
    def text(self) -> str:
        return self._convertd_text
