package css;

type Element interface {
	Length() int
	Parse() error
}

type Comment struct {
	BasicElement
	Content string
}

type BasicElement struct {

}

func (element BasicElement) Length() int {
	return 3
}



func (element *Comment) Parse() error {
	element.Content = "def"
	return nil
}