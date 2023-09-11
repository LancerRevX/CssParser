import re

from .elements import Element, Space, Comment


class Source:
    def __init__(self, text: str, pos=0) -> None:
        self.text = text
        self.pos = pos

    @property
    def tail(self):
        return self.text[self.pos:]
    
    def parse(self, element_class: type[Element]):
        element = element_class(self)
        self.pos += len(element)
        return element
    
    def __len__(self):
        return len(self.text)
    
    def find_spaces_and_comments(self) -> list[Space | Comment]:
        spaces_and_comments = []
        while self.tail:
            if re.match(r'^\s', self.tail):
                self.parse(Space)
            elif self.tail.startswith(Comment.START_STR):
                self.parse(Comment)
            else:
                break
        return spaces_and_comments