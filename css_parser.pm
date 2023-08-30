package CSS::Parser;

use v5.32;
use warnings;

package Project {

}

package Rule_Set {
    sub new {
        my $class = shift;
        my $self = {
            selectors => [],

        };
        bless $self, $class;
        return $self;
    }

    sub from_text {
        my $class = shift;
        my ($selectors_text, $declaration_block_text) = @_;

        my $self = {
            selectors => $class->_parse_selectors($selectors_text),
            
    }

    sub _parse_selectors {
        my $class = shift;
        my ($selectors_text) = @_;

    }

    sub _parse_declaration_block {
        my $class = shift;
        my ($declaration_block_text) = @_;
    }
}

package Selector {
    sub new {
        my $class = shift;
        my ($text) = @_;
        my $self = {
            text => $text
        };
        bless $self, $class;
        return $self;
    }

    sub from_text {
        my $class = shift;
        my ($text) = @_;
    }

    sub parse {
        ...
    }
}

package Declaration {
    sub new {
        my $class = shift;
        my () = @_;
        my $self = {
            property => undef,
            value => undef
        };
        bless $self, $class;
        return $self;
    }
}

package Comment {
    sub new {
        my $class = shift;
        my ($file, $text, $line_i, $char_i) = @_;

        my $self = {
            file => $file,
            text => $text,
            line_i => $line_i,
            char_i => $char_i,
        };
        bless $self, $class;
        return $self;
    }

    sub get_text { 
        my $self = shift;

        return $self->{text};
    }

    sub get_pos {
        my $self = shift;

        return $self->{line_i}, $self->{char_i};
    }
}

package File {
    sub new {
        my $class = shift;
        my $file_path = pop;
        my $project = pop;

        my $self = {
            project => $project,
            file_path => $file_path,
            text => '',
            cleared_text => '',
            rules => [],
            comments => [],
        };
        bless $self, $class;

        $self->_read_file;
        $self->_parse_comments;
        $self->_remove_comments;
        $self->_parse_rules;

        return $self;
    }


    sub get_comments {
        my $self = shift;

        return @{$self->{comments}};
    }

    sub get_vars {
        ...
    }
    

    sub _read_file {
        my $self = shift;

        my $file_path = $self->{file_path};
        if (!open(FILE, '<', $self->{file_path})) {
            die qq(Couldn't open file "$file_path"!);
        }

        while (my $line = <FILE>) {
            $self->{text} .= $line;
        }

        close FILE;
    }

    sub _parse_comments {
        my $self = shift;

        my $text = $self->{text};
        my $line_i = 0;
        my $comment = undef;
        my ($comment_line_i, $comment_char_i);
        for (my $word_i = 0; $word_i < length($text); $word_i++) {
            my $char = substr($text, $word_i, 1);
            my $word = substr($text, $word_i, 2);
            if ($word eq '/*' && !defined $comment) {
                $comment = '';
                $comment_line_i = $line_i;
                $comment_char_i = $word_i;
                $word_i += 1;
                next;
            }
            if ($word eq '*/') {
                if (defined $comment) {
                    $comment = new Comment($self, $comment, $comment_line_i, $comment_char_i);
                    push @{$self->{comments}}, $comment;
                    $comment = undef;
                    $word_i += 1;
                    next;
                } else {
                    my $file_path = $self->{file_path};
                    die qq(Unmatched */ at $file_path:$line_i:$word_i!);
                }
            }
            if (defined $comment) {
                $comment .= $char;
            }
            $line_i++ if $char eq "\n";
        }
    }

    sub _remove_comments {
        my $self = shift;

        my $text = $self->{text};
        foreach my $comment (@{$self->{comments}}) {
            my $comment_text = $comment->get_text;
            $text =~ s/\/\* \Q$comment_text\E \*\///x;
        }

        $self->{text} = $text;
    }

    sub _parse_rules {
        my $self = shift;
        
        my $text = $self->{text};
        my $line_i = 0;
        my ($selectors, $declaration_block);
        for (my $i = 0; $i < length($text); $i++) {
            my $char = substr($text, $i, 1);

            if ($char eq '{') {
                unless (defined $declaration_block) {
                    $declaration_block = '';
                    next;
                } else {
                    my $file_path = $self->{file_path};
                    die qq(Unexpected "{" at $file_path:$line_i);
                }
            }

            if ($char eq '}') {
                if (defined $declaration_block) {
                    my $rule_set = Rule_Set->from_text($selectors, $declaration_block);
                    push @{$self->{rule_sets}}, $rule_set;
                    ($selectors, $declaration_block) = (undef) x 2;
                    next;
                } else {
                    my $file_path = $self->{file_path};
                    die qq(Unmatched "}" at $file_path:$line_i);
                }
            }

            if (defined $declaration_block) {
                $declaration_block .= $char;
            } elsif (defined $selectors || $char =~ /\S/) {
                $selectors .= $char;
            } 
            
            $line_i ++ if $char eq "\n";
        }

        
    }

    sub _parse_declaration_block {

    }
}

1;