import css_parser

element = css_parser.File.parse('''
    /* Hello! */
    selector {
        property: val;
        new-property: group(va;lue)value;
    };
''')

if element:
    print(element)