import re
from typing import Self
import os
from abc import ABC, abstractmethod

from css.source import Source
from css.errors import ParseError

TAB_CHAR = '\t'


class Element(ABC):
    def __init__(self, source: str, start: int, end: int) -> None:
        self.source = source
        self.start = start
        self.end = end

    @property
    @abstractmethod
    def source_text(self) -> str:
        pass

    @property
    def name(self) -> str:
        return self.__class__.__name__

    @abstractmethod
    def __repr__(self, depth=0) -> str:
        pass

    @abstractmethod
    def __len__(self) -> int:
        pass

    @abstractmethod
    def parse(self) -> Self:
        pass

    def raiseParseError(self, message: str, pos: int, length=1):
        raise ParseError(self.name,
                         message,
                         self.source,
                         pos, length)


class SimpleElement(Element):
    def __init__(self, source=''):
        super(SimpleElement, self).__init__()
        self._source_text = source

    @property
    def source_text(self) -> str:
        return self._source_text

    def __len__(self) -> int:
        return len(self._source_text)

    def __repr__(self, depth=0) -> str:
        return (TAB_CHAR * depth) + self.name + f'"{self._source_text}"'
    
    def parse(self):
        return self


class ComplexElement(Element):
    @abstractmethod
    def __init__(self):
        super(ComplexElement, self).__init__()
        self.child_elements: list[Element] = []

    @property
    def source_text(self) -> str:
        return ''.join(map(
            lambda child: child.source_text, self.child_elements
        ))

    def __len__(self) -> int:
        return sum(map(len, self.child_elements))

    def __repr__(self, depth=0) -> str:
        result = (TAB_CHAR * depth) + self.name + '{'
        if len(self.child_elements) > 0:
            result += '\n'
            for child in self.child_elements:
                result += child.__repr__(depth + 1)
            result += '\n'
        else:
            result += ' '
        result += '}'
        return result


class Space(SimpleElement):
    def parse(self, source: str):
        match = re.match(r'^\s+$', source)
        if match is None:
            raise ParseError(self.name,
                             'space element must consist only of space characters',
                             source)

        super(Space, self).__init__(source)

    @classmethod
    def from_start(cls, source: str) -> Self | None:
        match = re.match(r'^\s+', source)
        if match:
            return cls(match.group())
        else:
            return None


class Comment(SimpleElement):
    START_STR = '/*'
    END_STR = '*/'

    def __init__(self, source: str):
        if not source.startswith(self.START_STR):
            raise ParseError(self.name,
                             f'comment must start with "{self.START_STR}"',
                             source,
                             0, len(self.START_STR))

        if not source.endswith(self.END_STR):
            raise ParseError(self.name,
                             f'comment must end with "{self.END_STR}"',
                             source,
                             -len(self.END_STR), len(self.END_STR))

        for i in range(len(self.START_STR), len(source) - len(self.END_STR)):
            if source[i:].startswith(self.END_STR):
                raise ParseError(self.name,
                                 'found nested comment',
                                 source,
                                 i, len(self.END_STR))

        super(Comment, self).__init__(source)

    @classmethod
    def from_start(cls, source: str) -> Self | None:
        if not source.startswith(cls.START_STR):
            return None

        i = 0
        while i < len(source):
            if source[i:].startswith(cls.END_STR):
                return cls(source[0:i + len(cls.END_STR)])

        return None


def find_spaces_and_comments(source: str) -> list[Space | Comment]:
    spaces_and_comments = []
    i = 0
    while i < len(source):
        space = Space.from_start(source[i:])
        if space:
            spaces_and_comments.append(space)
            i += len(space)

        comment = Comment.from_start(source[i:])
        if comment:
            spaces_and_comments.append(comment)
            i += len(comment)

        if space is None and comment is None:
            break
    return spaces_and_comments


class Comma(SimpleElement):
    def parse(self, source: Source):
        if not source.tail.startswith(','):
            raise ParseError(self.name, 'Expected a ","', source)
        self._source_text = source.tail[0]


class Semicolon(SimpleElement):
    def parse(self, source: Source):
        if not source.tail.startswith(';'):
            raise ParseError(self.name, 'Expected a ";"', source)
        self._source_text = source.tail[0]


class SelectorList(ComplexElement):
    def __init__(self, source: Source):
        self.selectors: list[Selector] = []
        super(SelectorList, self).__init__(source)

    def parse(self, source: Source):
        first_selector = Selector(source)
        self.selectors.append(first_selector)
        self.child_elements.append(first_selector)

        self.child_elements += find_spaces_and_comments(source)

        while source.tail.startswith(','):

            self.child_elements.append(Comma(source))

            self.child_elements += find_spaces_and_comments(source)

            next_selector = Selector(source)
            self.selectors.append(next_selector)
            self.child_elements.append(next_selector)

            self.child_elements += find_spaces_and_comments(source)


class Selector(SimpleElement):
    def parse(self, source: Source):
        match = re.match(
            r'([a-z#.][][\w-]+)(\s+[a-z#.][][\w-]+)*',
            source.tail)
        if not match:
            raise ParseError(self.name, 'Expected a selector', source)
        self._source_text = match.group()


