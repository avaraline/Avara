script = 
    _ ? all:(inline_comment / block_comment / declaration / unique / unique_start / enum / adjust / object)*
    { return {"instructions": [...all]} }

inline_comment = 
    "//" text:([^\n]*) "\n" _ ?
    { return {"type": "comment_inline", "text": text.reduce((res, e) => { return res + e[0] }, "")} }

block_comment = 
    "/*" text:(!"*/" .)* "*/" _ ?
    { return {"type": "comment_block", "text": text.reduce((res, e) => { return res + e[1] }, "")} }

num "number" = 
    [\-0-9\.]+
    { return parseFloat(text()) }
    
str "string" = 
    '"' text:(!'"' .)* '"'
    { return text.reduce((res, e) => { return res + e[1]; }, "") }

name "variable name, starting with a letter" =
    (!keywords [a-zA-Z][a-zA-Z\[\]\\\|\{\}\_0-9\.]*)
    { return {"name": text()} }
    
reference "reference of name" = 
    ("@" _ ? !keywords name)
    { return {"reference": text()} }

declaration "declaration" =
    lhs:(name) _ ? "=" _ ? rhs:(expr) _ ?
    { return {"type": "declaration", "variable": lhs["name"], "expr": rhs} }
    
tokenlist "a list of valid names" = 
    tokens:(!keywords name _)* 
    { return tokens.map((t) => { return t[1]["name"] }) }

adjust =
    "adjust"i _ classname:(name) _ "end"i _ ?
    { return {"type": "adjust", "class": classname["name"]} }
    
unique = 
    "unique"i _ tokens:(tokenlist) _ "end"i _ ? 
    { return {"type": "unique", "tokens": tokens} }
    
unique_start = 
    "unique"i _ start:num _ "end"i _ ?
    { return {"type": "set_unique_start", "start": start} }

enum = 
    "enum"i _ start:(num) _ tokens:(tokenlist) _ "end"i _ ?
    { return {"type": "enum", "tokens": tokens, "start": start} } 

prop = 
    lhs:(name) _ ? "=" _ ? rhs:(expr) _
    { return [lhs["name"], rhs] }
    
object = 
    "object"i _ classname:(name) _ props:(prop)* _ "end"i _ 
    {
        var result = {"type": "object", "class": classname["name"]}
        props.forEach((p) => { result[p[0]] = p[1] })
        return result
    }

expr_term = 
    (function / str / reference / name / num / parenthetical)

op "operator" = 
    [\+\-\*\/\%\^\|\<\>]
    { return {"op": text()} }
    
parenthetical "parenthetical" = 
    "(" _ ? expr:(expr) _ ? ")" _ ?
    { return {"expr": expr } }
    
function "function" = 
    name:(name) "(" _ ? head:(expr) tail:(_ ? "," _ ? expr)* _ ? ")" _ ?
    { return {"func": name["name"], "expr": [head, ...tail]} }
    
expr = 
    head:(expr_term) tail:(_ ? op _ ? expr_term)*
    { 
        if (tail.length == 0) return head
        var result = [head]
        tail.forEach((e) => {
            e.forEach((atom) => {
                if(Array.isArray(atom) && atom.length == 0) return;
                if(atom && atom[0] !== " ") result.push(atom)
            })
        })
        return result
    }

keywords = 
    "unique"i / "enum"i / "object"i / "end"i / "adjust"i

_ "whitespace"
  = [ \t\n\r]*