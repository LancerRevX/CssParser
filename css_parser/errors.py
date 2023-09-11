from .elements import Element
from .source import Source


class ParseError(RuntimeError):
    def __init__(self, element_class: type[Element], message: str, source: Source, length=None):
        super().__init__()
        self.element_class = element_class
        self.message = message
        self.source_text = source_text
        self.pos = pos
        self.length = length

    def __repr__(self) -> str:
        return f'Error while parsing element {self.element_class.__name__}'