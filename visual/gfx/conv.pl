#!/usr/bin/perl -w

open(IN, "moon.txt");

print 'static const unsigned char moon[] U8X8_PROGMEM = {'."\n";
print "\n";

while($row=<IN>)
{
  chomp($row);
  @bits=split(//, $row);
  for ($x=0; $x<8; $x++)
  {
    $val=0;
    for ($bp=7; $bp>=0; $bp--)
    {
      $bit=$bits[$x*8+$bp];
      $val*=2;
      $val+=$bit;
    }
    print($val.", ");
  }
  print "\n";
}
print "};";
close(IN);