class Declaration(ComplexElement):
    def __init__(self, source: str):
        super(Declaration, self).__init__()

        if ':' not in source:
            raise ParseError(self.name,
                             'Expected ":" to divide property and value',
                             source,
                             0, len(source))

        self.property, self.value = source.split(':')

        if not re.search(r'\S', self.property):
            raise ParseError(self.name,
                             'Property is empty',
                             source,
                             0, len(self.property))
        self.child_elements.append(SimpleElement(self.property))

        spaces_and_comments = find_spaces_and_comments(self.value)
        self.child_elements += spaces_and_comments
        self.value = self.value[sum(map(len, spaces_and_comments)):]

        if not re.search(r'\S+', self.value):
            raise ParseError(self.name,
                             'Value is empty',
                             source,
                             len(self.property) + 1, len(self.value))
        self.child_elements.append(SimpleElement(self.value))


class DeclarationBlock(ComplexElement):
    START_STR = '{'
    END_STR = '}'

    def __init__(self, source: str):
        super(DeclarationBlock, self).__init__()

        self.declarations: list[Declaration] = []

        # if not source.startswith(self.START_STR):
        #     raise ParseError(self.name,
        #                      f'Expected "{self.START_STR}" at the start of a declaration block',
        #                      source,
        #                      0, len(self.START_STR))
        # self.child_elements.append(SimpleElement(self.START_STR))
        # source = source[len(self.START_STR):]

        # if not source.endswith(self.END_STR):
        #     raise ParseError(self.name,
        #                      f'Expected "{self.END_STR}" at the end of a declaration block',
        #                      source,
        #                      -len(self.END_STR), len(self.END_STR))
        # source = source[:-len(self.END_STR)]

        declaration_source = ''
        parentheses = 0
        last_parenthesis_i = 0
        i = 0
        while i < len(source):
            if declaration_source == '':
                spaces_and_comments = find_spaces_and_comments(source[i:])
                self.child_elements += spaces_and_comments
                i += sum(map(len, spaces_and_comments))

            if source[i:].startswith(';') and parentheses == 0:
                if declaration_source == '':
                    raise ParseError(self.name,
                                     'Empty declaration',
                                     source,
                                     i, 1)

                self.child_elements.append(SimpleElement(';'))

                declaration = Declaration(declaration_source)
                self.declarations.append(declaration)
                self.child_elements.append(declaration)
                declaration_source = ''
                i += 1

                continue

            if source[i:].startswith('('):
                parentheses += 1
                last_parenthesis_i = i
            elif source[i:].startswith(')'):
                if parentheses == 0:
                    raise ParseError(self.name,
                                     f'Unmatched ")"',
                                     source,
                                     i, 1)
                else:
                    parentheses -= 1

            declaration_source += source[i]
            i += 1

        if parentheses > 0:
            raise ParseError(self.name,
                             f'Unmatched "("',
                             source,
                             last_parenthesis_i, 1)

        if declaration_source != '':
            declaration = Declaration(declaration_source)
            self.declarations.append(declaration)
            self.child_elements.append(declaration)


class Selector(ComplexElement):
    pass


class SelectorList(ComplexElement):
    def __init__(self):
        self.selectors: list[Selector] = []
        super(SelectorList, self).__init__()

    def parse(self):
        self.selectors = []

        selector_source = ''
        expected_selector = True
        i = 0
        while i < len(self.source):
            if self.source[i] == ',':
                if expected_selector:
                    self.raiseParseError('Expected selector', i)
                else:
                    expected_selector = True
                    self.child_elements.append(SimpleElement(self.source[i]))
                    i += 1
                    continue
            elif



class RuleSet(ComplexElement):
    def __init__(self, source: str):
        self.selector_list = None
        self.declaration_block = None
        super(RuleSet, self).__init__(source)

    def parse(self):
        self.selector_list = None
        self.declaration_block = None

        selector_list_source = ''
        i = self.start
        while i < self.end:
            if self.source[i] == '{':
                break
            else:
                selector_list_source += self.source[i]
                i += 1
        if selector_list_source == '':
            self.raiseParseError('Expected selector list', i)
        else:
            self.selector_list = SelectorList(selector_list_source).parse()
            self.child_elements.append(self.selector_list)

        declaration_block_source = ''
        while i < self.end:
            declaration_block_source += self.source[i]
            if self.source[i] == '}':
                break
            else:
                i += 1
        if declaration_block_source == '':
            self.raiseParseError('Missing declaration block', i)
        else:
            self.declaration_block = DeclarationBlock(declaration_block_source).parse()
            self.child_elements.append(self.declaration_block)

        if self.source[i:] != '':
            self.raiseParseError('Unexpected character', i)


# class File(ComplexElement):
#     def parse(self):
#         self.child_elements = []

#         i = pos
#         while i < len(self.source):
#             spaces_and_comments = find_spaces_and_comments(self.source, i)
#             if len(spaces_and_comments) > 0:
#                 self.child_elements += spaces_and_comments
#                 i += sum(map(len, spaces_and_comments))
#                 continue

#             elif self.source[i:].startswith('@'):
#                 at_rule = AtRule(self.source).parse(i)
#                 self.at_rules.append(at_rule)
#                 self.child_elements.append(at_rule)
#                 i += len(at_rule)
#                 continue

#             else:
#                 rule_set = RuleSet(self.source).parse(i)
#                 self.rule_sets.append(rule_set)
#                 self.child_elements.append(rule_set)
#                 i += len(rule_set)
#                 continue

#     def find_rule_set


# class RuleSet(Element):
#     def __init__(self, source: Source):
#         self.elements = []

#         self.selector = source.parse(Selector)
#         self.elements.append(self.selector)

#         self.elements += source.find_spaces_and_comments()

#         self.declaration_block = source.parse(DeclarationBlock)
#         self.elements.append(self.declaration_block)
