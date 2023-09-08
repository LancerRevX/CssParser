import css_parser

element = css_parser.File.parse('/*abc*/ ')

print(element)