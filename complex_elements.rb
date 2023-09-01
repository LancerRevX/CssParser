require_relative 'element'
require_relative 'basic_elements'

module Css 
  ElementDeclaration = Struct.new(:element_class, :optional, :many)

  class RuleSet < CombinedElement
    def RuleSet.parse(source)
      elements = parse_elements(source, [
        ElementDeclaration.new(Selector),
        ElementDeclaration.new(SpaceOrComment, optional: true, many: true),
        ElementDeclaration.new(DeclarationBlock)
      ])

      return RuleSet.new(elements) if elements
    end
  end

  class Selector < CombinedElement
    def Selector.parse(source)
      elements = parse_elements(source, [
        ElementDeclaration.new(SelectorItem),
        ElementDeclaration.new(SpaceOrComment, optional: true, many: true),
        ElementDeclaration.new(SelectorTail, optional: true, many: true)
      ])

      return Selector.new(elements) if elements
    end
  end

  class SelectorTail < CombinedElement
    def SelectorTail.parse(source)
      elements = parse_elements(source, [
        ElementDeclaration.new(Comma),
        ElementDeclaration.new(SpaceOrComment, optional: true, many: true),
        ElementDeclaration.new(SelectorItem)
      ])

      return SelectorTail.new(elements) if elements
    end
  end


  class DeclarationBlock < CombinedElement
    def DeclarationBlock.parse(source)
      elements = parse_elements(source, [
        ElementDeclaration.new(DeclarationBlockStart),
        ElementDeclaration.new(SpaceOrComment, optional: true, many: true),
        ElementDeclaration.new(Declaration, optional: true, many: true),
        ElementDeclaration.new(SpaceOrComment, optional: true, many: true),
        ElementDeclaration.new(DeclarationBlockEnd)
      ])

      return DeclarationBlock.new(elements) if elements
    end
  end

  class Declaration < CombinedElement
    def Declaration.parse(source)
      elements = parse_elements(source, [
        ElementDeclaration.new(Property),
        ElementDeclaration.new(SpaceOrComment, optional: true, many: true),
        ElementDeclaration.new(Colon),
        ElementDeclaration.new(SpaceOrComment, optional: true, many: true),
        ElementDeclaration.new(Value),
        ElementDeclaration.new(SpaceOrComment, optional: true, many: true),
        ElementDeclaration.new(Semicolon),
        ElementDeclaration.new(SpaceOrComment, optional: true, many: true),
      ])

      return Declaration.new(elements) if elements
    end
  end
end