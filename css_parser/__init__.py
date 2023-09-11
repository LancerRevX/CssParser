import re
from typing import Self
import os

import universal_parser
from universal_parser import (
    Element, BasicElement, RegexElement, ComplexElement, BlockElement, ListElement, ElementDefinition
)


class Space(RegexElement):
    REGEX = r'\s+'


class Comma(RegexElement):
    REGEX = r','


class Colon(RegexElement):
    REGEX = r':'


class Semicolon(RegexElement):
    REGEX = r';'


class PlainText(RegexElement):
    REGEX = r'[\s\S]*'


class Comment(BlockElement):
    ELEMENT_DEFINITIONS = [
        ElementDefinition(PlainText, required=False, single=True)
    ]

    START_STR = '/*'
    END_STR = '*/'

    def __repr__(self, depth=0) -> str:
        return '\t' * depth + self.__class__.__name__ + ': /*' + self.content + '*/'


SPACES_AND_COMMENTS = ElementDefinition([
    ElementDefinition(Space, required=False, single=False),
    ElementDefinition(Comment, required=False, single=False)
], required=False, single=False)


class SelectorItem(RegexElement):
    REGEX = r'[][\n@ ^<>()\w:*="#_.-]*[]@[^<>().:\w#_-]+'


class Selector(ListElement):
    ELEMENT_CLASS = SelectorItem
    DELIMETER = ','


class Property(RegexElement):
    REGEX = r'[-a-zA-Z0-9]+'




class ValueText(RegexElement):
    REGEX = r'[-\'\\\/\w.:!#_ ,%]+'


class ValueTextParentheses(RegexElement):
    REGEX = r'[-\'\\\/\w.:; ,%]+'


class ParenthesesGroup(BlockElement):
    START_STR = '('
    END_STR = ')'

    ELEMENT_DEFINITIONS = [
        ElementDefinition(ValueTextParentheses, required=True, single=True),
    ]


class QuotedValue(BlockElement):
    START_STR = '"'
    END_STR = '"'

    ELEMENT_DEFINITIONS = [
        ElementDefinition(ValueText, required=False, single=True)
    ]


class Value(ComplexElement):
    ELEMENT_DEFINITIONS = [
        ElementDefinition([
            ElementDefinition(ValueText, required=False, single=True),
            ElementDefinition(QuotedValue, required=False, single=True),
            ElementDefinition(ParenthesesGroup, required=False, single=True),
            ElementDefinition(ValueText, required=False, single=True),
        ], required=True, single=False)
    ]

    def __repr__(self, depth=0) -> str:
        return '\t' * depth + self.__class__.__name__ + ': "' + self.source_text + '"'


class Declaration(ComplexElement):
    ELEMENT_DEFINITIONS = [
        SPACES_AND_COMMENTS,
        ElementDefinition(Property, required=True, single=True),
        SPACES_AND_COMMENTS,
        ElementDefinition(Colon, required=True, single=True),
        SPACES_AND_COMMENTS,
        ElementDefinition(Value, required=True, single=True),
        SPACES_AND_COMMENTS
    ]

    @property
    def value(self) -> str:
        return self.get_elements_of_type(Value)[0].source_text
    
    @property
    def property(self) -> str:
        return self.get_elements_of_type(Property)[0].source_text



class DeclarationList(ListElement):
    ELEMENT_CLASS = Declaration
    DELIMETER = ';'


class DeclarationBlock(BlockElement):
    START_STR = '{'
    END_STR = '}'

    ELEMENT_DEFINITIONS = [
        SPACES_AND_COMMENTS,
        ElementDefinition(DeclarationList, required=False, single=True),
        SPACES_AND_COMMENTS
    ]

    @property
    def vars(self):
        vars = []
        for element in self.elements:
            if isinstance(element, Declaration) and element.is_var():
                vars.append(element)
        return vars


class RuleSet(ComplexElement):
    ELEMENT_DEFINITIONS = [
        SPACES_AND_COMMENTS,
        ElementDefinition(Selector, required=True, single=True),
        SPACES_AND_COMMENTS,
        ElementDefinition(DeclarationBlock, required=True, single=True),
        SPACES_AND_COMMENTS,
    ]

    @property
    def declarations(self) -> list[Declaration]:
        return self.get_elements_of_type(Declaration, recursive=True)

    @property
    def properties(self):
        return self.get_elements_of_type(Property, recursive=True)
    
    @property
    def selector(self):
        return self.get_elements_of_type(SelectorItem, recursive=True)


class AtRuleSelector(RegexElement):
    REGEX = r'@[][\n ^<>()\w:*="#_.-]*[]@[^<>().:\w#_-]+'


class AtRuleBlock(BlockElement):
    START_STR = '{'
    END_STR = '}'

    ELEMENT_DEFINITIONS = [
        ElementDefinition([
            ElementDefinition(RuleSet, required=False, single=False),
            SPACES_AND_COMMENTS,
            ElementDefinition(DeclarationList, required=False, single=False),
            SPACES_AND_COMMENTS,
        ], required=False, single=False)
    ]


class AtRule(ComplexElement):
    ELEMENT_DEFINITIONS = [
        SPACES_AND_COMMENTS,
        ElementDefinition(AtRuleSelector, required=True, single=True),
        SPACES_AND_COMMENTS,
        ElementDefinition(AtRuleBlock, required=True, single=True),
        SPACES_AND_COMMENTS
    ]


class File(universal_parser.File):
    ELEMENT_DEFINITIONS = [
        ElementDefinition([
            SPACES_AND_COMMENTS,
            ElementDefinition(AtRule, required=False, single=False),
            ElementDefinition(RuleSet, required=False, single=False)
        ], required=False, single=False),
    ]

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
