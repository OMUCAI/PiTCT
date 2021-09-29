import re

class DatInfo:
    def __init__(self, text: str) -> None:
        self._text = text
        
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
                prohibit = [int(i) for i in prohibit]  # change int e.g. ([15, 13])

                control_data[state] = prohibit
        return control_data

    def __str__(self) -> str:
        return self._text

    def __repr__(self) -> str:
        return f'DatInfo(\n  text="{self._text}",\n  is_controllable={self.is_controllable},\n  ' \
               f'control_data={self.control_data}\n)'

    @property
    def control_data(self) -> dict:
        return self._extract_control_data()

    @property
    def is_controllable(self) -> bool:
        return self._extract_is_controllable()
    
    @property
    def text(self) -> str:
        return self._text
