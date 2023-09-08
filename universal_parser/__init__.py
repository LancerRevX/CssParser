import re
from abc import ABC, abstractmethod
from dataclasses import dataclass
from typing import Type, Self


class Element(ABC):
    @classmethod
    @abstractmethod
    def parse(cls, source_text: str) -> Self | None:
        pass

    @property
    @abstractmethod
    def source_text(self) -> str:
        pass

    @abstractmethod
    def __len__(self) -> int:
        pass

    @abstractmethod
    def __repr__(self, depth) -> str:
        pass


class ParseError(RuntimeError):
    def __init__(self, element_class: type[Element], message: str, source_text: str, pos: tuple[int, int] | int = None):
        super().__init__()

        if pos is None:
            pos = (0, len(source_text))

        if isinstance(pos, int):
            pos = (pos, pos + 1)

        self.pos = pos
        self.source_text = source_text
        self.message = message
        self.element_class = element_class

    def __str__(self) -> str:
        return f'Error while parsing element "{self.element_class.__name__}": {self.message}'


@dataclass
class ElementDefinition:
    element: Type[Element] | list[Self]
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
    
    def __repr__(self, depth=0) -> str:
        return '\t' * depth + self.__class__.__name__ + ': { "' + self._source + '" }'


class RegexElement(BasicElement):
    @classmethod
    def parse(cls, source_text: str) -> Self | None:
        match = re.match('^' + cls.regex, source_text)
        if match:
            return cls(match.group())
        else:
            return None

    @property
    @abstractmethod
    def regex() -> str:
        pass


class ComplexElement(Element):
    def __init__(self, elements: list[Element]):
        self.elements = elements

    @classmethod
    def parse(cls, source_text: str) -> Self | None:
        if len(source_text) == 0:
            return None
        elements = cls.parse_by_definitions(source_text, cls.get_element_definitions())
        if elements is None:
            return None
        else:
            return cls(elements)
    
    @classmethod
    def parse_by_definitions(cls, source_text: str, definitions: list[ElementDefinition]) -> list[Element] | None:
        result_elements = []
        parsed_length = 0
        for i, definition in enumerate(definitions):
            
            elements = cls.parse_by_definition(source_text, definition)

            if elements is None:
                if definition.required:
                    if i == 0:
                        return None
                    else:
                        raise ParseError(cls, f'Expected element {definition.element}', source_text, parsed_length)
                else:
                    continue
            else:
                result_elements += elements
                parsed_length += sum(map(len, elements))
        
        return result_elements
    
    @classmethod
    def parse_by_definition(cls, source_text: str, definition: ElementDefinition) -> list[Element] | None:
        if isinstance(definition.element, list):
            result_elements = cls.parse_by_definitions(source_text, definition.element)
        else:
            element = definition.element.parse(source_text)
            if element is None:
                return None
            else:
                result_elements = [element]
        
        if result_elements is None:
            return None
        elif definition.single:
            return result_elements
        else:
            parsed_length = sum(map(len, result_elements))
            while True:
                if isinstance(definition.element, list):
                    elements = cls.parse_by_definitions(source_text[parsed_length:], definition.element)
                else:
                    element = definition.element.parse(source_text[parsed_length:])
                    if element is None:
                        return result_elements
                    else:
                        elements = [element]
                if elements is None:
                    return result_elements
                else:
                    result_elements += elements
                    parsed_length += sum(map(len, elements))


    @staticmethod
    def parse_element(element_class: type[Element], source_text: str, *, single: bool) -> list[Element] | None:
        element = element_class.parse(source_text)
        if element is None:
            return None
        elif single:
            return [element]
        else:
            parsed_length = len(element)
            elements = [element]
            while True:
                element = element_class.parse(source_text[parsed_length:])
                if element is None:
                    return elements
                else:
                    elements.append(element)
                    parsed_length += len(element)


    @property
    def source_text(self):
        return ''.join(map(lambda element: element.source_text, self.elements))

    def __len__(self):
        return sum(map(lambda el: len(el), self.elements))

    @staticmethod
    @abstractmethod
    def get_element_definitions() -> list[ElementDefinition]:
        pass

    def __repr__(self, depth=0) -> str:
        result = '\t' * depth + self.__class__.__name__ + ': {'
        if len(self.elements) == 0:
            result += ' '
        else:
            result += '\n'
            for child_element in self.elements:
                result += child_element.__repr__(depth + 1) + '\n'
            result += '\t' * depth
        result += '}'
        return result


# универсальный интерфейс для поиска групп, ограниченных скобками
class BlockElement(ComplexElement):
    start_str = None
    end_str = None

    @classmethod
    def parse(cls, source_text: str) -> Self | None:
        if not source_text.startswith(cls.start_str):
            return None
        nested_delimiter = 0
        content = ''
        i = len(cls.start_str)
        while i < len(source_text):
            if source_text[i:].startswith(cls.start_str):
                nested_delimiter += 1
            elif source_text[i:].startswith(cls.end_str):
                if nested_delimiter > 0:
                    nested_delimiter -= 1
                else:
                    inside_elements = ComplexElement.parse_by_definitions(content, cls.get_element_definitions())
                    return cls(inside_elements)

            content += source_text[i]
            i += 1
        
        raise ParseError(cls, f'Unmatched block delimeter "{cls.start_str}"', source_text, (0, len(cls.start_str)))
    
    def __len__(self):
        return len(self.start_str) + super(BlockElement, self).__len__() + len(self.end_str)
    
    @property
    def source_text(self):
        return self.start_str + super(BlockElement, self).source_text + self.end_str
    
    @property
    def content(self):
        return super(BlockElement, self).source_text


class ListElement(ComplexElement):
    @staticmethod
    def get_element_definitions() -> list[ElementDefinition]:
        return []

    @property
    @abstractmethod
    def delimeter():
        pass

    @property
    @abstractmethod
    def element_class() -> type[Element]:
        pass

    @classmethod
    def parse(cls, source_text: str) -> Self | None:
        i = 0
        elements = []
        delimeter_positions = []
        while True:
            if source_text[i:].startswith(cls.delimeter):
                i += len(cls.delimeter)
                delimeter_positions.append(len(elements))
                continue
            else:
                element = cls.element_class.parse(source_text[i:])
                if element is None:
                    if len(elements) > 0:
                        list_element = cls(elements)
                        list_element.delimeter_positions = delimeter_positions
                        return list_element
                    else:
                        return None
                elements.append(element)
                i += len(element)


class File(ComplexElement):
    @classmethod
    def from_text(cls, source_text: str) -> Self | None:
        return cls.parse(source_text)

    @classmethod
    def open(cls, file_path: str):
        with open(file_path, 'w') as file:
            source_text = file.read()
            return cls(source_text)
