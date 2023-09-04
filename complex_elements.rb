require_relative 'element'
require_relative 'basic_elements'

module Css 
  ElementDeclaration = Struct.new(:element_class, :optional, :many)

  class RuleSet < CombinedElement
    def RuleSet.parse(source)
      elements = Css.parse_elements(source, [
        ElementDeclaration.new(element_class: Selector),
        ElementDeclaration.new(element_class: SpaceOrComment, optional: true, many: true),
        ElementDeclaration.new(element_class: DeclarationBlock)
      ])

      return RuleSet.new(elements) if elements
    end
  end

  class Selector < CombinedElement
    def Selector.parse(source)
      elements = Css.parse_elements(source, [
        ElementDeclaration.new(element_class: SelectorItem),
        ElementDeclaration.new(element_class: SpaceOrComment, optional: true, many: true),
        ElementDeclaration.new(element_class: SelectorTail, optional: true, many: true)
      ])

      return Selector.new(elements) if elements
    end
  end

  class SelectorTail < CombinedElement
    def SelectorTail.parse(source)
      elements = Css.parse_elements(source, [
        ElementDeclaration.new(element_class: Comma),
        ElementDeclaration.new(element_class: SpaceOrComment, optional: true, many: true),
        ElementDeclaration.new(element_class: SelectorItem)
      ])

      return SelectorTail.new(elements) if elements
    end
  end


  class DeclarationBlock < CombinedElement
    def DeclarationBlock.parse(source)
      elements = Css.parse_elements(source, [
        ElementDeclaration.new(element_class: DeclarationBlockStart),
        ElementDeclaration.new(element_class: SpaceOrComment, optional: true, many: true),
        ElementDeclaration.new(element_class: Declaration, optional: true, many: true),
        ElementDeclaration.new(element_class: SpaceOrComment, optional: true, many: true),
        ElementDeclaration.new(element_class: DeclarationBlockEnd)
      ])

      return DeclarationBlock.new(elements) if elements
    end
  end

  class Declaration < CombinedElement
    def Declaration.parse(source)
      elements = Css.parse_elements(source, [
        ElementDeclaration.new(element_class: Property),
        ElementDeclaration.new(element_class: SpaceOrComment, optional: true, many: true),
        ElementDeclaration.new(element_class: Colon),
        ElementDeclaration.new(element_class: SpaceOrComment, optional: true, many: true),
        ElementDeclaration.new(element_class: Value),
        ElementDeclaration.new(element_class: SpaceOrComment, optional: true, many: true),
        ElementDeclaration.new(element_class: Semicolon),
        ElementDeclaration.new(element_class: SpaceOrComment, optional: true, many: true),
      ])

      return Declaration.new(elements) if elements
    end
  end
end