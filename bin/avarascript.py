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
inline_comment = pp.Literal("//").suppress() + pp.restOfLine
comment = pp.cStyleComment | inline_comment
name = ~keyword + pp.Word(pp.alphas, pp.alphanums + r"._[]{}\\|")
op = pp.oneOf("+ - * / % ^ | < >")
unary_op = pp.oneOf("- |")
reference = pp.Literal("@").suppress() + name
atom = string | reference | number | name
expr = pp.Optional(unary_op) + atom + pp.ZeroOrMore(op + atom)
declaration = name + pp.Literal("=").suppress() + expr
end = pp.Literal("end").suppress()
obj_body = pp.ZeroOrMore(comment.suppress() | declaration)
obj = pp.Literal("object") + name + obj_body + end
adjust = pp.Literal("adjust") + name + obj_body + end
unique = (
    pp.Literal("unique").suppress() + pp.Optional(number) + pp.ZeroOrMore(name) + end
)
enum = pp.Literal("enum").suppress() + number + pp.OneOrMore(name) + end
decl_group = pp.OneOrMore(declaration)
script = pp.ZeroOrMore(comment.suppress() | decl_group | unique | enum | adjust | obj)


DEFAULT_CONTEXT = ("fill", "frame", "cx", "cy", "r", "angle", "extent")
OBJ_CONTEXT = {
    "SkyColor": ("fill", "frame"),
    "GroundColor": ("fill", "frame"),
    "Wall": ("fill", "frame", "x", "y", "z", "w", "d", "h"),
    "WallDoor": ("fill", "frame", "x", "y", "z", "w", "d", "h"),
    "Ramp": ("fill", "frame", "x", "y", "z", "w", "d", "h", "r", "angle", "extent"),
}


def object_context(name, context):
    keys = OBJ_CONTEXT.get(name, DEFAULT_CONTEXT)
    return {k: context[k] for k in keys if k in context}


class Element(object):
    def __init__(self, tag, *children, **attrs):
        self.tag = tag
        self.children = list(children)
        self.attrs = attrs

    def __str__(self):
        return self.xml()

    def xml(self, indent=2):
        attrs = "".join(
            ' {}="{}"'.format(name, str(value).replace('"', "&quot;"))
            for name, value in self.attrs.items()
        )
        if self.children:
            text = "\n"
            for child in self.children:
                if isinstance(child, str):
                    text += child.replace("<", "&lt;").replace(">", "&gt;")
                else:
                    text += (" " * indent) + str(child) + "\n"
            return "<{tag}{attrs}>{text}</{tag}>".format(
                tag=self.tag,
                attrs=attrs,
                text=text,
            )
        else:
            return "<{tag}{attrs} />".format(
                tag=self.tag,
                attrs=attrs,
            )


class ScriptObject(object):
    def __repr__(self):
        return self.__class__.__name__

    def process(self, context):
        return True


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
        self.name = tokens[0].replace("[", ".").replace("]", "")
        self.expr = tokens[1:]

    @property
    def value(self):
        return " ".join(str(e) for e in self.expr)

    def __str__(self):
        return "{} = {}".format(self.name, self.value)

    def element(self, context):
        return Element("set", **{self.name: self.value})

    def process(self, context):
        context[self.name] = self.value
        if self.name in ("wa", "wallHeight"):
            try:
                # If these are just numbers, we promote them to "y" or "h" in the context.
                float(self.value)
                return False
            except ValueError:
                pass
        return True


class Unique(ScriptObject):
    def __init__(self, tokens):
        self.names = tokens

    def __str__(self):
        return "unique {} end".format(" ".join(str(name) for name in self.names))

    def element(self, context):
        return Element("unique", vars=" ".join(str(name) for name in self.names))


class Enum(ScriptObject):
    def __init__(self, tokens):
        self.start = tokens[0]
        self.names = tokens[1:]

    def __str__(self):
        return "enum {} {} end".format(
            self.start, " ".join(str(name) for name in self.names)
        )

    def element(self, context):
        return Element(
            "enum", start=self.start, vars=" ".join(str(name) for name in self.names)
        )


class Object(ScriptObject):
    def __init__(self, tokens):
        self.tag = tokens[0]
        self.name = tokens[1]
        self.declarations = tokens[2:]

    def __str__(self):
        decls = "\n".join("  " + str(d) for d in self.declarations)
        return "{} {}\n{}\nend".format(self.tag, self.name, decls)

    def element(self, context):
        attrs = {d.name: d.value for d in self.declarations}
        attrs.update(object_context(self.name, context))
        return Element(self.name, **attrs)


class DeclarationGroup(ScriptObject):
    def __init__(self, tokens):
        self.declarations = tokens

    def __str__(self):
        return "\n".join(str(d) for d in self.declarations)

    def element(self, context):
        # Only include declaration attributes which would have been processed.
        fake_context = {}
        attrs = {d.name: d.value for d in self.declarations if d.process(fake_context)}
        return Element("set", **attrs)

    def process(self, context):
        return any(d.process(context) for d in self.declarations)


string.setParseAction(String)
reference.setParseAction(Reference)
enum.setParseAction(Enum)
unique.setParseAction(Unique)
declaration.setParseAction(Declaration)
decl_group.setParseAction(DeclarationGroup)
obj.setParseAction(Object)
adjust.setParseAction(Object)


class ScriptParseError(Exception):
    pass


def parse_script(text, strict=True):
    try:
        return script.parseString(text, parseAll=strict)
    except pp.ParseException:
        raise ScriptParseError("Failed to parse AvaraScript:\n" + text)


if __name__ == "__main__":
    import sys

    for t in parse_script(sys.stdin.read()):
        print(t)
