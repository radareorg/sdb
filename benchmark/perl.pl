use JSON;

my $json = new JSON;

$content = <<EOF
{"glossary":{"title":"example glossary","page":1,"GlossDiv":{"title":"First gloss title","GlossList":{"GlossEntry":{"ID":"SGML","SortAs":"SGML","GlossTerm":"Standard Generalized Markup Language","Acronym":"SGML","Abbrev":"ISO 8879:1986","GlossDef":{"para":"A meta-markup language, used to create markup languages such as DocBook.","GlossSeeAlso":["OK","GML","XML"]},"GlossSee":"markup"}}}}} 
EOF
;

my %obj = ( "json" => $content );

sub dojson() {
	my $js = $obj{json};
	my $j = $json->allow_nonref->utf8->relaxed->decode($js);
	my $title = $j->{glossary}->{title};
#	print ("$title\n");
}

for ($i=0; $i<199999; $i++) {
	dojson();
}
