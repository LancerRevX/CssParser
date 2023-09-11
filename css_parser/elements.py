import re
from typing import Self
import os
from abc import ABC, abstractmethod

from css_parser.source import Source

from .source import Source
from .errors import ParseError


class Element(ABC):
    @abstractmethod
    def __init__(self, source: Source):
        pass

    @property
    @abstractmethod
    def source_text(self) -> str:
        pass

    @abstractmethod
    def __repr__(self, depth: int) -> str:
        pass

    @abstractmethod
    def __len__(self) -> int:
        pass


class Space(Element):
    def __init__(self, source: Source):
        match = re.match(r'^\s+', source.tail)
        if match is None:
            raise ParseError(Space, 'space element must consist only of space characters', source)

        self._source_text = match.group()
        self.pos = source.pos
        self.name = self.__class__.__name__
    

class Comment(Element):
    START_STR = '/*'
    END_STR = '*/'

    def __init__(self, source: Source):
        if not source.tail.startswith(self.START_STR):
            raise ParseError(Comment, 
                             f'comment must start with "{self.START_STR}"', 
                             source,
                             length=2)
        self.content = ''
        for i in range(source.pos, len(source)):
            if source.text[i:].startswith(self.END_STR):
                self.pos = source.pos
                return
            self.content += source.text[i]
        raise ParseError(Comment, 'Unmatched', source, length=2)

    @property
    def source_text(self) -> str:
        return self.START_STR + self.content + self.END_STR


class Selector(Element):
    def __init__(self, source: Source):
        



    
class RuleSet(Element):
    def __init__(self, source: Source):
        self.elements = []
        
        self.selector = source.parse(Selector)
        self.elements.append(self.selector)

        self.elements += source.find_spaces_and_comments()

        self.declaration_block = source.parse(DeclarationBlock)
        self.elements.append(self.declaration_block)


