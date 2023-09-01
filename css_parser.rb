require 'optparse'

OptionParser.new do |options|
  options.banner = 'Usage'
end

module Css
  class Project

  end

  class Element
    attr_accessor :source
    attr_reader :elements

    def initialize(source)
      @elements = []
      self.source = source
    end

    def source=(source)
      @source = source
      parse if defined? parse
    end

    def length
      @source.length
    end

    # def to_s(depth=0)
    #   result = ''
    #   if @elements.length > 0
    #     result += "\t" * (depth) + self.class.name + " {\n"
    #     @elements.each do |element|
    #       result += "\t" * (depth + 1) + element.to_s + "\n"
    #     end
    #     result += "\t" * (depth) + "}"
    #   else
    #     result += self.class.name + " { " + (defined? text ? text : source) + " }"
    #   end
    #   result
    # end

    private

    def parse_elements(optional_elements, required_elements)
      @elements = []
      source = @source.dup
      remaining_required = required_elements.dup
      while (source.length > 0)
        element = parse_element(source, optional_elements)
        if element.nil? and remaining_required.length > 0
          element = parse_element(source, [remaining_required.shift])
        end

        if element.nil?
          raise ParseError.new(@source, parsed_length, source.length - 1), %Q/Couldn't parse element/
        end

        @elements.push element
        source.slice!(0...element.length)

        if required_elements.length > 0 and remaining_required.length == 0
          break
        end
      end

      missing_required = required_elements - @elements.intersection(required_elements)
      if missing_required.length > 0
        raise ParseError.new(@source, 0, source.length - 1), %Q/Couldn't find element "#{missing_required.first.name}"/
      end
    end

    def parse_element(source, element_classes)
      begin
        element_classes.each do |element_class|
          element = element_class.from_source(source)
          break if element
        end
      rescue ParseError => parse_error
        raise ParseError.new(@source, parsed_length + parse_error.start_pos, parsed_length + parse_error.end_pos), parse_error.message
      end
    end

    def parsed_length
      @elements.map(&:length).sum
    end
  end

  class File < Element

    def self.open(path)
      file = ::File.open(path)
      Css::File.new(file.read)
    end

    def vars
      
    end

    private    

    def parse
      parse_elements [Space, Comment, RuleSet]
    end

    # def parse_comments
    #   @comments = []
    #   comment = nil
    #   (0...@source.length).step 2 do |i|
    #     word = @source[i..(i + 1)]

    #     if comment.nil? && word == '/*'
    #       comment = Comment.new(text: '', start_pos: i)
    #       next
    #     elsif word == '*/'
    #       raise UnmatchedCommentBoundError(i, i + 1) if comment.nil?

    #       @comments.push comment
    #       comment = nil
    #       next
    #     elsif comment
    #       comment.text += @source[i]
    #     end
    #   end
    # end

    # def parse_rule_sets
    #   @rule_sets = []
    #   selectors = nil
    #   declaration_block = nil
    #   (0...@source.length).each do |char_i|
    #     next if char_in_comment? char_i
    #     char = @source[char_i]

    #     if char == '{'
    #       if selectors and declaration_block.nil?
    #         selectors.end_pos = char_i - 1
    #         declaration_block = Element.new(text: char, start_pos: char_i)
    #         next
    #       elsif selectors.nil?
    #         raise SelectorsNotFoundError(char_i)
    #       elsif declaration_block != nil
    #         raise UnexpectedCharacterError(char_i)
    #       end
    #     elsif char == '}'
    #       if declaration_block
    #         declaration_block.end_pos = char_i
    #         @rule_sets.push RuleSet.new(selectors, declaration_block)
    #         selectors = nil
    #         declaration_block = nil
    #         next
    #       else
    #         raise UnexpectedCharacterError(char_i)
    #       end
    #     elsif selectors
    #       selectors.text += char
    #     elsif char =~ /\S/
    #       selectors = Element.new(text: char, start_pos: char_i)
    #     end
    #   end
    #   if selectors and declaration_block.nil?
    #     raise Error(), 
    #   end
    # end
  end

  

  class Space < Element
    def self.from_source(source)
      if source.match(/^\s+/)
        Space.new(Regexp.last_match(0))
      end
    end
  end

  class Comment < Element
    attr_accessor :text

    def initialize(source)
      super
      @text = source.gsub(/\/\*|\*\//, '')
    end

    def Comment.from_source(source)
      return nil unless source.start_with?('/*')

      (2...source.length).each do |char_i|
        word = source[char_i..(char_i + 1)]
        if word == '*/'
          return Comment.new(source[0..(char_i+1)])
        end
      end

      raise ParseError.new(source, 0, 1), %Q(Unmatched "/*")
    end
  end

  class RuleSet < Element
    def self.from_source(source)
      return nil unless source.match(/^\S+/)
    end

    def parse
      parse_elements [Space, Comment], [Selector, DeclarationBlock]
    end
  end

  class Selector < Element
    def self.from_source(source)
      return nil unless source.match(/^\S+/)

      (0...source.length).each do |char_i|
        char = source[char_i]
        
      end
    end
  end

  class DeclarationBlock < Element

  end

  class ParseError < RuntimeError
    attr_reader :source, :start_pos, :end_pos

    def initialize(source, *pos)
      super()
      @source = source
      if pos.length == 2
        @start_pos = pos[0]
        @end_pos = pos[1]
      elsif pos.length == 1
        @start_pos = @end_pos = pos.first
      end
    end
  end
end