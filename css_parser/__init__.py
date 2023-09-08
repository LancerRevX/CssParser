import re
from typing import Self

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
    REGEX = r'.*'


class Comment(BlockElement):
    ELEMENT_DEFINITIONS = [
        ElementDefinition(PlainText, required=False, single=True)
    ]

    START_STR = '/*'
    END_STR = '*/'


SPACES_AND_COMMENTS = ElementDefinition([
    ElementDefinition(Space, required=False, single=False),
    ElementDefinition(Comment, required=False, single=False)
], required=False, single=False)


class SelectorItem(RegexElement):
    REGEX = r'[][ ^<>()\w-]+'


class Selector(ListElement):
    ELEMENT_CLASS = SelectorItem
    DELIMETER = ','


class Property(RegexElement):
    REGEX = r'[-a-zA-Z]+'




class ValueText(RegexElement):
    REGEX = r'[-\w ,]+'


class ValueTextParentheses(RegexElement):
    REGEX = r'[-\w; ,]+'


class ParenthesesGroup(BlockElement):
    START_STR = '('
    END_STR = ')'

    ELEMENT_DEFINITIONS = [
        ElementDefinition([
            ElementDefinition(ValueText, required=False, single=True),
        ], required=True, single=False)
    ]


class Value(ComplexElement):
    ELEMENT_DEFINITIONS = [
        ElementDefinition([
            ElementDefinition(ValueText, required=False, single=True),
            ElementDefinition(ParenthesesGroup, required=False, single=True),
            ElementDefinition(ValueText, required=False, single=True),
        ], required=True, single=False)
    ]


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
        ElementDefinition(Selector, required=True, single=True),
        SPACES_AND_COMMENTS,
        ElementDefinition(DeclarationBlock, required=True, single=True),
    ]


class File(universal_parser.File):
    ELEMENT_DEFINITIONS = [
        ElementDefinition([
            SPACES_AND_COMMENTS,
            ElementDefinition(RuleSet, required=False, single=False)
        ], required=False, single=False),
    ]
