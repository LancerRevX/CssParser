require 'optparse'

require_relative 'element'
require_relative 'basic_elements'
require_relative 'complex_elements'

module Css
  


  #   while (source.length > 0)
  #     begin
  #       element_classes.each do |element_class|
  #         element = element_class.parse(source)
  #         break if element
  #       end
  #     rescue ParseError => parse_error
  #       raise ParseError.new(source, parsed_length + parse_error.start_pos, parsed_length + parse_error.end_pos), parse_error.message
  #     end

  #     if element.nil?
  #       raise ParseError.new(source, parsed_length, source.length - 1), %Q/Couldn't parse element/
  #     end

  #     elements.push element
  #     source.slice!(0...element.length)
  #     parsed_length += element.length

  #     if element.is_a?(final_element)
  #       break
  #     end
  #   end
  #   return elements
  # end

  class Project

  end



  class File < CombinedElement

    def self.open(path)
      file = ::File.open(path)
      Css::File.new(file.read)
    end

    def self.parse(source)
      self.new(Css.parse_elements source, [
        ElementDeclaration.new(element_class: SpaceOrComment, optional: true, many: true),
        ElementDeclaration.new(element_class: RuleSet, optional: true, many: true),
        ElementDeclaration.new(element_class: SpaceOrComment, optional: true, many: true),
      ])
    end

    def vars
      
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

    # def to_s
    #   printf(
    #     "at #@start_pos: %s > %s < %s", 
    #     @source[[@start_pos-16, 0].max...@start_pos], 
    #     @source[@start_pos..@end_pos], 
    #     @source[[@end_pos+1, @source.length].min...[@end_pos+16, @source.length].min])
    # end
  end
end