import unittest

import css

class TestCssFile(unittest.TestCase):
    def test_space(self):
        source = ' \n\t'
        space = css.Space(source)
        self.assertEqual(
            space.source_text,
            source
        )

    def test_comment(self):
        source = '/*abc*/ '
        comment = css.Comment(source)
        self.assertEqual(
            comment.source_text,
            source
        )
        self.assertEqual(
            len(comment),
            len(source)
        )

        # source = css.Source('/*abc*/   ')
        # comment = css.Comment(source)
        # self.assertEqual(
        #     comment.source_text,
        #     '/*abc*/'
        # )
        # self.assertEqual(
        #     len(comment),
        #     len('/*abc*/')
        # )
        # self.assertEqual(
        #     len(source.tail),
        #     3
        # )

    # def test_selector_list(self):
    #     source = css.Source('abc, efg')
    #     selector_list = css.SelectorList(source)
    #     self.assertEqual(
    #         len(selector_list.selectors),
    #         2
    #     )
    #     self.assertEqual(
    #         len(selector_list.child_elements),
    #         4
    #     )
    #     self.assertEqual(selector_list.selectors[0].source_text, 'abc')
    #     self.assertEqual(selector_list.selectors[1].source_text, 'efg')

    def test_declaration_block(self):
        source = 'abc: defg; property: value'
        declaration_block = css.DeclarationBlock(source)
        self.assertEqual(
            len(declaration_block.declarations),
            2
        )
        self.assertEqual(
            declaration_block.declarations[0].property,
            'abc'
        )
        self.assertEqual(
            declaration_block.declarations[1].value,
            'value'
        )

    # def test_rule_set(self):
    #     source = '''
    #         selector {
    #             property1: value1;
    #             property2: value(with-nested)group;
    #         }
    #     '''
    #     rule_set = css.RuleSet.parse(source)
    #     self.assertIsNotNone(rule_set)
    #     if rule_set:
    #         self.assertEqual(len(rule_set), len(source))
    #         self.assertEqual(rule_set.source_text, source)
    #         self.assertEqual(len(rule_set.properties), 2)
    #         self.assertEqual(len(rule_set.selector), 1)
    #         self.assertEqual(rule_set.selector[0].source_text, 'selector')
    #         self.assertEqual(rule_set.properties[0].source_text, 'property1')
    #         self.assertEqual(rule_set.declarations[1].value, 'value(with-nested)group')

    # def test_multiple_elements(self):
    #     source = '''
    #         /* this is
    #         a
    #         multiline comment
    #         */

    #         rule-set, with, multiple, selectors {
    #             property1: value1;
    #             property2: value(with-nested)group;
    #             /* a comment inside a rule set */
    #             some-url: url(https://yandex.ru)
    #         }

    #         /* another comment */
    #     '''
    #     file = css.File.parse(source)
    #     self.assertIsNotNone(file)
    #     if file:
    #         self.assertEqual(file.source_text, source)

    # def test_at_rule(self):
    #     source = '''
    #         /* this is
    #         a
    #         multiline comment
    #         */

    #         rule-set, with, multiple, selectors {
    #             property1: value1;
    #             property2: value(with-nested)group;
    #             /* a comment inside a rule set */
    #             some-url: url(https://yandex.ru)
    #         }

    #         @media (min-width: 300px) {
    #             property: 300px;

    #             nested, rule-set {
    #                 property1: value1;
    #                 property2: value(with-nested)group;
    #                 /* a comment inside a rule set */
    #                 some-url: url(https://yandex.ru)
    #             }
                
    #             another-property: 1;
    #             and-another-one: 2;

    #             one-more, rule-set {
    #                 property-10: value3;
    #             }
    #         }

    #         /* another comment */
    #     '''
    #     file = css.File.parse(source)
    #     self.assertIsNotNone(file)
    #     if file:
    #         self.assertEqual(file.source_text, source)
    #         self.assertEqual(
    #             len(file.get_elements_of_type(css.AtRule)),
    #             1
    #         )

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