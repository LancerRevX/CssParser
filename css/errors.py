from .source import Source


class ParseError(RuntimeError):
    def __init__(self, element_name: str, message: str, source: str, pos=0, length=1):
        super().__init__()
        self.element_name = element_name
        self.message = message
        self.source = source
        self.pos = pos
        self.length = length

    def __str__(self) -> str:
        result = f'Error while parsing element "{self.element_name}": {self.message}\n'
        result += self.source + '\n'
        result += (' ' * self.pos) + ('^' * self.length)
        return result
        
