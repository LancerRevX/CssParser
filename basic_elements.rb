require_relative 'element'

module Css

  class BasicElementClass
    def BasicElementClass.new(regex)
      new_class = Class.new(Element)
      new_class.class_eval "
        def self.parse(source)
          if source.match(/^#{regex}/)
            self.new(Regexp.last_match[0])
          end
        end
      "
      return new_class
    end
  end

  class SpaceOrComment
    def SpaceOrComment.parse(source)
      element = Space.parse(source)
      if element
        return element
      else
        return Comment.parse(source)
      end
    end
  end


  Space = BasicElementClass.new /\s+/

  DeclarationBlockStart = BasicElementClass.new /{/

  DeclarationBlockEnd = BasicElementClass.new /}/

  SelectorItem = BasicElementClass.new /[-^\.\w:\(\)\[\] ]+/

  Semicolon = BasicElementClass.new(/;/)

  Comma = BasicElementClass.new(/,/)

  Colon = BasicElementClass.new(/:/)

  Property = BasicElementClass.new(/[-\w]/)

  class Value < Element
    def self.parse(source)
      return nil unless source.match(/^\w/)

      value = ''
      parentheses = 0
      (0...source.length).each do |char_i|
        char = source[char_i]
        word = source[char_i..(char_i + 1)]
        if char == ';' and parentheses == 0
          break
        elsif char == '('
          parentheses += 1
        elsif char == ')'
          unless parentheses == 0
            parentheses -= 1
          else
            raise ParseError.new(source, char_i), %Q/Unmatched ")"/
          end
        end
        
        value += char
      end

      if parentheses > 0
        raise ParseError.new(source, source.index('(')), %Q/Unmatched "("/
      end

      return Value.new(value)
    end
  end

  class Comment < Element
    attr_accessor :text

    def initialize(source, text)
      super(source)
      @text = text
    end

    def self.parse(source)
      return nil unless source.start_with?('/*')

      text = ''
      (2...source.length).each do |char_i|
        char = source[char_i]
        word = source[char_i..(char_i + 1)]
        if word == '*/'
          return Comment.new(source[0..(char_i+1)], text)
        else
          text += char
        end
      end

      raise ParseError.new(source, 0, 1), %Q(Unmatched "/*")
    end
  end

end