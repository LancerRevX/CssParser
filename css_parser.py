import argparse

import css

arg_parser = argparse.ArgumentParser(prog='CSS Parser',
                                     description='Parses CSS files',
                                     epilog='Hope it works')
arg_parser.add_argument('-f', '--file-path')
args = arg_parser.parse_args()

if args.file_path is None:
    arg_parser.print_help()
    exit(0)

file = css.File.open(args.file_path)

print(file)
