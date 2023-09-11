package main;

import (
	"fmt"
	"css_parser_go/css"
)

type ParseError struct {
	element css.Element
}

func (parseError ParseError) Error() string {
	return fmt.Sprintf("Error while parsing element %T", parseError.element)
}

type Source struct {
	text string
	pos  int
}




func main() {
	comment := css.Comment {
		Content: "abc",
	}
	err := ParseError {
		&comment,
	}
	fmt.Println(err)
}