use v5.32;
use warnings;

use Test;
use File::Temp qw(tempfile);

require './css_parser.pm';

my $test_id = $ARGV[1];

BEGIN {
    plan tests => 1
}

do {
    my ($css_file, $css_file_path) = tempfile;
    my $css = '/* this is my comment */';
    print $css_file $css;
    close $css_file;

    my $parser_file = new File($css_file_path);
    ok($parser_file->get_comments == 1);
};

do {
    my ($css_file, $css_file_path) = tempfile;
    my $css = '/* this is a broken comment */ */';
    print $css_file $css;
    close $css_file;

    eval { my $parser_file = new File($css_file_path) };
    ok($@);
};

do {
    my $css_file = new File('d:/projects/atribut-local/wp-content/themes/woodmart-child/style.css');
    my @comments = $css_file->get_comments;

    ok($comments[0]->get_text, '
 Theme Name:   Woodmart Child
 Description:  Woodmart Child Theme
 Author:       XTemos
 Author URI:   http://xtemos.com
 Template:     woodmart
 Version:      1.0.0
 Text Domain:  woodmart
');

    ok($comments[-2]->get_text, 'удаление огромного отступа у заголовка страницы');


};