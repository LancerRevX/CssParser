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
    
    @property
    def tail(self):
        return self.text[self.pos:]
    
    def parse_element()
    
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
    def source_text(self):
        pass
    
    @abstractmethod
    def __len__(self):
        pass
    


@dataclass
class ElementDefinition:
    element: Type[Element] | list[ElementDefinition]
    required: bool
    single: bool


class BasicElement(Element):
    def __init__(self, source_text: str):
        self._source = source_text

    @property
    def source_text(self):
        return self._source

    def __len__(self):
        return len(self._source)


class RegexElement(BasicElement):
    @classmethod
    def parse(cls, source: Source):
        match = re.match(cls.regex(), source.tail)
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
    
class Colon(RegexElement):
    @staticmethod
    def regex():
        return r':'
    
class Semicolon(RegexElement):
    @staticmethod
    def regex():
        return r';'
    
class BlockStart(RegexElement):
    @staticmethod
    def regex():
        return r'{'
    
class BlockEnd(RegexElement):
    @staticmethod
    def regex():
        return r'}'


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
    def source_text(self):
        return ''.join(map(lambda element: element.source_text, self.elements))
    
    def __len__(self):
        return sum(map(lambda el: len(el), self.elements))
    
    @staticmethod
    @abstractmethod
    def element_declarations() -> list[ElementDefinition]:
        pass

    @staticmethod
    @abstractmethod
    def regex() -> re.Pattern:
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
        
            
class CommentStart(RegexElement):
    @staticmethod
    def regex():
        return r'/\*'
    
class CommentEnd(RegexElement):
    @staticmethod
    def regex():
        return r'\*/'
    
class CommentText(BasicElement):
    @classmethod
    def parse(cls, source: Source) -> CommentText | None:
        text = ''
        for i in range(start_pos, len(source) - 1):
            word = source[i:i+2]
            if word == CommentEnd.regex():
                break
            text += source[i]
        return cls(text)
    
class Comment(ComplexElement):
    @staticmethod
    def element_declarations() -> list[ElementDefinition]:
        return [
            ElementDefinition(CommentStart, required=True, single=True),
            ElementDefinition(CommentText, required=False, single=True),
            ElementDefinition(CommentEnd, required=True, single=True)
        ]
    
    @property
    def text(self):
        if isinstance(self.elements[1], CommentText):
            return self.elements[1].source_text
        else:
            return ''
            
SPACES_AND_COMMENTS = ElementDefinition([
    ElementDefinition(Space), 
    ElementDefinition(Comment)
], required=False, single=False)
    
class Selector(ComplexElement):
    @staticmethod
    def element_declarations() -> list[ElementDefinition]:
        return [
            ElementDefinition(SelectorItem, required=True, single=True),
            ElementDefinition([
                ElementDefinition(Comma, required=True, single=True),
                ElementDefinition(SelectorItem, required=True, single=True)
            ], required=False, single=False)
        ]
class DeclarationBlock(ComplexElement):
    @staticmethod
    def element_declarations() -> list[ElementDefinition]:
        return [
            ElementDefinition(BlockStart, required=True, single=True),
            SPACES_AND_COMMENTS,
            ElementDefinition(Declaration, required=False, single=True),
            SPACES_AND_COMMENTS,
            ElementDefinition([
                ElementDefinition(Semicolon, required=True, single=True),
                SPACES_AND_COMMENTS,
                ElementDefinition(Declaration, required=True, single=True),
                SPACES_AND_COMMENTS,
            ], required=False, single=False),
            ElementDefinition(BlockEnd, required=True, single=True)
        ]
    
    @property
    def vars(self):
        vars = []
        for element in self.elements:
            if isinstance(element, Declaration) and element.is_var():
                vars.append(element)
        return vars
            
class RuleSet(ComplexElement):
    @staticmethod
    def element_declarations() -> list[ElementDefinition]:
        return [
            ElementDefinition(Selector, required=True, single=True),
            ElementDefinition([
                ElementDefinition(Space), 
                ElementDefinition(Comment)
            ], required=False, single=False),
            ElementDefinition(DeclarationBlock, required=True, single=True),
        ]

class File(ComplexElement):
    def __init__(self, source_text: str):
        self.source_text = Source(source_text)

    @classmethod
    def open(file_path: str):
        pass

    @staticmethod
    def element_declarations() -> list[ElementDefinition]:
        return {
            {
                {
                    Space: {}, 
                    Comment: {}
                }: {'repeat': True},
                RuleSet: {}
            }: {'repeat': True}
        }
