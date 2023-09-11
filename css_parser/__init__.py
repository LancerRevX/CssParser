import re
from typing import Self
import os
from abc import ABC, abstractmethod

import universal_parser

class Element:
    @property
    @abstractmethod
    def source_text(self):
        pass


class Space(Element):
    def __init__(self, source_text: str, pos: int | tuple[int, int]):
        if type(pos) is int:
            pos = (pos, pos)
        match = re.match(r'^\s+', source_text[pos[0], pos[1]])
        if match is None:
            raise ParseError()
        self.space_text = source_text

    @property
    def source_text(self):
        return self.space_text


class File(universal_parser.File):
    ELEMENT_DEFINITIONS = [
        ElementDefinition([
            SPACES_AND_COMMENTS,
            ElementDefinition(AtRule, required=False, single=False),
            ElementDefinition(RuleSet, required=False, single=False)
        ], required=False, single=False),
    ]

    @classmethod
    def parse(cls, source_text: str) -> Self | None:
        elements = []
        for i in range(len(source_text)):
            if re.match(r'^\s', source_text[i:]):
                elements.append(Space.parse(source_text, i))
            elif source_text[i:].startswith('/*'):
                elements.append(Comment.parse(source_text, i))
            elif source_text[i:].startswith('@'):
                elements.append(AtRule.parse(source_text, i))
            elif re.match(r'^[\w#.]', source_text[i:]):
                elements.append(RuleSet.parse(source_text, i))
            else:
                raise ParseError()

    @property
    def comments(self):
        return self.get_elements_of_type(Comment, recursive=True)
    
class Project(universal_parser.Project):
    def __init__(self, elements: list[Element], folder_path: str | None = None):
        super(Project, self).__init__(elements)
        self.folder_path = folder_path
        self.name = f'{self.__class__.__name__} "{folder_path}'

    @classmethod
    def open(cls, folder_path: str):
        css_file_paths = cls.find_css_files(folder_path)
        files = []
        for file_path in css_file_paths:
            file = File.open(file_path)
            if file:
                files.append(file)
        return cls(files, folder_path)

    @classmethod
    def find_css_files(cls, folder_path: str) -> list[str]:
        css_file_paths = []
        for file in os.scandir(folder_path):
            if file.is_dir(follow_symlinks=False):
                css_file_paths += cls.find_css_files(file.path)
            elif os.path.splitext(file.path)[1] == '.css':
                css_file_paths.append(file.path)
        return css_file_paths