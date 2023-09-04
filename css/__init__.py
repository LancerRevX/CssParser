import re
from abc import ABC, abstractmethod
from dataclasses import dataclass
from typing import Type

class ParseError(RuntimeError):
    def __init__(self, source: str, pos: tuple[int, int]):
        super().__init__()


class Source:
    def __init__(self, text: str):
        self.text = text
        self.pos = 0

    @property
    def current_pos(self):
        return self.pos
    
    def move(self, chars_number: int):
        self.pos += chars_number

    def roll_back(self, chars_number: int):
        self.pos -= chars_number


class Element(ABC):
    @classmethod
    @abstractmethod
    def parse(cls, source, start_pos):
        pass

    @property
    @abstractmethod
    def source(self):
        pass
    
    @abstractmethod
    def __len__(self):
        pass
    


@dataclass
class ElementDeclaration:
    element_class: Type[Element]
    required: bool
    many: bool


class BasicElement(Element):
    def __init__(self, source: str):
        self._source = source

    @property
    def source(self):
        return self._source

    def __len__(self):
        return len(self._source)


class RegexElement(BasicElement):
    @classmethod
    def parse(cls, source: str, start_pos: int):
        match = re.match(cls.regex(), source)
        if match:
            return cls(match.group)
        else:
            return None
        
    @staticmethod
    @abstractmethod
    def regex():
        pass


class Space(RegexElement):
    @staticmethod
    def regex():
        return r'\s+'
    
class Comma(RegexElement):
    @staticmethod
    def regex():
        return r','


class ComplexElement(Element):
    def __init__(self, elements: list[Element]):
        self.elements = elements

    @classmethod
    def parse(cls, source: Source):
        elements = []
        for element_declaration in cls.element_declarations():
            element = element_declaration.element_class.parse(Source)
            if element:
                elements.append(element)
                while element_declaration.many and element:
                    element = element_declaration.element_class.parse(Source)
                    if element:
                        elements.append(element)
            elif element_declaration.required:
                parsed_length = sum(map(len, elements))
                source.roll_back(parsed_length)
                return None
        return elements
                

    @property
    def source(self):
        return ''.join(map(lambda element: element.source, self.elements))
    
    def __len__(self):
        return sum(map(lambda el: len(el), self.elements))
    
    @staticmethod
    @abstractmethod
    def element_declarations() -> list[ElementDeclaration]:
        pass

    @property
    def comments(self):
        # TODO all nested comments
        comments = filter(lambda el: isinstance(el, Comment), self.elements)
        return comments

class Comment(BasicElement):
    @classmethod
    def parse(cls, source: str, start_pos: int):
        if not re.match('^/\*'):
            return None
        text = ''
        for i in range(start_pos, len(source) - 1):
            word = source[i:i+2]
            if word == '*/':
                return cls(source[0:i+2], text)
            elif word == '/*':
                raise ParseError()
            
class SpaceOrComment(BasicElement):
    @classmethod
    def parse(cls, source: Source):
        return Space.parse(source) or Comment.parse(source)
            
class RuleSet(ComplexElement):
    @staticmethod
    def element_declarations() -> list[ElementDeclaration]:
        return [
            ElementDeclaration(element_class=SpaceOrComment,
                               required=False,
                               many=True)
        ]
                

class File(ComplexElement):
    def __init__(self, source_text: str):
        self.source = Source(source_text)

    @classmethod
    def open(file_path: str):
        pass

    @staticmethod
    def element_declarations() -> list[ElementDeclaration]:
        return [
            ElementDeclaration(element_class=SpaceOrComment,
                               required=False,
                               many=True),
            ElementDeclaration(element_class=RuleSet,
                               required=False,
                               many=True),
            ElementDeclaration(element_class=SpaceOrComment,
                               required=False,
                               many=True)
        ]
