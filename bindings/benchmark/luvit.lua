local JSON = require ("json")
local item = [[
{"glossary":{"title":"example glossary","page":1,"GlossDiv":{"title":"First gloss title","GlossList":{"GlossEntry":{"ID":"SGML","SortAs":"SGML","GlossTerm":"Standard Generalized Markup Language","Acronym":"SGML","Abbrev":"ISO 8879:1986","GlossDef":{"para":"A meta-markup language, used to create markup languages such as DocBook.","GlossSeeAlso":["OK","GML","XML"]},"GlossSee":"markup"}}}}} 
]]


local obj = {}
obj.item = item
for i=1,99999 do
	local o = JSON.parse (obj.item)
	local food = o.glossary.title
end
