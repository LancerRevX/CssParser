module Css
  def Css.parse_elements(source, element_declarations)
    source = source.dup
    elements = []
    parsed_length = 0
    element_declarations.each do |element_declaration|
      if not element_declaration.optional and source.length == 0
        return nil
      end
      element = element_declaration.element_class.parse(source)
      if element.nil?
        unless element_declaration.optional
          return nil
        else
          next
        end
      end
      while element
        elements.push element
        source.slice!(0...element.length)
        parsed_length += element.length
        if element_declaration.many
          element = element_declaration.element_class.parse(source)
        else
          element = nil
        end
      end
    end
    elements
  end
  
  class Element
    attr_accessor :source

    def initialize(source)
      @source = source
    end

    def length
      @source.length
    end

    def to_s(depth=0)
      result = "\t" * depth
      result += self.class.name + %Q( { ") + (defined?(@text) ? @text : @source) + %Q(" })
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

  class CombinedElement < Element
    attr_reader :elements

    def initialize(elements)
      super(nil)
      @elements = elements
    end

    def source 
      @elements.map(&:source).join
    end

    def to_s(depth=0)
      result = ''
      if @elements.length > 0
        result += "\t" * (depth) + self.class.name + " {\n"
        @elements.each do |element|
          result += "\t" * (depth + 1) + element.to_s + "\n"
        end
        result += "\t" * (depth) + "}"
      else
        result += self.class.name + " { }"
      end
      result
    end
  end
end