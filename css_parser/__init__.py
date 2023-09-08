import re
from typing import Self

import universal_parser
from universal_parser import (
    Element, BasicElement, RegexElement, ComplexElement, BlockElement, ListElement, ElementDefinition
)


class Space(RegexElement):
    regex = r'\s+'


class Comma(RegexElement):
    regex = r','


class Colon(RegexElement):
    regex = r':'


class Semicolon(RegexElement):
    regex = r';'


class PlainText(RegexElement):
    regex = r'.*'


class Comment(BlockElement):
    def get_element_definitions():
        return [
            ElementDefinition(PlainText, required=False, single=True)
        ]

    start_str = '/*'
    end_str = '*/'


SPACES_AND_COMMENTS = ElementDefinition([
    ElementDefinition(Space, required=False, single=False),
    ElementDefinition(Comment, required=False, single=False)
], required=False, single=False)


class SelectorItem(RegexElement):
    regex = r'[][ ^<>()\w-]+'


class Selector(ListElement):
    element_class = SelectorItem
    delimeter = ','


class Declaration(ComplexElement):
    def get_element_definitions():
        return [

        ]


class DeclarationList(ListElement):
    element_class = Declaration
    delimeter = ';'


class DeclarationBlock(BlockElement):
    start_str = '{'
    end_str = '}'

    def get_element_definitions():
        return [
            ElementDefinition(DeclarationList, required=False, single=True)
        ]

    @property
    def vars(self):
        vars = []
        for element in self.elements:
            if isinstance(element, Declaration) and element.is_var():
                vars.append(element)
        return vars


class RuleSet(ComplexElement):
    def get_element_definitions():
        return [
            ElementDefinition(Selector, required=True, single=True),
            SPACES_AND_COMMENTS,
            ElementDefinition(DeclarationBlock, required=True, single=True),
        ]


class File(universal_parser.File):
    def get_element_definitions():
        return [
            ElementDefinition([
                SPACES_AND_COMMENTS,
                ElementDefinition(RuleSet, required=False, single=False)
            ], required=False, single=False),
        ]
