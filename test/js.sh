#!/bin/sh
SDB=../src/sdb
echo 'foo={"a":3,"pop":123,"bar":"cow"}'
${SDB} - 'foo={"a":3,"pop":1234,"bar":"cowa"}' 'foo:a=;foo'
${SDB} - 'foo={"a":3,"pop":1234,"bar":"cowa"}' 'foo:pop=;foo'
${SDB} - 'foo={"a":3,"pop":1234,"bar":"cowa"}' 'foo:bar=;foo'

${SDB} - 'foo={"jajaja":"fuck","a":3,"pop":1234,"bar":"cowa"}' 'foo:jajaja=true;foo'
