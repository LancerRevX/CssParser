import unittest

import css

class TestCssElements(unittest.TestCase):
    def test_space(self):
        self.assertEqual(
            css.Space.parse(' \n\t').source_text,
            ' \n\t'
        )

if __name__ == '__main__':
    unittest.main()