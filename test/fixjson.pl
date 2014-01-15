#!/usr/bin/perl
my $str = "";
while(<STDIN>) {
	s,\n,,g;
	$str .= $_;
}
$str=~s,(\s+):,:,g;
$str=~s,}(\s+),},g;
$str=~s/,(\s+)/,/g;
$str=~s/:(\s+)/:/g;
$str=~s,{([^"])([^:]*),{"$1$2",g;
$str=~s/,([^"])([^:]*)/,"$1$2"/g;
print $str."\n";
