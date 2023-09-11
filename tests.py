import unittest

import css_parser

class TestCssFile(unittest.TestCase):
    def test_space(self):
        space = css_parser.Space.parse(' \n\t')
        self.assertIsNotNone(space)
        if space is not None:
            self.assertEqual(
                space.source_text,
                ' \n\t'
            )

    def test_comment(self):
        comment = css_parser.Comment.parse('/*abc*/')
        self.assertIsNotNone(comment)
        if comment:
            self.assertEqual(
                comment.content,
                'abc'
            )
            self.assertEqual(
                len(comment),
                len('/*abc*/')
            )

    def test_rule_set(self):
        source = '''
            selector {
                property1: value1;
                property2: value(with-nested)group;
            }
        '''
        rule_set = css_parser.RuleSet.parse(source)
        self.assertIsNotNone(rule_set)
        if rule_set:
            self.assertEqual(len(rule_set), len(source))
            self.assertEqual(rule_set.source_text, source)
            self.assertEqual(len(rule_set.properties), 2)
            self.assertEqual(len(rule_set.selector), 1)
            self.assertEqual(rule_set.selector[0].source_text, 'selector')
            self.assertEqual(rule_set.properties[0].source_text, 'property1')
            self.assertEqual(rule_set.declarations[1].value, 'value(with-nested)group')

    def test_multiple_elements(self):
        source = '''
            /* this is
            a
            multiline comment
            */

            rule-set, with, multiple, selectors {
                property1: value1;
                property2: value(with-nested)group;
                /* a comment inside a rule set */
                some-url: url(https://yandex.ru)
            }

            /* another comment */
        '''
        file = css_parser.File.parse(source)
        self.assertIsNotNone(file)
        if file:
            self.assertEqual(file.source_text, source)

    def test_at_rule(self):
        source = '''
            /* this is
            a
            multiline comment
            */

            rule-set, with, multiple, selectors {
                property1: value1;
                property2: value(with-nested)group;
                /* a comment inside a rule set */
                some-url: url(https://yandex.ru)
            }

            @media (min-width: 300px) {
                property: 300px;

                nested, rule-set {
                    property1: value1;
                    property2: value(with-nested)group;
                    /* a comment inside a rule set */
                    some-url: url(https://yandex.ru)
                }
                
                another-property: 1;
                and-another-one: 2;

                one-more, rule-set {
                    property-10: value3;
                }
            }

            /* another comment */
        '''
        file = css_parser.File.parse(source)
        self.assertIsNotNone(file)
        if file:
            self.assertEqual(file.source_text, source)
            self.assertEqual(
                len(file.get_elements_of_type(css_parser.AtRule)),
                1
            )

    # def test_real_file(self):
    #     file_path = 'd:/projects/atribut-local/wp-content/themes/woodmart-child/style.css'
    #     with open(file_path, 'r', encoding='utf-8') as file:
    #         source_text = file.read()
    #     css_file = css_parser.File.open(file_path)
    #     self.assertIsNotNone(file)
    #     if css_file:
    #         self.assertEqual(css_file.source_text, source_text)


if __name__ == '__main__':
    unittest.main()