import css_parser

element = css_parser.File.parse('''
    selector {
        property: value;
    }
''')

print(element)