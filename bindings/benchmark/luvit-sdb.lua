local JSON = require ("json")
local item = [[
{"glossary":{"title":"example glossary","page":1,"GlossDiv":{"title":"First gloss title","GlossList":{"GlossEntry":{"ID":"SGML","SortAs":"SGML","GlossTerm":"Standard Generalized Markup Language","Acronym":"SGML","Abbrev":"ISO 8879:1986","GlossDef":{"para":"A meta-markup language, used to create markup languages such as DocBook.","GlossSeeAlso":["OK","GML","XML"]},"GlossSee":"markup"}}}}} 
]]

function test_sdb()
	local SDB = require ("sdb")
	local db = SDB:new ()
	db:set ("g", item)
	
	for i=1,199999 do
		local a = db:json_get ("g", "glossary.title")
	end
	db:free ()
end

function test_yajl()
	local obj = {}
	obj.item = item
	for i=1,199999 do
		local o = JSON.parse (obj.item)
		local food = o.glossary.title
	end
end

test_sdb()
