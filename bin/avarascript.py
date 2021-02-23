#!/usr/bin/env python

import pyparsing as pp

quote = pp.Literal('"').suppress()
string = quote + pp.SkipTo(quote) + quote
keyword = (
    pp.Keyword("unique")
    | pp.Keyword("enum")
    | pp.Keyword("object")
    | pp.Keyword("end")
    | pp.Keyword("adjust")
)
number = pp.Regex(r"\-?[0-9\.]+")
name = ~keyword + pp.Word(pp.alphas, pp.alphanums + r"._[]{}\\|")
op = pp.oneOf("+ - * / % ^ | < >")
reference = pp.Literal("@").suppress() + name
atom = string | reference | name | number
expr = atom + pp.ZeroOrMore(op + atom)
declaration = name + pp.Literal("=").suppress() + expr
end = pp.Literal("end").suppress()
obj = pp.Literal("object").suppress() + name + pp.ZeroOrMore(declaration) + end
adjust = pp.Literal("adjust").suppress() + name + end
unique = (
    pp.Literal("unique").suppress() + pp.Optional(number) + pp.ZeroOrMore(name) + end
)
enum = pp.Literal("enum").suppress() + number + pp.OneOrMore(name) + end
inline_comment = pp.Literal("//").suppress() + pp.restOfLine
comment = pp.cStyleComment | inline_comment
script = pp.ZeroOrMore(comment.suppress() | declaration | unique | enum | adjust | obj)


class ScriptObject(object):
    def __repr__(self):
        return self.__class__.__name__


class String(ScriptObject):
    def __init__(self, tokens):
        self.text = tokens[0].strip()

    def __str__(self):
        return '"{}"'.format(self.text)


class Number(ScriptObject):
    def __init__(self, tokens):
        self.num = float(tokens[0]) if "." in tokens[0] else int(tokens[0])

    def __str__(self):
        return str(self.num)


class Reference(ScriptObject):
    def __init__(self, tokens):
        self.name = tokens[0]

    def __str__(self):
        return "@{}".format(self.name)


class Declaration(ScriptObject):
    def __init__(self, tokens):
        self.name = tokens[0]
        self.expr = tokens[1:]

    def __str__(self):
        return "{} = {}".format(self.name, " ".join(str(e) for e in self.expr))


class Unique(ScriptObject):
    def __init__(self, tokens):
        self.names = tokens

    def __str__(self):
        return "unique {} end".format(" ".join(str(name) for name in self.names))


class Enum(ScriptObject):
    def __init__(self, tokens):
        self.start = tokens[0]
        self.names = tokens[1:]

    def __str__(self):
        return "enum {} {} end".format(
            self.start, " ".join(str(name) for name in self.names)
        )


class Adjust(ScriptObject):
    def __init__(self, tokens):
        self.name = tokens[0]

    def __str__(self):
        return "adjust {} end".format(self.name)


class Object(ScriptObject):
    def __init__(self, tokens):
        self.name = tokens[0]
        self.declarations = tokens[1:]

    def __str__(self):
        decls = "\n  ".join(str(d) for d in self.declarations)
        return "object {}\n  {}\nend".format(self.name, decls)


string.setParseAction(String)
reference.setParseAction(Reference)
enum.setParseAction(Enum)
unique.setParseAction(Unique)
adjust.setParseAction(Adjust)
declaration.setParseAction(Declaration)
obj.setParseAction(Object)


def parse_script(text):
    return script.parseString(text)
