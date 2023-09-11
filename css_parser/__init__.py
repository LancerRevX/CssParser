import re
from typing import Self, type
import os
from abc import ABC, abstractmethod

from .source import Source
from .elements import Element

TAB_CHARACTER = '\t'


class BasicElement(Element):
    @property
    def source_text(self) -> str:
        return self._source_text
    
    def __repr__(self, depth: int) -> str:
        return TAB_CHARACTER * depth + f'{self.name}: "{self._source_text}"'




                





class File:

    @classmethod
    def parse(cls, source_text: str) -> Self | None:
        source = Source(source_text, 0)
        elements = []
        while source.tail:
            spaces_and_comments = find_spaces_and_comments(source)
            if spaces_and_comments:
                elements += spaces_and_comments
                continue

            if source.tail.startswith('@'):
                elements.append(AtRule(source))
            elif re.match(r'^[\w#.]', source.tail):
                elements.append(RuleSet(source))
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