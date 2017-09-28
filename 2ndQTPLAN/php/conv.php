<?php
function enc(&$x) {
  for ($i = 0; $i < strlen($x); $i++) {
    $x[$i] = chr(ord($x[$i]) ^ ((($i + 101) * 43) % 256));
  }
}
$a = file_get_contents('conv.txt');
enc($a);
$b = urlencode($a);
file_put_contents('conv.url.txt', $b);
$c = 'QByteArray::fromRawData("';
for ($i = 0; $i < strlen($a); $i++) {
  $c .= "\\x".sprintf("%02s", strtoupper(dechex(ord($a{$i}))));
}
$c .= '",'.strlen($a).');';
$b = str_replace("%","\x",$b);
$b = str_replace("+"," ",$b);
file_put_contents('conv.c.txt', $c);
?>